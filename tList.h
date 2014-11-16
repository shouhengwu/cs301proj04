
#ifndef _TLIST_GUARD_
#define __TLIST_GUARD_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ucontext.h>
#include <assert.h>

struct node {
	int threadNum;
    ucontext_t *threadContext;
    struct node *next; 
};

struct node **list_init();

void list_clear(struct node **);

void list_print(const struct node *);

int list_delete(struct node **);

void list_append(ucontext_t *, int, struct node **);

void list_append_node(struct node *, struct node **);

int make_node(void (*func)(void *), void *arg);

//int main();

#endif
