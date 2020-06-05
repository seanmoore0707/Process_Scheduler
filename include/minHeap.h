/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#ifndef MINHEAP_H
#define MINHEAP_H

#include <stdio.h>
#include <stdlib.h>
#include "include/process.h"

typedef struct heapNode {
    struct process *data;
    struct heapNode *left;
    struct heapNode *right;
    struct heapNode *parent;
} heapNode;

typedef struct heapQueue {
    struct heapQueue *next;
    heapNode *element;
} heapQueue;

struct heapNode *create_node(struct process *p, struct heapNode *parent);

struct heapQueue *create_queue(struct heapNode *element);

void free_heapQueue(struct heapQueue **q);

void heapQueue_put(struct heapNode *n, struct heapQueue **last);

void heap_insert(struct heapNode **root, struct process *x);

struct process *heap_remove(heapNode **root);

void print_heap(struct heapNode **root);


#endif
