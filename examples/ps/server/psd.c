/******************************************************************************
 * @file psd.c
 *
 * @brief ps obus server
 *
 * @author yves-marie.morgan@parrot.com
 *
 * Copyright (c) 2013 Parrot S.A.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Parrot Company nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PARROT COMPANY BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/stat.h>


#include <linux/param.h>

#include "libobus.h"

#include "ps_bus.h"
#include "ps_process.h"
#include "ps_summary.h"

#include "log.h"

/* timer fd defines */
#ifndef TFD_NONBLOCK
#  define TFD_NONBLOCK O_NONBLOCK
#endif

#ifndef TFD_CLOEXEC
#  define TFD_CLOEXEC O_CLOEXEC
#endif

/**
 *
 */
#define SIZEOF_ARRAY(x) (sizeof((x)) / sizeof((x)[0]))

/* this structure stores one piece of 'history' information */
typedef struct HST_t {
	unsigned long long tics;
	uint32_t pid;
} HST_t;

/* this structure stores a frame's cpu tics used in history calculations. */
typedef struct CPU_t {
	/* current values */
	unsigned long long utime; /* user mode */
	unsigned long long ntime; /* user mode with nice priority */
	unsigned long long stime; /* system mode */
	unsigned long long itime; /* idle  task */

	/* saved values */
	unsigned long long sav_utime; /* user mode */
	unsigned long long sav_ntime; /* user mode with nice priority */
	unsigned long long sav_stime; /* system mode */
	unsigned long long sav_itime; /* idle  task */

	unsigned int id; /* the CPU ID number */
	float pcpu;   /* %CPU usage */
} CPU_t;

/* Sort support, callback function signature */
typedef int (*QFP_t)(const void *, const void *);

/* context structure to compute %CPU */
struct pcpu_context {
	uint32_t task_total;
	uint32_t task_running;
	uint32_t task_sleeping;
	uint32_t task_stopped;
	uint32_t task_zombie;
	long long et;
	uint32_t hertz;
	uint32_t cpu_tot;
	HST_t *hist_sav;
	HST_t *hist_new;
	unsigned int hist_siz;
	unsigned int maxt_sav;
	struct timeval oldtimev;
	unsigned int pcpu_max_value;
	enum ps_summary_mode mode;
	uint32_t refresh_rate;
	CPU_t *cpus;
};

/**
 *
 */
struct process_stat {
	uint32_t pid;  /* pid of process */
	char *name;    /* process name */
	char *exe;     /* executable file path */
	float pcpu; /* %CPU usage */

	uint32_t ppid; /* pid of parent process */
	char state;    /* single-char code for process state */

	unsigned long long utime; /* user-mode CPU time accumulated by process */
	unsigned long long stime; /* kernel-mode CPU time accumulated by process */
	unsigned long long cutime; /* cumulative utime of process and reaped children */
	unsigned long long cstime; /* cumulative stime of process and reaped children */

	unsigned long flags;   /* kernel flags for the process */
	unsigned long minflt;  /* number of minor page faults since process start */
	unsigned long majflt;  /* number of major page faults since process start */
	unsigned long cminflt; /* cumulative min_flt of process and child processes */
	unsigned long cmajflt; /* cumulative maj_flt of process and child processes */

	int pgrp;    /* process group id */
	int tpgid;   /* terminal process group id */
	int session; /* session id */
	int tty_nr;  /* full device number of controlling terminal */
};

/**
 *
 */
static int s_fdpipes[2];
static int s_tfd;
static struct obus_server *s_server;
static struct pcpu_context s_pcpu_ctx;
static struct ps_summary *s_summary;

/**
 *
 */
static void sig_handler(int signum)
{
	ssize_t ret;
	uint8_t dummy = 0xff;
	diag("signal %d(%s) received !", signum, strsignal(signum));
	ret = write(s_fdpipes[1], &dummy , sizeof(dummy));
	if (ret < 0)
		diag_error("can't write in pipe, error:%s", strerror(errno));
}

/**
 *
 */
static struct ps_process *find_process(uint32_t pid)
{
	struct ps_process *process = NULL;
	const struct ps_process_info *info;

	/* walk list of process in obus server */
	process = ps_process_next(s_server, NULL);
	while (process != NULL) {
		info = ps_process_get_info(process);
		if (info && info->pid == pid)
			return process;
		process = ps_process_next(s_server, process);
	}

	/* not found */
	return NULL;
}

/**
 *
 */
static int sort_HST_t(const HST_t *P, const HST_t *Q)
{
	return (int32_t)P->pid - (int32_t)Q->pid;
}

/**
 *
 */
static void update_cpus(struct pcpu_context *ctx, struct ps_bus_event *bus_event)
{
	FILE *fp = NULL;
	static char buf[512] = "";
	CPU_t *cpu = NULL;
	uint32_t i = 0;
	struct ps_summary_info info_new;
	const struct ps_summary_info *info_cur = NULL;
	float *pcpus_new = NULL;
	int pcpus_changed = 0;

	/* open '/proc/stat' file */
	fp = fopen("/proc/stat", "r");
	if (fp == NULL) {
		diag_errno("open");
		return;
	}

	/* first line is for all cpus */
	cpu = &ctx->cpus[ctx->cpu_tot];
	if (fgets(buf, sizeof(buf), fp)) {
		sscanf(buf, "cpu %llu %llu %llu %llu",
				&cpu->utime, &cpu->ntime,
				&cpu->stime, &cpu->itime);
		cpu->id = 0;
	}

	/* per cpu data */
	for (i = 0; i < ctx->cpu_tot; i++) {
		cpu = &ctx->cpus[i];
		if (fgets(buf, sizeof(buf), fp)) {
			sscanf(buf, "cpu%d %llu %llu %llu %llu", &cpu->id,
					&cpu->utime, &cpu->ntime,
					&cpu->stime, &cpu->itime);
		}
	}

	/* for all cpus and summary */
	pcpus_new = calloc(ctx->cpu_tot + 1, sizeof(float));
	for (i = 0; i < ctx->cpu_tot + 1; i++) {
		unsigned long long tot, sav_tot, per_tot, per_itime;
		cpu = &ctx->cpus[i];
		tot = cpu->utime + cpu->ntime +
				cpu->stime + cpu->itime;
		sav_tot = cpu->sav_utime + cpu->sav_ntime +
				cpu->sav_stime + cpu->sav_itime;
		per_itime = cpu->itime - cpu->sav_itime;
		per_tot = tot - sav_tot;
		if (per_tot != 0)
			cpu->pcpu = (float)(100.f - (100.f * (float) per_itime) / (float) per_tot);

		pcpus_new[i] = cpu->pcpu;

		/* save values for next iteration */
		cpu->sav_utime = cpu->utime;
		cpu->sav_ntime = cpu->ntime;
		cpu->sav_stime = cpu->stime;
		cpu->sav_itime = cpu->itime;
	}

	fclose(fp);

	/* init new info, get current one */
	ps_summary_info_init(&info_new);
	info_cur = ps_summary_get_info(s_summary);

	/* don't give total cpu in obus */
	if (info_cur->pcpus == NULL || info_cur->n_pcpus != ctx->cpu_tot) {
		pcpus_changed = 1;
	} else {
		for (i = 0; i < ctx->cpu_tot; i++) {
			if (info_cur->pcpus[i] != pcpus_new[i])
				pcpus_changed = 1;
		}
	}

	/* update summary object */
	if (pcpus_changed)
		OBUS_ARRAY_SET(&info_new, pcpus, pcpus_new, ctx->cpu_tot);

	if (ctx->task_total != info_cur->task_total)
		OBUS_SET(&info_new, task_total, ctx->task_total);

	if (ctx->task_running != info_cur->task_running)
		OBUS_SET(&info_new, task_running, ctx->task_running);

	if (ctx->task_sleeping != info_cur->task_sleeping)
		OBUS_SET(&info_new, task_sleeping, ctx->task_sleeping);

	if (ctx->task_stopped != info_cur->task_stopped)
		OBUS_SET(&info_new, task_stopped, ctx->task_stopped);

	if (ctx->task_zombie != info_cur->task_zombie)
		OBUS_SET(&info_new, task_zombie, ctx->task_zombie);

	if (ps_summary_info_is_empty(&info_new))
		goto out;

	ps_bus_event_add_summary_event(bus_event, s_summary,
				       PS_SUMMARY_EVENT_UPDATED, &info_new);

out:
	free(pcpus_new);
}

/**
 *
 */
static void compute_pcpu(struct pcpu_context *ctx, struct process_stat *pst)
{
	unsigned long long tics;

	if (pst == NULL) {
		struct timeval timev;
		struct timezone timez;
		HST_t *hist_tmp;

		/* elapsed time in us */
		gettimeofday(&timev, &timez);
		ctx->et = (timev.tv_sec - ctx->oldtimev.tv_sec) * 1000000LL;
		ctx->et += (timev.tv_usec - ctx->oldtimev.tv_usec);
		if (ctx->et < 0)
			ctx->et = 0;

		ctx->oldtimev.tv_sec = timev.tv_sec;
		ctx->oldtimev.tv_usec = timev.tv_usec;

		ctx->maxt_sav = ctx->task_total;
		ctx->task_total = 0;
		ctx->task_running = 0;
		ctx->task_sleeping = 0;
		ctx->task_stopped = 0;
		ctx->task_zombie = 0;

		/* reuse memory each time around */
		hist_tmp = ctx->hist_sav;
		ctx->hist_sav = ctx->hist_new;
		ctx->hist_new = hist_tmp;
		qsort(ctx->hist_sav, ctx->maxt_sav, sizeof(HST_t), (QFP_t)&sort_HST_t);
		return;
	}

	switch (pst->state) {
	case 'R':
		ctx->task_running++;
		break;
	case 'S':
	case 'D':
	case 'W':
		ctx->task_sleeping++;
		break;
	case 't':
	case 'T':
		ctx->task_stopped++;
		break;
	case 'Z':
		ctx->task_zombie++;
		break;
	default:
		diag_error("unknown task state %d", pst->state);
		break;
	}

	if (ctx->task_total+1 >= ctx->hist_siz) {
		ctx->hist_siz = ctx->hist_siz * 5 / 4 + 100; /* grow by at least 25% */
		ctx->hist_sav = realloc(ctx->hist_sav, sizeof(HST_t) * ctx->hist_siz);
		ctx->hist_new = realloc(ctx->hist_new, sizeof(HST_t) * ctx->hist_siz);
	}
	/* calculate time in this process; the sum of user time (utime) and
	   system time (stime) -- but PLEASE dont waste time and effort on
	   calcs and saves that go unused, like the old top! */
	ctx->hist_new[ctx->task_total].pid  = pst->pid;
	tics = (pst->utime + pst->stime);
	ctx->hist_new[ctx->task_total].tics = tics;

	{
		HST_t tmp;
		const HST_t *ptr;
		tmp.pid = pst->pid;
		ptr = bsearch(&tmp, ctx->hist_sav, ctx->maxt_sav,
				sizeof(HST_t), (QFP_t)&sort_HST_t);
		if (ptr != NULL)
			tics -= ptr->tics;
	}

	/* convert tics to percent */
	pst->pcpu = (float)(tics * 100 * 1000000ULL
			/ (float)(ctx->hertz * ctx->et));
	if (ctx->mode == PS_SUMMARY_MODE_SOLARIS)
		pst->pcpu /= ctx->cpu_tot;
	if (pst->pcpu >= ctx->pcpu_max_value)
		pst->pcpu = ctx->pcpu_max_value;

	/* shout this to the world with the final call (or us the next time in) */
	ctx->task_total++;
}

/**
 *
 */
static void parse_process_stat(uint32_t pid, const char *buf, struct process_stat *pst)
{
	char path[50] = "";
	ssize_t res = 0;
	size_t len = 0;
	const char *p1 = NULL;
	const char *p2 = NULL;

	/* reset structure */
	memset(pst, 0, sizeof(*pst));
	pst->pid = pid;
	pst->pcpu = 0;

	/* get process name (between parenthesis) */
	if ((p1 = strchr(buf, '(')) != NULL && (p2 = strrchr(buf, ')')) != NULL) {
		if (p2 > p1) {
			len = (size_t)(p2 - p1 - 1);
			pst->name = calloc(1, len + 1);
			memcpy(pst->name, p1 + 1, len);
			pst->name[len] = '\0';
			/* skip ') ' */
			buf = p2 + 2;
		}
	}
	/* make sure name is valid */
	if (pst->name == NULL)
		pst->name = strdup("");

	/* get exe path */
	pst->exe = calloc(1, 1024);
	snprintf(path, sizeof(path), "/proc/%d/exe", pid);
	res = readlink(path, pst->exe, 1023);
	if (res < 0) {
		if (errno != EACCES &&
		    errno != ENOENT)
			diag_error("readlink('%s') error=%d(%s)", path, errno,
				   strerror(errno));
		pst->exe[0] = '\0';
	} else {
		pst->exe[(size_t)res] = '\0';
	}

	/* parse buffer */
	sscanf(buf,
		"%c " /* state */
		"%d %d %d %d %d "       /* ppid pgrp session tty_nr tpgid */
		"%lu %lu %lu %lu %lu "  /* flags minflt cminflt majflt cmajflt */
		"%llu %llu %llu %llu ", /* utime stime cutime cstime */
		&pst->state,
		&pst->ppid, &pst->pgrp, &pst->session, &pst->tty_nr, &pst->tpgid,
		&pst->flags, &pst->minflt, &pst->cminflt, &pst->majflt, &pst->cmajflt,
		&pst->utime, &pst->stime, &pst->cutime, &pst->cstime);
}

/**
 *
 */
static enum ps_process_state convert_process_state(char state)
{
	switch (state) {
	case 'R':
		return PS_PROCESS_STATE_RUNNING;
	case 'S':
	case 'D':
	case 'W':
		return PS_PROCESS_STATE_SLEEPING;
	case 't':
	case 'T':
		return PS_PROCESS_STATE_STOPPED;
	case 'Z':
		return PS_PROCESS_STATE_ZOMBIE;
	default:
		return PS_PROCESS_STATE_UNKNOWN;
	}
}

/**
 *
 */
static void update_process(uint32_t pid, const struct process_stat *pst,
		struct ps_bus_event *bus_event)
{
	struct ps_process_info info_new;
	const struct ps_process_info *info_cur = NULL;
	struct ps_process *process = NULL;
	enum ps_process_state ps_state = convert_process_state(pst->state);

	/* get current process_info structure */
	process = find_process(pid);
	if (process != NULL)
		info_cur = ps_process_get_info(process);

	/* fill new structure */
	ps_process_info_init(&info_new);
	if (info_cur == NULL || pst->pid != info_cur->pid)
		OBUS_SET(&info_new, pid, pst->pid);

	if (info_cur == NULL || pst->ppid != info_cur->ppid)
		OBUS_SET(&info_new, ppid, pst->ppid);

	if (info_cur == NULL || strcmp(pst->name, info_cur->name) != 0)
		OBUS_SET(&info_new, name, pst->name);

	if (info_cur == NULL || strcmp(pst->exe, info_cur->exe) != 0)
		OBUS_SET(&info_new, exe, pst->exe);

	if (info_cur == NULL || pst->pcpu != info_cur->pcpu)
		OBUS_SET(&info_new, pcpu, pst->pcpu);

	if (info_cur == NULL || ps_state != info_cur->state)
		OBUS_SET(&info_new, state, ps_state);

	/* create and register new object or send update event */
	if (process == NULL) {
		process = ps_process_new(s_server, &info_new);
		ps_bus_event_register_process(bus_event, process);
	} else if (!ps_process_info_is_empty(&info_new)) {
		ps_bus_event_add_process_event(bus_event, process,
					       PS_PROCESS_EVENT_UPDATED,
					       &info_new);
	}
}

/**
 *
 */
static void cleanup_process_list(struct ps_bus_event *bus_event)
{
	struct ps_process *process = NULL;
	struct ps_process *next = NULL;
	const struct ps_process_info *info;
	char path[50] = "";
	struct stat st;

	/* walk list of process in obus server */
	process = ps_process_next(s_server, NULL);
	while (process != NULL) {
		/* get its pid */
		info = ps_process_get_info(process);

		/* get next entry (before removing current from list */
		next = ps_process_next(s_server, process);

		/* check if process is still present */
		snprintf(path, sizeof(path), "/proc/%d/stat", info->pid);
		if (stat(path, &st) < 0 && errno == ENOENT) {
			/* unregister it in bus event*/
			ps_bus_event_unregister_process(bus_event, process);
		}

		/* iterate */
		process = next;
	}
}

/**
 *
 */
static void scan_process(void)
{
	DIR *dir = NULL;
	struct dirent entry;
	struct dirent *result;
	uint32_t pid = 0;
	int fd = -1;
	ssize_t res = 0;
	struct process_stat pst;
	struct ps_bus_event *bus_event = NULL;

	/* initialize pcpu computation */
	compute_pcpu(&s_pcpu_ctx, NULL);

	/* open '/proc' directory */
	dir = opendir("/proc");
	if (dir == NULL) {
		diag_errno("opendir");
		return;
	}

	bus_event = ps_bus_event_new(PS_BUS_EVENT_UPDATED);

	/* browse entries */
	while (readdir_r(dir, &entry, &result) == 0 && result != NULL) {

		char buf[256] = "";
		char path[50] = "";

		/* skip '.' and '..' */
		if (strcmp(result->d_name, ".") == 0
				|| strcmp(result->d_name, "..") == 0) {
			continue;
		}

		/* only keep directories */
		if (result->d_type != DT_DIR)
			continue;

		/* only keep numeric folder as pid */
		res = atoi(result->d_name);
		if (res <= 0)
			continue;
		pid = (uint32_t)res;

		/* open '/proc/pid/stat' file */
		snprintf(path, sizeof(path), "/proc/%d/stat", pid);
		fd = open(path, O_RDONLY);
		if (fd < 0) {
			/* no log, process may have already died */
			continue;
		}

		/* read its content */
		res = read(fd, buf, sizeof(buf)-1);
		if (res < 0) {
			diag_errno("read");
			close(fd);
			continue;
		}

		/* parse buffer */
		buf[res] = '\0';
		parse_process_stat(pid, buf, &pst);

		/* compute %CPU */
		compute_pcpu(&s_pcpu_ctx, &pst);

		/* update process and free resources */
		update_process(pid, &pst, bus_event);
		free(pst.exe);
		free(pst.name);
		close(fd);
	}

	/* close '/proc' directory */
	closedir(dir);

	update_cpus(&s_pcpu_ctx, bus_event);

	/* cleanup dead processes */
	cleanup_process_list(bus_event);

	/* send bus event */
	/*if (!ps_bus_event_is_empty(bus_event)) */
	ps_bus_event_send(s_server, bus_event);

	/* free resources */
	ps_bus_event_destroy(bus_event);
}

/**
 *
 */
static void print_usage(void)
{
	fprintf(stderr, "psd <address1> <address2> ...\n\n");

	fprintf(stderr, "<address>: server listen socket address\n");
	fprintf(stderr, "\tipv4 address \"inet:<address>:<port>\"\n");
	fprintf(stderr, "\tipv6 address \"inet6:<address>:<port>\"\n");
	fprintf(stderr, "\tunix address \"unix:<path>\""
			"(use '@' for abstract socket)\n\n");
}

/**
 *
 */
static void summary_set_refresh_rate(struct ps_summary *object,
		obus_handle_t handle,
		const struct ps_summary_set_refresh_rate_args *args)
{
	struct itimerspec nval;
	int ret = 0;
	struct ps_summary_info info_new;

	/* check parameters */
	if (args == NULL
			|| !args->fields.refresh_rate
			|| args->refresh_rate <= 0) {
		obus_server_send_ack(s_server, handle,
				OBUS_CALL_INVALID_ARGUMENTS);
		return;
	}

	/* send call ack status */
	obus_server_send_ack(s_server, handle, OBUS_CALL_ACKED);

	/* nothing more to do if value does not change */
	if (s_pcpu_ctx.refresh_rate == args->refresh_rate)
		return;

	/* update state */
	ps_summary_info_init(&info_new);
	OBUS_SET(&info_new, refresh_rate, args->refresh_rate);
	s_pcpu_ctx.refresh_rate = args->refresh_rate;
	ps_summary_send_event(s_server, s_summary,
			PS_SUMMARY_EVENT_REFRESH_RATE_CHANGED,
			&info_new);

	/* update internal timer */
	nval.it_interval.tv_sec = s_pcpu_ctx.refresh_rate;
	nval.it_interval.tv_nsec = 0;
	nval.it_value.tv_sec = s_pcpu_ctx.refresh_rate;
	nval.it_value.tv_nsec = 0;
	ret = timerfd_settime(s_tfd, 0, &nval, NULL);
	if (ret < 0) {
		diag_errno("timerfd_settime");
	}
}

/**
 *
 */
static void summary_set_mode(struct ps_summary *object,
		obus_handle_t handle,
		const struct ps_summary_set_mode_args *args)
{
	struct ps_summary_info info_new;

	/* check parameters */
	if (args == NULL
			|| !args->fields.mode
			|| !ps_summary_mode_is_valid(args->mode)) {
		obus_server_send_ack(s_server, handle,
				OBUS_CALL_INVALID_ARGUMENTS);
		return;
	}

	/* send call ack status */
	obus_server_send_ack(s_server, handle, OBUS_CALL_ACKED);

	/* nothing more to do if value does not change */
	if (s_pcpu_ctx.mode == args->mode)
		return;

	/* update state */
	ps_summary_info_init(&info_new);
	OBUS_SET(&info_new, mode, args->mode);
	s_pcpu_ctx.mode = args->mode;
	ps_summary_send_event(s_server, s_summary,
			PS_SUMMARY_EVENT_MODE_CHANGED,
			&info_new);

	/* adjust max %CPU value */
	s_pcpu_ctx.pcpu_max_value = 100;
	if (s_pcpu_ctx.mode)
		s_pcpu_ctx.pcpu_max_value = 100 * s_pcpu_ctx.cpu_tot;
}

/** */
const struct ps_summary_method_handlers s_summary_handlers = {
	.method_set_refresh_rate = summary_set_refresh_rate,
	.method_set_mode = summary_set_mode,
};

/**
 *
 */
int main(int argc, char *argv[])
{
	int ret, i, stop;
	uint64_t u;
	const char **addrs = NULL;
	size_t n_addrs;
	struct itimerspec nval;
	struct pollfd fds[3];
	struct ps_summary_info summary_info;

	if (argc < 2) {
		fprintf(stderr, "expected server address argument\n");
		print_usage();
		ret = EXIT_FAILURE;
		goto out;
	}

	n_addrs = (size_t)argc - 1;
	addrs = calloc(n_addrs, sizeof(char *));
	if (!addrs) {
		ret = EXIT_FAILURE;
		goto out;
	}

	for (i = 1; i < argc; i++)
		addrs[i - 1] = argv[i];

	/* mode :
	  SOLARIS : %CPU will never be > 100%, even with several cpus
	  IRIX : %CPU can be > 100%, up to number of cpu */
	s_pcpu_ctx.mode = PS_SUMMARY_MODE_IRIX;

	/* refresh rate in seconds */
	s_pcpu_ctx.refresh_rate = 3;

	/* get number of cpu and tics per second */
	s_pcpu_ctx.cpu_tot = (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
	s_pcpu_ctx.hertz = (uint32_t)sysconf(_SC_CLK_TCK);
	diag("cpu_tot=%d hertz=%d", s_pcpu_ctx.cpu_tot, s_pcpu_ctx.hertz);

	/* adjust max %CPU value */
	s_pcpu_ctx.pcpu_max_value = 100;
	if (s_pcpu_ctx.mode)
		s_pcpu_ctx.pcpu_max_value = 100 * s_pcpu_ctx.cpu_tot;

	/* per cpu data (1 more for cumulative values) */
	s_pcpu_ctx.cpus = (CPU_t *)calloc(s_pcpu_ctx.cpu_tot + 1, sizeof(CPU_t));

	/* open pipes */
	ret = pipe(s_fdpipes);
	if (ret < 0) {
		ret = EXIT_FAILURE;
		goto out;
	}

	/* create timerfd used for network scan */
	s_tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (s_tfd == -1) {
		diag_errno("timerfd_create");
		ret = EXIT_FAILURE;
		goto close_pipes;
	}

	/* set timer based on refresh rate */
	nval.it_interval.tv_sec = s_pcpu_ctx.refresh_rate;
	nval.it_interval.tv_nsec = 0;
	nval.it_value.tv_sec = s_pcpu_ctx.refresh_rate;
	nval.it_value.tv_nsec = 0;
	ret = timerfd_settime(s_tfd, 0, &nval, NULL);
	if (ret < 0) {
		diag_errno("timerfd_settime");
		ret = EXIT_FAILURE;
		goto close_timer;
	}

	/* create server */
	s_server = obus_server_new(ps_bus_desc);
	if (!s_server) {
		diag_error("can't create obus net server");
		ret = EXIT_FAILURE;
		goto close_timer;
	}

	/* attach sig handler */
	signal(SIGINT, &sig_handler);
	signal(SIGTERM, &sig_handler);

	/* create summary object */
	ps_summary_info_init(&summary_info);
	OBUS_SET(&summary_info, refresh_rate, s_pcpu_ctx.refresh_rate);
	OBUS_SET(&summary_info, mode, s_pcpu_ctx.mode);
	OBUS_SET(&summary_info, method_set_refresh_rate, OBUS_METHOD_ENABLED);
	OBUS_SET(&summary_info, method_set_mode, OBUS_METHOD_ENABLED);
	s_summary = ps_summary_new(s_server, &summary_info, &s_summary_handlers);
	ps_summary_register(s_server, s_summary);

	/* scan process, creates obus interface objects */
	scan_process();

	/* start server */
	ret = obus_server_start(s_server, addrs, n_addrs);
	if (ret < 0) {
		diag_error("can't start obus process server");
		ret = EXIT_FAILURE;
		goto destroy_server;
	}

	/* build poll fds */
	fds[0].fd = s_fdpipes[0];
	fds[0].events = POLLIN;
	fds[1].fd = obus_server_fd(s_server);
	fds[1].events = POLLIN;
	fds[2].fd = s_tfd;
	fds[2].events = POLLIN;

	/* run main loop */
	stop = 0;
	do {
		/* wait fd I/O occurs */
		do {
			ret = poll(fds, SIZEOF_ARRAY(fds), -1);
		} while (ret == -1 && errno == EINTR);

		if (ret < 0) {
			diag_errno("poll");
			stop = 1;
		}

		/* check read pipe events*/
		if (fds[0].revents)
			stop = 1;

		/* process obus event */
		if (!stop && fds[1].revents)
			obus_server_process_fd(s_server);

		/* update timer */
		if (!stop && ((fds[2].revents & POLLIN) == POLLIN)) {
			/* read timer */
			do {
				ret = (int)read(s_tfd, &u, sizeof(u));
			} while (ret < 0 && errno == EINTR);

			/* rescan process */
			scan_process();
		}

	} while (!stop);

	/* free %CPU context resources */
	free(s_pcpu_ctx.hist_sav);
	free(s_pcpu_ctx.hist_new);
	free(s_pcpu_ctx.cpus);

	ret = EXIT_SUCCESS;

destroy_server:
	obus_server_destroy(s_server);
close_timer:
	close(s_tfd);
close_pipes:
	close(s_fdpipes[0]);
	close(s_fdpipes[1]);
out:
	free(addrs);
	return ret;
}

