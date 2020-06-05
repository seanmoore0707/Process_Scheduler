/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <math.h>
#include "include/process.h"
#include "include/double_linked_list.h"
#include "include/memory.h"
#include "include/vm_array.h"
#include "include/node.h"
#include "include/deque.h"

//////////////////////////////// helper function for Swapping Memory Management/////////////////////////////////////////////////////////////
long long int getPageLength(long long int memsize);

struct list_node *combineWithNext(struct doubly_list *dl, struct list_node *node_evict);

struct list_node *combineWithPrev(struct doubly_list *dl, struct list_node *node_evict);

void print_page_num(long long int page_begin, long long int page_len);

struct list_node *evictMemory(struct doubly_list *dl, struct list_node *node_evict);

void allocateMemory(struct doubly_list *dl, struct list_node *target, struct process *p, long long int time);

struct list_node *firstFit(struct doubly_list *dl, struct process *p);

struct list_node *firstUse(struct doubly_list *dl, struct process *p);

void updateLastAccessTime(struct doubly_list *dl, struct process *p, long long int time);

void initListOnMemory(struct doubly_list *dl, long long int maxMem);

struct list_node *get_node_LRU(struct doubly_list *dl);

void deallocate_memory(struct doubly_list *dl, struct process *current, long long int time);

void printUsage(struct doubly_list *dl, struct list_node *temp, struct process *p, long long int time);


//////////////////////////////// helper function for VM Memory Management/////////////////////////////////////////////////////////////
int cmpfunc(const void *a, const void *b);

void print_vm_page(struct process *p);

void print_evict_page(long long int *arr, long long int time, long long int len);

long long int get_LRU_process(struct vm_manager *vm, struct process *p);

long long int get_LRU_VM_page(struct vm_manager *vm, struct process *p);

long long int get_LFRU_VM_page(struct vm_manager *vm);

long long int get_vm_allocated(struct process *p);

void remove_page_num(struct process *p, long long int pos);

void allocateFullMemory(struct vm_manager *vm, struct process *p, long long int time, long long int requested);

void allocateMixMemory(struct vm_manager *vm, struct process *p, long long int time, long long int remain,
                       long long int toEvict, \
                       struct deque *processor, char* mem_flag);

void deallocate_memory_vm(struct vm_manager *vm, struct process *p, long long int time);

#endif
