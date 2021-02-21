#include <lib/list/list.h>


void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

void __list_add(struct list_head *new,
                       struct list_head *prev,
                       struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void list_add(struct list_head *new, 
                     struct list_head *head)
{
    __list_add(new, head, head->next);
}

void list_add_tail(struct list_head *new, 
                          struct list_head *head)
{
    __list_add(new, head->prev, head);
}


static inline void __list_del(struct list_head *prev, 
                       struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}


void list_replace(struct list_head *old,
                         struct list_head *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

void list_replace_init(struct list_head *old,
                              struct list_head *new)
{
    list_replace(old, new);
    INIT_LIST_HEAD(old);
}

void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

void list_move(struct list_head *list, 
                      struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add(list, head);
}


void list_move_tail(struct list_head *list,
                           struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

int list_is_last(const struct list_head *list,
                        const struct list_head *head)
{
    return list->next == head;
}

int list_empty(const struct list_head *head)
{
    return head->next == head;
}


int list_empty_careful(const struct list_head *head)
{
    struct list_head *next = head->next;
    return (next == head) && (next == head->prev);
}

static inline void __list_splice(struct list_head *list,
                          struct list_head *head)
{
    struct list_head *first = list->next;
    struct list_head *last = list->prev;
    struct list_head *at = head->next;

    first->prev = head;
    head->next = first;

    last->next = at;
    at->prev = last;
}


void list_splice(struct list_head *list, 
                        struct list_head *head)
{
    if (!list_empty(list))
        __list_splice(list, head);
}

void list_splice_init(struct list_head *list,
                             struct list_head *head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head);
        INIT_LIST_HEAD(list);
    }
}
