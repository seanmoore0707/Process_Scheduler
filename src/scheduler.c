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
#include "include/node.h"
#include "include/deque.h"
#include "include/minHeap.h"
#include "include/double_linked_list.h"
#include "include/memory.h"
#include "include/helper.h"
#include "include/vm_array.h"
#include "include/recorder.h"

/* Constants */
#define DEFAULT_TIME 10
#define FILE_FORMAT ".txt"
#define LINE_FORMAT "%lld %lld %lld %lld\n"
#define MEMORY_THREAD 16
#define FAKE_PAGE_NUM -1
#define PAGE_FAULT_PENALTY 1

/** Execute virtual memory management for target process (in "v" and "cm" memory management options)
* @param vm virtual memory manager
* @param p target process
* @param time current time
* @param processor the deque to store processes that are waiting to execute
* @param mem_flag indicate whether it is "v" or "cm" memory management option
*/
void executeVMAllocation(struct vm_manager *vm, struct process *p, long long int time, struct deque *processor, char* mem_flag) {


    long long int remain = vm->free_num;
    long long int requested = getPageLength(p->memorySize);
    long long int actual = 0;
    for (long long int i = 0; i < requested; i++) {
        if (p->memory_array[i] != FAKE_PAGE_NUM) {
            actual++;
        }
    }
    // if the target process is already assigned more than 4 pages and there is no empty pages currently,
    // then no need to assign more pages to the process.
    if(actual >= getPageLength(MEMORY_THREAD) && vm->free_num == 0){
        long long int percentage = ceil(((double) (vm->total_num - vm->free_num) * 100 / (double) (vm->total_num)));
        printf("%lld, RUNNING, id=%lld, remaining-time=%lld, load-time=%d, mem-usage=%lld%%, mem-addresses=[",
               time, p->id, (p->expectedEnd + p->execTime - time), 0, percentage);
        print_vm_page(p);
        printf("]\n");
        return;
    }

    // record the number of pages still needed by the process
    long long int still_need = 0;
    if (actual > 0) {
        if (remain + actual >= requested) {
            still_need = requested - actual;
        } else {
            still_need = getPageLength(MEMORY_THREAD) - actual;
        }
    } else {
        still_need = requested - actual;
    }

    // if the target process doesn't need more pages, then just print its usage.
    if (still_need == 0) {
        long long int percentage = ceil(((double) (vm->total_num - vm->free_num) * 100 / (double) (vm->total_num)));
        printf("%lld, RUNNING, id=%lld, remaining-time=%lld, load-time=%d, mem-usage=%lld%%, mem-addresses=[",
               time, p->id, (p->expectedEnd + p->execTime - time), 0, percentage);
        print_vm_page(p);
        printf("]\n");
        return;
    }

    // if the remaining memory pages in system are more than what the process needed, then directly allocate pages,
    // no need to evict memory pages.
    if (remain >= still_need) {
        allocateFullMemory(vm, p, time, still_need);
    } else {
        // if the remaining memory pages in system are more than 4 pages, then directly allocate pages,
        // no need to evict memory pages.
        if (remain >= getPageLength(MEMORY_THREAD)) {
            p->penalty = (still_need - remain) * PAGE_FAULT_PENALTY;   
            allocateFullMemory(vm, p, time, remain);
        } else {
            // if the remaining memory pages in system are less than 4 pages, need to evict memory pages from other processes.
            // if the process is not executed, then need to evict pages to make the process to run.
            if (p->isExecuted == 0) {
                p->penalty = (still_need - getPageLength(MEMORY_THREAD)) * PAGE_FAULT_PENALTY;
                long long int toEvict = getPageLength(MEMORY_THREAD) - remain;
                allocateMixMemory(vm, p, time, remain, toEvict, processor, mem_flag);
            // if the process is already executed at least once, and some of its memory pages are evicted by other processes,
            // then need to allocate more pages to it to make it run.
            } else {
                p->penalty = (getPageLength(p->memorySize) - getPageLength(MEMORY_THREAD)) * PAGE_FAULT_PENALTY;
                long long int toEvict = getPageLength(MEMORY_THREAD) - actual;
                allocateMixMemory(vm, p, time, remain, toEvict, processor, mem_flag);
            }
        }
    }
}

/** Execute swapping memory management for target process (in "p" memory management options)
* @param dl the memory list to store memory node
* @param p target process
* @param time current time
*/
void executeSwapAllocation(struct doubly_list *dl, struct process *p, long long int time) {
    // find the first memory node that can be allocated to target process.
    struct list_node *res = firstFit(dl, p);
    // if fitful memory node is not found, need to evict memory.
    while (res == NULL) {
        // choose the least-recently-used process to evict the memory.
        struct list_node *evict_node = get_node_LRU(dl);
        if (evict_node != NULL) {
            struct memory_unit *m = evict_node->data;
            printf("%lld, EVICTED, mem-addresses=[", time);
            print_page_num(m->page_begin, m->page_len);
            printf("]\n");
            evictMemory(dl, evict_node);
            res = firstFit(dl, p);
        // if no least-recently-used process found, then out of memory --- impossible in this assignment.
        } else {
            fprintf(stderr, "Error: Out Of Memory!\n");
            exit(EXIT_FAILURE);
        }
    }
    // after evicting the memory and finding a fitful memory node, allocate memory for target process
    allocateMemory(dl, res, p, time);
}

/** Execute First-Come-Frist-Served algorithm
 * @param d the deque that stores the process reading from the file
 * @param time current time
 * @param maxMem the memory size of the system
 * @param flag indicate which memory management option is used
 * @param recorder the recorder used to store data for performance statistics
 * */
void executeFFAlgo(struct deque *d, long long int *time, long long int maxMem, char *flag, struct performance_recorder* recorder) {
    // the deque to store processes that already arrive in scheduler.
    struct deque *processor = initDeque();
    // indicate whether the scheduler is currently running a process
    int isProcessorOccupied = 0;
    // a counter for performance recorder
    long long int recorder_counter = 0;
    // virtual memory manager used for "v" and "cm" memory management options
    struct vm_manager *vm = createVMManager(maxMem);
    // memory list used in "p" memory management option
    struct doubly_list *memoryList = initDlist();
    initListOnMemory(memoryList, maxMem);


    while (1) {


        if (getFront(d) == NULL && getFront(processor) == NULL) {
            recorder->makespan = (*time) - 1;
            break;
        }
        // handle processes that arriving at the same time.
        struct deque *buffer = initDeque();
        for (struct node *i = d->front; i != NULL;) {
            if (i->data->arrive == (*time)) {
                insertRear(buffer, i->data);
                i = i->next;
                deleteFront(d);
                continue;
            }
            if (i->data->arrive > (*time)) {
                break;
            }
        }
        sortDequeById(buffer);
        for (struct node *j = buffer->front; j != NULL; j = j->next) {
            insertRear(processor, j->data);
        }
        struct process *current = getFront(processor);
        if (current == NULL) {
            (*time)++;
            continue;
        }
        // if there is no process currently running in the scheduler, then execute the next process awaiting.
        if (isProcessorOccupied == 0) {
            current->start = *time;

            current->expectedEnd = current->start + current->execTime;
            current->execTime = 0;
            isProcessorOccupied = 1;

            if (strcmp(flag, "u") == 0) {
                printf("%lld, RUNNING, id=%lld, remaining-time=%lld\n", (*time), current->id,
                       (current->expectedEnd + current->execTime - (*time)));
            } else if (strcmp(flag, "p") == 0) {
                executeSwapAllocation(memoryList, current, (*time));
            } else if (strcmp(flag, "v") == 0 || strcmp(flag, "cm") == 0) {
                executeVMAllocation(vm, current, (*time), processor, flag);
            }
        /**if there is process running in the scheduler, and if this process is expected to end now,
         * then end this process, deallocating its memory. Need to check whether there is another process waiting at
         * the same time, if it is, then execute that process and allocate memory for it.
         */
        } else {

            if (current->expectedEnd == *time) {
                if (strcmp(flag, "p") == 0) {
                    deallocate_memory(memoryList, current, (*time));
                } else if (strcmp(flag, "v") == 0 || strcmp(flag, "cm") == 0) {
                    deallocate_memory_vm(vm, current, (*time));
                }

                recorder->turnaround[recorder_counter] = (*time) - current->arrive;
                recorder->overhead[recorder_counter] = (double)((*time) - current->arrive)/current->job_time;
                recorder->finish_time[recorder_counter] = (*time);
                recorder_counter++;

                
                printf("%lld, FINISHED, id=%lld, proc-remaining=%d\n", (*time), current->id, processor->size - 1);
                deleteFront(processor);
                struct process *successor = getFront(processor);
                if (successor == NULL) {
                    isProcessorOccupied = 0;
                } else {
                    processor->front->data->start = *time;
                    if (processor->front->data->penalty > 0) {
                        processor->front->data->execTime += processor->front->data->penalty;
                        processor->front->data->penalty = 0;
                    }
                    processor->front->data->expectedEnd =
                            processor->front->data->start + processor->front->data->execTime;
                    processor->front->data->execTime = 0;
                    if (strcmp(flag, "u") == 0) {
                        printf("%lld, RUNNING, id=%lld, remaining-time=%lld\n", (*time),
                               processor->front->data->id,
                               (processor->front->data->expectedEnd + processor->front->data->execTime - (*time)));
                    } else if (strcmp(flag, "p") == 0) {
                        executeSwapAllocation(memoryList, processor->front->data, (*time));
                    } else if (strcmp(flag, "v") == 0 || strcmp(flag, "cm") == 0) {
                        executeVMAllocation(vm, processor->front->data, (*time), processor, flag);
                    }
                }
            }
        }
        (*time)++;

    }

}

/** Execute Robin-Round algorithm
 * @param d the deque that stores the process reading from the file
 * @param time current time
 * @param quantum the quantum time
 * @param maxMem the memory size of the system
 * @param flag indicate which memory management option is used
 * @param recorder the recorder used to store data for performance statistics
 * */
void executeRRAlgo(struct deque *d, long long int *time, long long int quantum, long long int maxMem, char *flag, struct performance_recorder* recorder) {
    // the deque to store processes that already arrive in scheduler.
    struct deque *processor = initDeque();
    // indicate whether the scheduler is currently running a process
    int isProcessorOccupied = 0;
    // a counter for performance recorder
    long long int recorder_counter = 0;
    // virtual memory manager used for "v" and "cm" memory management options
    struct vm_manager *vm = createVMManager(maxMem);
    // memory list used in "p" memory management option
    struct doubly_list *memoryList = initDlist();
    initListOnMemory(memoryList, maxMem);
    struct list_node *temp = NULL;

    while (1) {

        if (getFront(d) == NULL && getFront(processor) == NULL) {
            recorder->makespan = (*time) - 1;
            break;
        }
        // handle processes that arriving at the same time.
        struct deque *buffer = initDeque();
        for (struct node *i = d->front; i != NULL;) {
            if (i->data->arrive == (*time)) {
                insertRear(buffer, i->data);
                i = i->next;
                deleteFront(d);
                continue;
            }
            if (i->data->arrive > (*time)) {
                break;
            }
        }
        sortDequeById(buffer);
        for (struct node *j = buffer->front; j != NULL; j = j->next) {
            insertRear(processor, j->data);
        }

        struct process *current = getFront(processor);
        if (current == NULL) {
            (*time)++;
            continue;
        }
        // if there is no process currently running in the scheduler, then execute the next process awaiting.
        if (isProcessorOccupied == 0) {
            current->start = *time;

            if (current->execTime <= quantum) {
                current->expectedEnd = current->start + current->execTime;
                current->execTime = 0;
                current->flag = 1;
            } else {
                current->expectedEnd = current->start + quantum;
                current->execTime = current->execTime - quantum;
            }
            isProcessorOccupied = 1;

            if (strcmp(flag, "u") == 0) {
                printf("%lld, RUNNING, id=%lld, remaining-time=%lld\n", (*time), current->id,
                       (current->expectedEnd + current->execTime - (*time)));
            } else if (strcmp(flag, "p") == 0) {
                temp = firstUse(memoryList, current);
                if (temp == NULL) {
                    executeSwapAllocation(memoryList, current, (*time));
                    updateLastAccessTime(memoryList, current, (*time));
                } else {
                    printUsage(memoryList, temp, current, (*time));
                    updateLastAccessTime(memoryList, current, (*time));
                }
            } else if (strcmp(flag, "v") == 0 || strcmp(flag, "cm") == 0) {
                executeVMAllocation(vm, current, (*time), processor, flag);
                if (current->isExecuted == 0) {
                    current->isExecuted = 1;
                }
            }
         /** if there is process running in the scheduler, and if this process is expected to end now,
        * then end this process, deallocating its memory. Need to check whether there is another process waiting at
        * the same time, if it is, then execute that process and allocate memory for it.
        */
        } else {
            if (current->expectedEnd == (*time)) {

                if (current->execTime == 0) {

                    if (strcmp(flag, "p") == 0) {
                        deallocate_memory(memoryList, current, (*time));
                    } else if (strcmp(flag, "v") == 0 || strcmp(flag, "cm") == 0) {
                        deallocate_memory_vm(vm, current, (*time));
                    }

                    recorder->turnaround[recorder_counter] = (*time) - current->arrive;
                    recorder->overhead[recorder_counter] = (double)((*time) - current->arrive)/current->job_time;
                    // printf("process: %lld, job_time: %lld, turnaround:%lld\n", current->id, current->job_time, ((*time) - current->arrive));
                    recorder->finish_time[recorder_counter] = (*time);
                    recorder_counter++;
                    printf("%lld, FINISHED, id=%lld, proc-remaining=%d\n", (*time), current->id, processor->size - 1);
                    deleteFront(processor);
                } else {
                    insertRear(processor, current);
                    deleteFront(processor);
                }

                struct process *successor = getFront(processor);
                if (successor == NULL) {
                    isProcessorOccupied = 0;
                } else {
                    processor->front->data->start = *time;
                    if (processor->front->data->penalty > 0) {
                        processor->front->data->execTime += processor->front->data->penalty;
                        processor->front->data->penalty = 0;
                    }
                    if (processor->front->data->execTime <= quantum) {
                        processor->front->data->expectedEnd = processor->front->data->start + processor->front->data->execTime;
                        processor->front->data->execTime = 0;
                        processor->front->data->flag = 1;
                    } else {
                        processor->front->data->expectedEnd = processor->front->data->start + quantum;
                        processor->front->data->execTime = processor->front->data->execTime - quantum;
                    }
                    if (strcmp(flag, "u") == 0) {
                        printf("%lld, RUNNING, id=%lld, remaining-time=%lld\n", (*time), processor->front->data->id,
                               (processor->front->data->expectedEnd + processor->front->data->execTime - (*time)));
                    } else if (strcmp(flag, "p") == 0) {
                        temp = firstUse(memoryList, processor->front->data);
                        if (temp == NULL) {
                            executeSwapAllocation(memoryList, processor->front->data, (*time));
                            updateLastAccessTime(memoryList, processor->front->data, (*time));
                        } else {
                            printUsage(memoryList, temp, processor->front->data, (*time));
                            updateLastAccessTime(memoryList, processor->front->data, (*time));
                        }
                    } else if (strcmp(flag, "v") == 0 || strcmp(flag, "cm") == 0) {
                        long long int num = get_vm_allocated(processor->front->data);
                        if (num == getPageLength(processor->front->data->memorySize)) {
                            long long int percentage = ceil(
                                    ((double) (vm->total_num - vm->free_num) * 100 / (double) (vm->total_num)));
                            printf("%lld, RUNNING, id=%lld, remaining-time=%lld, load-time=%d, mem-usage=%lld%%, mem-addresses=[",
                                   (*time), processor->front->data->id, \
                                    (processor->front->data->expectedEnd + processor->front->data->execTime - (*time)), 0,percentage);
                            print_vm_page(processor->front->data);
                            printf("]\n");
                        } else {
                            executeVMAllocation(vm, processor->front->data, (*time), processor, flag);
                            if (processor->front->data->isExecuted == 0) {
                                processor->front->data->isExecuted = 1;
                            }
                        }
                    }
                }
            }
        }
        (*time)++;
    }

}

/** Execute Shortest-Process-First(SPF) algorithm, using a min heap to handle the process
 * @param d the deque that stores the process reading from the file
 * @param time current time
 * @param maxMem the memory size of the system
 * @param flag indicate which memory management option is used
 * @param recorder the recorder used to store data for performance statistics
 * */
/** Note: in SPF Algorithm, processes are executed one by one, just like FF algorithm, there is no quantum. Since the deque "processer" will
* only be used when need to evict page from other processes' memory, which is impossible in SPF algorithm, therefore, we only need to pass
* an empty deque "processor" to the method to execute VM memory management. 
*/
void executeSPFAlgo(struct deque *d, long long int *time, long long int maxMem, char *flag, struct performance_recorder* recorder) {
    // the deque to store processes that already arrive in scheduler.
    struct deque *processor = initDeque();
    struct heapNode *root = NULL;
    // indicate whether the scheduler is currently running a process
    int isProcessorOccupied = 0;
    // a counter for performance recorder
    long long int recorder_counter = 0;
    // virtual memory manager used for "v" and "cm" memory management options
    struct vm_manager *vm = createVMManager(maxMem);
    struct process *current = NULL;
    // memory list used in "p" memory management option
    struct doubly_list *memoryList = initDlist();
    initListOnMemory(memoryList, maxMem);
    int count = 0;
    while (1) {
        if (getFront(d) == NULL && root == NULL && isProcessorOccupied == 0) {
            recorder->makespan = (*time) - 1;
            break;
        }
        // handle processes that arriving at the same time.
        for (struct node *i = d->front; i != NULL;) {
            if (i->data->arrive == (*time)) {
                heap_insert(&root, i->data);
                count++;
                i = i->next;
                deleteFront(d);
                continue;
            }
            if (i->data->arrive > (*time)) {
                break;
            }
        }

        if (root == NULL && isProcessorOccupied == 0) {
            (*time)++;
            continue;
        }
        if (current == NULL && root != NULL) {
            current = heap_remove(&root);
        }

        if (current != NULL) {
            // if there is no process currently running in the scheduler, then execute the next process awaiting.
            if (isProcessorOccupied == 0) {
                current->start = *time;
                current->expectedEnd = current->start + current->execTime;
                current->execTime = 0;
                isProcessorOccupied = 1;
                if (strcmp(flag, "u") == 0) {
                    printf("%lld, RUNNING, id=%lld, remaining-time=%lld\n", (*time), current->id,
                           (current->expectedEnd + current->execTime - (*time)));
                } else if (strcmp(flag, "p") == 0) {
                    executeSwapAllocation(memoryList, current, (*time));
                }else if (strcmp(flag, "v") == 0 || strcmp(flag, "cm") == 0) {
                    executeVMAllocation(vm, current, (*time), processor, flag);
                }
                /** if there is process running in the scheduler, and if this process is expected to end now,
               * then end this process, deallocating its memory. Need to check whether there is another process waiting at
               * the same time, if it is, then execute that process and allocate memory for it.
               */
            } else {
                if (current->expectedEnd == *time) {
                    if (strcmp(flag, "p") == 0) {
                        deallocate_memory(memoryList, current, (*time));
                    } else if (strcmp(flag, "v") == 0 || strcmp(flag, "cm") == 0) {
                        deallocate_memory_vm(vm, current, (*time));
                    }
                    count--;
                    recorder->turnaround[recorder_counter] = (*time) - current->arrive;
                    recorder->overhead[recorder_counter] = (double)((*time) - current->arrive)/current->job_time;
                    recorder->finish_time[recorder_counter] = (*time);
                    recorder_counter++;
                    printf("%lld, FINISHED, id=%lld, proc-remaining=%d\n", (*time), current->id, count);

                    if (root == NULL) {
                        isProcessorOccupied = 0;
                        current = NULL;
                    } else {
                        current = heap_remove(&root);
                        current->start = *time;
                        current->expectedEnd = current->start + current->execTime;
                        current->execTime = 0;
                        if (strcmp(flag, "u") == 0) {
                            printf("%lld, RUNNING, id=%lld, remaining-time=%lld\n", (*time), current->id,
                                   (current->expectedEnd + current->execTime - (*time)));
                        } else if (strcmp(flag, "p") == 0) {
                            executeSwapAllocation(memoryList, current, (*time));
                        }else if (strcmp(flag, "v") == 0 || strcmp(flag, "cm") == 0) {
                            executeVMAllocation(vm, current, (*time), processor, flag);               
                        }
                    }
                }
            }
        }

        (*time)++;

    }
}


int main(int argc, char **argv) {
    /* Check the correctness of command line inputs
    * At least, the command line inputs should include: 1. "./scheduler", 2. "-f","-a","-m" and their values
    */
    if (argc < 7) {
        fprintf(stderr, "Usage %s Make sure following inputs exist: -f, -a, -m\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *fileName = NULL;
    char *algorithm = NULL;
    char *memalloc = NULL;
    char *memsize = NULL;
    char *quantum = NULL;

    long long int memsize_num = 0;
    long long int quantum_num = DEFAULT_TIME;

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            fileName = argv[i + 1];
        } else if (strcmp(argv[i], "-a") == 0) {
            algorithm = argv[i + 1];
        } else if (strcmp(argv[i], "-m") == 0) {
            memalloc = argv[i + 1];
        } else if (strcmp(argv[i], "-s") == 0) {
            memsize = argv[i + 1];
        } else if (strcmp(argv[i], "-q") == 0) {
            quantum = argv[i + 1];
        }
    }

    if (algorithm == NULL || (algorithm != NULL && strcmp(algorithm, "ff") != 0 && strcmp(algorithm, "rr") != 0\
        && strcmp(algorithm, "cs") != 0)) {
        fprintf(stderr, "Usage %s Make sure the scheduling algorithm is one of {ff, rr, cs}\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (memalloc == NULL || (memalloc != NULL && strcmp(memalloc, "u") != 0 && strcmp(memalloc, "p") != 0\
        && strcmp(memalloc, "v") != 0 && strcmp(memalloc, "cm") != 0)) {
        fprintf(stderr, "Usage %s Make sure the memory allocation is one of {u,p,v,cm}\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (fileName == NULL) {
        fprintf(stderr, "Usage %s Make sure the file name is not NULL\n", argv[0]);
        exit(EXIT_FAILURE);
    } else {
        char *dot = strrchr(fileName, '.');
        if (!(dot && strcmp(dot, FILE_FORMAT) == 0)) {
            fprintf(stderr, "Usage %s Make sure the file a txt file\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (memsize != NULL && strcmp(memsize, "u") != 0 && atoi(memsize) != 0) {
        memsize_num = atoi(memsize);
    }

    if (quantum != NULL && atoi(quantum) != 0) {
        quantum_num = atoi(quantum);
    }

    FILE *fptr;
    if ((fptr = fopen(fileName, "r")) == NULL) {
        fprintf(stderr, "Error in Opening File, make sure the txt file and executable are in the same directory\n");
        exit(EXIT_FAILURE);
    }

    long long int input_start = 0;
    long long int input_id = 0;
    long long int input_mem = 0;
    long long int input_time = 0;
    long long int count = 0;
    // deque to store the process reading from the file.
    struct deque *processQueue = initDeque();
    // read the process details from the file and make new process, storing process into the deque.
    while (fscanf(fptr, LINE_FORMAT, &input_start, &input_id, &input_mem, &input_time) != EOF) {
        struct process *p = (struct process *) malloc(sizeof(struct process));
        count++;
        p->arrive = input_start;
        p->id = input_id;
        p->memorySize = input_mem;
        p->execTime = input_time;
        p->job_time = input_time;
        p->penalty = 0;
        p->flag = 0;
        p->isExecuted = 0;
        p->memory_array = (long long int *) malloc(sizeof(long long int) * getPageLength(p->memorySize));
        for (long long int i = 0; i < getPageLength(p->memorySize); i++) {
            p->memory_array[i] = -1;
        }

        count++;
        insertRear(processQueue, p);
    }

    long long int time = 0;

    struct performance_recorder *recorder = createRecorder(getSize(processQueue));

    if (strcmp(algorithm, "ff") == 0) {
        executeFFAlgo(processQueue, &time, memsize_num, memalloc,recorder);
    } else if (strcmp(algorithm, "rr") == 0) {
        executeRRAlgo(processQueue, &time, quantum_num, memsize_num, memalloc,recorder);
    } else {
        executeSPFAlgo(processQueue, &time, memsize_num, memalloc,recorder);
    }

    // print the performance stats
    executePerformanceStats(recorder);


    return 0;


}