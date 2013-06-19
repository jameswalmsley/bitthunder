/**
 *	Internal Kernel - Unordered, singly linked list.
 *
 *	Circular.
 **/

#ifndef _BT_LIST_H_
#define _BT_LIST_H_

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

static inline int bt_list_is_last(const struct bt_list_head *list, const struct bt_list_head *head) {
	return list->next == head;
}

static inline int bt_list_empty(const struct bt_list_head *head) {
	return head->next == head;
}

#define bt_list_for_each(pos, head) \
	for(pos = (head)->next; pos != (head); pos = pos->next)

#endif
