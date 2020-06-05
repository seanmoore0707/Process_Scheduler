/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#ifndef NODE_H
#define NODE_H

#include <string.h>
#include "include/process.h"

struct node {
    struct process *data;
    struct node *prev;
    struct node *next;

};

struct node *createNewNode(struct process *x);

#endif