/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include "include/memory.h"
#include "include/double_linked_list.h"

//Creates a new Node and returns pointer to it. 
struct list_node *createListNode(struct memory_unit *m) {
    struct list_node *newNode = (struct list_node *) malloc(sizeof(struct list_node));
    newNode->data = m;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

// return a new double linked list
struct doubly_list *initDlist() {
    struct doubly_list *dl = (struct doubly_list *) malloc(sizeof(struct doubly_list));
    dl->head = NULL;
    dl->tail = NULL;
    dl->size = 0;
    dl->curr_used = 0;
    return dl;
}

// indicate whether the double linked list is empty or not
int isListEmpty(struct doubly_list *dl) {
    if (dl->head == NULL) {
        return 1;
    } else {
        return 0;
    }
}

// return the number of pages left so far in memory
long long int getMemSize(struct doubly_list *dl) {
    return dl->memory_size;
}

void setMemSize(struct doubly_list *dl, long long int n) {
    dl->memory_size = n;
}

// return the number of elements in the double linked list
long long int getListSize(struct doubly_list *dl) {
    return dl->size;
}

// insert an element at the head end
void insertStart(struct doubly_list *dl, struct memory_unit *m) {
    struct list_node *newNode = createListNode(m);
    // when list is empty 
    if (dl->head == NULL) {
        dl->tail = newNode;
        dl->head = newNode;
    }
        // insert the node at the head end
    else {
        newNode->next = dl->head;
        dl->head->prev = newNode;
        dl->head = newNode;
    }
    // increments count of elements by 1 
    dl->size++;
}

// insert an element at the tail end
void insertEnd(struct doubly_list *dl, struct memory_unit *m) {
    struct list_node *newNode = createListNode(m);
    // when list is empty 
    if (dl->tail == NULL) {
        dl->tail = newNode;
        dl->head = newNode;
    }
        // inserts the node at the tail end
    else {
        newNode->prev = dl->tail;
        dl->tail->next = newNode;
        dl->tail = newNode;
    }

    dl->size++;

}


// given a node, insert a new node after the given node 
void insertAfter(struct doubly_list *dl, struct list_node *after, struct memory_unit *m) {
    assert(dl != NULL);
    assert(dl->size != 0);

    struct list_node *curr = dl->head;
    while (curr != NULL) {
        if (curr == after) {
            struct list_node *new_node = (struct list_node *) malloc(sizeof(struct list_node));
            new_node->data = m;
            new_node->prev = curr;
            new_node->next = curr->next;
            if (curr->next != NULL) {
                curr->next->prev = new_node;
            }
            curr->next = new_node;
            if (new_node->prev == NULL) {
                dl->head = new_node;
            } else if (new_node->next == NULL) {
                dl->tail = new_node;
            }
            dl->size++;
            return;
        }
        curr = curr->next;
    }

}


// delete the element from the head end 
void deleteStart(struct doubly_list *dl) {
    if (isListEmpty(dl)) {
        return;
    }
    struct list_node *temp = dl->head;
    dl->head = dl->head->next;
    // when there is only one element
    if (dl->head == NULL) {
        dl->tail = NULL;
    } else {
        dl->head->prev = NULL;
    }
    free(temp);
    dl->size--;
}

// delete the element from the tail end 
void deleteEnd(struct doubly_list *dl) {
    if (isListEmpty(dl)) {
        return;
    }
    struct list_node *temp = dl->tail;
    dl->tail = dl->tail->prev;
    //  when there is only one element
    if (dl->tail == NULL) {
        dl->head = NULL;
    } else {
        dl->tail->next = NULL;
    }
    free(temp);
    dl->size--;
    assert(dl->head != NULL);
}


// given a node, delete the node
void deleteNode(struct doubly_list *dl, struct list_node *remove) {
    assert(dl != NULL);
    assert(dl->size > 0);
    assert(remove != NULL);

    if (remove->prev != NULL) {
        remove->prev->next = remove->next;
    } else {
        dl->head = remove->next;
    }
    if (remove->next != NULL) {
        remove->next->prev = remove->prev;
    } else {
        dl->tail = remove->prev;
    }
    dl->size--;
    free(remove);
}

// return the element at the head end 
struct memory_unit *getStart(struct doubly_list *dl) {
    // if list is empty, then returns null
    if (isListEmpty(dl)) {
        return NULL;
    }
    return dl->head->data;
}

// return the element at the tail end 
struct memory_unit *getEnd(struct doubly_list *dl) {
    // if list is empty, then returns null
    if (isListEmpty(dl)) {
        return NULL;
    }
    return dl->tail->data;
}

// delete all the elements from list 
void freeList(struct doubly_list *dl) {
    dl->tail = NULL;
    while (dl->head != NULL) {
        struct list_node *temp = dl->head;
        dl->head = dl->head->next;
        free(temp);
    }
    dl->size = 0;
}


void printList(struct doubly_list *dl) {
    for (struct list_node *i = dl->head; i != NULL; i = i->next) {
        printf("[ id: %lld, occupied: %d, page_begin: %lld, page_len: %lld, byte_begin: %lld, byte_len: %lld,\
        process_id: %lld, last_access: %lld, loading_time: %lld ]\n", i->data->process_id, i->data->type, \
        i->data->page_begin, i->data->page_len, i->data->byte_begin, i->data->byte_len, i->data->process_id, \
        i->data->last_access, i->data->loading_time);
    }
}
