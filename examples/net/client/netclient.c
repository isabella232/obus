/******************************************************************************
 * @file netclient.c
 *
 * @brief net client
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/time.h>
#include <poll.h>
#include <stdarg.h>
#include <getopt.h>
#include <inttypes.h>
#include <ctype.h>
#include <libobus.h>
#include "net_bus.h"
#include "net_interface.h"

#define SIZEOF_ARRAY(x) (sizeof((x)) / sizeof((x)[0]))

enum cmd_type {
	CMD_INVALID = -1,
	CMD_LIST = 0,
	CMD_MONITOR,
	CMD_UP,
	CMD_DOWN,
};

static enum cmd_type cmd = CMD_LIST;
static char *iface;
static int verbose;
static int fdpipes[2];
static struct obus_client *client;
static obus_handle_t call_handle = OBUS_INVALID_HANDLE;
static enum obus_call_status call_status = OBUS_CALL_INVALID;

static void do_log(const char *fmt, ...)
{
	va_list args;

	if (!fmt)
		return;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

#define diag(_fmt, args...) \
	do_log(_fmt , ##args)

#define diag_error(_fmt, args...) \
	do_log("error: "_fmt , ##args)

#define diag_errno(func) \
	do_log("error: %s error=%d(%s)",func, errno, strerror(errno))

static void stop_client(void)
{
	ssize_t ret;
	uint8_t dummy = 0xff;
	ret  = write(fdpipes[1], &dummy , sizeof(dummy));
	if (ret < 0)
		diag_error("can't write in pipe, error:%s\n", strerror(errno));
}

static void sig_handler(int signum)
{
	if (verbose)
		diag("signal %d(%s) received !\n", signum, strsignal(signum));

	stop_client();
}

static void print_usage(void)
{
	diag("netclient [options] [command] <address>\n\n");
	diag("[options]\n");
	diag("\t-v   verbose\n");

	diag("[command]\n");
	diag("\t-h   show this help message and exit\n");
	diag("\t-m   monitor net bus\n");
	diag("\t-l   list interface (default)\n");
	diag("\t-u <itfname> set interface up\n");
	diag("\t-d <itfname> set interface down\n");

	diag("<address>: net bus server address\n");
	diag("\tipv4 address \"inet:<address>:<port>\"\n");
	diag("\tipv6 address \"inet6:<address>:<port>\"\n");
	diag("\tunix address \"unix:<path>\""
			"(use '@' for abstract socket)\n\n");
}

/* parse arguments */
static void parse_args(int argc, char *argv[])
{
	int c;

	opterr = 0;
	while ((c = getopt(argc, argv, "mlhu:d:v")) != -1) {
		switch (c) {
		case 'm':
			cmd = CMD_MONITOR;
		break;
		case 'h':
			print_usage();
			free(iface);
			exit(EXIT_SUCCESS);
		break;
		case 'l':
			cmd = CMD_LIST;
		break;
		case 'u':
			cmd = CMD_UP;
			free(iface);
			iface = strdup(optarg);
		break;
		case 'd':
			cmd = CMD_DOWN;
			free(iface);
			iface = strdup(optarg);
		break;
		case 'v':
			verbose = 1;
		break;

		case '?':
			if (optopt == 'u' || optopt == 'd')
				diag_error("Option -%c requires an argument.\n",
					   optopt);
		else if (isprint (optopt))
			diag_error("Unknown option `-%c'.\n", optopt);
		else
			diag_error("Unknown option character `\\x%x'.\n",
				   optopt);
			free(iface);
			exit(EXIT_FAILURE);
		break;
		default:
			free(iface);
			exit(EXIT_FAILURE);
		}
	}
}


static char *format_bytes(uint64_t bytes)
{
	uint64_t int_part;
	char *result;
	const char *ext;
	unsigned int frac_part;
	int i, ret;
	static const char TRext[] = "\0\0\0Ki\0Mi\0Gi\0Ti";

	frac_part = 0;
	int_part = bytes;
	ext = TRext;
	i = 4;
	do {
		if (int_part >= 1024) {
			frac_part = ((((unsigned int) int_part) & (1024-1))
					* 10) / 1024;
			int_part /= 1024;
			ext += 3;	/* KiB, MiB, GiB, TiB */
		}
		--i;
	} while (i);

	ret = asprintf(&result, "%" PRIu64 " (%" PRIu64 ".%u %sB)",
		       bytes, int_part, frac_part, ext);
	if (ret < 0)
		return NULL;

	return result;
}

static void print_interface(struct net_interface *itf)
{
	const struct net_interface_info *info;
	char *bytes[2];

	info = net_interface_get_info(itf);

	diag("%-10.10s%s  HWaddr:%s\n", info->name,
	     (info->state == NET_INTERFACE_STATE_UP) ? "UP" : "DOWN",
	      info->hw_addr);

	diag("%-10.10sinet addr:%s  Bcast:%s  Mask:%s\n", "",
			info->ip_addr , info->broadcast ,info->netmask);

	/* format rx and tx bytes */
	bytes[0] = format_bytes(info->bytes[0]);
	bytes[1] = format_bytes(info->bytes[1]);

	diag("%-10.10sRX bytes:%s  TX bytes:%s\n", "", bytes[0], bytes[1]);
	diag("\n");
	free(bytes[0]);
	free(bytes[1]);
}

static struct net_interface *
get_net_interface_by_name(struct obus_client *clt, const char *name)
{
	struct net_interface *itf;
	const struct net_interface_info *info;

	itf = net_interface_next(clt, NULL);
	while (itf) {
		info = net_interface_get_info(itf);
		if (info && info->name && (strcmp(info->name, name) == 0))
			return itf;

		itf = net_interface_next(clt, itf);
	}
	return NULL;
}

static void print_interfaces(struct obus_client *clt)
{
	struct net_interface *itf;

	itf = net_interface_next(clt, NULL);
	while (itf) {

		print_interface(itf);
		itf = net_interface_next(clt, itf);
	}
}


static void method_status_cb(struct net_interface *object,
			     obus_handle_t handle,
			     enum obus_call_status status)
{
	call_status = status;
	if (status != OBUS_CALL_ACKED)
		stop_client();

	/* else wait for next event */
}

static void execute_command(void)
{
	struct net_interface *itf;
	struct net_interface_up_args up_args;
	int ret;

	switch (cmd) {
	case CMD_UP:
	case CMD_DOWN:
		if(call_handle == OBUS_INVALID_HANDLE) {
			itf = get_net_interface_by_name(client, iface);
			if (!itf) {
				diag_error("interface '%s' not found\n",
					   iface);
				stop_client();
				break;
			}

			if (cmd == CMD_DOWN) {
				ret = net_interface_call_down(client, itf,
							      method_status_cb,
							      &call_handle);
			} else {
				net_interface_up_args_init(&up_args);
				ret = net_interface_call_up(client, itf,
							    &up_args,
							    &method_status_cb,
							    &call_handle);
			}
			/* on call failure stop client */
			if (ret < 0) {
				diag_error("interface call error\n");
				stop_client();
			}
		}
	break;

	case CMD_LIST:
	case CMD_MONITOR:
		print_interfaces(client);
		if (cmd == CMD_LIST)
			stop_client();
	break;
	case CMD_INVALID:
	default:
		stop_client();
	break;
	}
}

static void net_interface_add(struct net_interface *itf,
			      struct net_bus_event *bus_event,
			      void *user_data)
{
	const struct net_interface_info *info = net_interface_get_info(itf);
	if (cmd == CMD_MONITOR) {
		diag("net_interface:[ADD] name=%s\n", info->name);
		print_interface(itf);
	}
}

static void net_interface_remove(struct net_interface *itf,
				 struct net_bus_event *bus_event,
				 void *user_data)
{
	const struct net_interface_info *info = net_interface_get_info(itf);

	if (cmd == CMD_MONITOR)
		diag("net_interface:[REMOVE] name=%s\n",  info->name);
}

static void net_interface_event(struct net_interface *itf,
				struct net_interface_event *event,
				struct net_bus_event *bus_event,
				void *user_data)
{
	enum net_interface_event_type type = net_interface_event_get_type(event);
	const struct net_interface_info *info = net_interface_get_info(itf);

	if (cmd == CMD_MONITOR)
		diag("net_interface:[EVENT:%d] name=%s\n", type, info->name);

	/* commit event */
	net_interface_event_commit(event);

	/* print interface */
	if (cmd == CMD_MONITOR)
		print_interface(itf);

	if ((cmd == CMD_UP || cmd == CMD_DOWN) &&
	     call_status == OBUS_CALL_ACKED &&
	     iface && strcmp(iface, info->name) == 0) {

		if (type == NET_INTERFACE_EVENT_UP ||
		    type == NET_INTERFACE_EVENT_UP_FAILED ||
		    type == NET_INTERFACE_EVENT_DOWN ||
		    type == NET_INTERFACE_EVENT_DOWN_FAILED) {
			stop_client();
		}

	}
}

static void netclient_event(struct obus_bus_event *bus_event, void *user_data)
{
	struct net_bus_event *event;

	/* get net bus event */
	event = net_bus_event_from_obus_event(bus_event);
	if (!event)
		return;

	switch(net_bus_event_get_type(event)) {
	case NET_BUS_EVENT_CONNECTED:
		if (verbose)
			diag("client connected !\n");
		/* commit bus_event */
		obus_client_commit_bus_event(client, bus_event);
		/* execute client command */
		execute_command();
	break;
	case NET_BUS_EVENT_DISCONNECTED:
		if (verbose)
			diag("client disconnected !\n");
	break;
	case NET_BUS_EVENT_CONNECTION_REFUSED:
		diag_error("connection refused !\n");
		stop_client();
	break;
	case NET_BUS_EVENT_SCAN_COMPLETED:
	case NET_BUS_EVENT_COUNT:
	break;
	default:
	break;
	}
}

int main(int argc, char *argv[])
{
	int ret, stop;
	struct pollfd fds[2];
	const char *addr;
	struct net_interface_provider provider = {
		.add = net_interface_add,
		.remove = net_interface_remove,
		.event = net_interface_event,
	};

	/* parse arguments */
	parse_args(argc, argv);

	if (optind >= argc) {
		diag_error("expected server address argument\n");
		print_usage();
		free(iface);
		exit(EXIT_FAILURE);
	}

	addr = argv[optind];

	/* open pipes */
	ret = pipe(fdpipes);
	if (ret < 0) {
		ret = EXIT_FAILURE;
		goto out;
	}

	/* create client */
	client = obus_client_new("netclient", net_bus_desc,
				 netclient_event, NULL);
	if (!client) {
		diag_error("can't create obus net client\n");
		ret = EXIT_FAILURE;
		goto close_pipes;
	}

	/* subscribe to net interface */
	ret = net_interface_subscribe(client, &provider, NULL);
	if (ret < 0) {
		diag_error("can't subscribe to net interface\n");
		ret = EXIT_FAILURE;
		goto destroy_client;
	}

	/* start client */
	ret = obus_client_start(client, addr);
	if (ret < 0) {
		diag_error("can't start obus net client\n");
		ret = EXIT_FAILURE;
		goto unsubscribe_provider;
	}

	/* attach sig handler */
	signal(SIGINT, &sig_handler);

	/* build poll fds */
	fds[0].fd = fdpipes[0];
	fds[0].events = POLLIN;
	fds[1].fd = obus_client_fd(client);
	fds[1].events = POLLIN;

	/* run main loop */
	stop = 0;
	do {
		/* wait fd I/O occurs */
		do {
			ret = poll(fds, SIZEOF_ARRAY(fds), -1);
		} while (ret == -1 && errno == EINTR);

		if (ret < 0) {
			diag_errno("poll\n");
			stop = 1;
		}

		/* check read pipe events*/
		if (fds[0].revents)
			stop = 1;

		/* process obus event */
		if (!stop && fds[1].revents)
			obus_client_process_fd(client);

	} while (!stop);

	ret = EXIT_SUCCESS;

unsubscribe_provider:
	net_interface_unsubscribe(client, &provider);
destroy_client:
	obus_client_destroy(client);
close_pipes:
	close(fdpipes[0]);
	close(fdpipes[1]);
out:
	free(iface);
	return ret;
}
