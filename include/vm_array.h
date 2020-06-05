/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#ifndef VM_ARRAY_H
#define VM_ARRAY_H

#include <string.h>
#include <stdlib.h>
#include "include/helper.h"

// the struct to manage vm page swapping in "v" and "cm" memory management options
struct vm_manager {
    long long int *total_memory;
    long long int *time_recorder;
    long long int *used_times;
    long long int total_num;
    long long int free_num;
};

struct vm_manager *createVMManager(long long int max);

#endif