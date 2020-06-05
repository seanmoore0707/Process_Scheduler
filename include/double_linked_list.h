/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#ifndef DOUBLE_LINKED_LIST_H
#define DOUBLE_LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include "include/memory.h"

struct list_node {
    struct memory_unit *data;
    struct list_node *next;
    struct list_node *prev;
};

// the struct doubly linked list is used to store memory pages in memory management (in "p" option)
struct doubly_list {
    struct list_node *head;
    struct list_node *tail;
    long long int size;
    long long int memory_size;
    long long int curr_used;
};

struct list_node *createListNode(struct memory_unit *m);

struct doubly_list *initDlist();

int isListEmpty(struct doubly_list *d);

long long int getMemSize(struct doubly_list *dl);

void setMemSize(struct doubly_list *dl, long long int n);

long long int getListSize(struct doubly_list *d);

void insertStart(struct doubly_list *d, struct memory_unit *p);

void insertEnd(struct doubly_list *d, struct memory_unit *p);

// int node_cmp(struct list_node* a, struct list_node* b);
void insertAfter(struct doubly_list *dl, struct list_node *after, struct memory_unit *m);

void deleteStart(struct doubly_list *d);

void deleteEnd(struct doubly_list *d);

void deleteNode(struct doubly_list *dl, struct list_node *remove);

struct memory_unit *getStart(struct doubly_list *d);

struct memory_unit *getEnd(struct doubly_list *d);

void freeList(struct doubly_list *d);

void printList(struct doubly_list *dl);

#endif