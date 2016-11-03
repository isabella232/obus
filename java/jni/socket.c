/******************************************************************************
 * @file socket.c
 *
 * @brief obus socket implementation for android
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

/* C headers */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <android/log.h>
#include <jni.h>

#define SIZEOF_ARRAY(x) (sizeof((x)) / sizeof((x)[0]))

#define LIBOBUSJNI_TAG "libobus-jni"

#ifndef ANDROID_LOG_INFO
#define ANDROID_LOG_INFO 4
#endif

#ifndef ANDROID_LOG_WARN
#define ANDROID_LOG_WARN 5
#endif

#ifndef ANDROID_LOG_ERROR
#define ANDROID_LOG_ERROR 6
#endif

#define LOGE(...) \
	((void)__android_log_print(ANDROID_LOG_ERROR, LIBOBUSJNI_TAG, __VA_ARGS__))

#define LOGW(...) \
	((void)__android_log_print(ANDROID_LOG_WARN, LIBOBUSJNI_TAG, __VA_ARGS__))

#define LOGI(...) \
	((void)__android_log_print(ANDROID_LOG_INFO, LIBOBUSJNI_TAG, __VA_ARGS__))


static int sockaddr_from_obus_canonical(struct sockaddr_storage *addr,
					const char *format)
{
	int ret;
	size_t i;
	char *str_addr;
	const char *remain;
	char *port;
	struct sockaddr_in *in;
	struct sockaddr_in6 *in6;
	struct sockaddr_un *un;

	static const struct {
		const char *prefix;
		size_t len;
		sa_family_t type;
	} families[] = {
		{"inet:", 5, AF_INET},
		{"inet6:", 6 , AF_INET6},
		{"unix:", 5, AF_UNIX},
	};

	if (!addr || !format)
		return -EINVAL;

	memset(addr, 0, sizeof(*addr));

	/* find socket family prefix */
	remain = NULL;
	for (i = 0; i < SIZEOF_ARRAY(families); i++) {
		if (strncmp(format, families[i].prefix, families[i].len) != 0)
			continue;

		/* init socket address family */
		addr->ss_family = families[i].type;

		/* handle address */
		remain = format + families[i].len;
		break;
	}

	switch (addr->ss_family) {
	case AF_INET:
	case AF_INET6:
		str_addr = strdup(remain);
		if (!str_addr)
			return -EINVAL;

		/* find port separator */
		port = strrchr(str_addr, ':');
		if (!port) {
			free(str_addr);
			return -EINVAL;
		}

		port[0] = '\0';
		port++;
		if (addr->ss_family == AF_INET) {
			in = (struct sockaddr_in *)addr;
			in->sin_port = htons((uint16_t)atoi(port));
			if (in->sin_port == 0) {
				LOGE("invalid inet port: %s", format);
				ret = -EINVAL;
			} else {
				ret = inet_pton(AF_INET, str_addr, &in->sin_addr);
				if (ret <= 0) {
					LOGE("inet address: %s not in presentation format",
					     str_addr);
					ret = -EINVAL;
				} else {
					ret = 0;
				}
			}

		} else {
			in6 = (struct sockaddr_in6 *)addr;
			in6->sin6_port = htons((uint16_t)atoi(port));
			if (in6->sin6_port == 0) {
				LOGE("invalid inet6 port: %s", format);
				ret = -EINVAL;
			} else {
				ret = inet_pton(AF_INET6, str_addr, &in6->sin6_addr);
				if (ret <= 0) {
					LOGE("inet6 address: %s not in presentation format",
					     str_addr);
					ret = -EINVAL;
				} else {
					ret = 0;
				}
			}
		}

		free(str_addr);
	break;
	case AF_UNIX:
		un = (struct sockaddr_un *)addr;

		/* copy sun path */
		snprintf(un->sun_path, sizeof(un->sun_path), "%s", remain);

		/*replace @ to '\0' for abstract socket */
		if (un->sun_path[0] == '@')
			un->sun_path[0] = '\0';
	break;
	default:
		LOGE("socket family %d not supported", addr->ss_family);
		return -EINVAL;
		break;
	}

	return 0;
}


/* add fd O_CLOEXEC flag in fd */
static int obus_fd_set_close_on_exec(int fd)
{
	int old, ret;

	old = fcntl(fd, F_GETFD, 0);
	if (old < 0)
		return -errno;

	ret = fcntl(fd, F_SETFD, FD_CLOEXEC | old);
	if (ret < 0)
		return -errno;

	return 0;
}

static size_t socketaddr_length(const struct sockaddr *addr)
{
	size_t len;

	switch (addr->sa_family) {
	case AF_INET:
		len = sizeof(struct sockaddr_in);
	break;
	case AF_INET6:
		len = sizeof(struct sockaddr_in6);
	break;
	case AF_UNIX:
		len = sizeof(struct sockaddr_un);
	break;
	default:
		len = 0;
	break;
	}

	return len;
}

static int set_socket_keepalive(int fd, int keepidle, int keepintvl,
					  	  	    int keepcnt)
{
	int ret;
	int keepalive = 1;

	/* activate keep alive on socket */
	ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive,
			sizeof(int));
	if (ret < 0) {
		LOGW("setsockopt(SO_KEEPALIVE) error:%s", strerror(errno));
		goto out;
	}

	/*
	 * TCP_KEEPIDLE:
	 * The time (in seconds) the connection needs to remain
	 * idle before TCP starts sending keepalive probes,
	 * if the socket
	 * option SO_KEEPALIVE has been set on this socket. */
	ret = setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int));
	if (ret < 0) {
		LOGW("setsockopt(TCP_KEEPIDLE) error:%s", strerror(errno));
		goto out;
	}

	/*
	 * TCP_KEEPINTVL:
	 * The time (in seconds) between individual keepalive
	 * probes. */
	ret = setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int));
	if (ret < 0) {
		LOGW("setsockopt(TCP_KEEPINTVL) error:%s", strerror(errno));
		goto out;
	}

	/*
	 * TCP_KEEPCNT:
	 * The maximum number of keepalive probes TCP
	 * should send before dropping the connection */
	ret = setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &keepcnt, sizeof(int));
	if (ret < 0) {
		LOGW("setsockopt(TCP_KEEPCNT) error:%s", strerror(errno));
		goto out;
	}
out:
	return ret;
}


static int open_socket(const struct sockaddr *addr)
{
	int ret, fd, opt = 1;

	/* create socket stream */
	fd = socket(addr->sa_family, SOCK_STREAM, 0);
	if (fd < 0) {
		LOGE("socket create error:%s", strerror(errno));
		return -1;
	}

	/* set socket fd close on exec flag */
	ret = obus_fd_set_close_on_exec(fd);
	if (ret < 0)
		goto close_socket;

	/**
	 * allow reuse of socket
	 * tells the kernel that even if this port is busy
	 * go ahead and reuse it anyway (useful if process crashed)
	 */
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	if (ret < 0) {
		LOGW("setsockopt error:%s", strerror(errno));
		goto close_socket;
	}

	/**
	 * set keepalive for tcp socket (same values as in libobus.so)
	 **/
	if (addr->sa_family == AF_INET ||
		addr->sa_family == AF_INET6)
		set_socket_keepalive(fd, 5, 1, 2);

	return fd;

close_socket:
	close(fd);
	return -1;
}

static jint ObusSocket_get_fd(C_JNIEnv *env, jobject socket)
{
	/* Get a reference to this object's class */
	jclass socketClass = (*env)->GetObjectClass(env, socket);

	/* Get the Field ID of the instance variables "fd" */
	jfieldID fidNumber = (*env)->GetFieldID(env, socketClass, "fd", "I");

	/* get field */
	return (*env)->GetIntField(env, socket, fidNumber);
}

static void ObusSocket_set_fd(C_JNIEnv *env, jobject socket, jint fd)
{
	/* Get a reference to this object's class */
	jclass socketClass = (*env)->GetObjectClass(env, socket);

	/* Get the Field ID of the instance variables "fd" */
	jfieldID fidNumber = (*env)->GetFieldID(env, socketClass, "fd", "I");

	/* set field */
	(*env)->SetIntField(env, socket, fidNumber, fd);
}

JNIEXPORT jboolean JNICALL jni_socket_do_connect(C_JNIEnv *env, jobject socket,
						 jstring address)
{
	int ret, fd = -1;
	struct sockaddr_storage st;
	const struct sockaddr *addr = (const struct sockaddr *)&st;
	const char *format;

	/* init sockaddr from obus canonical representation format */
	format = (*env)->GetStringUTFChars(env, address , NULL);
	ret = sockaddr_from_obus_canonical(&st, format);
	if (ret < 0)
		goto error;

	/* open & connect socket */
	fd = open_socket(addr);
	if (fd < 0)
		goto error;

	/* set fd in ObusSocket class */
	ObusSocket_set_fd(env, socket, fd);

	/* now connect socket (blocking)
	 * closing fd will return from this syscall */
	do {
		ret = connect(fd, addr, socketaddr_length(addr));
	} while (ret < 0 && errno == EINTR);

	if (ret < 0) {
		if (errno != ECONNREFUSED &&
		    errno != EHOSTUNREACH &&
		    errno != ENETUNREACH &&
		    errno != EHOSTDOWN &&
		    errno != ENOENT) {
			LOGE("connect '%s' error:%s", format, strerror(errno));
		}
		goto error;
	}

	(*env)->ReleaseStringUTFChars(env, address, format);
	return JNI_TRUE;

error:

	ObusSocket_set_fd(env, socket, -1);

	/* close socket */
	if (fd >= 0) {
		ret = close(fd);
		if (ret < 0)
			LOGW("socket close in connect error:%s", strerror(errno));
	}

	(*env)->ReleaseStringUTFChars(env, address, format);
	return JNI_FALSE;
}

JNIEXPORT jint JNICALL jni_socket_do_read(C_JNIEnv *env, jobject socket,
					  jbyteArray array, jint offset,
					  jint size)
{
	int fd;
	ssize_t ret;
	jbyte *buf;

	/* get fd */
	fd = ObusSocket_get_fd(env, socket);

	/* get buffer */
	buf = (*env)->GetByteArrayElements(env, array, NULL);

	/* read in blocking mode */
	do {
		ret = read(fd, buf + offset, size);
	} while (ret == -1 && errno == EINTR);

	if (ret == -1) {
		/* ETIMEDOUT raised on keepalive failure */
		if (errno != ETIMEDOUT)
			LOGW("socket read error:%s", strerror(errno));
	}

	(*env)->ReleaseByteArrayElements(env, array, buf, 0);

	return (int)ret;
}

JNIEXPORT jint JNICALL jni_socket_do_write(C_JNIEnv *env, jobject socket,
					   jbyteArray array, jint offset,
					   jint size)
{
	int fd;
	ssize_t ret;
	jbyte *buf;
	jint remain, written;

	/* get fd */
	fd = ObusSocket_get_fd(env, socket);

	/* get buffer */
	buf = (*env)->GetByteArrayElements(env, array, NULL);

	/* write in blocking mode */
	remain = size;
	written = 0;
	do {
		ret = write(fd, buf + offset + written, remain);
		if (ret == -1 && errno == EINTR)
			continue;

		if (ret < 0) {
			LOGW("socket write error:%s", strerror(errno));
			break;
		}

		written += ret;
		remain -= ret;
	} while (remain > 0);

	(*env)->ReleaseByteArrayElements(env, array, buf, 0);
	return written;
}


JNIEXPORT jint JNICALL jni_socket_shutdown(C_JNIEnv *env, jobject socket)
{
	int fd;
	int ret;

	/* get fd */
	fd = ObusSocket_get_fd(env, socket);
	if (fd < 0)
		return 0;

	/* shutdown socket */
	ret = shutdown(fd, 2);
	if (ret < 0)
		LOGW("socket shutdown error:%s", strerror(errno));

	return 0;
}

JNIEXPORT jint JNICALL jni_socket_close(C_JNIEnv *env, jobject socket)
{
	int fd;
	int ret;

	/* get fd */
	fd = ObusSocket_get_fd(env, socket);
	if (fd < 0)
		return 0;

	/* set fd to -1 */
	ObusSocket_set_fd(env, socket, -1);

	/* close socket */
	ret = close(fd);
	if (ret < 0)
		LOGW("socket close error:%s", strerror(errno));

	return 0;
}

static const JNINativeMethod methods[] = {
	{ "doConnect", "(Ljava/lang/String;)Z", (void *) jni_socket_do_connect },
	{ "doRead", "([BII)I", (void *) jni_socket_do_read },
	{ "doWrite", "([BII)I", (void *) jni_socket_do_write },
	{ "shutdown", "()V", (void *) jni_socket_shutdown } ,
	{ "close", "()V", (void *) jni_socket_close }
};

jint JNI_OnLoad(const struct JNIInvokeInterface **vm, void *reserved) {
	C_JNIEnv *env;
	jclass clazz;
	jint ret;

	if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK)
		return JNI_ERR;

	clazz = (*env)->FindClass(env, "com/parrot/obus/internal/ObusSocket");
	if (!clazz)
		return JNI_ERR;

	ret = (*env)->RegisterNatives(env, clazz, methods, SIZEOF_ARRAY(methods));
	(*env)->DeleteLocalRef(env, clazz);
	return ret == 0 ? JNI_VERSION_1_6 : JNI_ERR;
}
