/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_platform.h
 *
 * @brief obus platform header
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

#ifndef _OBUS_PLATFORM_H_
#define _OBUS_PLATFORM_H_

/* definition of unix socket path max */
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

#ifndef O_CLOEXEC
#ifdef ARCH_ARM
	/* value taken from linux kernel header
	 * include/asm-generic/fcntl.h */
	#define O_CLOEXEC 02000000
#else
	#error O_CLOEXEC not defined !
#endif
#endif

/* eventfd / signalfd / timerfd not supported by bionic
 * __NR_eventfd2 is not defined also in bionic headers */
#ifdef ANDROID
#include <sys/syscall.h>
#include <linux/fcntl.h>

#ifndef __NR_timerfd_create
	#ifdef ARCH_ARM
		/*
		 * syscall number taken from linux kernel header
		 * linux/arch/arm/include/asm/unistd.h
		 */
		#define __NR_timerfd_create (__NR_SYSCALL_BASE+350)
		#define __NR_timerfd_settime (__NR_SYSCALL_BASE+353)
		#define __NR_timerfd_gettime (__NR_SYSCALL_BASE+354)
	#else
		#error __NR_timerfd_create not defined !
	#endif
#endif

/* timer fd defines */
#define TFD_TIMER_ABSTIME (1 << 0)
#define TFD_CLOEXEC O_CLOEXEC
#define TFD_NONBLOCK O_NONBLOCK
static inline int timerfd_create(int clockid, int flags)
{
	return syscall(__NR_timerfd_create, clockid, flags);
}

static inline int timerfd_settime(int fd, int flags,
		const struct itimerspec *new_value,
		struct itimerspec *old_value)
{
	return syscall(__NR_timerfd_settime, fd, flags, new_value, old_value);
}

static inline int timerfd_gettime(int fd, struct itimerspec *curr_value)
{
	return syscall(__NR_timerfd_gettime, fd, curr_value);
}

#ifndef __NR_eventfd2
	#ifdef ARCH_ARM
		/* syscall number taken from linux kernel header
		 * linux/arch/arm/include/asm/unistd.h */
		#define __NR_eventfd2 (__NR_SYSCALL_BASE+356)
	#else
		#error __NR_eventfd2 not defined !
	#endif
#endif

/* flags for eventfd.  */
#define EFD_SEMAPHORE (1 << 0)
#define EFD_CLOEXEC O_CLOEXEC
#define EFD_NONBLOCK O_NONBLOCK

/* Return file descriptor for generic event */
static inline int eventfd(int count, int flags)
{
	return syscall(__NR_eventfd2, count, flags);
}

#ifndef __NR_signalfd4
	#ifdef ARCH_ARM
		/* syscall number taken from linux kernel header
		 * linux/arch/arm/include/asm/unistd.h */
		#define __NR_signalfd4 (__NR_SYSCALL_BASE+355)
	#else
		#error __NR_signalfd4 not defined !
	#endif
#endif

struct signalfd_siginfo {
	uint32_t ssi_signo;
	int32_t ssi_errno;
	int32_t ssi_code;
	uint32_t ssi_pid;
	uint32_t ssi_uid;
	int32_t ssi_fd;
	uint32_t ssi_tid;
	uint32_t ssi_band;
	uint32_t ssi_overrun;
	uint32_t ssi_trapno;
	int32_t ssi_status;
	int32_t ssi_int;
	uint64_t ssi_ptr;
	uint64_t ssi_utime;
	uint64_t ssi_stime;
	uint64_t ssi_addr;
	uint8_t __pad[48];
};

/* flags for signalfd.  */
#define SFD_CLOEXEC O_CLOEXEC
#define SFD_NONBLOCK O_NONBLOCK

/* Return file descriptor for signal event */
static inline int signalfd(int fd, const sigset_t *mask, int flags)
{
	sigset_t sigset[2] = {*mask, 0};
	/* warning: bionic sigset_t size is half kernel sigset size */
	return syscall(__NR_signalfd4, fd, mask, sizeof(sigset), flags);
}

#else

#ifdef HAVE_SYS_TIMERFD_H
#include <sys/timerfd.h>
#endif /* HAVE_TIMERFD */

#ifdef HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#endif /* HAVE_SYS_EVENTFD_H */

#ifdef HAVE_SYS_SIGNALFD_H
#include <sys/signalfd.h>
#endif /* HAVE_SYS_SIGNALFD_H */

	/* on linux toolchain timerfd, signalfd flags may not exists
	 * ex: arm-2009q1/arm-none-linux-gnueabi */
#ifndef TFD_CLOEXEC
#define TFD_CLOEXEC O_CLOEXEC
#endif

#ifndef TFD_NONBLOCK
#define TFD_NONBLOCK O_NONBLOCK
#endif

#ifndef SFD_CLOEXEC
#define SFD_CLOEXEC O_CLOEXEC
#endif

#ifndef SFD_NONBLOCK
#define SFD_NONBLOCK O_NONBLOCK
#endif

#endif
#endif /* _OBUS_PLATFORM_H_ */
