/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_hash.h
 *
 * @brief obus hash
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

#ifndef _OBUS_HASH_H_
#define _OBUS_HASH_H_


/**
 * hash entry container
 */
struct obus_hash_entry {
	struct obus_node node;		/* node in hash list entries */
	int is_const;			/* is entry const */
	union {
		void *data;			/* entry data */
		const void *const_data;		/* entry const data */
	};
	uint32_t key;			/* entry key */
	struct obus_hash_entry *next;	/* next entry with same hash value*/
};

/**
 * hash structure
 */
struct obus_hash {
	struct obus_hash_entry **buckets;	/* hash table buckets */
	uint32_t size;				/* hash table size */
	struct obus_node entries;		/* node entries */
};

/**
 * create a new hash table
 * @param hash
 * @param size
 * @return 0 on success
 */
int obus_hash_init(struct obus_hash *hash, size_t size);

/**
 * destroy hash table
 * @param hash
 * @return 0 on success
 */
int obus_hash_destroy(struct obus_hash *hash);

/**
 * insert an entry in hash table
 *
 * @param hash hash table
 * @param key entry key
 * @param data entry data to be stored
 * @return 0 if entry inserted, -EEXIST if another entry with same key.
 */
int obus_hash_insert(struct obus_hash *hash, uint32_t key, void *data);

/**
 * insert a const entry in hash table
 *
 * @param hash hash table
 * @param key entry key
 * @param data entry data to be stored
 * @return 0 if entry inserted, -EEXIST if another entry with same key.
 */
int obus_hash_insert_const(struct obus_hash *hash, uint32_t key,
			   const void *data);

/**
 * remove an entry from hash table
 *
 * @param tab hash table
 * @param key entry key to be removed
 * @return 0 if entry is found and removed
 */
int obus_hash_remove(struct obus_hash *hash, uint32_t key);


/**
 * lookup to an entry in hash table
 *
 * @param tab hash table
 * @param key entry key
 * @param data entry data pointer if entry found
 * @return return 0 if entry is found
 */
int obus_hash_lookup(const struct obus_hash *hash, uint32_t key, void **data);

/**
 * lookup to a const entry in hash table
 *
 * @param tab hash table
 * @param key entry key
 * @param data entry data pointer if entry found
 * @return return 0 if entry is found
 */
int obus_hash_lookup_const(const struct obus_hash *hash, uint32_t key,
			   const void **data);

#endif /*_MB_HASH_H_*/
