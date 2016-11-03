/******************************************************************************
 * @file netd.c
 *
 * @brief obus net server daemon
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
#include <sys/timerfd.h>
#include <poll.h>
#include <stdarg.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <time.h>
#include <libobus.h>
#include "net_bus.h"
#include "net_interface.h"
#include "netif.h"
#include "log.h"

/* timer fd defines */
#ifndef TFD_NONBLOCK
#define TFD_NONBLOCK O_NONBLOCK
#endif

#ifndef TFD_CLOEXEC
#define TFD_CLOEXEC O_CLOEXEC
#endif

#define SIZEOF_ARRAY(x) (sizeof((x)) / sizeof((x)[0]))

static int fdpipes[2];
static struct obus_server *server;

static void sig_handler(int signum)
{
	ssize_t ret;
	uint8_t dummy = 0xff;
	diag("signal %d(%s) received !", signum, strsignal(signum));
	ret = write(fdpipes[1], &dummy , sizeof(dummy));
	if (ret < 0)
		diag_error("can't write in pipe, error:%s", strerror(errno));
}

static struct net_interface *
get_net_interface_by_name(struct obus_server *srv, const char *name)
{
	struct net_interface *itf;
	const struct net_interface_info *info;

	itf = net_interface_next(srv, NULL);
	while (itf) {
		info = net_interface_get_info(itf);
		if (info && info->name && (strcmp(info->name, name) == 0))
			return itf;

		itf = net_interface_next(srv, itf);
	}
	return NULL;
}

static void
update_traffic(struct obus_server *srv, struct net_interface *itf)
{
	const struct net_interface_info *current;
	struct net_interface_info event_info;
	uint64_t bytes[2];
	int ret;

	/* get current interface info */
	current = net_interface_get_info(itf);

	/* init event info */
	net_interface_info_init(&event_info);

	/* update traffic */
	ret = netitf_read_traffic(current->name, &bytes[0], &bytes[1]);
	if (ret == 0 &&
	    (current->n_bytes < 2 ||
	     current->bytes[0] != bytes[0] ||
	     current->bytes[1] != bytes[1])) {
		OBUS_ARRAY_SET(&event_info, bytes, bytes, 2);

		/* send event  */
		net_interface_send_event(srv, itf,  NET_INTERFACE_EVENT_TRAFFIC,
					 &event_info);
	}
}

static void
update_interface(struct obus_server *srv, struct net_interface *itf)
{
	const struct net_interface_info *current;
	enum net_interface_event_type event_type;
	struct net_interface_info event_info;
	char ip_addr[50];
	char hw_addr[50];
	char bcast[50];
	char netmask[50];
	int is_up, ret;


	/* get current interface info */
	current = net_interface_get_info(itf);

	/* init event info */
	net_interface_info_init(&event_info);

	/* update ip address changed */
	ret = netif_read_ip_address(current->name, ip_addr, sizeof(ip_addr));
	if (ret < 0)
		ip_addr[0] = '\0';

	if ((current->ip_addr && strcmp(ip_addr, current->ip_addr) != 0) ||
	    (!current->ip_addr && ip_addr[0] != '\0')) {
		OBUS_SET(&event_info, ip_addr, ip_addr);
	}

	/* update hw address */
	ret = netif_read_hw_address(current->name, hw_addr, sizeof(hw_addr));
	if (ret < 0)
		hw_addr[0] = '\0';

	if ((current->hw_addr && strcmp(hw_addr, current->hw_addr) != 0) ||
	    (!current->hw_addr && hw_addr[0] != '\0')) {
		OBUS_SET(&event_info, hw_addr, hw_addr);
	}

	/* read broadcast address */
	ret = netif_read_bcast_address(current->name, bcast, sizeof(bcast));
	if (ret < 0)
		bcast[0] = '\0';

	if ((current->broadcast && strcmp(bcast, current->broadcast) != 0) ||
	    (!current->broadcast && bcast[0] != '\0')) {
		OBUS_SET(&event_info, broadcast, bcast);
	}

	/* read netmask */
	ret = netif_read_netmask(current->name, netmask, sizeof(netmask));
	if (ret < 0)
		netmask[0] = '\0';

	if ((current->netmask && strcmp(netmask, current->netmask) != 0) ||
	    (!current->netmask && netmask[0] != '\0')) {
		OBUS_SET(&event_info, netmask, netmask);
	}

	/* check interface status */
	is_up = netif_is_up(current->name);

	/* check state up */
	if (is_up && current->state == NET_INTERFACE_STATE_DOWN) {
		/* generate up event */
		event_type = NET_INTERFACE_EVENT_UP;

		/* update interface state */
		OBUS_SET(&event_info, state, NET_INTERFACE_STATE_UP);

		/* update interface method state */
		OBUS_DISABLE_METHOD(&event_info, up);
		OBUS_ENABLE_METHOD(&event_info, down);

	/* check state down */
	} else if (!is_up && current->state == NET_INTERFACE_STATE_UP) {
		/* generate down event */
		event_type = NET_INTERFACE_EVENT_DOWN;

		/* update interface state */
		OBUS_SET(&event_info, state, NET_INTERFACE_STATE_DOWN);

		/* update interface method state */
		OBUS_DISABLE_METHOD(&event_info, down);
		OBUS_ENABLE_METHOD(&event_info, up);

	} else if (event_info.fields.ip_addr || event_info.fields.hw_addr) {
		/* generate configured event */
		event_type = NET_INTERFACE_EVENT_CONFIGURED;
	} else {
		/* nothing has changed ! */
		return;
	}

	/* send event  */
	net_interface_send_event(srv, itf, event_type, &event_info);
}

static void netif_call_up(struct net_interface *itf, obus_handle_t handle,
			 const struct net_interface_up_args *args)
{
	int ret;
	struct net_interface_info event_info;
	const struct net_interface_info *info;
	struct sockaddr_in in_ip_addr, in_ip_netmask;

	info = net_interface_get_info(itf);

	/* check ip_addr argument */
	memset(&in_ip_addr, 0 , sizeof(in_ip_addr));
	in_ip_addr.sin_family = AF_INET;
	if (args->fields.ip_addr &&
	    (inet_pton(AF_INET, args->ip_addr, &in_ip_addr.sin_addr) != 1)) {
		diag("bad ip_addr argument %s", args->ip_addr);
		goto invalid_args;
	}

	/* check netmask argument */
	memset(&in_ip_netmask, 0 , sizeof(in_ip_netmask));
	in_ip_netmask.sin_family = AF_INET;
	if (args->fields.netmask &&
	    (inet_pton(AF_INET, args->netmask, &in_ip_netmask.sin_addr) != 1)) {
		diag("bad netmask argument %s", args->netmask);
		goto invalid_args;
	}

	/* send call ack status */
	obus_server_send_ack(server, handle, OBUS_CALL_ACKED);

	/* set interface up */
	ret = netif_set_up(info->name);
	if (ret < 0)
		goto failure;

	/* set interface address */
	if (args->fields.ip_addr)
		netif_set_ip_address(info->name,
				     (const struct sockaddr *)&in_ip_addr);

	/* set netmask address */
	if (args->fields.ip_addr)
		netif_set_netmask(info->name,
				  (const struct sockaddr *)&in_ip_netmask);

	/* update interface */
	update_interface(server, itf);
	return;

invalid_args:
	obus_server_send_ack(server, handle, OBUS_CALL_INVALID_ARGUMENTS);
	return;

failure:
	net_interface_info_init(&event_info);
	net_interface_send_event(server, itf, NET_INTERFACE_EVENT_UP_FAILED,
				 &event_info);
	return;
}

static void netif_call_down(struct net_interface *itf, obus_handle_t handle,
			    void *unused)
{
	int ret;
	struct net_interface_info event_info;
	const struct net_interface_info *info;

	info = net_interface_get_info(itf);

	/* send call ack status */
	obus_server_send_ack(server, handle, OBUS_CALL_ACKED);

	/* set interface down */
	ret = netif_set_down(info->name);
	if (ret < 0)
		goto failure;

	/* update interface */
	update_interface(server, itf);
	return;

failure:
	net_interface_info_init(&event_info);
	net_interface_send_event(server, itf, NET_INTERFACE_EVENT_DOWN_FAILED,
				 &event_info);
	return;
}

static struct net_interface *create_interface(const char *itfname)
{
	struct net_interface_info info;
	char ip_addr[50];
	char hw_addr[50];
	char bcast[50];
	char netmask[50];
	uint64_t bytes[2];
	int ret;

	struct net_interface_method_handlers handlers = {
		.method_up = netif_call_up,
		.method_down = netif_call_down
	};

	/* init info */
	net_interface_info_init(&info);

	/* set name */
	OBUS_SET(&info, name, itfname);

	/* read hw address */
	ret = netif_read_hw_address(itfname, hw_addr, sizeof(hw_addr));
	if (ret < 0)
		hw_addr[0] = '\0';

	OBUS_SET(&info, hw_addr, hw_addr);

	/* read ip address */
	ret = netif_read_ip_address(itfname, ip_addr, sizeof(ip_addr));
	if (ret < 0)
		ip_addr[0] = '\0';

	OBUS_SET(&info, ip_addr, ip_addr);

	/* read broadcast address */
	ret = netif_read_bcast_address(itfname, bcast, sizeof(bcast));
	if (ret < 0)
		bcast[0] = '\0';

	OBUS_SET(&info, broadcast, bcast);

	/* read netmask */
	ret = netif_read_netmask(itfname, netmask, sizeof(netmask));
	if (ret < 0)
		netmask[0] = '\0';

	OBUS_SET(&info, netmask, netmask);

	/* set traffic */
	ret = netitf_read_traffic(itfname, &bytes[0], &bytes[1]);
	if (ret == 0)
		OBUS_ARRAY_SET(&info, bytes, bytes, 2);
	else
		diag_error("netitf_read_traffic %s failed", itfname);

	/* set methods state */
	if (netif_is_up(itfname)) {
		OBUS_SET(&info, state, NET_INTERFACE_STATE_UP);

		OBUS_DISABLE_METHOD(&info, up);
		OBUS_ENABLE_METHOD(&info, down);
	} else {
		OBUS_SET(&info, state, NET_INTERFACE_STATE_DOWN);

		OBUS_DISABLE_METHOD(&info, down);
		OBUS_ENABLE_METHOD(&info, up);
	}

	return net_interface_new(server, &info, &handlers);
}

static void update_network_traffic(struct obus_server *srv)
{
	struct net_interface *itf;

	itf = net_interface_next(srv, NULL);
	while (itf) {
		/* update interface traffic */
		update_traffic(srv, itf);

		/* get next interface in list */
		itf = net_interface_next(srv, itf);
	}
}

static void scan_network(struct obus_server *srv)
{
	struct net_interface *itf, *next;
	const struct net_interface_info *info;
	size_t i, nbr_itf;
	char **itfnames;
	int ret, itf_found;

	/* get all network interfaces name present*/
	ret = netif_scan(&itfnames, &nbr_itf);
	if (ret < 0)
		return;

	/* create new interfaces object, update existing ones */
	for (i = 0; i < nbr_itf; i++) {
		/* find corresponding netmon interface given name */
		itf = get_net_interface_by_name(srv, itfnames[i]);
		if (!itf) {
			/* no interface found, create one */
			itf = create_interface(itfnames[i]);
			if (!itf)
				continue;

			/* register interface on bus */
			net_interface_register(srv, itf);
		} else {
			/* update interface  */
			update_interface(srv, itf);
		}
	}

	/* destroy interfaces that have removed from system */
	itf = net_interface_next(srv, NULL);
	while (itf) {

		info = net_interface_get_info(itf);

		/* check if interface is still present */
		itf_found = 0;
		for (i = 0; i < nbr_itf; i++) {
			if (info->name &&
			   (strcmp(info->name, itfnames[i]) == 0)) {
				itf_found = 1;
				break;
			}
		}

		/* get next interface in list */
		next = net_interface_next(srv, itf);

		/* unregister interface if not found */
		if (!itf_found) {
			net_interface_unregister(srv, itf);
			net_interface_destroy(itf);
		}

		/* iterate */
		itf = next;
	}

	/* free interfaces */
	for (i = 0; i < nbr_itf; i++)
		free(itfnames[i]);

	free(itfnames);
}

static void print_usage(void)
{
	fprintf(stderr, "netd <address1> <address2> ...\n\n");

	fprintf(stderr, "<address>: server listen socket address\n");
	fprintf(stderr, "\tipv4 address \"inet:<address>:<port>\"\n");
	fprintf(stderr, "\tipv6 address \"inet6:<address>:<port>\"\n");
	fprintf(stderr, "\tunix address \"unix:<path>\""
			"(use '@' for abstract socket)\n\n");
}

int main(int argc, char *argv[])
{
	int i, ret, stop, tfd, trafficfd;
	const char **addrs = NULL;
	size_t n_addrs;
	uint64_t u;
	struct itimerspec nval;
	struct pollfd fds[4];

	/* static const char *const addrs[] = {
		"inet6::::58001",
		"inet:0.0.0.0:58000",
		"unix:@/obus/net",
	};*/

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

	/* open pipes */
	ret = pipe(fdpipes);
	if (ret < 0) {
		ret = EXIT_FAILURE;
		goto out;
	}

	/* create timerfd used for network scan */
	tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (tfd == -1) {
		diag_errno("timerfd_create");
		ret = EXIT_FAILURE;
		goto close_pipes;
	}

	/* monitor interfaces changes every 500 ms */
	nval.it_interval.tv_sec = 0;
	nval.it_interval.tv_nsec = 1000*1000*500;
	nval.it_value.tv_sec = 0;
	nval.it_value.tv_nsec = 1000*1000*500;
	ret = timerfd_settime(tfd, 0, &nval, NULL);
	if (ret < 0) {
		diag_errno("timerfd_settime");
		ret = EXIT_FAILURE;
		goto close_tfd;
	}

	/* create timerfd used for network traffic update */
	trafficfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (trafficfd == -1) {
		diag_errno("timerfd_create");
		ret = EXIT_FAILURE;
		goto close_tfd;
	}

	/* update interfaces traffic (rx & tx bytes) every 10s */
	nval.it_interval.tv_sec = 10;
	nval.it_interval.tv_nsec = 0;
	nval.it_value.tv_sec = 10;
	nval.it_value.tv_nsec = 0;
	ret = timerfd_settime(trafficfd, 0, &nval, NULL);
	if (ret < 0) {
		diag_errno("timerfd_settime");
		ret = EXIT_FAILURE;
		goto close_trafficfd;
	}

	/* create server */
	server = obus_server_new(net_bus_desc);
	if (!server) {
		diag_error("can't create obus net server");
		ret = EXIT_FAILURE;
		goto close_trafficfd;
	}

	/* attach sig handler */
	signal(SIGINT, &sig_handler);

	/* scan network, creates obus interface objects */
	scan_network(server);

	/* start server */
	ret = obus_server_start(server, addrs, n_addrs);
	if (ret < 0) {
		diag_error("can't start obus net server");
		ret = EXIT_FAILURE;
		goto destroy_server;
	}

	/* build poll fds */
	fds[0].fd = fdpipes[0];
	fds[0].events = POLLIN;
	fds[1].fd = obus_server_fd(server);
	fds[1].events = POLLIN;
	fds[2].fd = tfd;
	fds[2].events = POLLIN;
	fds[3].fd = trafficfd;
	fds[3].events = POLLIN;

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
			obus_server_process_fd(server);

		/* scan network if timer raised */
		if (!stop && ((fds[2].revents & POLLIN) == POLLIN)) {
			/* read timer */
			do {
				ret = (int)read(tfd, &u, sizeof(u));
			} while (ret < 0 && errno == EINTR);

			/* rescan network */
			scan_network(server);
		}

		/* update traffic statistics if timer raised */
		if (!stop && ((fds[3].revents & POLLIN) == POLLIN)) {
			/* read timer */
			do {
				ret = (int)read(trafficfd, &u, sizeof(u));
			} while (ret < 0 && errno == EINTR);

			/* update traffic */
			update_network_traffic(server);
		}

	} while (!stop);

	ret = EXIT_SUCCESS;

destroy_server:
	obus_server_destroy(server);
close_trafficfd:
	close(trafficfd);
close_tfd:
	close(tfd);
close_pipes:
	close(fdpipes[0]);
	close(fdpipes[1]);
out:
	free(addrs);
	return ret;
}
