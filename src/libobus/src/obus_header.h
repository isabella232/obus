/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_header.h
 *
 * @brief brief obus private header
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

#ifndef _OBUS_HEADER_H_
#define _OBUS_HEADER_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112
#endif

#define GCC_VERSION (__GNUC__ * 10000 \
		+ __GNUC_MINOR__ * 100 \
		+ __GNUC_PATCHLEVEL__)

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* C headers */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#ifdef HAVE_EPOLL
#include <sys/epoll.h>
#endif /* HAVE_EPOLL */
#include <sys/poll.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

/* gcc likely support */
#define obus_likely(x)		__builtin_expect(!!(x), 1)
#define obus_unlikely(x)	__builtin_expect(!!(x), 0)

/* macro used to declare public api */
#define OBUS_API __attribute__((visibility("default")))
#define OBUS_USE_PRIVATE

/* obus headers */
#include "libobus.h"
#include "libobus_private.h"
#include "obus_log.h"
#include "obus_platform.h"
#include "obus_list.h"
#include "obus_buffer.h"
#include "obus_io.h"
#include "obus_hash.h"
#include "obus_utils.h"
#include "obus_loop.h"
#include "obus_socket.h"
#include "obus_timer.h"
#include "obus_struct.h"
#include "obus_field.h"
#include "obus_packet.h"
#include "obus_bus_api.h"
#include "obus_bus_event.h"
#include "obus_object.h"
#include "obus_event.h"
#include "obus_call.h"
#include "obus_bus.h"

#endif /* _OBUS_HEADER_H_ */
