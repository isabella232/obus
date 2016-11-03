/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_io.c
 *
 * @brief obus io
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

/* set default write ready timeout to 5s */
#define OBUS_IO_WRITE_READY_DEFAULT_TIMEOUT 5000

/* io buffer node */
struct obus_io_buffer {
	struct obus_node node;
	struct obus_buffer *buf;
};

/* obus io */
struct obus_io {
	int lograw;
	char *name;
	struct obus_loop *loop;
	struct obus_fd read_fd;
	struct obus_fd write_fd;
	obus_io_read_event_cb_t read_cb;
	obus_io_write_cb_t write_cb;
	struct obus_buffer *current;
	size_t nbr_written;
	struct obus_node write_buffers;
	struct obus_timer *write_timer;
	int write_ready_timeout;
	void *user_data;
};

static void obus_io_buffer_destroy(struct obus_io_buffer *iobuf)
{
	free(iobuf);
};

static struct obus_io_buffer *obus_io_buffer_new(struct obus_buffer *buf)
{
	struct obus_io_buffer *iobuf;

	iobuf = calloc(1, sizeof(*iobuf));
	if (iobuf) {
		/* increment buffer refcounting */
		iobuf->buf = obus_buffer_ref(buf);
	}
	return iobuf;
};

ssize_t obus_io_read(struct obus_io *io, struct obus_buffer *buf, size_t size)
{
	int fd, ret;
	void *ptr;
	size_t length;
	ssize_t nbytes;

	if (!io || !buf || size == 0)
		return -EINVAL;

	fd = io->read_fd.fd;
	ret = obus_buffer_ensure_write_space(buf, size);
	if (ret < 0)
		return ret;

	/* get write ptr */
	ptr = obus_buffer_write_ptr(buf);

	/* read without blocking */
	do {
		nbytes = read(fd, ptr, size);
	} while (nbytes == -1 && errno == EINTR);

	if (nbytes == -1) {
		nbytes = -errno;
		/*read error */
		if (errno != EAGAIN)
			obus_log_fd_errno("read", fd);

	} else if (nbytes == 0) {
		/* read end of file */
		obus_debug("%s fd=%d, read EOF", io->name, fd);
	} else {
		length = (size_t)nbytes;
		obus_buffer_inc_write_ptr(buf, length);
		if (io->lograw)
			obus_log_raw(OBUS_LOG_DEBUG, ptr, length,
				     "%s read fd=%d length=%zu", io->name, fd,
				     length);
	}

	return nbytes;
}

static void obus_io_read_events(struct obus_fd *fd, int events, void *data)
{
	struct obus_io *io = (struct obus_io *)data;
	(*io->read_cb) (events, io->user_data);
}

static void obus_io_write_buffer_complete(struct obus_io *io,
					  enum obus_io_status status)
{
	struct obus_node *first;
	struct obus_buffer *buf;
	struct obus_io_buffer *iobuf;

	if (obus_list_is_empty(&io->write_buffers))
		return;

	/* get first buffer from list */
	first = obus_list_first(&io->write_buffers);
	iobuf = obus_list_entry(first, struct obus_io_buffer, node);

	/* decrement buffer refcnt */
	buf = iobuf->buf;
	obus_buffer_unref(buf);

	/* remove buffer from list*/
	obus_list_del(&iobuf->node);
	obus_io_buffer_destroy(iobuf);

	/* clear written bytes */
	io->nbr_written = 0;

	/* call write complete callback */
	(*io->write_cb) (status, buf, io->user_data);
}

static void obus_io_write_timer(struct obus_timer *timer,
				uint64_t *nbexpired, void *data)
{
	struct obus_io *io = (struct obus_io *)data;

	/* clear timer */
	obus_timer_clear(io->write_timer);

	/* complete write buffer */
	obus_io_write_buffer_complete(io, OBUS_IO_TIMEOUT);
}

static int obus_io_write_buffer(struct obus_io *io, struct obus_buffer *buf,
				size_t offset, size_t *nbr_written)
{
	int ret, fd;
	uint8_t *base;
	size_t size, length;
	ssize_t nbytes;

	if (!io || !buf || !nbr_written)
		return -EINVAL;

	fd = io->write_fd.fd;
	size = obus_buffer_length(buf) - offset;
	base = obus_buffer_ptr(buf) + offset;
	*nbr_written = 0;

	while (size > 0) {
		/* write without blocking */
		do {
			nbytes = write(fd, base, size);
		} while (nbytes == -1 && errno == EINTR);

		if (nbytes < 0) {
			ret = -errno;
			/* log error */
			if (errno != EAGAIN)
				obus_log_fd_errno("write", fd);

			/* return -errno */
			return ret;
		}

		length = (size_t)nbytes;
		if (io->lograw)
			obus_log_raw(OBUS_LOG_DEBUG, base, length,
				     "%s write fd=%d length=%zu", io->name, fd,
				     length);

		base += length;
		size -= length;
		*nbr_written += length;
	}

	return 0;
}

static void obus_io_write_events(struct obus_fd *fd, int events, void *data)
{
	struct obus_io *io = (struct obus_io *)data;
	struct obus_node *first;
	struct obus_io_buffer *iobuf;
	int ret;
	size_t nbr_written;

	/* clear write timer */
	obus_timer_clear(io->write_timer);

	if (obus_fd_event_error(events)) {
		/* complete write buffer */
		obus_io_write_buffer_complete(io, OBUS_IO_ERROR);
		return;
	}

	/* do not treat event other than write available */
	if (!obus_fd_event_write(events))
		return;

	/* write pending buffers */
	while (!obus_list_is_empty(&io->write_buffers)) {
		/* get first buffer from list */
		first = obus_list_first(&io->write_buffers);
		iobuf = obus_list_entry(first, struct obus_io_buffer, node);

		/* write buffer */
		ret = obus_io_write_buffer(io, iobuf->buf, io->nbr_written,
					   &nbr_written);
		if (ret == -EAGAIN) {
			/* if buffer has been partially written write number
			 * of bytes already written */
			io->nbr_written += nbr_written;
			break;
		}

		/* complete write buffer */
		obus_io_write_buffer_complete(io, (ret == 0) ? OBUS_IO_OK :
					      OBUS_IO_ERROR);
	}

	/* if write buffer list is non empty trigger timer */
	if (!obus_list_is_empty(&io->write_buffers)) {
		/* write block, trigger write ready timer */
		obus_timer_set(io->write_timer, io->write_ready_timeout);
	} else {
		/* no pending write buffers */

		/* remove wait write ready event */
		obus_loop_remove(io->loop, &io->write_fd);

		/* log exit async mode */
		obus_debug("io %s fd=%d write exit async mode",
			   io->name, io->write_fd.fd);
	}
}

struct obus_io *obus_io_new(struct obus_loop *loop, const char *name, int fd,
			    obus_io_write_cb_t write_cb,
			    obus_io_read_event_cb_t read_cb,
			    void *user_data)
{
	struct obus_io *io;
	int ret;

	if (!loop || (fd < 0) || !write_cb || !read_cb)
		return NULL;

	io = calloc(1, sizeof(*io));
	if (!io)
		return NULL;

	io->read_cb = read_cb;
	io->write_cb = write_cb;
	io->user_data = user_data;
	io->loop = obus_loop_ref(loop);
	io->name = strdup(name ? name : "???");
	obus_list_init(&io->write_buffers);
	io->write_ready_timeout = OBUS_IO_WRITE_READY_DEFAULT_TIMEOUT;
	io->write_timer = obus_timer_new(io->loop, &obus_io_write_timer, io);
	if (!io->write_timer)
		goto unref_loop;

	/* add read fd in loop */
	obus_fd_init(&io->read_fd, fd, OBUS_FD_IN, &obus_io_read_events, io);
	ret = obus_loop_add(io->loop, &io->read_fd);
	if (ret < 0)
		goto destroy_timer;

	/* prepare write fd */
	obus_fd_init(&io->write_fd, fd, OBUS_FD_OUT, &obus_io_write_events, io);
	return io;

destroy_timer:
	obus_timer_destroy(io->write_timer);

unref_loop:
	obus_loop_unref(loop);
	free(io->name);
	free(io);
	return NULL;
}

int obus_io_destroy(struct obus_io *io)
{
	struct obus_io_buffer *iobuf, *tmp;

	if (!io)
		return -EINVAL;

	/* remove fd read from loop */
	obus_loop_remove(io->loop, &io->read_fd);

	/* remove fd write from loop*/
	if (obus_fd_is_used(&io->write_fd))
		obus_loop_remove(io->loop, &io->write_fd);

	/* give back pending non written buffer */
	obus_list_walk_entry_forward_safe(&io->write_buffers,
					  iobuf, tmp, node) {
		/* unref buffer */
		obus_buffer_unref(iobuf->buf);

		/* destroy io buf wrapper */
		obus_list_del(&iobuf->node);
		obus_io_buffer_destroy(iobuf);
	}

	/* destroy timer */
	obus_timer_destroy(io->write_timer);

	/* unref loop */
	obus_loop_unref(io->loop);
	free(io->name);
	free(io);
	return 0;
}

int obus_io_log_traffic(struct obus_io *io, int enable)
{
	if (!io)
		return -EINVAL;

	io->lograw = enable ? 1 : 0;
	return 0;
}

int obus_io_write(struct obus_io *io, struct obus_buffer *buf)
{
	struct obus_io_buffer *iobuf;
	size_t nbr_written;
	int ret;

	if (!io || !buf)
		return -EINVAL;

	/* add buffer in pending write buffers list for async write */
	if (!obus_list_is_empty(&io->write_buffers)) {
		/* create io buf wrapper */
		iobuf = obus_io_buffer_new(buf);
		if (!iobuf)
			return -ENOMEM;

		/* add io buf in pending list */
		obus_list_add_before(&io->write_buffers, &iobuf->node);
		return -EAGAIN;
	}

	/*  if write buffer list is empty try to write synchrously */
	ret = obus_io_write_buffer(io, buf, 0, &nbr_written);
	if (ret == -EAGAIN) {
		/* create io buf wrapper */
		iobuf = obus_io_buffer_new(buf);
		if (!iobuf)
			return -ENOMEM;

		/* log async write */
		obus_debug("io %s fd=%d write enter async mode",
			   io->name, io->write_fd.fd);

		/* write sync failed wait write ready event */
		(void)obus_loop_add(io->loop, &io->write_fd);

		/* trigger write ready timer */
		obus_timer_set(io->write_timer, io->write_ready_timeout);

		/* add io buf in pending list */
		obus_list_add_before(&io->write_buffers, &iobuf->node);

		/* if buffer has been partially written write number of bytes
		 * already written */
		io->nbr_written = nbr_written;
	}

	return ret;
}
