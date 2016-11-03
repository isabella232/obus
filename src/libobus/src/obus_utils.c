/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_utils.c
 *
 * @brief obus utilities
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

/* add fd O_CLOEXEC flag in fd */
int obus_fd_set_close_on_exec(int fd)
{
	int old, ret;

	old = fcntl(fd, F_GETFD, 0);
	if (old < 0)
		return -errno;

	ret = fcntl(fd, F_SETFD, FD_CLOEXEC | old);
	if (ret < 0)
		return -errno;

	return 0;
}

/* check if fd has O_CLOEXEC flag */
int obus_fd_has_close_on_exec(int fd)
{
	int flags;
	flags = fcntl(fd, F_GETFD, 0);
	if (flags < 0)
		return 0;

	return flags & FD_CLOEXEC ? 1 : 0;
}

/* add fd flags */
int obus_fd_add_flags(int fd, int flags)
{
	int old, ret;

	old = fcntl(fd, F_GETFL, 0);
	if (old < 0)
		return -errno;

	ret = fcntl(fd, F_SETFL, old | flags);
	if (ret < 0)
		return -errno;

	return 0;
}

/* duplicate fd with same flags (O_NONBLOCK ...) */
int obus_fd_dup(int fd)
{
	int newfd, ret;

	/* duplicate file descriptor */
	newfd = dup(fd);
	if (newfd < 0)
		return -errno;

	/* Duplicated file descriptors
	 * (made with dup(2), fcntl(F_DUPFD), fork(2), etc.)
	 * refer to the  same  open file description, and thus share the same
	 * file status flags */

	/* report close on exec fd flags */
	if (obus_fd_has_close_on_exec(fd)) {
		ret = obus_fd_set_close_on_exec(newfd);
		if (ret < 0) {
			close(newfd);
			return ret;
		}
	}

	return newfd;
}

/* never modify env value */
const char *obus_get_env(const char *key)
{
	return key ? getenv(key) : NULL;
}

/* never modify env value */
int obus_get_env_uint32(const char *key, uint32_t *value)
{
	const char *env;
	char *endptr;
	unsigned long int val;

	if (!key)
		return -ENOENT;

	env = getenv(key);
	if (!env || env[0] == '\0')
		return -ENOENT;

	/* try to convert string to uint32 */
	val = strtoul(env, &endptr, 10);
	if (endptr == env)
		return -ENOENT;

	*value = (uint32_t)val;
	return 0;
}

/* never modify env value */
int obus_check_env(const char *key, const char *value)
{
	const char *v;
	v = obus_get_env(key);
	if (v && value && (strcmp(v, value) == 0))
		return 1;
	else
		return 0;
}

static const char * const rand_files[] = {
	"/dev/urandom",		/* linux */
	"/system/dev/urandom",	/* android */
	NULL
};

obus_handle_t obus_rand_handle(void)
{
	int i, fd;
	ssize_t ret;
	union {
		obus_handle_t handle;
		char buffer[2];
	} r;

	/* open random file */
	fd = -1;
	for (i = 0; rand_files[i]; i++) {
		fd = open(rand_files[i], O_RDONLY);
		if (fd >= 0)
			break;
	}

	if (fd == -1) {
		obus_critical("can't open random file %s", rand_files[0]);
		return OBUS_INVALID_HANDLE;
	}

	/* read random file */
	do {
		do {
			ret = read(fd, r.buffer, sizeof(r));
		} while (ret == -1 && errno == EINTR);

		if (ret < 0) {
			obus_critical("read %s error:%d(%s)\n", rand_files[i],
				      errno, strerror(errno));
			close(fd);
			return OBUS_INVALID_HANDLE;
		}

		/* handle case where urandom return 0 */
	} while (r.handle == OBUS_INVALID_HANDLE);

	close(fd);
	return r.handle;
}

/* get log flags from env */
static int obus_bus_has_flag(const char *flag, const char *bus_name)
{
	const char *v, *start, *end, *pos;
	size_t len;

	v = obus_get_env(flag);
	if (!v)
		return 0;

	if ((strcmp(v, "all") == 0) || (strcmp(v, "1") == 0))
		return 1;

	len = strlen(bus_name);
	pos = v;
	while (1) {
		start = strstr(pos, bus_name);
		if (!start)
			break;

		end = start + len;

		if ((start == v || (*(start - 1)) == ',') &&
		    ((*end == '\0') || (*end == ',')))
			return 1;

		pos = end;
	}

	return 0;
}

/* get log flags from env */
uint32_t obus_get_log_flags_from_env(const char *bus_name)
{
	int all = 0;
	uint32_t flags = 0;

	if (obus_bus_has_flag("OBUS_LOG_ALL", bus_name))
		all = 1;

	if (all || obus_bus_has_flag("OBUS_LOG_IO", bus_name))
		flags |= OBUS_LOG_IO;

	if (all || obus_bus_has_flag("OBUS_LOG_BUS", bus_name))
		flags |= OBUS_LOG_BUS;

	if (all || obus_bus_has_flag("OBUS_LOG_SOCKET", bus_name))
		flags |= OBUS_LOG_SOCKET;

	if (all || obus_bus_has_flag("OBUS_LOG_CONNECTION", bus_name))
		flags |= OBUS_LOG_CONNECTION;

	return flags;
}
