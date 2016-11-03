/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_loop_posix.c
 *
 * @brief obus fd event loop with posix implementation
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

#ifndef HAVE_EPOLL

/* hash size for object fds */
#define OBUS_LOOP_HASH_SIZE 31

/* extra number of event allocated when events array growth  */
#define EXTRA_ALLOC_EVENT_NB 10

/* event loop object */
struct obus_loop {
	struct obus_hash ofds;	/* registered fd hash (key=fd data=obus_fd) */
	struct pollfd *pfds;	/* poll array */
	int nallocevents;	/* number of allocated events */
	int nfds;		/* number of fds */
	size_t ref;		/* loop reference count */

};

static short int fd_to_poll_events(int events)
{
	short int ret = 0;
	if (events & OBUS_FD_IN)
		ret |= POLLIN;

	if (events & OBUS_FD_OUT)
		ret |= POLLOUT;

	if (events & OBUS_FD_ERR)
		ret |= POLLERR;

	if (events & OBUS_FD_HUP)
		ret |= POLLHUP;

	return ret;
}

static int poll_to_fd_events(short int events)
{
	int ret = 0;
	if ((events & POLLIN) || (events & POLLPRI))
		ret |= OBUS_FD_IN;

	if (events & POLLOUT)
		ret |= OBUS_FD_OUT;

	if (events & POLLERR)
		ret |= OBUS_FD_ERR;

	if (events & POLLHUP)
		ret |= OBUS_FD_HUP;
#ifdef POLLRDHUP
	if (events & POLLRDHUP)
		ret |= OBUS_FD_HUP;
#endif
	return ret;
}

/* create event loop */
struct obus_loop *obus_loop_new(void)
{
	struct obus_loop *loop;
	int ret;

	/* allocate loop struct */
	loop = calloc(1, sizeof(struct obus_loop));
	if (!loop)
		goto error;

	/* init hash of fd objects*/
	ret = obus_hash_init(&loop->ofds, OBUS_LOOP_HASH_SIZE);
	if (ret < 0)
		goto error;

	loop->ref = 1;
	loop->nfds = 0;
	loop->nallocevents = 0;
	return loop;
error:
	return NULL;
}

static int obus_loop_destroy(struct obus_loop *loop)
{
	if (!loop)
		return -EINVAL;

	/* check all fd objects have been removed */
	if (!obus_list_is_empty(&loop->ofds.entries)) {
		obus_critical("%s: %zu fds remained in loop", __func__,
			      obus_list_length(&loop->ofds.entries));
	}

	obus_hash_destroy(&loop->ofds);
	free(loop->pfds);
	free(loop);
	return 0;
}

struct obus_loop *obus_loop_ref(struct obus_loop *loop)
{
	loop->ref++;
	return loop;
}

int obus_loop_unref(struct obus_loop *loop)
{
	loop->ref--;
	if (loop->ref == 0)
		return obus_loop_destroy(loop);

	return 0;
}

/* get loop fd */
int obus_loop_fd(struct obus_loop *loop)
{
	/* not supported by the posix implementation */
	return -ENOSYS;
}

/* do process loop */
static int obus_loop_do_process(struct obus_loop *loop, int timeout)
{
	struct pollfd *pfds;
	int revents;
	size_t size;
	int ret, j, i;

	/* grow up poll array if needed */
	if (loop->nfds > loop->nallocevents) {
		size = (EXTRA_ALLOC_EVENT_NB + (size_t)loop->nfds) *
						sizeof(struct pollfd);
		pfds = realloc(loop->pfds, size);
		if (!pfds)
			return -ENOMEM;

		loop->pfds = pfds;
		loop->nallocevents = EXTRA_ALLOC_EVENT_NB + loop->nfds;
	}

	/* fill poll array */
	ret = obus_loop_posix_get_fds(loop, loop->pfds, loop->nfds);
	if (ret < 0)
		return ret;

	/* wait without blocking */
	do {
		ret = poll(loop->pfds, (nfds_t)loop->nfds, timeout);
	} while (ret < 0 && errno == EINTR);


	if (ret < 0) {
		obus_log_errno("poll");
		return -errno;
	} else if (ret == 0) {
		obus_warn("%s", "no fd ready for I/O request !");
		return 0;
	}

	/* avoid loop to be destroyed in fd object callback */
	obus_loop_ref(loop);

	/* iterate on all events */
	j = 0;
	for (i = 0; i < loop->nfds && j < ret; i++) {
		/* check if fd is ready for I/O request */
		if (!loop->pfds[i].revents)
			continue;

		revents = poll_to_fd_events(loop->pfds[i].revents);
		j++;
		if (revents)
			obus_loop_posix_process_fd(loop, &loop->pfds[i]);
	}

	obus_loop_unref(loop);
	return 0;
}

/* process loop fd */
int obus_loop_process(struct obus_loop *loop)
{
	/* process event without blocking*/
	return obus_loop_do_process(loop, 0);
}

/* wait loop event fd */
int obus_loop_wait(struct obus_loop *loop)
{
	/* process event with infinite blocking*/
	return obus_loop_do_process(loop, -1);
}

int obus_loop_add(struct obus_loop *loop, struct obus_fd *ofd)
{
	int ret;
	struct obus_fd *ofd_check;

	if (!loop || !ofd || (ofd->fd < 0) || (ofd->events == 0) || !ofd->cb) {
		ret = -EINVAL;
		obus_error("invalid ofd %p", ofd);
		goto error;
	}

	if (ofd->loop) {
		ret = -EEXIST;
		obus_error("ofd %p already used", ofd);
		goto error;
	}

	/* check if fd is already in hash */
	ofd_check = NULL;
	ret = obus_hash_lookup(&loop->ofds, (uint32_t)ofd->fd,
			       (void **)&ofd_check);
	if (ret == 0) {
		/* fd already added in hash, duplicate fd and add it */
		ret = obus_fd_dup(ofd->fd);
		if (ret < 0) {
			obus_error("obus_fd_dup: fd=%d error=%d(%s)",
				   ofd->fd, -ret, strerror(-ret));
			goto error;
		}
		ofd->dupfd = ret;
		ret = obus_hash_insert(&loop->ofds, (uint32_t)ofd->dupfd, ofd);
	} else {
		/* fd not in hash, add it */
		ret = obus_hash_insert(&loop->ofds, (uint32_t)ofd->fd, ofd);
	}

	/* handle error and close duplicate fd if exist */
	if (ret < 0) {
		if (ofd->dupfd != -1) {
			close(ofd->dupfd);
			ofd->dupfd = -1;
		}
		goto error;
	}

	/* Associate fd with loop */
	ofd->loop = loop;
	loop->nfds++;
	return 0;

error:
	return ret;
}

/* update fd in loop */
int obus_loop_update(struct obus_loop *loop, struct obus_fd *ofd)
{
	int ret, fd;
	struct obus_fd *ofd_check;

	if (!loop || !ofd) {
		ret = -EINVAL;
		obus_error("invalid ofd %p", ofd);
		goto error;
	}

	/* check fd object is in our hash */
	fd = ofd->dupfd != -1 ? ofd->dupfd : ofd->fd;
	ofd_check = NULL;
	ret = obus_hash_lookup(&loop->ofds, (uint32_t)fd, (void **)&ofd_check);
	if (ret != EEXIST || ofd_check != ofd) {
		ret = -ENOENT;
		obus_error("update a ofd %p not added", ofd);
		goto error;
	}

	/* Nothing more to do, wait for next call to obus_loop_posix_get_fds */
	return 0;

error:
	return ret;
}

int obus_loop_remove(struct obus_loop *loop, struct obus_fd *ofd)
{
	int ret, fd;

	if (!loop || !ofd) {
		ret = -EINVAL;
		obus_error("invalid ofd %p", ofd);
		goto error;
	}

	/* remove fd object from hash */
	fd = ofd->dupfd != -1 ? ofd->dupfd : ofd->fd;
	ret = obus_hash_remove(&loop->ofds, (uint32_t)fd);
	if (ret < 0) {
		obus_error("remove a ofd %p not added", ofd);
		goto error;
	}

	/* close duplicate fd if exist */
	if (ofd->dupfd != -1) {
		close(ofd->dupfd);
		ofd->dupfd = -1;
	}

	/* remove association */
	ofd->loop = NULL;
	loop->nfds--;
	return 0;

error:
	return ret;
}

int obus_loop_posix_get_fd_count(struct obus_loop *loop)
{
	if (!loop)
		return -EINVAL;
	return loop->nfds;
}

int obus_loop_posix_get_fds(struct obus_loop *loop,
			    struct pollfd *pfds, int size)
{
	int i = 0;
	struct obus_hash_entry *entry;
	struct obus_fd *ofd;

	if (!loop || !pfds || !size)
		return -EINVAL;

	/* walk fd object in hash */
	obus_list_walk_entry_forward(&loop->ofds.entries, entry, node) {
		ofd = (struct obus_fd *)entry->data;
		if (i < size) {
			pfds[i].fd = (int)entry->key;
			pfds[i].events = fd_to_poll_events(ofd->events);
			pfds[i].revents = 0;
			i++;
		}
	}

	/* return number of entries added */
	return i;

}

int obus_loop_posix_process_fd(struct obus_loop *loop, const struct pollfd *pfd)
{
	struct obus_fd *ofd;
	int ret;

	if (!loop || !pfd)
		return -EINVAL;

	/* nothing to to if no event set */
	if (pfd->revents == 0)
		return 0;

	/* get fd object*/
	ofd = NULL;
	ret = obus_hash_lookup(&loop->ofds, (uint32_t)pfd->fd, (void **)&ofd);
	if (ret < 0) {
		return ret;
	}

	/* invoke fd object callback */
	(*ofd->cb) (ofd, poll_to_fd_events(pfd->revents), ofd->data);

	return 0;
}

#endif /* !HAVE_EPOLL */
