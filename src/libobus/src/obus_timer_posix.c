/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_timer_posix.c
 *
 * @brief obus timer using posix implementation
 *
 * @author jean-baptiste.dubois@parrot.com
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

#include "obus_header.h"

#ifndef HAVE_SYS_TIMERFD_H

#define MSEC_PER_SEC	1000L
#define USEC_PER_MSEC	1000L
#define NSEC_PER_USEC	1000L
#define NSEC_PER_MSEC	1000000L
#define USEC_PER_SEC	1000000L
#define NSEC_PER_SEC	1000000000L

#define OBUS_SIGALARM SIGRTMIN

/* obus timer */
struct obus_timer {
	struct obus_fd ofd;
	struct obus_loop *loop;
	int timeout;
	obus_timer_cb_t cb;
	void *data;
	int pipefd[2];
	timer_t timerid;
};

static void obus_timer_sighandler(int signum, siginfo_t *si, void *unused)
{
	ssize_t ret;
	struct obus_timer *timer;
	uint8_t dummy = 0;

	/* extract timer from siginfo */
	timer = si->si_value.sival_ptr;
	if (timer) {
		/* write in pipe to notify loop */
		do {
			ret = write(timer->pipefd[1], &dummy, sizeof(dummy));
		} while (ret < 0 && errno == EINTR);
	}
}

static int obus_timer_read(struct obus_timer *timer)
{
	uint8_t u;
	ssize_t ret;

	/* Read the pipe */
	do {
		ret = read(timer->pipefd[0], &u, sizeof(u));
	} while (ret < 0 && errno == EINTR);

	if (ret < 0) {
		obus_log_fd_errno("timer_read", timer->pipefd[0]);
		ret = -errno;
	} else {
		ret = 0;
	}

	return (int)ret;
}

static void obus_timer_events(struct obus_fd *ofd, int events, void *data)
{
	struct obus_timer *timer = (struct obus_timer *)data;
	int ret;
	uint64_t nbexpired = 0;

	if (obus_fd_event_error(events)) {
		/* remove source from loop */
		obus_loop_remove(timer->loop, &timer->ofd);
	}

	if (!obus_fd_event_read(events))
		return;

	/* read timer value (force nbexpired to 1) */
	nbexpired = 1;
	ret = obus_timer_read(timer);
	if (ret < 0)
		return;

	/* invoke timer callback */
	(*timer->cb) (timer, &nbexpired, timer->data);
}

struct obus_timer *obus_timer_new(struct obus_loop *loop, obus_timer_cb_t cb,
				  void *data)
{
	struct obus_timer *timer;
	struct sigevent sev;
	struct sigaction sa;
	int ret;

	if (!loop || !cb)
		return NULL;

	timer = calloc(1, sizeof(*timer));
	if (!timer)
		return NULL;

	timer->cb = cb;
	timer->data = data;
	timer->timeout = 0;
	timer->loop = obus_loop_ref(loop);

	/* create timer with CLOCK_MONOTONIC clock*/
	memset(&sev, 0, sizeof(sev));
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = OBUS_SIGALARM;
	sev.sigev_value.sival_ptr = timer;
	ret = timer_create(CLOCK_MONOTONIC, &sev, &timer->timerid);
	if (ret < 0) {
		obus_log_errno("timer_create");
		goto unref_loop;
	}

	/* setup signal handler */
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = &obus_timer_sighandler;
	sa.sa_flags = SA_SIGINFO;
	ret = sigaction(OBUS_SIGALARM, &sa, NULL);
	if (ret < 0) {
		obus_log_errno("sigaction");
		goto close_timer;
	}

	/* create pipe for notification */
	ret = pipe(timer->pipefd);
	if (ret < 0) {
		obus_log_errno("pipe");
		goto close_timer;
	}

	/* setup pipe fds flags */
	ret = obus_fd_set_close_on_exec(timer->pipefd[0]);
	if (ret < 0)
		goto close_pipe;
	ret = obus_fd_set_close_on_exec(timer->pipefd[1]);
	if (ret < 0)
		goto close_pipe;
	ret = obus_fd_add_flags(timer->pipefd[0], O_NONBLOCK);
	if (ret < 0)
		goto close_pipe;
	ret = obus_fd_add_flags(timer->pipefd[1], O_NONBLOCK);
	if (ret < 0)
		goto close_pipe;

	/* add pipe input fd object in loop */
	obus_fd_init(&timer->ofd, timer->pipefd[0], OBUS_FD_IN,
		     &obus_timer_events, timer);
	ret = obus_loop_add(loop, &timer->ofd);
	if (ret < 0)
		goto close_pipe;

	return timer;

close_pipe:
	close(timer->pipefd[0]);
	close(timer->pipefd[1]);
close_timer:
	timer_delete(timer->timerid);
unref_loop:
	obus_loop_unref(loop);
	free(timer);
	return NULL;
}

int obus_timer_destroy(struct obus_timer *timer)
{
	if (!timer)
		return -EINVAL;

	obus_loop_remove(timer->loop, &timer->ofd);
	close(timer->pipefd[0]);
	close(timer->pipefd[1]);
	/* FIXME: The treatment  of  any  pending signal generated by the
	 * deleted timer is unspecified */
	timer_delete(timer->timerid);
	obus_loop_unref(timer->loop);
	free(timer);
	return 0;
}

int obus_timer_set(struct obus_timer *timer, int timeout)
{
	int ret = 0;
	struct itimerspec nval, oval;

	/* configure one shot */
	nval.it_interval.tv_sec = 0;
	nval.it_interval.tv_nsec = 0;

	if (timeout > 0) {
		nval.it_value.tv_sec = timeout / MSEC_PER_SEC;
		nval.it_value.tv_nsec = (timeout % MSEC_PER_SEC)*NSEC_PER_MSEC;
	} else {
		/* clear timer */
		nval.it_value.tv_sec = 0;
		nval.it_value.tv_nsec = 0;
	}
	ret = timer_settime(timer->timerid, 0, &nval, &oval);
	if (ret == -1) {
		ret = -errno;
		obus_log_fd_errno("timerfd_settime", timer->ofd.fd);
	} else {
		timer->timeout = timeout;
	}
	return ret;
}

int obus_timer_clear(struct obus_timer *timer)
{
	return obus_timer_set(timer, 0);
}

#endif /* !HAVE_SYS_TIMERFD_H */
