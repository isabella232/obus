/******************************************************************************
 * @file psclient.c
 *
 * @brief ps client
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
#include <libobus.h>
#include <ctype.h>
#include "ps_bus.h"
#include "ps_process.h"
#include "ps_summary.h"
#include "log.h"


#define SIZEOF_ARRAY(x) (sizeof((x)) / sizeof((x)[0]))

static int fdpipes[2];
static struct obus_client *client;


static void stop_client(void)
{
	ssize_t ret;
	uint8_t dummy = 0xff;
	ret  = write(fdpipes[1], &dummy , sizeof(dummy));
	if (ret < 0)
		diag_error("can't write in pipe, error:%s", strerror(errno));
}

static void sig_handler(int signum)
{
	ssize_t ret;
	uint8_t dummy = 0xff;
	diag("signal %d(%s) received !", signum, strsignal(signum));
	ret = write(fdpipes[1], &dummy , sizeof(dummy));
	if (ret < 0)
		diag_error("can't write in pipe, error:%s", strerror(errno));
}

static void ps_summary_add(struct ps_summary *summary,
			   struct ps_bus_event *bus_event,
			   void *user_data)
{

}

static void ps_summary_remove(struct ps_summary *summary,
			      struct ps_bus_event *bus_event,
			      void *user_data)
{

}

static void ps_summary_event(struct ps_summary *summary,
			     struct ps_summary_event *event,
			     struct ps_bus_event *bus_event,
			     void *user_data)
{

}

static void ps_process_add(struct ps_process *proc,
			   struct ps_bus_event *bus_event,
			   void *user_data)
{

}

static void ps_process_remove(struct ps_process *proc,
			      struct ps_bus_event *bus_event,
			      void *user_data)
{

}

static void ps_process_event(struct ps_process *proc,
			     struct ps_process_event *event,
			     struct ps_bus_event *bus_event,
			     void *user_data)
{

}

static void psclient_event(struct obus_bus_event *bus_event, void *user_data)
{
	struct ps_bus_event *event;

	/* get ps bus event */
	event = ps_bus_event_from_obus_event(bus_event);
	if (!event)
		return;

	switch(ps_bus_event_get_type(event)) {
	case PS_BUS_EVENT_CONNECTED:
		diag("client connected !");
	break;
	case PS_BUS_EVENT_DISCONNECTED:
		diag("client disconnected !");
	break;
	case PS_BUS_EVENT_CONNECTION_REFUSED:
		diag("connection refused !");
		stop_client();
	break;

	default:
	break;
	}
}


static void print_usage(void)
{
	diag("psclient <address>\n\n");

	diag("<address>: ps bus server address\n");
	diag("\tipv4 address \"inet:<address>:<port>\"\n");
	diag("\tipv6 address \"inet6:<address>:<port>\"\n");
	diag("\tunix address \"unix:<path>\""
			"(use '@' for abstract socket)\n\n");
}

int main(int argc, char *argv[])
{
	int ret, stop;
	struct pollfd fds[2];
	struct ps_process_provider process_provider = {
		.add = ps_process_add,
		.remove = ps_process_remove,
		.event = ps_process_event,
	};
	struct ps_summary_provider summary_provider = {
		.add = ps_summary_add,
		.remove = ps_summary_remove,
		.event = ps_summary_event,
	};

	if (argc < 2) {
		diag_error("expected server address argument\n");
		print_usage();
		exit(EXIT_FAILURE);
	}

	/* open pipes */
	ret = pipe(fdpipes);
	if (ret < 0) {
		ret = EXIT_FAILURE;
		goto out;
	}

	/* create client */
	client = obus_client_new("psclient", ps_bus_desc,
				 psclient_event, NULL);
	if (!client) {
		diag_error("can't create obus ps client");
		ret = EXIT_FAILURE;
		goto close_pipes;
	}

	/* subscribe to ps process objects */
	ret = ps_process_subscribe(client, &process_provider, NULL);
	if (ret < 0) {
		diag_error("can't subscribe to ps process");
		ret = EXIT_FAILURE;
		goto destroy_client;
	}

	/* subscribe to ps summary objects */
	ret = ps_summary_subscribe(client, &summary_provider, NULL);
	if (ret < 0) {
		diag_error("can't subscribe to ps process");
		ret = EXIT_FAILURE;
		goto destroy_client;
	}

	/* start client */
	ret = obus_client_start(client, argv[1]);
	if (ret < 0) {
		diag_error("can't start obus ps client");
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
			diag_errno("poll");
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
	ps_process_unsubscribe(client, &process_provider);
	ps_summary_unsubscribe(client, &summary_provider);
destroy_client:
	obus_client_destroy(client);
close_pipes:
	close(fdpipes[0]);
	close(fdpipes[1]);
out:
	return ret;
}
