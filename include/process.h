/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#ifndef PROCESS_H
#define PROCESS_H

#include <string.h>

// struct to record the fields of process
struct process {
    // the time the process arrives
    long long int arrive;
    // the time the process starts to execute
    long long int start;
    // the process id
    long long int id;
    // the memory size needed by the process
    long long int memorySize;
    // the time needs to execute by the process(equals to job time at the beginning)
    long long int execTime;
    // the expected end time of the process
    long long int expectedEnd;
    // the time to load pages of the process (in "v" and "cm" memory management options)
    long long int loadingTime;
    // the remaining execute time of the process
    long long int remainingTime;
    // the memory page array of the process (in "v" and "cm" memory management options)
    long long int *memory_array;
    // the penalty time to allocate memory of the process  (in "v" and "cm" memory management options)
    long long int penalty;
    // the job time of the process
    long long int job_time;
    // indicates whether this process is in its final round of execution (in "rr" scheduling option)
    int flag;
    // indicates whether this process is already executed at least once
    int isExecuted;
};


#endif