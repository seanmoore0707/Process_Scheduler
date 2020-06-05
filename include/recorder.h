/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#ifndef RECORDER_H
#define RECORDER_H

#include <string.h>
#include <stdlib.h>

// the struct to store data for performance analysis
struct performance_recorder{
    long long int *turnaround;
    long long int *finish_time;
    double *overhead;
    long long int process_num;
    long long int makespan;
    long long int avgThroughput;
    long long int minThroughput;
    long long int maxThroughput;
};

struct performance_recorder *createRecorder(long long int num);
double getMaxOverhead(double* arr, long long int len);
double getAvgOverhead(double* arr, long long int len);
long long int getAvgTurnaround(long long int* arr, long long int len);
void getAvgThroughput(struct performance_recorder * recorder);
void executePerformanceStats(struct performance_recorder * recorder);

#endif