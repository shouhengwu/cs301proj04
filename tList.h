
#ifndef _TLIST_GUARD_
#define __TLIST_GUARD_





struct node **list_init();

void list_clear(struct node **);

void list_print(const struct node *);

int list_delete(struct node **);

void list_append(ucontext_t *, int, struct node **);

void list_append_node(struct node *, struct node **);

int list_destroy_node(struct node **);

int make_node(void (*func)(void *), void *arg);

int main();

#endif
