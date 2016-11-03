/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_loop.h
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

#ifndef _OBUS_LOOP_H_
#define _OBUS_LOOP_H_

/* fd object */
struct obus_fd;

/* event fd loop object */
struct obus_loop;

/* fd events callback */
typedef void (*obus_fd_event_cb_t) (struct obus_fd *fd, int events, void *data);

/* fd struct */
struct obus_fd {
	struct obus_node node;
	struct obus_loop *loop;
	int dupfd;
	int fd;
	int events;
	obus_fd_event_cb_t cb;
	void *data;
};

/* fd event */
enum obus_fd_event {
	OBUS_FD_IN = (1 << 0),
	OBUS_FD_OUT = (1 << 1),
	OBUS_FD_ERR = (1 << 2),
	OBUS_FD_HUP = (1 << 3),
};

/* create fd struct */
static inline
int obus_fd_init(struct obus_fd *ofd, int fd, int events,
		 obus_fd_event_cb_t cb, void *data)
{
	if (!ofd)
		return -EINVAL;

	ofd->loop = NULL;
	ofd->events = events;
	ofd->cb = cb;
	ofd->data = data;
	ofd->fd = fd;
	ofd->dupfd = -1;
	return 0;
}

/* check if fd is used (added in fd loop) */
static inline
int obus_fd_is_used(struct obus_fd *ofd)
{
	return ofd && ofd->loop ? 1 : 0;
}

/* reset fd struct */
static inline
int obus_fd_reset(struct obus_fd *ofd)
{
	if (!ofd)
		return -EINVAL;

	/* check fd struct is not used */
	if (ofd->loop)
		return -EBUSY;

	return obus_fd_init(ofd, -1, 0, NULL, NULL);
}

/* create event fd loop */
struct obus_loop *obus_loop_new(void);

/* add reference loop */
struct obus_loop *obus_loop_ref(struct obus_loop *loop);

/* unref loop */
int obus_loop_unref(struct obus_loop *loop);

/* get loop fd */
int obus_loop_fd(struct obus_loop *loop);

/* process loop fd */
int obus_loop_process(struct obus_loop *loop);

/* posix get fd count */
int obus_loop_posix_get_fd_count(struct obus_loop *loop);

/* posix get fds */
int obus_loop_posix_get_fds(struct obus_loop *loop, struct pollfd *pfds,
			    int size);

/* posix process fd */
int obus_loop_posix_process_fd(struct obus_loop *loop,
			       const struct pollfd *pfd);

/* wait for next event fd */
int obus_loop_wait(struct obus_loop *loop);

/* add fd in loop */
int obus_loop_add(struct obus_loop *loop, struct obus_fd *ofd);

/* update fd in loop */
int obus_loop_update(struct obus_loop *loop, struct obus_fd *ofd);

/* remove fd from loop */
int obus_loop_remove(struct obus_loop *loop,  struct obus_fd *ofd);

/* check if fd has error event */
#define obus_fd_event_error(events)			\
	(obus_unlikely(((events) & OBUS_FD_ERR) || ((events) & OBUS_FD_HUP)))

/* check if fd is ready for read operation */
#define obus_fd_event_read(events)			\
	((events) & OBUS_FD_IN)

/* check if fd is ready for write operation */
#define obus_fd_event_write(events)			\
	((events) & OBUS_FD_OUT)

/* log FD events */
#define obus_fd_event_log(ofd, events, level)			\
	obus_log((level), "fd=%d, cb=%p, events=%s%s%s%s",	\
		(ofd)->dupfd == -1 ? (ofd)->fd : (ofd)->dupfd,	\
		(ofd)->cb,					\
		((events) & OBUS_FD_IN) ? "OBUS_FD_IN|" : "",	\
		((events) & OBUS_FD_OUT) ? "OBUS_FD_OUT|" : "",	\
		((events) & OBUS_FD_ERR) ? "OBUS_FD_ERR|" : "",	\
		((events) & OBUS_FD_HUP) ? "OBUS_FD_HUP|" : "")

#endif /* _OBUS_LOOP_H_ */
