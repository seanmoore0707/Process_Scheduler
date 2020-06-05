/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <math.h>
#include "include/process.h"
#include "include/double_linked_list.h"
#include "include/memory.h"
#include "include/helper.h"
#include "include/vm_array.h"
#include "include/node.h"
#include "include/deque.h"

/* Constants */
#define SEG_SIZE 4
#define UNOCCUPIED 0
#define OCCUPIED 1
#define LOADING_TIME 2
#define FAKE_PID -1
#define FAKE_ACCESS_TIME -1
#define FAKE_PAGE_NUM -1
#define PAGE_FAULT_PENALTY 1
#define MEMORY_THREAD 16

/** Convert byte memory size to page length.
 * @param memsize byte memory size
 * @return length of memory page
 * */
long long int getPageLength(long long int memsize) {
    return memsize / SEG_SIZE;
}

//////////////////////////////// helper function for Swapping Memory Management/////////////////////////////////////////////////////////////
/** Combine the node with its next node in memory list to form a new node,
 * the combination aims to combine the byte length and page length of the node.
 * @param dl memory list --- doubly linked list
 * @param node_evict the node whose memory needs to be evicted
 * @return the new combined node
 * */
struct list_node *combineWithNext(struct doubly_list *dl, struct list_node *node_evict) {
    assert(dl && node_evict);
    struct memory_unit *m = node_evict->data;
    struct memory_unit *m_next = node_evict->next->data;
    assert(m->type == UNOCCUPIED && m_next->type == UNOCCUPIED);
    m->byte_len += m_next->byte_len;
    m->page_len = getPageLength(m->byte_len);
    struct list_node *res = node_evict;
    deleteNode(dl, node_evict->next);
    return res;
}

/** Combine the node with its next node in memory list to form a new node,
 * the combination aims to combine the byte length and page length of the node.
 * @param dl memory list --- doubly linked list
 * @param node_evict the node whose memory needs to be evicted
 * @return the new combined node
 * */
struct list_node *combineWithPrev(struct doubly_list *dl, struct list_node *node_evict) {
    assert(dl && node_evict);
    struct memory_unit *m = node_evict->data;
    struct memory_unit *m_prev = node_evict->prev->data;
    assert(m->type == UNOCCUPIED && m_prev->type == UNOCCUPIED);
    m_prev->byte_len += m->byte_len;
    m_prev->page_len = getPageLength(m_prev->byte_len);
    struct list_node *res = node_evict->prev;
    deleteNode(dl, node_evict);
    return res;
}

/** Print the page number given with the page start index and number of pages.
 * @param page_begin start index of page
 * @param page_len number of pages
 * */
void print_page_num(long long int page_begin, long long int page_len) {
    long long int *arr = malloc(sizeof(*arr) * page_len);
    for (long long int i = 0; i < page_len; i++) {
        arr[i] = page_begin + i;
    }
    for (long long int i = 0; i < page_len; i++) {
        if (i == 0) {
            printf("%lld", arr[i]);
        } else {
            printf(",%lld", arr[i]);
        }
    }
    free(arr);
}

/** return the node after its memory has been evicted.
 * @param node_evict node whose memory needs to be evicted
 * @param dl memory list --- doubly linked list
 * @return the node after evicting the memory
 * */
struct list_node *evictMemory(struct doubly_list *dl, struct list_node *node_evict) {
    struct list_node *temp = node_evict;
    struct memory_unit *m = node_evict->data;
    assert(m->type == OCCUPIED);
    // node_evict->data->type = UNOCCUPIED;
    // node_evict->data->last_access = FAKE_ACCESS_TIME;
    // node_evict->data->process_id = FAKE_PID;
    temp->data->type = UNOCCUPIED;
    temp->data->last_access = FAKE_ACCESS_TIME;
    temp->data->process_id = FAKE_PID;
    dl->curr_used -= temp->data->page_len;
    // if the previous node's memory is not occupied by any process, combine this node with it.
    if (temp->prev != NULL) {
        struct memory_unit *m_prev = temp->prev->data;
        if (m_prev->type == UNOCCUPIED) {
            temp = combineWithPrev(dl, temp);
        }
    }
    // if the next node's memory is not occupied by any process, combine this node with it.
    if (temp->next != NULL) {
        struct memory_unit *m_next = temp->next->data;
        if (m_next->type == UNOCCUPIED) {
            temp = combineWithNext(dl, temp);
        }
    }
    assert(dl->head != NULL);
    return temp;
}

/** Allocate the memory of process on a specific node.
 * @param dl memory list --- doubly linked list
 * @param target the node where the memory of process should be allocated
 * @param p the process to be allocated with memory
 * @param time current time
 * */
void allocateMemory(struct doubly_list *dl, struct list_node *target, struct process *p, long long int time) {
    struct memory_unit *m = target->data;
    long long int page_requested = getPageLength(p->memorySize);

    // when allocating memory, we split a part of memory from a suitable memory node, thus causing one node which
    // holds the memory of process and a new unoccupied node.
    struct memory_unit *temp = createMemory(UNOCCUPIED, (m->page_begin + page_requested),
                                            getPageLength(m->byte_len - p->memorySize), \
                              (m->byte_begin + p->memorySize), (m->byte_len - p->memorySize));
    insertAfter(dl, target, temp);
    m->type = OCCUPIED;
    m->page_len = page_requested;
    m->byte_len = p->memorySize;
    m->process_id = p->id;
    m->loading_time = LOADING_TIME * page_requested;
    dl->curr_used += m->page_len;
    long long int percentage = ceil(((double) (dl->curr_used) * 100 / (double) (getPageLength(getMemSize(dl)))));
    printf("%lld, RUNNING, id=%lld, remaining-time=%lld, load-time=%lld, mem-usage=%lld%%, mem-addresses=[",
           time, p->id, (p->expectedEnd + p->execTime - time), m->loading_time, percentage);
    print_page_num(m->page_begin, m->page_len);
    printf("]\n");
    p->expectedEnd += m->loading_time;
}

/** Find the first fit node to allocate process's memory.
 * @param dl memory list --- doubly linked list
 * @param p the process to be allocated with memory
 * @return the first node in memory list that is suitable to allocate process memory
 * */
struct list_node *firstFit(struct doubly_list *dl, struct process *p) {
    assert(dl);
    assert(p);
    long long int page_requested = getPageLength(p->memorySize);
    struct list_node *current = dl->head;
    while (current != NULL) {
        struct memory_unit *temp = current->data;
        if (temp->type == UNOCCUPIED && temp->page_len >= page_requested) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/** Find the first node in memory list that holds process's memory (memory that is used by target process).
 * Actually, in Swapping situation, usually one process only occupy one node in memory list.
 * @param dl memory list --- doubly linked list
 * @param p the target process
 * @return the first node in memory list that holds target process's memory
 * */
struct list_node *firstUse(struct doubly_list *dl, struct process *p) {
    struct list_node *current = dl->head;
    while (current != NULL) {
        struct memory_unit *temp = current->data;
        if (temp->type == OCCUPIED && temp->process_id == p->id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/** Update the last_access time on memory.
 * @param dl memory list --- doubly linked list
 * @param p the target process
 * @param time current time
 * */
void updateLastAccessTime(struct doubly_list *dl, struct process *p, long long int time) {
    struct list_node *current = dl->head;
    while (current != NULL) {
        struct memory_unit *temp = current->data;
        if (temp->type == OCCUPIED && temp->process_id == p->id) {
            temp->last_access = time;
        }
        current = current->next;
    }
}

/** Initiate the memory list.
 * @param dl memory list --- doubly linked list
 * @param maxMem the total byte memory in system
 * */
void initListOnMemory(struct doubly_list *dl, long long int maxMem) {
    assert(dl);
    setMemSize(dl, maxMem);
    struct memory_unit *m = createMemory(UNOCCUPIED, 0, getPageLength(maxMem), 0, maxMem);
    insertEnd(dl, m);
}

/** Get the Least-Recently-Used process's memory node
 * @param dl memory list --- doubly linked list
 * @return the Least-Recently-Used process's memory node
 * */
struct list_node *get_node_LRU(struct doubly_list *dl) {
    assert(dl);
    struct list_node *temp = dl->head;
    struct memory_unit* memory = NULL;
    struct list_node* swap = NULL;
    long long int least_time = FAKE_ACCESS_TIME;

    while(temp!=NULL){
        memory = temp->data;
        if(memory->type == OCCUPIED){
            if(swap==NULL){
                least_time = memory->last_access;
                swap = temp;
            }
            if(memory->last_access<least_time){
                least_time = memory->last_access;
                swap = temp;
            }
        }
        temp = temp->next;
    }
    return swap;
}

/** Release the memory of target process
 * @param dl memory list --- doubly linked list
 * @param current current target process
 * @param time current time
 * */
void deallocate_memory(struct doubly_list *dl, struct process *current, long long int time) {
    assert(dl != NULL && current != NULL);
    struct list_node *temp = dl->head;
    while (temp != NULL) {
        struct memory_unit *m = temp->data;
        if (m->type == OCCUPIED && m->process_id == current->id) {
            printf("%lld, EVICTED, mem-addresses=[", time);
            print_page_num(temp->data->page_begin, temp->data->page_len);
            printf("]\n");
            temp = evictMemory(dl, temp);
        } else {
            temp = temp->next;
        }
    }
}

/** Print the memory usage statistics of target process.
 *  @param dl memory list --- doubly linked list
 *  @param temp the target node that holds process's memory
 *  @param p the target process
 *  @param time current time
 * */
void printUsage(struct doubly_list *dl, struct list_node *temp, struct process *p, long long int time) {
    long long int percentage = ceil(((double) (dl->curr_used) * 100 / (double) (getPageLength(getMemSize(dl)))));
    printf("%lld, RUNNING, id=%lld, remaining-time=%lld, load-time=%d, mem-usage=%lld%%, mem-addresses=[",
           time, p->id, (p->expectedEnd + p->execTime - time), 0, percentage);
    print_page_num(temp->data->page_begin, temp->data->page_len);
    printf("]\n");
}

/** Return the min value of two numbers 
 * */
long long int min(long long int num1, long long int num2) {
    return (num1 > num2) ? num2 : num1;
}


//////////////////////////////// helper function for VM Memory Management/////////////////////////////////////////////////////////////

/** Comparator function for quick sort
 * */
int cmpfunc(const void *a, const void *b) {
    return (*(long long int *) a - *(long long int *) b);
}

/** Print the virtual memory page of a process
 * @param p target process
 * */
void print_vm_page(struct process *p) {
    int arr_len = getPageLength(p->memorySize);
    long long int *temp = (long long int *) malloc(sizeof(long long int) * arr_len);
    for (long long int i = 0; i < arr_len; i++) {
        temp[i] = p->memory_array[i];
    }
    int flag = 0;
    qsort(temp, arr_len, sizeof(long long int), cmpfunc);

    for (long long int j = 0; j < arr_len; j++) {
        if (temp[j] != -1) {
            if (flag == 0) {
                printf("%lld", temp[j]);
                flag = 1;
            } else {
                printf(",%lld", temp[j]);
            }
        }
    }
    free(temp);
}

/** Print the evicted memory pages from a process
 * @param arr the array to store the evicted memory page
 * @param time current time
 * @param len the length of array that stores the evicted memory page
 * */
void print_evict_page(long long int *arr, long long int time, long long int len) {
    long long int *temp = (long long int *) malloc(sizeof(long long int) * len);
    for (long long int i = 0; i < len; i++) {
        // printf("page for arr %lld at time %lld\n", arr[i], time);
        temp[i] = arr[i];
    }
    printf("%lld, EVICTED, mem-addresses=[", time);
    int flag = 0;
    qsort(temp, len, sizeof(long long int), cmpfunc);
    for (long long int j = 0; j < len; j++) {
        if (flag == 0) {
            printf("%lld", temp[j]);
            flag = 1;
        } else {
            printf(",%lld", temp[j]);
        }
    }
    printf("]\n");
    free(temp);
}

/** Return the process id of the least-recently-used process
 * @param vm the virtual memory manager
 * @param p target process
 * @return the least-recently-used process id
 * */
long long int get_LRU_process(struct vm_manager *vm, struct process *p) {
    long long int temp = -1;
    for (long long int i = 0; i < vm->total_num; i++) {
        if (vm->total_memory[i] >= 0 && vm->time_recorder[0] >= 0 && vm->total_memory[i] != p->id) {
            temp = i;
            break;
        }
    }

    long long int lru_id = vm->total_memory[temp];
    long long int lru_time = vm->time_recorder[temp];
    for (long long int i = 0; i < vm->total_num; i++) {
        if (vm->time_recorder[i] < lru_time && vm->total_memory[i] != p->id) {
            lru_id = vm->total_memory[i];
            lru_time = vm->time_recorder[i];
        }
    }
    return lru_id;
}
/** Return the page to evict from the least-recently-used process (used in "v" option)
 * @param vm the virtual memory manager
 * @param p the least-recently-used process
 * @return evicted page index
 * */
long long int get_LRU_VM_page(struct vm_manager *vm, struct process *p) {
    long long int id = get_LRU_process(vm, p);
    assert(id != p->id);
    for (long long int i = 0; i < vm->total_num; i++) {
        if (vm->total_memory[i] == id) {
            return i;
        }
    }
    return -1;
}

/** Return the page to evict from the least-frequently-used and least-recently-used page (used in "cm" option)
 * assign each page a score based on their last access time and how many times it is used
 * @param vm the virtual memory manager
 * @return evicted page index
 * */
long long int get_LFRU_VM_page(struct vm_manager *vm) {
    long long int page_num = FAKE_PAGE_NUM;
    // long long int min_times = vm->used_times[0];
    double min_times = 0.6*(double)vm->used_times[0] + 0.4*(double)vm->time_recorder[0];
    for(long long int i=0; i<vm->total_num; i++){
        if(min_times>(0.6*(double)vm->used_times[i] + 0.4*(double)vm->time_recorder[i])){
            min_times = 0.6*(double)vm->used_times[i] + 0.4*(double)vm->time_recorder[i];
        }
        // if(min_times>vm->used_times[i]){
        //     min_times = vm->used_times[i];
        // }
    }
    for(long long int i=0; i<vm->total_num; i++){
        if( (0.6*(double)vm->used_times[i] + 0.4*(double)vm->time_recorder[i]) == min_times){
        // if(vm->used_times[i] == min_times){
            page_num = i;
            break;
        }
    }
    return page_num;

}

/** Return the number of vm pages allocated to the targe process
 * @param p target process
 * @return the number of pages allocated to the target process
 * */
long long int get_vm_allocated(struct process *p) {
    long long int res = 0;
    for (long long int i = 0; i < getPageLength(p->memorySize); i++) {
        if (p->memory_array[i] != FAKE_PAGE_NUM) {
            res++;
        }
    }
    return res;
}

/** Evict the target page from the target process
 * @param p target process
 * @param pos target page index
 */
void remove_page_num(struct process *p, long long int pos) {
    for (long long int i = 0; i < getPageLength(p->memorySize); i++) {
        if (p->memory_array[i] == pos) {
            p->memory_array[i] = FAKE_PAGE_NUM;
        }
    }
}

/** Allocate vm page memory for a process, no need to evict pages
 * @param vm the virtual memory manager
 * @param p target process
 * @param time current time
 * @param requested the number of pages required by the process
 * */
void allocateFullMemory(struct vm_manager *vm, struct process *p, long long int time, long long int requested) {
    long long int index = 0;
    vm->free_num -= requested;
    long long int counter = requested;
    long long int percentage = ceil(((double) (vm->total_num - vm->free_num) * 100 / (double) (vm->total_num)));
    long long int load = requested * LOADING_TIME;
    printf("%lld, RUNNING, id=%lld, remaining-time=%lld, load-time=%lld, mem-usage=%lld%%, mem-addresses=[",
           time, p->id, (p->expectedEnd + p->execTime + p->penalty - time), load, percentage);
    p->expectedEnd += load;
    if (p->flag == 1) {
        p->expectedEnd += p->penalty;
    }

    for (long long int i = 0; i < vm->total_num; i++) {
        if (vm->total_memory[i] == -1 && vm->time_recorder[i] == -1) {
            while (p->memory_array[index] != FAKE_PAGE_NUM) {
                index++;
            }
            p->memory_array[index] = i;
            vm->total_memory[i] = p->id;
            vm->time_recorder[i] = time;
            vm->used_times[i]+=1;
            counter--;
            if (counter == 0) {
                break;
            }
        }
    }

    print_vm_page(p);
    printf("]\n");


}

/** Allocate vm page memory for a process, need to evict pages
 * @param vm the virtual memory manager
 * @param p target process
 * @param time current time
 * @param remain number of pages that allocated to the process without evicting vm pages
 * @param toEvict number of pages to evict from memory
 * @param processor the deque that stores processes in scheduler
 * @param mem_flag the flag indicating the memory management policy --- "v" or "cm"
 * */
void allocateMixMemory(struct vm_manager *vm, struct process *p, long long int time, long long int remain,
                       long long int toEvict, \
                       struct deque *processor, char* mem_flag) {
    long long int index = 0;
    vm->free_num -= remain;
    long long int counter = remain;
    long long int counter_evict = toEvict;
    long long int percentage = ceil(((double) (vm->total_num - vm->free_num) * 100 / (double) (vm->total_num)));
    long long int load = toEvict * LOADING_TIME;

    for (long long int i = 0; i < vm->total_num; i++) {
        if (vm->total_memory[i] == -1 && vm->time_recorder[i] == -1) {
            while (p->memory_array[index] != FAKE_PAGE_NUM) {
                index++;
            }
            p->memory_array[index] = i;
            vm->total_memory[i] = p->id;
            vm->time_recorder[i] = time;
            vm->used_times[i]+=1;
            counter--;
            if (counter == 0) {
                break;
            }
        }
    }

    long long int *arr = (long long int *) malloc(sizeof(long long int) * counter_evict);
    long long int k = 0;
    long long int n = counter_evict;
    while (counter_evict > 0) {
        long long int pos = -1;
        if(strcmp(mem_flag,"v")==0){
            pos = get_LRU_VM_page(vm, p);
        }else{
            pos = get_LFRU_VM_page(vm);
            assert(pos!=-1);
        }
        if (pos != -1) {
            arr[k++] = pos;
            while (p->memory_array[index] != FAKE_PAGE_NUM) {
                index++;
            }
            p->memory_array[index] = pos;
            index = 0;

            long long int temp_id = vm->total_memory[pos];
            index++;
            // evict the vm page from the corresponding process
            for (struct node *j = processor->front; j != NULL; j = j->next) {
                if (j->data->id == temp_id) {
                    remove_page_num(j->data, pos);
                }
            }
            vm->total_memory[pos] = p->id;
            vm->time_recorder[pos] = time;
            vm->used_times[pos]+=1;
            counter_evict--;
        } else {
            printf("VM error, fail to get LRU VM page, process: %lld\n", p->id);
        }
    }
    print_evict_page(arr, time, n);


    printf("%lld, RUNNING, id=%lld, remaining-time=%lld, load-time=%lld, mem-usage=%lld%%, mem-addresses=[",
           time, p->id, (p->expectedEnd + p->execTime + p->penalty - time), load, percentage);
    p->expectedEnd += load;
    if (p->flag == 1) {
        p->expectedEnd += p->penalty;
    }
    print_vm_page(p);
    printf("]\n");


}

/** Deallocate the vm pages from the target process when the process finishes
 * @param vm the virtual memory manager
 * @param p target process
 * @param time current time
 * */
void deallocate_memory_vm(struct vm_manager *vm, struct process *p, long long int time) {
    long long int page_num = 0;
    for (long long int i = 0; i < getPageLength(p->memorySize); i++) {
        if (p->memory_array[i] != FAKE_PAGE_NUM) {
            page_num++;
        }
    }
    printf("%lld, EVICTED, mem-addresses=[", time);
    vm->free_num += page_num;
    print_vm_page(p);
    printf("]\n");
    for (long long int i = 0; i < getPageLength(p->memorySize); i++) {
        p->memory_array[i] = FAKE_PAGE_NUM;
    }
    for (long long int j = 0; j < vm->total_num; j++) {
        if (vm->total_memory[j] == p->id) {
            vm->total_memory[j] = FAKE_PID;
            vm->time_recorder[j] = FAKE_ACCESS_TIME;
        }
    }

}



