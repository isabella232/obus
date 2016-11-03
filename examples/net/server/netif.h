/******************************************************************************
 * @file netif.h
 *
 * @brief network interface utility
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

#ifndef _NETITF_H_
#define _NETITF_H_

int netif_scan(char ***itfnames, size_t *nbr_itf);

int netif_is_up(const char *itfname);

int netif_set_up(const char *itfname);

int netif_set_down(const char *itfname);

int netif_set_ip_address(const char *itfname, const struct sockaddr *addr);

int netif_set_netmask(const char *itfname, const struct sockaddr *addr);

int netitf_read_traffic(const char *itfname, uint64_t *tx_bytes,
			uint64_t *rx_bytes);

int netif_read_hw_address(const char *itfname, char *buffer, size_t size);

int netif_read_ip_address(const char *itfname, char *buffer, size_t size);

int netif_read_netmask(const char *itfname, char *buffer, size_t size);

int netif_read_bcast_address(const char *itfname, char *buffer, size_t size);

int netif_read_mtu(const char *itfname, uint32_t *mtu);

#endif /* _NETITF_H_ */
