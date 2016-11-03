/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_log.h
 *
 * @brief obus log api
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

#ifndef _OBUS_LOG_H_
#define _OBUS_LOG_H_

/**
 * obus log raw function
 *
 * @param level log level (see @enum obus_log_level)
 * @param buffer buffer to be logged
 * @param size buffer size to be logged
 * @param fmt format log before buffer (like printf !)
 */
void obus_log_raw(enum obus_log_level level, const void *buffer, size_t length,
		  const char *fmt, ...)
		  __attribute__ ((format (printf, 4, 5)));

#define obus_critical(fmt, ...)	\
	obus_log(OBUS_LOG_CRITICAL, (fmt), ##__VA_ARGS__)

#define obus_error(fmt, ...)	\
	obus_log(OBUS_LOG_ERROR, (fmt), ##__VA_ARGS__)

#define obus_warn(fmt, ...)	\
	obus_log(OBUS_LOG_WARNING, (fmt), ##__VA_ARGS__)

#define obus_notice(fmt, ...)	\
	obus_log(OBUS_LOG_NOTICE, (fmt), ##__VA_ARGS__)

#define obus_info(fmt, ...)	\
	obus_log(OBUS_LOG_INFO, (fmt), ##__VA_ARGS__)

#define obus_debug(fmt, ...)	\
	obus_log(OBUS_LOG_DEBUG, (fmt), ##__VA_ARGS__)

#define obus_log_errno(func)	\
	obus_error("%s error=%d(%s)", func, (errno), strerror(errno))

#define obus_log_fd_errno(func, fd)	\
	obus_error("%s(fd=%d) error=%d(%s)", func, fd, (errno),	\
		   strerror(errno))

#define obus_log_func_error(ret)	\
	obus_error("%s error=%d(%s)", __func__, (-ret), strerror(-ret))

#define LOG_STR_NULL(x) ((x) ? (x) : "")

/**
 * obus function error log
 * log internal obus function error when return is
 */
#define obus_log_func_ret(func, ret, fmt, ...)	\
	obus_error("%s error=%d(%s)", func, fd,	\
		   (-ret), strerror(-ret))

#endif /* _OBUS_LOG_H_ */
