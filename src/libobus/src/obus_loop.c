/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_loop.c
 *
 * @brief obus fd event loop
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

#ifdef HAVE_EPOLL

/* extra number of event allocated when events array growth  */
#define EXTRA_ALLOC_EVENT_NB 10

/* event loop object */
struct obus_loop {
	struct obus_node ofds;		/* registered fd object list */
	struct obus_log *log;	/* log context */
	struct epoll_event *events;	/* epoll events array */
	int nallocevents;		/* number of allocated events */
	int nfds;			/* number of fds */
	size_t ref;			/* loop reference count */
	int epfd;			/* epoll fd */
};

static uint32_t fd_to_epoll_events(int events)
{
	uint32_t ret = 0;
	if (events & OBUS_FD_IN)
		ret |= EPOLLIN;

	if (events & OBUS_FD_OUT)
		ret |= EPOLLOUT;

	if (events & OBUS_FD_ERR)
		ret |= EPOLLERR;

	if (events & OBUS_FD_HUP)
		ret |= EPOLLHUP;

	return ret;
}

static int epoll_to_fd_events(uint32_t events)
{
	int ret = 0;
	if ((events & EPOLLIN) || (events & EPOLLPRI))
		ret |= OBUS_FD_IN;

	if (events & EPOLLOUT)
		ret |= OBUS_FD_OUT;

	if (events & EPOLLERR)
		ret |= OBUS_FD_ERR;

	if (events & EPOLLHUP)
		ret |= OBUS_FD_HUP;
#ifdef EPOLLRDHUP
	if (events & EPOLLRDHUP)
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

	/* create epoll fd
	 * Since Linux 2.6.8, the size argument is unused,
	 * but must be greater than zero.
	 * (The kernel dynamically sizes the required data structures
	 * without needing this initial hint.) */
	loop->epfd = epoll_create(1);
	if (loop->epfd < 0) {
		obus_log_errno("epoll_create");
		goto destroy;
	}

	/* set epoll fd close on exec flag */
	ret = obus_fd_set_close_on_exec(loop->epfd);
	if (ret < 0)
		goto close;

	/* set epoll fd non blocking */
	ret = obus_fd_add_flags(loop->epfd, O_NONBLOCK);
	if (ret < 0)
		goto close;

	/* init list of fd objects*/
	obus_list_init(&loop->ofds);

	loop->ref = 1;
	loop->nfds = 0;
	loop->nallocevents = 0;
	return loop;
close:
	close(loop->epfd);
destroy:
	free(loop);
error:
	return NULL;
}

static int obus_loop_destroy(struct obus_loop *loop)
{
	/* all fd objects should have been removed */
	if (!obus_list_is_empty(&loop->ofds)) {
		obus_critical("%s: %zu fds remained in loop", __func__,
			      obus_list_length(&loop->ofds));
	}

	close(loop->epfd);
	free(loop->events);
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
	return loop ? loop->epfd : -1;
}

/* do process loop */
static int obus_loop_do_process(struct obus_loop *loop, int timeout)
{
	struct epoll_event *ep_events;
	struct obus_fd *current, *ofd;
	int events;
	size_t size;
	int ret, j, i;

	/* grow up poll events array if needed */
	if (loop->nfds > loop->nallocevents) {
		size = (EXTRA_ALLOC_EVENT_NB + (size_t)loop->nfds) *
						sizeof(struct epoll_event);
		ep_events = realloc(loop->events, size);
		if (!ep_events)
			return -ENOMEM;

		loop->events = ep_events;
		loop->nallocevents = EXTRA_ALLOC_EVENT_NB + loop->nfds;
	}

	/* wait without blocking */
	do {
		ret = epoll_wait(loop->epfd, loop->events, loop->nfds, timeout);
	} while (ret < 0 && errno == EINTR);


	if (ret < 0) {
		obus_log_fd_errno("epoll_wait", loop->epfd);
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
		if (!loop->events[i].events)
			continue;

		events = epoll_to_fd_events(loop->events[i].events);
		j++;
		ofd = NULL;
		if (!events)
			continue;

		/* check if fd object is well in our list (not removed)*/
		obus_list_walk_entry_forward(&loop->ofds, current, node) {
			if (current == loop->events[i].data.ptr) {
				/* get fd object */
				ofd = loop->events[i].data.ptr;
				break;
			}
		}

		/* fd object has been removed ?*/
		if (obus_unlikely(!ofd)) {
			obus_debug("fd ready on ofd %p removed !",
				   ofd);
			continue;
		}

		/* invoke fd object callback */
		(*ofd->cb) (ofd, events, ofd->data);
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
	struct obus_fd *current;
	struct epoll_event evt;

	if (!loop || !ofd || (ofd->fd < 0) || ofd->events == 0 ||
	    !ofd->cb) {
		ret = -EINVAL;
		obus_error("invalid ofd %p", ofd);
		goto error;
	}

	if (ofd->loop) {
		ret = -EEXIST;
		obus_error("ofd %p already used", ofd);
		goto error;
	}

	/* check fd object is not already in list */
	obus_list_walk_entry_forward(&loop->ofds, current, node) {
		if (current == ofd) {
			obus_error("ofd %p already added", ofd);
			ret = -EEXIST;
			goto error;
		}
	}

	/* init epoll event */
	memset(&evt, 0, sizeof(evt));
	evt.data.ptr = ofd;
	evt.events = fd_to_epoll_events(ofd->events);

	/* add fd in poll */
	ret = epoll_ctl(loop->epfd, EPOLL_CTL_ADD, ofd->fd, &evt);
	if (ret < 0 && errno == EEXIST) {
		/* if fd already added in epoll
		 * duplicate fd and add it */
		ret = obus_fd_dup(ofd->fd);
		if (ret < 0) {
			obus_error("obus_fd_dup: fd=%d error=%d(%s)",
				   ofd->fd, -ret, strerror(-ret));
			goto error;
		}

		ofd->dupfd = ret;
		ret = epoll_ctl(loop->epfd, EPOLL_CTL_ADD, ofd->dupfd, &evt);
	}

	if (ret < 0) {
		obus_log_fd_errno("epoll_ctl(EPOLL_CTL_ADD)",
				  ofd->dupfd == -1 ? ofd->fd : ofd->dupfd);
		/* close duplicate fd if exist */
		if (ofd->dupfd != -1) {
			close(ofd->dupfd);
			ofd->dupfd = -1;
		}

		ret = -errno;
		goto error;
	}

	/* add fd object in list */
	obus_list_add_before(&loop->ofds, &ofd->node);
	ofd->loop = loop;
	loop->nfds++;
	return 0;

error:
	return ret;
}

/* update fd in loop */
int obus_loop_update(struct obus_loop *loop, struct obus_fd *ofd)
{
	int ret;
	struct obus_fd *current;
	struct epoll_event evt;

	if (!loop || !ofd) {
		ret = -EINVAL;
		obus_error("invalid ofd %p", ofd);
		goto error;
	}

	/* check fd object is in our list */
	ret = -ENOENT;
	obus_list_walk_entry_forward(&loop->ofds, current, node) {
		if (current == ofd) {
			ret = 0;
			break;
		}
	}

	if (ret == -ENOENT) {
		obus_error("update a ofd %p not added", ofd);
		goto error;
	}

	/* init epoll event */
	memset(&evt, 0, sizeof(evt));
	evt.data.ptr = ofd;
	evt.events = fd_to_epoll_events(ofd->events);

	/* update fd object from poll */
	ret = epoll_ctl(loop->epfd, EPOLL_CTL_MOD,
			ofd->dupfd == -1 ? ofd->fd : ofd->dupfd, NULL);
	if (ret < 0) {
		obus_log_fd_errno("epoll_ctl(EPOLL_CTL_MOD)",
				  loop->epfd);
		ret = -errno;
		goto error;
	}
	return 0;

error:
	return ret;
}

int obus_loop_remove(struct obus_loop *loop, struct obus_fd *ofd)
{
	int ret;
	struct obus_fd *current;

	if (!loop || !ofd) {
		ret = -EINVAL;
		obus_error("invalid ofd %p", ofd);
		goto error;
	}

	/* check fd object is in our list */
	ret = -ENOENT;
	obus_list_walk_entry_forward(&loop->ofds, current, node) {
		if (current == ofd) {
			ret = 0;
			break;
		}
	}

	if (ret == -ENOENT) {
		obus_error("remove a ofd %p not added", ofd);
		goto error;
	}

	/* remove fd object from poll */
	ret = epoll_ctl(loop->epfd, EPOLL_CTL_DEL,
			ofd->dupfd == -1 ? ofd->fd : ofd->dupfd, NULL);
	if (ret < 0) {
		ret = -errno;
		obus_log_fd_errno("epoll_ctl(EPOLL_CTL_DEL)",
				  ofd->dupfd == -1 ? ofd->fd : ofd->dupfd);
		goto error;
	}

	/* close duplicate fd if exist */
	if (ofd->dupfd != -1) {
		close(ofd->dupfd);
		ofd->dupfd = -1;
	}

	/* remove fd object from list */
	obus_list_del(&ofd->node);
	ofd->loop = NULL;
	loop->nfds--;
	return 0;
error:
	return ret;
}

int obus_loop_posix_get_fd_count(struct obus_loop *loop)
{
	/* not supported by the epoll implementation */
	return -ENOSYS;
}

int obus_loop_posix_get_fds(struct obus_loop *loop, struct pollfd *pfds,
			    int size)
{
	/* not supported by the epoll implementation */
	return -ENOSYS;
}

int obus_loop_posix_process_fd(struct obus_loop *loop, const struct pollfd *pfd)
{
	/* not supported by the epoll implementation */
	return -ENOSYS;
}

#endif /* HAVE_EPOLL */
