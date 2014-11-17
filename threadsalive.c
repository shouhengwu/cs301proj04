/*
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include "threadsalive.h"

/* ***************************** 
     global variables
   ***************************** */

static struct node **ready; //this linked list stores all threads that are in ready state. The righter-most/last item on this list holds the currently running thread. 
static struct node **waiting;
static ucontext_t mainthread;
static int threadNumber = 0;


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

struct node *list_pop(struct node**head){//if the list is empty, returns NULL; otherwise removes the last item from the list without destroying that item, and then returns a pointer to that item.
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

struct node *list_last(struct node **head){ // if the list is empty, returns NULL; otherwise returns a pointer to the last item on the list
	assert(head != NULL);
	
	if(*head == NULL){ // if the list is empty
		return NULL;
	}

	struct node *curr = *head;
	while(curr->next != NULL){
		curr = curr->next;
	}//end if
	
	return curr;

}

int list_delete(struct node **head) { //destroy the last item of a list. Returns 0 if the list is empty; returns 1 if deletion has been successfully carried out

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
	free((curr->next->threadContext->uc_stack).ss_sp);
	free(curr->next->threadContext);
	free(curr->next);
	curr->next = NULL;
	return 1;
}

int list_destroy_node(struct node **nd){ //returns 0 if the node does not exist; returns 1 if deletection is successfully carried out
	assert(nd != NULL);
	if(*nd == NULL){
		return 0;
	}	

	struct node *n = *nd;
	free((n->threadContext->uc_stack).ss_sp);
	free(n->threadContext);
	free(n);
	*nd = NULL;
	return 1;

}

int list_destroy_mainthread_node(struct node **md){ //returns 0 if the node does not exist; returns 1 if deletection is successfully carried out. This function differs from the normal list_destroy_node in that it does not need to free the node's thread context and its associated resources, since the memory for the thread context of main thread is not malloc'ed and need not be freed. 
	assert(md != NULL);
	if(*md == NULL){
		return 0;
	}	

	struct node *n = *md;
	free(n);
	*md = NULL;
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

void list_append_node(struct node *n, struct node **head){ //add a node to the beginning of the list, and changes the uc_link of the next node to point to the node that's just been added.
	assert(head != NULL);

	if(n == NULL){
		return;
	}
	
	struct node *tmp = NULL;
	if(*head != NULL){ // if the list is not empty
		tmp = *head;	
	}//end if

	*head = n;
	n->next = tmp;
	if(n->next != NULL){
		n->next->threadContext->uc_link = n->threadContext;
	}
}

void list_sema_append_node(struct node *n, struct node **head){ // add a node to the beginning of the list without changing the uc_link of the next node in line.
	assert(head != NULL);
	
	struct node *tmp = NULL;
	if(*head != NULL){ // if the list is not empty
		tmp = *head;	
	}//end if

	*head = n;
	n->next = tmp;
}

bool list_empty(struct node **head){
	assert(head != NULL);
	if(*head == NULL){
		return true;
	}//end if
	else{
		return false;
	}//end else
}


/* ***************************** 
     stage 1 library functions
   ***************************** */

void ta_libinit() {
	ready = list_init();
	waiting = list_init();
	getcontext(&mainthread);//stores the context of the current thread - namely the main thread - into mainthread
	
	//add the main thread to ready queue. Since the thread stored in the last item of the ready queue is the currently running thread, this append operation is done to reflect the fact that the main thread is the currently running thread and has thread number 0.
	list_append(&mainthread, threadNumber++, ready);

	return;
}

void ta_create(void (*func)(void *), void *arg) {
	#define STACKSIZE 128000
	unsigned char *stack_space = (unsigned char*)malloc(STACKSIZE);
	ucontext_t *ctx = malloc(sizeof(ucontext_t));
	getcontext(ctx);
	ctx->uc_stack.ss_sp = stack_space;
    ctx->uc_stack.ss_size = STACKSIZE;
	ctx->uc_link = &mainthread; 
	makecontext(ctx, (void (*)(void*))func, 1, arg);
	list_append(ctx, threadNumber++, ready);

	return;
}

void ta_yield() {

	struct node *yielded = list_pop(ready);

	if(yielded == NULL){
		return;
	}	

	yielded->threadContext->uc_link = &mainthread;
	list_append_node(yielded, ready);
	assert(!list_empty(ready));
	swapcontext(yielded->threadContext, list_last(ready)->threadContext);

    return;
}

int ta_waitall() {
	struct node *mainthread_node = list_pop(ready);//The last item of the ready queue holds the currently running thread. Since the main thread is running and we wish it to go to sleep, we need to "pop" it off the ready queue, and give the CPU to the next thread in queue.
	if(list_empty(ready) && list_empty(waiting)){
		return 0;
	}
	
	if(list_empty(ready) && !list_empty(waiting)){
		return -1;
	}

	while(!list_empty(ready)){
		swapcontext(&mainthread, list_last(ready)->threadContext);
		list_delete(ready); 
	}

	list_clear(ready);
	list_destroy_mainthread_node(&mainthread_node);
	free(ready);

	if(list_empty(waiting)){
		list_clear(waiting);
		free(waiting);
		return 0;
	}//end if
	else{
		list_clear(waiting);
		free(waiting);
		return -1;
	}//end else
	
}

/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {

	assert(sem != NULL);

	sem->queue = malloc(sizeof(struct node *));
	*(sem->queue) = NULL;
	sem->value = value;

}

void ta_sem_destroy(tasem_t *sem) {

	list_clear(sem->queue);	
	free(sem->queue);

}

void ta_sem_post(tasem_t *sem) {

	(sem->value)++; //increment the semaphore value by 1
	if(!list_empty(sem->queue)){
		struct node *waken = list_pop(sem->queue);
		assert(waken != NULL);
		list_append_node(waken, ready);
	}//end if
	
}

void ta_sem_wait(tasem_t *sem) {

	(sem->value)--;
	if(sem->value >= 0){
		// if the semaphore's value is greater or equal to 0 after decrementing, do nothing, allows the thread to keep running
	}//end if
	else{ //else, put the thread to sleep
		struct node *sleep = list_pop(ready);
		assert (sleep != NULL);
		list_append_node(sleep, sem->queue);
		ucontext_t current_context;
		if(!list_empty(ready)){ // if there is a thread waiting to be run, run that thread
			swapcontext(&current_context, list_last(ready)->threadContext);
		}//end inner if
		else{ //there is no thread to be run, go back to main thread
			swapcontext(&current_context, &mainthread);
		}//end inner else
	}//end else

}

void ta_lock_init(talock_t *mutex) {
	ta_sem_init(&(mutex->binary_sem), 1);
}

void ta_lock_destroy(talock_t *mutex) {
	ta_sem_destroy(&(mutex->binary_sem));
}

void ta_lock(talock_t *mutex) {
	ta_sem_wait(&mutex->binary_sem);
}

void ta_unlock(talock_t *mutex) {
	if(mutex->binary_sem.value == 1){ //if the lock is not locked, do nothing

	}
	else{
		ta_sem_post(&mutex->binary_sem);
	}

}


/* ***************************** 
     stage 3 library functions
   ***************************** */

void ta_cond_init(tacond_t *cond) {
	assert(cond != NULL);
	cond->queue = malloc(sizeof(struct node *));
	*(cond->queue) = NULL;

}

void ta_cond_destroy(tacond_t *cond) {
	list_clear(cond->queue);	
	free(cond->queue);

}

void ta_wait(talock_t *mutex, tacond_t *cond) {

	struct node *current_thread = list_pop(ready);
	list_append_node(current_thread, cond->queue);
	ta_unlock(mutex);
	if(!list_empty(ready)){
		swapcontext(current_thread->threadContext, list_last(ready)->threadContext);
	}
	else{
		swapcontext(current_thread->threadContext, &mainthread);
	}
	ta_lock(mutex);
}

void ta_signal(tacond_t *cond) {
	if(!list_empty(cond->queue)){
		struct node *waken = list_pop(cond->queue);
		list_append_node(waken, ready);
	}

}


