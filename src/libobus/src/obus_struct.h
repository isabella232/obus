/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_struct.h
 *
 * @brief obus internal struct api
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


#ifndef _OBUS_STRUCT_H_
#define _OBUS_STRUCT_H_

int obus_struct_init(const struct obus_struct *st);

void obus_struct_destroy(const struct obus_struct *st);

int obus_struct_encode(const struct obus_struct *st, struct obus_buffer *buf);

int obus_struct_decode(const struct obus_struct *st, struct obus_buffer *buf);

void obus_struct_log(const struct obus_struct *st, enum obus_log_level level);

int obus_struct_has_field(const struct obus_struct *st,
			  const struct obus_field_desc *desc);

int obus_struct_set_has_field(const struct obus_struct *st,
			      const struct obus_field_desc *desc);

int obus_struct_clear_has_field(const struct obus_struct *st,
				const struct obus_field_desc *desc);

int obus_struct_set_has_fields(const struct obus_struct *st);
int obus_struct_clear_has_fields(const struct obus_struct *st);

int obus_struct_copy(const struct obus_struct *dst,
		     const struct obus_struct *src);

int obus_struct_merge(const struct obus_struct *dst,
		      const struct obus_struct *src);

int obus_struct_is_empty(const struct obus_struct *st);

const struct obus_field_desc *
obus_struct_get_field_desc(const struct obus_struct *st, uint16_t uid);

#endif /* _OBUS_STRUCT_H_ */
