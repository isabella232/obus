/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_timer.h
 *
 * @brief obus timer api
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

#ifndef _OBUS_TIMER_H_
#define _OBUS_TIMER_H_

/**
 * obus timer object
 */
struct obus_timer;

/**
 * timer callback
 * @param timer timer object
 * @param nbexpired how many time counter has expired
 * @param data user data pointer given back
 */
typedef void (*obus_timer_cb_t) (struct obus_timer *timer,
				 uint64_t *nbexpired, void *data);

/**
 * create a new timer
 * @param loop file descriptor events loop
 * @param cb user timer callback
 * @param data user data
 * @return obus timer object or NULL on error
 */
struct obus_timer *obus_timer_new(struct obus_loop *loop, obus_timer_cb_t cb,
				  void *data);

/**
 * destroy obus timer object
 * @param timer obus timer object
 * @return 0 on success
 */
int obus_timer_destroy(struct obus_timer *timer);

/**
 * set timer timeout
 *
 * @param timer obus timer object
 * @param timeout relative timeout in milliseconds
 * @return 0 on success
 */
int obus_timer_set(struct obus_timer *timer, int timeout);

/**
 * clear timer
 *
 * @param timer obus timer object
 * @return 0 on success
 */
int obus_timer_clear(struct obus_timer *timer);

#endif /*_OBUS_TIMER_H_*/
