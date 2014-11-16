#include "tList.h"


struct node **list_init(){
	struct node **head = malloc(sizeof(struct node *));
	*head = NULL;
	return head;
}

void list_clear(struct node *list) {
    while (list != NULL) {
        struct node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

void list_print(const struct node *list) {
    printf("Thread list_print\n");
    while (list != NULL) {
        printf("Thread %d\n", list->threadNum);
        list = list->next;
    }
}//end list_print

int list_delete(struct node **head) { //delete the last item of a list

	if(head == NULL){
		printf("Error! List_delete() receives a NULL node**.\n");
		return -1;
	}//end if

	if(*head == NULL){ //if the list is empty, return 0
		return 0;
	}//end if

	struct node *curr = *head;
	if(curr->next == NULL){ //if the list contains only 1 item, delete it and then return 1
		free(curr);
		*head = NULL;
		return 1;
	}//end if

	//if the list contains 2 or more items
	while(curr->next->next != NULL){
		curr = curr->next;
	}//end while
	free(curr->next);
	curr->next = NULL;
	return 1;
}

int list_append(ucontext_t *ctx, int threadNum, struct node **head) { //add an item to the head of the list

	if(head == NULL){
		printf("Error! List_append() receives a NULL node**.\n");
		return -1;
	}//end if

	struct node *tmp = NULL;
	if(*head != NULL){ // if the list is not empty
		tmp = *head;	
	}//end if
	*head = malloc(sizeof(struct node));
	(*head)->threadContext = ctx;
	(*head)->threadNum = threadNum;
	(*head)->next = tmp;
	return 1;
}
