
#ifndef __THREADSALIVE_H__
#define __THREADSALIVE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ucontext.h>
#include <assert.h>

/* ***************************
        type definitions
   *************************** */

typedef struct {
	struct node **queue;
	int value;

} tasem_t;

typedef struct {
	tasem_t binary_sem;
} talock_t;

typedef struct {

	talock_t *partner_lock;

} tacond_t;

struct sem_node {
	tasem_t *sem;
	struct sem_node *next;
};

struct node {
	int threadNum;
    ucontext_t *threadContext;
    struct node *next; 
};

/* *****************************************************
       list functions for linked lists of semaphores
   *********************************************************** */

struct sem_node **list_sem_list_init();

void list_append_sem_node(tasem_t *, struct sem_node **);

bool list_sem_node_all_clear(struct sem_node **);

void list_sem_destroy_list(struct sem_node **);

int list_delete_sem_node(tasem_t *sem, struct sem_node **sem_list);

/* ********************************************************
       list functions for linked lists of thread contexts
   ******************************************************* */

struct node **list_init(); // returns a node**

void list_clear(struct node **);

void list_print(const struct node *);

struct node *list_pop(struct node**);//if the list is empty, returns NULL; otherwise removes the last item from the list without destroying that item, and then returns a pointer to that item.

struct node *list_last(struct node **); // if the list is empty, returns NULL; otherwise returns a pointer to the last item on the list

int list_delete(struct node **);//destroy the last item of a list. Returns 0 if the list is empty; returns 1 if deletion has been successfully carried out

int list_destroy_node(struct node **);//returns 0 if the node does not exist; returns 1 if deletection is successfully carried out

int list_destroy_mainthread_node(struct node **); //returns 0 if the node does not exist; returns 1 if deletection is successfully carried out. This function is specifically used to destroy a node that contains the context of the main thread. This function differs from the normal list_destroy_node in that it does not free the node's thread context and its associated resources, since the memory for the thread context of main thread is not malloc'ed and need not be freed. 

void list_append(ucontext_t *, int , struct node **);//add an item to the beginning of the list.

void list_append_node(struct node *, struct node **); //add a node to the beginning of the list, and changes the uc_link of the next node to point to the node that's just been added.

void list_sema_append_node(struct node *, struct node **); // add a node to the beginning of the list without changing the uc_link of the next node in line.

bool list_empty(struct node **);//return true if the list is empty; otherwise return false.


/* ***************************
       stage 1 functions
   *************************** */

void ta_libinit(void);
void ta_create(void (*)(void *), void *);
void ta_yield(void);
int ta_waitall(void);

/* ***************************
       stage 2 functions
   *************************** */

void ta_sem_init(tasem_t *, int);
void ta_sem_destroy(tasem_t *);
void ta_sem_post(tasem_t *);
void ta_sem_wait(tasem_t *);
void ta_lock_init(talock_t *);
void ta_lock_destroy(talock_t *);
void ta_lock(talock_t *);
void ta_unlock(talock_t *);

/* ***************************
       stage 3 functions
   *************************** */

void ta_cond_init(tacond_t *);
void ta_cond_destroy(tacond_t *);
void ta_wait(talock_t *, tacond_t *);
void ta_signal(tacond_t *);

#endif /* __THREADSALIVE_H__ */
