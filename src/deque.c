/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#include<stdio.h>
#include<stdlib.h>
#include "include/process.h"
#include "include/node.h"
#include "include/deque.h"

// return a new deque
struct deque *initDeque() {
    struct deque *dq = (struct deque *) malloc(sizeof(struct deque));
    dq->front = NULL;
    dq->rear = NULL;
    dq->size = 0;
    return dq;
}

// indicate whether the deque is empty or not
int isEmpty(struct deque *d) {
    if (d->front == NULL) {
        return 1;
    } else {
        return 0;
    }
}

// return the number of elements in the deque 
int getSize(struct deque *d) {
    return d->size;
}

// insert an element at the front end
void insertFront(struct deque *d, struct process *p) {
    struct node *newNode = createNewNode(p);
    // when deque is empty 
    if (isEmpty(d)) {
        d->rear = newNode;
        d->front = newNode;
    }
        // insert the node at the front end
    else {
        newNode->next = d->front;
        d->front->prev = newNode;
        d->front = newNode;
    }
    // increments count of elements by 1 
    d->size++;
}

// insert an element at the rear end
void insertRear(struct deque *d, struct process *p) {
    struct node *newNode = createNewNode(p);
    // when deque is empty 
    if (isEmpty(d)) {
        d->rear = newNode;
        d->front = newNode;
    }
        // inserts the node at the rear end
    else {
        newNode->prev = d->rear;
        d->rear->next = newNode;
        d->rear = newNode;
    }

    d->size++;

}

// delete the element from the front end 
void deleteFront(struct deque *d) {
    if (isEmpty(d)) {
        return;
    }
    struct node *temp = d->front;
    d->front = d->front->next;
    // when there is only one element
    if (d->front == NULL) {
        d->rear = NULL;
    } else {
        d->front->prev = NULL;
    }
    free(temp);
    d->size--;
}

// delete the element from the rear end 
void deleteRear(struct deque *d) {
    if (isEmpty(d)) {
        return;
    }
    struct node *temp = d->rear;
    d->rear = d->rear->prev;
    //  when there is only one element
    if (d->rear == NULL) {
        d->front = NULL;
    } else {
        d->rear->next = NULL;
    }
    free(temp);
    d->size--;
}

// return the element at the front end 
struct process *getFront(struct deque *d) {
    // if deque is empty, then returns null
    if (isEmpty(d)) {
        return NULL;
    }
    return d->front->data;
}

// return the element at the rear end 
struct process *getRear(struct deque *d) {
    // if deque is empty, then returns null
    if (isEmpty(d)) {
        return NULL;
    }
    return d->rear->data;
}

// delete all the elements from Deque 
void freeAll(struct deque *d) {
    d->rear = NULL;
    while (d->front != NULL) {
        struct node *temp = d->front;
        d->front = d->front->next;
        free(temp);
    }
    d->size = 0;
}

void sortDequeById(struct deque *d) {
    for (struct node *i = d->front; i != NULL; i = i->next) {
        for (struct node *j = i->next; j != NULL; j = j->next) {
            if (i->data->id > j->data->id) {
                struct process *temp = i->data;
                i->data = j->data;
                j->data = temp;
            }
        }
    }
}

void printDeque(struct deque *d) {
    for (struct node *i = d->front; i != NULL; i = i->next) {
        printf("id is %lld\n", i->data->id);
    }
}



