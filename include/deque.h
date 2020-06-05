/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#ifndef DEQUE_H
#define DEQUE_H

#include <stdio.h>
#include <stdlib.h>
#include "include/process.h"
#include "include/node.h"
// the struct deque is used to store process in process scheduling
struct deque {
    struct node *front;
    struct node *rear;
    int size;
};

struct deque *initDeque();

void insertFront(struct deque *d, struct process *p);

void insertRear(struct deque *d, struct process *p);

void deleteFront(struct deque *d);

void deleteRear(struct deque *d);

struct process *getFront(struct deque *d);

struct process *getRear(struct deque *d);

int getSize(struct deque *d);

int isEmpty(struct deque *d);

void freeAll(struct deque *d);

void sortDequeById(struct deque *d);

void printDeque(struct deque *d);


#endif
