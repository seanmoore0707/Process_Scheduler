/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/


#include <stdio.h>
#include <stdlib.h>
#include "include/memory.h"


struct memory_unit *createMemory(int type, long long int page_begin, long long int page_len, \
                                long long int byte_begin, long long int byte_len) {
    struct memory_unit *mem = (struct memory_unit *) malloc(sizeof(struct memory_unit));
    mem->type = type;
    mem->page_begin = page_begin;
    mem->page_len = page_len;
    mem->byte_begin = byte_begin;
    mem->byte_len = byte_len;
    mem->last_access = -1;
    mem->process_id = -1;
    mem->loading_time = 0;
    return mem;
}