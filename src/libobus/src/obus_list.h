/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_list.h
 *
 * @brief obus double chained list
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

#ifndef _OBUS_LIST_H_
#define _OBUS_LIST_H_

#define OBUS_LIST_POISON1 ((void *)0xDEADBEEF)
#define OBUS_LIST_POISON2 ((void *)0xDEADDEAD)


#ifndef _LIBOBUS_PRIVATE_H_
/**
 * list node (also defined in private headers)
 */
struct obus_node {
	struct obus_node *next, *prev;
};
#endif

static inline void
obus_node_unref(struct obus_node *node)
{
	node->next = (struct obus_node *)OBUS_LIST_POISON1;
	node->prev = (struct obus_node *)OBUS_LIST_POISON2;
}

static inline int
obus_node_is_unref(struct obus_node *node)
{
	return (node->next == (struct obus_node *)OBUS_LIST_POISON1) &&
	       (node->prev == (struct obus_node *)OBUS_LIST_POISON2);
}

static inline int
obus_node_is_ref(struct obus_node *node)
{
	return !obus_node_is_unref(node);
}

static inline void
obus_list_init(struct obus_node *list)
{
	list->next = list;
	list->prev = list;
}

static inline struct obus_node*
obus_list_next(const struct obus_node *list, const struct obus_node *item)
{
	return (item->next != list) ? list->next : NULL;
}

static inline struct obus_node*
obus_list_first(const struct obus_node *list)
{
	return list->next;
}
static inline int
obus_list_is_empty(const struct obus_node *list)
{
	return list->next == list;
}

static inline struct obus_node *
obus_list_last(const struct obus_node *list)
{
	return list->prev;
}

static inline void
obus_list_add(struct obus_node *novel, struct obus_node *prev,
	      struct obus_node *next)
{
	next->prev = novel;
	novel->next = next;
	novel->prev = prev;
	prev->next = novel;
}

static inline void
obus_list_add_after(struct obus_node *node, struct obus_node *novel)
{
	obus_list_add(novel, node, node->next);
}

static inline void
obus_list_add_before(struct obus_node *node, struct obus_node *novel)
{
	obus_list_add(novel, node->prev, node);
}

static inline void
obus_list_detach(struct obus_node *prev, struct obus_node *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void
obus_list_del(struct obus_node *node)
{
	obus_list_detach(node->prev, node->next);
	obus_node_unref(node);
}

static inline void
obus_list_del_init(struct obus_node *node)
{
	obus_list_detach(node->prev, node->next);
	obus_list_init(node);
}

static inline void
obus_list_replace(struct obus_node *old, struct obus_node *novel)
{
	novel->next = old->next;
	novel->next->prev = novel;
	novel->prev = old->prev;
	novel->prev->next = novel;
}

static inline void
obus_list_replace_init(struct obus_node *old, struct obus_node *novel)
{
	obus_list_replace(old, novel);
	obus_list_init(old);
}

static inline void
obus_list_move_after(struct obus_node *to, struct obus_node *from)
{
	obus_list_detach(from->prev, from->next);
	obus_list_add_after(to, from);
}

static inline void
obus_list_move_before(struct obus_node *to, struct obus_node *from)
{
	obus_list_detach(from->prev, from->next);
	obus_list_add_before(to, from);
}

static inline int
obus_list_is_last(const struct obus_node *list, const struct obus_node *node)
{
	return list->prev == node;
}

#define obus_list_head_init(name) { &(name), &(name) }

#define obus_list_entry(ptr, type, member)\
	obus_container_of(ptr, type, member)

#define obus_list_walk_forward(list, pos)\
	for (pos = (list)->next; pos != (list); pos = pos->next)

#define obus_list_walk_backward(list, pos)\
	for (pos = (list)->prev; pos != (list);	obus_pos = pos->prev)

#define obus_list_walk_forward_safe(list, pos, tmp)	\
	for (pos = (list)->next,			\
			tmp = pos->next; pos != (list);	\
			pos = tmp, tmp = pos->next)

#define obus_list_walk_entry_forward(list, pos, member)			\
	for (pos = obus_list_entry((list)->next, typeof(*pos), member);	\
		&pos->member != (list);					\
		pos = obus_list_entry(pos->member.next, typeof(*pos), member))

#define obus_list_walk_entry_backward(list, pos, member)		\
	for (pos = obus_list_entry((list)->prev, typeof(*pos), member);	\
		&pos->member != (list);					\
		pos = obus_list_entry(pos->member.prev, typeof(*pos), member))

#define obus_list_walk_entry_forward_safe(list, pos, tmp, member)	\
	for (pos = obus_list_entry((list)->next, typeof(*pos), member),	\
			tmp = obus_list_entry(pos->member.next,		\
					typeof(*pos), member);		\
		&pos->member != (list);					\
		pos = tmp, tmp = obus_list_entry(tmp->member.next,	\
			typeof(*tmp), member))

#define obus_list_walk_entry_backward_safe(list, pos, tmp, member)	\
	for (pos = obus_list_entry((list)->prev, typeof(*pos), member),	\
			tmp = obus_list_entry(pos->member.prev,		\
			typeof(*pos), member);				\
		&pos->member != (list);					\
		pos = tmp, tmp = obus_list_entry(tmp->member.prev,	\
			typeof(*tmp), member))


static inline size_t
obus_list_length(const struct obus_node *list)
{
	size_t length = 0;
	const struct obus_node *tmp;
	const struct obus_node *current;

	obus_list_walk_forward_safe(list, current, tmp) {
		length++;
	}
	return length;
}

#endif /* _OBUS_LIST_H_ */
