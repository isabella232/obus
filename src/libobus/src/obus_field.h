/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_field.h
 *
 * @brief obus field header
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

#ifndef _OBUS_FIELD_H_
#define _OBUS_FIELD_H_


uint32_t *obus_field_array_nb_address(const struct obus_struct *st,
				      const struct obus_field_desc *desc);

void *obus_field_address(const struct obus_struct *st,
			 const struct obus_field_desc *desc);

void obus_field_format(const struct obus_struct *st,
		       const struct obus_field_desc *desc,
		       char *buf, size_t size);

int obus_field_init(const struct obus_struct *st,
		    const struct obus_field_desc *desc);

void obus_field_destroy(const struct obus_struct *st,
			const struct obus_field_desc *desc);

int obus_field_encode(const struct obus_struct *st,
		      const struct obus_field_desc *desc,
		      struct obus_buffer *buf);

const struct obus_field_desc *
obus_field_decode(const struct obus_struct *st,
		  struct obus_buffer *buf);

int obus_field_copy(const struct obus_struct *dst,
		    const struct obus_struct *src,
		    const struct obus_field_desc *desc);

#endif /* _OBUS_FIELD_H_ */
