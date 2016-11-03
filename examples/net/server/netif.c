/******************************************************************************
 * @file netif.c
 *
 * @brief network interface utility
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
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>
#include <libobus.h>
#include <ifaddrs.h>
#include "netif.h"
#include "log.h"

static ssize_t netif_read_file(const char *file, char *buffer, size_t size)
{
	int fd;
	ssize_t ret;
	size_t nbytes;

	/* open file */
	fd = open(file, O_RDONLY);
	if (fd == -1) {
		diag_error("can't open %s error:%d(%s)\n", file, errno,
			   strerror(errno));
		return -errno;
	}

	/* read file */
	ret = read(fd, buffer, size);
	if (ret < 0) {
		ret = -errno;
		diag_error("read %s error:%d(%s)\n", file, errno,
			   strerror(errno));
		close(fd);
		return 0;
	}

	nbytes = (size_t)ret;
	if (nbytes >= size)
		nbytes = size - 1;

	buffer[nbytes] = '\0';
	close(fd);
	return ret;
}

static void get_in_addr(struct sockaddr *sa, char *buffer, size_t size)
{
	union {
		struct sockaddr     *sa;
		struct sockaddr_in  *sa_in;
		struct sockaddr_in6 *sa_in6;
	} u;

	u.sa = sa;
	if (sa->sa_family == AF_INET)
		inet_ntop(AF_INET, &u.sa_in->sin_addr, buffer,
			  (socklen_t)size);
	else if (sa->sa_family == AF_INET6)
		inet_ntop(AF_INET6, &u.sa_in6->sin6_addr, buffer,
			  (socklen_t)size);
	else {
		diag_error("unknown familiy %d", sa->sa_family);
		buffer[0] = '\0';
	}
	buffer[size - 1] = '\0';
}

int netitf_read_traffic(const char *itfname, uint64_t *tx_bytes,
			  uint64_t *rx_bytes)
{
	char fname[50];
	char buffer[50];
	char *endptr;
	ssize_t ret;

	if (!itfname || !tx_bytes || !rx_bytes)
		return -EINVAL;

	/* format tx bytes file */
	snprintf(fname, sizeof(fname), "/sys/class/net/%s/statistics/tx_bytes",
		 itfname);

	/* read from file */
	ret = netif_read_file(fname, buffer, sizeof(buffer));
	if (ret < 0)
		return (int)ret;

	/* convert buffer to u64*/
	*tx_bytes = strtoull(buffer, &endptr, 10);
	if (endptr == buffer) {
		diag_error("can't parse %s\n", fname);
		*tx_bytes = 0;
	}


	/* format rx bytes file */
	snprintf(fname, sizeof(fname), "/sys/class/net/%s/statistics/rx_bytes",
		 itfname);

	/* read from file */
	ret = netif_read_file(fname, buffer, sizeof(buffer));
	if (ret < 0)
		return (int)ret;

	/* convert buffer to u64*/
	*rx_bytes = strtoull(buffer, &endptr, 10);
	if (endptr == buffer) {
		diag_error("can't parse %s\n", fname);
		*rx_bytes = 0;
	}

	return 0;
}
#define PROC_NET_DEV "/proc/net/dev"
int netif_scan(char ***itfnames, size_t *nbr_itf)
{
	char buf[512];
	int ret;
	size_t line, i, nbr_itfs;
	char **itfs, **tmp, *itfname, *end;
	FILE *fp;

	/* open file in read mode */
	fp = fopen(PROC_NET_DEV, "r");
	if (!fp) {
		diag_errno("fopen " PROC_NET_DEV " ");
		return -1;
	}

	nbr_itfs = 0;
	itfs = NULL;

	/* read by line */
	line = 0;
	while (fgets(buf, sizeof(buf), fp)) {
		line++;
		/* skip 2 first lines */
		if (line < 3)
			continue;

		/* extract interface name:    <name>: */
		itfname = buf;
		while (isspace(*itfname))
			itfname++;

		end = strchr(itfname, ':');
		if (!end)
			continue;

		*end = '\0';

		/* read iface name */
		tmp = realloc(itfs, (nbr_itfs + 1) * sizeof(char *));
		if (!tmp) {
			ret = -ENOMEM;
			goto free_itfs;
		}

		/* copy interface name */
		itfs = tmp;
		itfs[nbr_itfs] = strdup(itfname);
		if (!itfs[nbr_itfs]) {
			ret = -ENOMEM;
			goto free_itfs;
		}

		nbr_itfs++;
	}
	fclose(fp);

	*itfnames = itfs;
	*nbr_itf = nbr_itfs;
	return 0;

free_itfs:
	fclose(fp);
	for (i = 0; i < nbr_itfs; i++)
		free(itfs[i]);
	free(itfs);
	return ret;
}

int netif_is_up(const char *itfname)
{
	struct ifreq ifr;
	int fd, ret;

	if (!itfname)
		return 0;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return 0;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
		diag_errno("ioctl(SIOCGIFFLAGS)");
		close(fd);
		return 0;
	}

	ret = (ifr.ifr_flags & IFF_UP);
	close(fd);
	return ret;
}

int netif_set_up(const char *itfname)
{
	struct ifreq ifr;
	int fd, ret;

	if (!itfname)
		return -EINVAL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return -errno;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
		ret = -errno;
		diag_errno("ioctl(SIOCGIFFLAGS)");
		close(fd);
		return ret;
	}

	ifr.ifr_flags |= IFF_UP;
	if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
		ret = -errno;
		diag_errno("ioctl(SIOCSIFFLAGS)");
		close(fd);
		return ret;
	}

	close(fd);
	return 0;
}


int netif_set_down(const char *itfname)
{
	struct ifreq ifr;
	int ret, fd;

	if (!itfname)
		return -EINVAL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return -errno;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
		ret = -errno;
		diag_errno("ioctl(SIOCGIFFLAGS)");
		close(fd);
		return ret;
	}

	ifr.ifr_flags &= ~IFF_UP;
	if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
		ret = -errno;
		diag_errno("ioctl(SIOCSIFFLAGS)");
		diag_error("return %d", ret);
		close(fd);
		return ret;
	}
	close(fd);
	return 0;
}


int netif_set_netmask(const char *itfname, const struct sockaddr *addr)
{
	struct ifreq ifr;
	int fd;

	if (!itfname)
		return -EINVAL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return -errno;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	memcpy(&ifr.ifr_netmask, addr, sizeof(*addr));

	if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0) {
		diag_errno("ioctl(SIOCSIFNETMASK)");
		close(fd);
		return -errno;
	}

	close(fd);
	return 0;
}

int netif_set_ip_address(const char *itfname, const struct sockaddr *addr)
{
	struct ifreq ifr;
	int fd;

	if (!itfname)
		return -EINVAL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return -errno;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	memcpy(&ifr.ifr_addr, addr, sizeof(*addr));

	if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
		diag_errno("ioctl(SIOCSIFADDR)");
		close(fd);
		return -errno;
	}

	close(fd);
	return 0;
}

int netif_read_hw_address(const char *itfname, char *buffer, size_t size)
{
	struct ifreq ifr;
	int fd;

	if (!itfname || !buffer || size == 0)
		return -EINVAL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return -errno;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
		diag_errno("ioctl(SIOCGIFHWADDR)");
		close(fd);
		return -errno;
	}

	snprintf(buffer, size, "%02x:%02x:%02x:%02x:%02x:%02x",
	    (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[0],
	    (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[1],
	    (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[2],
	    (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[3],
	    (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[4],
	    (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[5]);

	close(fd);
	return 0;
}

int netif_read_ip_address(const char *itfname, char *buffer, size_t size)
{
	struct ifreq ifr;
	int fd;

	if (!itfname || !buffer || size == 0)
		return -EINVAL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return -errno;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		if (errno != EADDRNOTAVAIL)
			diag_errno("ioctl(SIOCGIFADDR)");

		close(fd);
		return -errno;
	}

	get_in_addr(&ifr.ifr_addr, buffer, size);
	close(fd);
	return 0;
}

int netif_read_netmask(const char *itfname, char *buffer, size_t size)
{
	struct ifreq ifr;
	int fd;

	if (!itfname || !buffer || size == 0)
		return -EINVAL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return -errno;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
		if (errno != EADDRNOTAVAIL)
			diag_errno("ioctl(SIOCGIFNETMASK)");

		close(fd);
		return -errno;
	}

	get_in_addr(&ifr.ifr_netmask, buffer, size);
	close(fd);
	return 0;
}

int netif_read_bcast_address(const char *itfname, char *buffer, size_t size)
{
	struct ifreq ifr;
	int fd;

	if (!itfname || !buffer || size == 0)
		return -EINVAL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return -errno;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(fd, SIOCGIFBRDADDR, &ifr) < 0) {
		if (errno != EADDRNOTAVAIL)
			diag_errno("ioctl(SIOCGIFBRDADDR)");

		close(fd);
		return -errno;
	}

	get_in_addr(&ifr.ifr_broadaddr, buffer, size);
	close(fd);
	return 0;
}


int netif_read_mtu(const char *itfname, uint32_t *mtu)
{
	struct ifreq ifr;
	int fd;

	if (!itfname || !mtu)
		return -EINVAL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag_errno("socket");
		return -errno;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, itfname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(fd, SIOCGIFMTU, &ifr) < 0) {
		diag_errno("ioctl(SIOCGIFMTU)");
		close(fd);
		return -errno;
	}

	*mtu = (uint32_t)ifr.ifr_mtu;
	close(fd);
	return 0;
}

