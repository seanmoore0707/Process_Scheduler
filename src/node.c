/** COMP30023 Project2
* Haonan Chen, ID: 930614 
* Email: haonanc1@student.unimelb.edu.au
*/

#include <stdio.h>
#include <stdlib.h>
#include "include/process.h"
#include "include/node.h"

//Creates a new Node and returns pointer to it. 
struct node *createNewNode(struct process *x) {
    struct node *newNode = (struct node *) malloc(sizeof(struct node));
    newNode->data = x;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}