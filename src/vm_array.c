/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#include <string.h>
#include <stdlib.h>
#include "include/helper.h"
#include "include/vm_array.h"

#define FAKE_PID -1
#define FAKE_ACCESS_TIME -1

struct vm_manager *createVMManager(long long int max) {
    struct vm_manager *res = (struct vm_manager *) malloc(sizeof(struct vm_manager));
    res->total_memory = (long long int *) malloc(sizeof(long long int) * getPageLength(max));
    res->time_recorder = (long long int *) malloc(sizeof(long long int) * getPageLength(max));
    res->used_times = (long long int *) malloc(sizeof(long long int) * getPageLength(max));
    res->total_num = getPageLength(max);
    res->free_num = getPageLength(max);
    for (long long int i = 0; i < getPageLength(max); i++) {
        res->total_memory[i] = FAKE_PID;
        res->time_recorder[i] = FAKE_ACCESS_TIME;
        res->used_times[i] = 0;
    }
    return res;
}
