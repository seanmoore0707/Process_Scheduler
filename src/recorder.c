/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "include/recorder.h"
#define INTERVAL 60

struct performance_recorder *createRecorder(long long int num){
    struct performance_recorder *res = (struct performance_recorder *) malloc(sizeof(struct performance_recorder));
    res->turnaround = (long long int *) malloc(sizeof(long long int) * num);
    res->finish_time = (long long int *) malloc(sizeof(long long int) * num);
    res->overhead = (double *) malloc(sizeof(double) * num);
    res->process_num = num;
    res->makespan = 0;

    for(long long int i=0; i<num; i++){
        res->turnaround[i] = 0;
        res->finish_time[i] = 0;
        res->overhead[i] = 0.0;
    }
    return res;
}

double getMaxOverhead(double* arr, long long int len){
    double res = arr[0];
    for(long long int i=1; i<len; i++){
        if(res<arr[i]){
            res = arr[i];
        }
    }
    return res;
}

double getAvgOverhead(double* arr, long long int len){
    double sum = 0.0;
    for(long long int i=0; i<len; i++){
        sum+=arr[i];
    }
    return (double)sum/(double)len;
}

long long int getAvgTurnaround(long long int* arr, long long int len){
    long long int sum = 0;
    for(long long int i=0; i<len; i++){
        sum+=arr[i];
    }
    return (long long int)ceil((float)sum/(float)len);
}

void getAvgThroughput(struct performance_recorder * recorder){
    long long int intervals = 0;
    if(recorder->makespan % INTERVAL == 0){
        intervals = recorder->makespan/INTERVAL;
    }else{
        intervals = recorder->makespan/INTERVAL + 1;
    }

    long long int* throughput = (long long int *) malloc(sizeof(long long int) * intervals);

    for(long long int i=0; i<intervals; i++){
        throughput[i] = 0;
    }
    long long int index = 1;


    while(index<=intervals){
        long long int upthread = index * INTERVAL;
        long long int downthread = (index-1) * INTERVAL;
        for(long long int i=0; i<recorder->process_num;i++){
            if(recorder->finish_time[i]<=upthread && recorder->finish_time[i]>downthread){
                throughput[index-1]+=1;
            }
        }
        index++;
    }


    long long int sum = 0;
    long long int max = throughput[0];
    long long int min = throughput[0];

    for(long long int i=0; i<intervals; i++){
        sum+=throughput[i];
        if(max<throughput[i]){
            max = throughput[i];
        }
        if(min>throughput[i]){
            min = throughput[i];
        }
    }

    long long int avg = (long long int)ceil((float)sum/(float)intervals);

    printf("Throughput %lld, %lld, %lld\n", avg, min, max);
}




void executePerformanceStats(struct performance_recorder * recorder){
    double maxOverhead = getMaxOverhead(recorder->overhead, recorder->process_num);
    double avgOverhead = getAvgOverhead(recorder->overhead, recorder->process_num);
    long long int avgTurnaround = getAvgTurnaround(recorder->turnaround, recorder->process_num);
    getAvgThroughput(recorder);
    printf("Turnaround time %lld\n", avgTurnaround);
    printf("Time overhead %.2f %.2f\n", maxOverhead, avgOverhead);
    printf("Makespan %lld\n", recorder->makespan);
}