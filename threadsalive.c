/*
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include "tList.h"
#include "threadsalive.h"

/* ***************************** 
     list functions
   ***************************** */


struct node **list_init(){
	struct node **head = malloc(sizeof(struct node *));
	*head = NULL;
	return head;
}

void list_clear(struct node **head) {
	struct node *curr = *head;    
	while (curr != NULL) {
        struct node *tmp = curr;
        curr = curr->next;
		free((tmp->threadContext->uc_stack).ss_sp);
		free(tmp->threadContext);
		free(tmp);
    }
	*head = NULL;
}

void list_print(const struct node *list) {
    printf("Thread list_print\n");
    while (list != NULL) {
        printf("Thread %d\n", list->threadNum);
        list = list->next;
    }
}//end list_print

struct node *list_pop(struct node**head){//removes the last item from the list without destroying that item, and then returns a pointer to that item
	assert(head != NULL);

	if(*head == NULL){ // if the list is empty
		return NULL;
	}

	struct node *curr = *head;
	if(curr->next == NULL){
		*head = NULL;
		return curr;
	}//end if

	while(curr->next->next != NULL){
		curr = curr->next;
	}//end while
	struct node *tmp = curr->next;	
	curr->next = NULL;
	return tmp;

}

int list_delete(struct node **head) { //delete the last item of a list. Returns 0 if the list is empty; returns 1 if deletion has been successfully carried out

	assert(head != NULL);

	if(*head == NULL){ //if the list is empty, return 0
		return 0;
	}//end if

	struct node *curr = *head;
	if(curr->next == NULL){ //if the list contains only 1 item, delete it and then return 1	
		free((curr->threadContext->uc_stack).ss_sp);
		free(curr->threadContext);
		free(curr);
		*head = NULL;
		return 1;
	}//end if

	//if the list contains 2 or more items
	while(curr->next->next != NULL){
		curr = curr->next;
	}//end while
	free((curr->threadContext->uc_stack).ss_sp);
	free(curr->threadContext);
	free(curr->next);
	curr->next = NULL;
	return 1;
}

void list_append(ucontext_t *ctx, int threadNum, struct node **head) { //add an item to the beginning of the list.

	assert(head != NULL);

	struct node *tmp = NULL;
	if(*head != NULL){ // if the list is not empty
		tmp = *head;	
	}//end if
	*head = malloc(sizeof(struct node));
	(*head)->threadContext = ctx;
	(*head)->threadNum = threadNum;
	(*head)->next = tmp;

	if((*head)->next != NULL){ //Change the link of the thread contained in the next node to point to the current thread
		(*head)->next->threadContext->uc_link = ctx;
	}
}

void list_append_node(struct node *n, struct node **head){ //add a node to the beginning of the list. 
	assert(head != NULL);
	
	struct node *tmp = NULL;
	if(*head != NULL){ // if the list is not empty
		tmp = *head;	
	}//end if

	*head = n;
	n->next = tmp;

}

void list_destroy_node(struct node **handle){ //used to destroy individual nodes. Do not use on nodes that are part of a list. 
	free((*handle)->threadContext->uc_stack.ss_sp);
	free((*handle)->threadContext);
	free(*handle);
	*handle = NULL;
}


/* ***************************** 
     stage 1 library functions
   ***************************** */

static struct node **ready;
static struct node **waiting;
static struct node *running;
static ucontext_t mainthread;
static int threadNumber = 0;

void ta_libinit() {
	ready = list_init();
	waiting = list_init();
	swapcontext(&mainthread, &mainthread);//stores the context of the calling thread - namely the mainthread - into mainthread
	
	//use the ready_queue as a buffer to pass the context of mainthread to running
	list_append(&mainthread, threadNumber++, ready);
	running = list_pop(ready);

	return;
}

void ta_create(void (*func)(void *), void *arg) {
	#define STACKSIZE 128000
	unsigned char *stack_space = (unsigned char*)malloc(STACKSIZE);
	ucontext_t *ctx = malloc(sizeof(ucontext_t));
	getcontext(ctx);
	ctx->uc_stack.ss_sp = stack_space;
    ctx[1].uc_stack.ss_size = STACKSIZE;
	ctx->uc_link = &mainthread; 
	makecontext(ctx, func, 1, arg);
	list_append(ctx, threadNumber++, ready);

	return;
}

void ta_yield() {
	
	assert(running != NULL);

	struct node *yielded = running;
	list_append_node(yielded, ready);
	running = list_pop(ready);
	swapcontext(yielded->threadContext, running->threadContext);

    return;
}

int ta_waitall() {
	running = list_pop(ready);
	
	
    return -1;
}



/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {
}

void ta_sem_destroy(tasem_t *sem) {
}

void ta_sem_post(tasem_t *sem) {
}

void ta_sem_wait(tasem_t *sem) {
}

void ta_lock_init(talock_t *mutex) {
}

void ta_lock_destroy(talock_t *mutex) {
}

void ta_lock(talock_t *mutex) {
}

void ta_unlock(talock_t *mutex) {
}


/* ***************************** 
     stage 3 library functions
   ***************************** */

void ta_cond_init(tacond_t *cond) {
}

void ta_cond_destroy(tacond_t *cond) {
}

void ta_wait(talock_t *mutex, tacond_t *cond) {
}

void ta_signal(tacond_t *cond) {
}


