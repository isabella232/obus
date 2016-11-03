/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_io.h
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

#ifndef _OBUS_IO_H_
#define _OBUS_IO_H_

/**
 * obus io
 */
struct obus_io;
struct obus_loop;
struct obus_buffer;

/**
 * obus io operation status
 **/
enum obus_io_status {
	/* no error */
	OBUS_IO_OK,
	/* read end of file */
	OBUS_IO_EOF,
	/* write / read error */
	OBUS_IO_ERROR,
	/* operation timeout */
	OBUS_IO_TIMEOUT,
	/* operation aborted */
	OBUS_IO_ABORT,
};

/**
 * obus io callback invoked when async io write operation  is completed
 * @param buffer
 * @param user_data
 */
typedef void (*obus_io_write_cb_t) (enum obus_io_status status,
				    struct obus_buffer *buf,
				    void *user_data);
/**
 * obus read event callback
 * @param events (read event)
 * @param user_data
 */
typedef void (*obus_io_read_event_cb_t) (int events, void *user_data);

/**
 * create a new io from fd
 * @param loop obus file descriptor loop
 * @param name io name (label)
 * @param fd io file descriptor used to read/write
 * @param write_cb io write async callback
 * @param user_data callback user_data
 */
struct obus_io *obus_io_new(struct obus_loop *loop, const char *name, int fd,
			    obus_io_write_cb_t write_cb,
			    obus_io_read_event_cb_t read_cb,
			    void *user_data);

/**
 * destroy io
 * @param io
 * @return
 */
int obus_io_destroy(struct obus_io *io);

/**
 * read buffer on io (sync or async)
 * @param io obus io
 * @param buffer buffer to be written
 * @param size max size to be read
 * @return number of bytes read or errno  negative value on error
 */
ssize_t obus_io_read(struct obus_io *io, struct obus_buffer *buf, size_t size);

/**
 * write buffer on io (sync or async)
 * @param io obus io
 * @param buffer buffer to be written
 * @return 0 if write succeed synchronously (-EAGAIN if busy)
 */
int obus_io_write(struct obus_io *io, struct obus_buffer *buf);

/**
 * enable/disable io data log traffic
 * @param io obus io
 * @param enable set 1 or 0 to enable/disable log
 * @return 0 on success
 */
int obus_io_log_traffic(struct obus_io *io, int enable);

#endif /*_OBUS_IO_H_*/
