/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_log.c
 *
 * @brief obus log
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

/* obus default log callback */
static void obus_log_stderr(enum obus_log_level level, const char *fmt,
			    va_list args)
{
	static const char levels[] = {
		[OBUS_LOG_CRITICAL]	= 'C',
		[OBUS_LOG_ERROR]	= 'E',
		[OBUS_LOG_WARNING]	= 'W',
		[OBUS_LOG_NOTICE]	= 'N',
		[OBUS_LOG_INFO]		= 'I',
		[OBUS_LOG_DEBUG]	= 'D',
	};
	static int use_colors = -1;
	static const char * const colors[] = {
		[OBUS_LOG_CRITICAL]	= "\033[31m",
		[OBUS_LOG_ERROR]	= "\033[31m",
		[OBUS_LOG_WARNING]	= "\033[33m",
		[OBUS_LOG_NOTICE]	= "\033[34m",
		[OBUS_LOG_INFO]		= "\033[32m",
		[OBUS_LOG_DEBUG]	= "\033[36m",
	};

	char buffer[512];
	int len = 0;

	if (level >= OBUS_LOG_DEBUG)
		level = OBUS_LOG_DEBUG;

	/* check if we shall use colors */
	if (use_colors == -1)
		use_colors = obus_check_env("OBUS_LOG_COLOR", "1");

	/* append message */
	len = vsnprintf(buffer, sizeof(buffer), fmt, args);
	if (len > 0 && (size_t)len >= sizeof(buffer))
		len = sizeof(buffer) - 1;

	/* append '\n' if needed */
	if (len > 0 && ((size_t)len + 1 < sizeof(buffer)) &&
	    buffer[len - 1] != '\n') {
		buffer[len++] = '\n';
		buffer[len] = '\0';
	}

	/* use colors */
	if (use_colors) {
		fprintf(stderr, "%s[%c]: %s\033[m", colors[level],
			levels[level], buffer);
	} else {
		fprintf(stderr, "[%c]: %s", levels[level], buffer);
	}
}

/* obus callback */
static obus_log_cb_t g_cb = obus_log_stderr;

OBUS_API int obus_log_set_cb(obus_log_cb_t cb)
{
	g_cb = cb;
	return 0;
}

OBUS_API void obus_log_raw(enum obus_log_level level, const void *buffer,
		  size_t length, const char *fmt, ...)
{
	static const char hexdigits[] = "0123456789ABCDEF";
	char str[68];
	va_list args;
	uint8_t byte;
	size_t n = 0, p = 0;
	size_t i;

	if (!buffer || !fmt || !g_cb)
		return;

	/* log prefix */
	va_start(args, fmt);
	(*g_cb) (level, fmt, args);
	va_end(args);

	/* log format */
	/* xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
	 * .................*/
	for (i = 0; i < length; i++) {
		byte = ((const uint8_t *)buffer)[i] & 0xff;
		str[((n % 16) * 3)] = hexdigits[byte >> 4];
		str[((n % 16) * 3) + 1] = hexdigits[byte & 0xf];
		str[((n % 16) * 3) + 2] = ' ';
		if (isprint(byte))
			str[(n % 16) + 51] = (char)byte;
		else
			str[(n % 16) + 51] = '.';

		if ((n + 1) % 16 == 0) {
			str[48] = ' ';
			str[49] = '|';
			str[50] = ' ';
			str[67] = '\0';
			obus_log(level, "%s", str);
		}
		n++;
	}

	if (n % 16 > 0) {
		p = (n % 16);
		while (p < 16) {
			str[((p % 16) * 3)] = ' ';
			str[((p % 16) * 3) + 1] = ' ';
			str[((p % 16) * 3) + 2] = ' ';
			str[(p % 16) + 51] = ' ';
			p++;
		}
		str[48] = ' ';
		str[49] = '|';
		str[50] = ' ';
		str[67] = '\0';
		obus_log(level, "%s", str);
	}
}

OBUS_API void obus_log(enum obus_log_level level, const char *fmt, ...)
{
	va_list args;

	if (!fmt || !g_cb)
		return;

	va_start(args, fmt);
	(*g_cb) (level, fmt, args);
	va_end(args);
}
