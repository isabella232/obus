/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_utils.h
 *
 * @brief obus utilities api
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

#ifndef _OBUS_UTILS_H_
#define _OBUS_UTILS_H_

/* add fd O_CLOEXEC flag in fd */
int obus_fd_set_close_on_exec(int fd);

/* check if fd has O_CLOEXEC flag */
int obus_fd_has_close_on_exec(int fd);

/* add fd flags */
int obus_fd_add_flags(int fd, int flags);

/* duplicate fd with same flags (O_NONBLOCK ...) */
int obus_fd_dup(int fd);

/* never modify env value */
const char *obus_get_env(const char *key);

/* never modify env value */
int obus_get_env_uint32(const char *key, uint32_t *value);

/* never modify env value */
int obus_check_env(const char *key, const char *value);

/* get random handle */
obus_handle_t obus_rand_handle(void);

enum obus_log_flags {
	OBUS_LOG_BUS = (1 << 0),
	OBUS_LOG_IO = (1 << 1),
	OBUS_LOG_SOCKET = (1 << 2),
	OBUS_LOG_CONNECTION = (1 << 3),
};

/* get log flags from env */
uint32_t obus_get_log_flags_from_env(const char *bus_name);

#endif /* _OBUS_UTILS_H_ */
