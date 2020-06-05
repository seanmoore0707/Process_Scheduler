/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#ifndef MEMORY_H
#define MEMORY_H

#include<stdio.h>
#include<stdlib.h>

// the memory struct used in "p" memory management option, each node of the memory list holds a memory unit.
struct memory_unit {
    int type;
    long long int page_begin;
    long long int page_len;
    long long int byte_begin;
    long long int byte_len;
    long long int process_id;
    long long int last_access;
    // the loading time of swapping memory page
    long long int loading_time;
};

struct memory_unit *createMemory(int type, long long int page_begin, long long int page_len, \
                                long long int byte_begin, long long int byte_len);

#endif