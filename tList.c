#include "tList.h"

//format:
//an empty list: (*head) == NULL
//a non-empty list: the last node has node->next = NULL;

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

/*
int main(){
	ucontext_t *ctx1 = malloc(sizeof(ucontext_t));
	ucontext_t *ctx2 = malloc(sizeof(ucontext_t));
	ucontext_t *ctx3 = malloc(sizeof(ucontext_t));

	getcontext(ctx1);
	getcontext(ctx2);
	getcontext(ctx3);
	struct node **head = list_init();
	list_append(ctx1, 1, head);
	list_append(ctx2, 2, head);
	list_append(ctx3, 3, head);
	list_print(*head);
	
	struct node *n = list_pop(head);
	list_print(n);
	list_append_node(n, head);
	list_print(*head);
	list_clear(head);

	free(head);

	return 0;
}//end main
*/

