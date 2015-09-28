/**
 *	Internal Kernel - Unordered, singly linked list.
 *
 *	Circular.
 **/

#ifndef _BT_LIST_H_
#define _BT_LIST_H_

#include <bt_types.h>

struct bt_list_head {
	struct bt_list_head *next, *prev;
};

#define BT_LIST_HEAD_INIT(name)		{ &(name), &(name) }
#define BT_LIST_HEAD(name)			struct bt_list_head name = BT_LIST_HEAD_INIT(name)

static inline void BT_LIST_INIT_HEAD(struct bt_list_head *list) {
	list->next = list;
	list->prev = list;
}

static inline void __bt_list_add(struct bt_list_head *new, struct bt_list_head *prev, struct bt_list_head *next) {
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void bt_list_add(struct bt_list_head *new, struct bt_list_head *head) {
	__bt_list_add(new, head, head->next);
}

static inline void bt_list_add_tail(struct bt_list_head *new, struct bt_list_head *head) {
	__bt_list_add(new, head->prev, head);
}

static inline void __bt_list_del(struct bt_list_head *prev, struct bt_list_head *next) {
	next->prev = prev;
	prev->next = next;
}

static inline void bt_list_del(struct bt_list_head *entry) {
	__bt_list_del(entry->prev, entry->next);
}

static inline void __bt_list_del_entry(struct bt_list_head *entry) {
	__bt_list_del(entry->prev, entry->next);
}

static inline void bt_list_del_init(struct bt_list_head *entry){
	__bt_list_del_entry(entry);
	BT_LIST_INIT_HEAD(entry);
}

static inline int bt_list_is_last(const struct bt_list_head *list, const struct bt_list_head *head) {
	return list->next == head;
}

static inline int bt_list_empty(const struct bt_list_head *head) {
	return head->next == head;
}

static inline void __bt_list_splice(const struct bt_list_head *list,
				 struct bt_list_head *prev,
				 struct bt_list_head *next) {
	struct bt_list_head *first = list->next;
	struct bt_list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

static inline void bt_list_splice(const struct bt_list_head *list,
				struct bt_list_head *head) {
	if (!bt_list_empty(list))
		__bt_list_splice(list, head, head->next);
}

static inline void bt_list_splice_tail_init(struct bt_list_head *list,
					 struct bt_list_head *head) {
	if (!bt_list_empty(list)) {
		__bt_list_splice(list, head->prev, head);
		BT_LIST_INIT_HEAD(list);
	}
}


#define bt_list_entry(ptr, type, member)								\
	bt_container_of(ptr, type, member)

#define bt_list_first_entry(ptr, type, member)		 					\
	list_entry((ptr)->next, type, member)

#define bt_list_for_each(pos, head) \
	for(pos = (head)->next; pos != (head); pos = pos->next)

#define bt_list_for_each_safe(pos, n, head) \
	for(pos = (head)->next, n = pos->next; pos != (head); \
			pos = n, n = pos->next)

#define bt_list_for_each_entry(pos, head, member)						\
	for(pos = bt_list_entry((head)->next, typeof(*pos), member);		\
		&pos->member != (head);											\
		pos = bt_list_entry(pos->member.next, typeof(*pos), member))

#define bt_list_for_each_entry_safe(pos, n, head, member)				\
	for (pos = bt_list_entry((head)->next, typeof(*pos), member),		\
		n = bt_list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head); 										\
	     pos = n, n = bt_list_entry(n->member.next, typeof(*n), member))


struct bt_hlist_node {
	struct bt_hlist_node *next, **pprev;
};

struct bt_hlist_head {
	struct bt_hlist_node *first;
};

#define BT_HLIST_HEAD_INIT		{ .first = NULL }
#define BT_HLIST_HEAD(name)		struct hlist_head name = { .first = NULL }
#define BT_INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)

static inline void BT_INIT_HLIST_NODE(struct bt_hlist_node *h) {
	h->next = NULL;
	h->pprev = NULL;
}

static inline int bt_hlist_unhashed(const struct bt_hlist_node *h) {
	return !h->pprev;
}

static inline int bt_hlist_empty(const struct bt_hlist_head *h) {
	return !h->first;
}

static inline void __bt_hlist_del(struct bt_hlist_node *n) {
	struct bt_hlist_node *next = n->next;
	struct bt_hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next) {
		next->pprev = pprev;
	}
}

#define BT_POISON_POINTER_DELTA 0
#define BT_LIST_POISON1 ((void *) 0x00100100 + BT_POISON_POINTER_DELTA)
#define BT_LIST_POISON2 ((void *) 0x00200200 + BT_POISON_POINTER_DELTA)

static inline void bt_hlist_del(struct bt_hlist_node *n) {
	__bt_hlist_del(n);
	n->next = BT_LIST_POISON1;
	n->pprev = BT_LIST_POISON2;
}

static inline void bt_hlist_del_init(struct bt_hlist_node *n) {
	if (!bt_hlist_unhashed(n)) {
		__bt_hlist_del(n);
		BT_INIT_HLIST_NODE(n);
	}
}

static inline void bt_hlist_add_head(struct bt_hlist_node *n, struct bt_hlist_head *h) {
	struct bt_hlist_node *first = h->first;
	n->next = first;
	if (first) {
		first->pprev = &n->next;
	}
	h->first = n;
	n->pprev = &h->first;
}

static inline void bt_hlist_add_before(struct bt_hlist_node *n,
                                        struct bt_hlist_node *next) {
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

static inline void bt_hlist_add_after(struct bt_hlist_node *n,
                                        struct bt_hlist_node *next) {
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if(next->next)
		next->next->pprev  = &next->next;
}

static inline void bt_hlist_add_fake(struct bt_hlist_node *n) {
	n->pprev = &n->next;
}

static inline void bt_hlist_move_list(struct bt_hlist_head *old,
                                   struct bt_hlist_head *new) {
	new->first = old->first;
	if (new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}

#define bt_hlist_entry(ptr, type, member) bt_container_of(ptr,type,member)

#define bt_hlist_for_each(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)

#define bt_hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; });	\
		 pos = n)

#define bt_hlist_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr);								\
		____ptr ? hlist_entry(____ptr, type, member) : NULL;	\
	})

#define bt_hlist_for_each_entry(pos, head, member)                         \
	for (pos = bt_hlist_entry_safe((head)->first, typeof(*(pos)), member); \
		 pos;															\
		 pos = bt_hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define bt_hlist_for_each_entry_continue(pos, member)                      \
	for (pos = bt_hlist_entry_safe((pos)->member.next, typeof(*(pos)), member);	\
		 pos;															\
		 pos = bt_hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define bt_hlist_for_each_entry_from(pos, member)                          \
	for (; pos;															\
		 pos = bt_hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define bt_hlist_for_each_entry_safe(pos, n, head, member)                 \
	for (pos = bt_hlist_entry_safe((head)->first, typeof(*pos), member); \
		 pos && ({ n = pos->member.next; 1; });							\
		 pos = bt_hlist_entry_safe(n, typeof(*pos), member))

#endif
