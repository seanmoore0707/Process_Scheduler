#include <stdio.h>
#include <stdlib.h>

#include "include/process.h"
#include "include/minHeap.h"


//a factory to create a node
struct heapNode *create_node(struct process *p, struct heapNode *parent) {
    struct heapNode *n = (struct heapNode *) malloc(sizeof(struct heapNode));
    n->data = p;
    n->left = NULL;
    n->right = NULL;
    n->parent = parent;
    return n;
}

//a factory to create a queue element
struct heapQueue *create_queue(struct heapNode *element) {
    struct heapQueue *q = (struct heapQueue *) malloc(sizeof(struct heapQueue));
    q->next = NULL;
    q->element = element;
    return q;
}

//function to free queue's memory
void free_heapQueue(struct heapQueue **q) {
    struct heapQueue *current_q = *q;
    while (current_q != NULL) {
        *q = current_q->next;
        free(current_q);
        current_q = *q;
    }
}

//put nodes into q
void heapQueue_put(struct heapNode *n, struct heapQueue **last) {
    if (n->left != NULL) {
        (*last)->next = create_queue(n->left);
        (*last) = (*last)->next;
    }
    if (n->right != NULL) {
        (*last)->next = create_queue(n->right);
        (*last) = (*last)->next;
    }
}

//insert a node into a heap
void heap_insert(struct heapNode **root, struct process *x) {
    //insert new node if the heap is empty
    if (*root == NULL) {
        *root = create_node(x, NULL);
    }
        //serach for a proper insert point
    else {
        struct heapQueue *q = create_queue(*root);
        struct heapQueue *current_q = q;
        struct heapQueue *last_q = q;
        struct heapNode *current_node;

        while (current_q != NULL) {
            //get one node out of queue
            current_node = current_q->element;

            if (current_node->left == NULL) {
                current_node->left = create_node(x, current_node);
                current_node = current_node->left;
            } else if (current_node->right == NULL) {
                current_node->right = create_node(x, current_node);
                current_node = current_node->right;
            } else {
                heapQueue_put(current_node, &last_q);
                //increment the current pointer
                current_q = current_q->next;
                continue;
            }

            //put the node to proper point
            while (current_node->parent != NULL &&
                   current_node->parent->data->execTime > current_node->data->execTime) {
                struct process *temp = current_node->parent->data;
                current_node->parent->data = current_node->data;
                current_node->data = temp;
                current_node = current_node->parent;

            }
            break;

        }

        free_heapQueue(&q);
    }
    return;
}

struct process *heap_remove(heapNode **root) {
    if (*root != NULL) {
        struct process *res = (*root)->data;
        struct heapQueue *q = create_queue(*root);
        struct heapQueue *current_q = q;
        struct heapQueue *last_q = q;
        struct heapQueue *previous_q;
        struct heapNode *current_node;

        //find the last node
        while (current_q != NULL) {
            current_node = current_q->element;

            heapQueue_put(current_node, &last_q);

            previous_q = current_q;
            current_q = current_q->next;
        }

        current_node = previous_q->element;
        free_heapQueue(&q);

        //remove last node and get the value to root
        if (current_node->parent == NULL) {
            free(current_node);
            *root = NULL;
        } else {
            (*root)->data = current_node->data;
            current_node = current_node->parent;
            if (current_node->right != NULL) {
                free(current_node->right);
                current_node->right = NULL;
            } else {
                free(current_node->left);
                current_node->left = NULL;
            }
            //move down the value
            struct process *a;
            // = (struct process*)malloc(sizeof(struct process));
            struct process *b;
            // = (struct process*)malloc(sizeof(struct process));
            struct process *c;
            // = (struct process*)malloc(sizeof(struct process));

            current_node = *root;
            while (1) {
                if (current_node->left == NULL) {
                    //implies current->right == NULL is true
                    break;
                } else if (current_node->right == NULL) {
                    a = current_node->data;
                    b = current_node->left->data;
                    if (a->execTime > b->execTime) {
                        current_node->data = b;
                        current_node->left->data = a;
                        current_node = current_node->left;
                    } else {
                        break;
                    }
                } else {
                    a = current_node->data;
                    b = current_node->left->data;
                    c = current_node->right->data;
                    //a is the largest, do nothing
                    if (a->execTime <= b->execTime && a->execTime <= c->execTime) {
                        break;
                    }
                        //b is the largest, swap a and b
                    else if (b->execTime < a->execTime && b->execTime <= c->execTime) {
                        current_node->left->data = a;
                        current_node->data = b;
                        current_node = current_node->left;
                    }
                        //c is the largest, swap a and c
                    else {
                        current_node->right->data = a;
                        current_node->data = c;
                        current_node = current_node->right;
                    }
                }
            }
        }

        return res;
    } else {
        return NULL;
    }
}

void print_heap(struct heapNode **root) {
    if (*root != NULL) {
        struct heapQueue *q = create_queue(*root);
        struct heapQueue *current_q = q;
        struct heapQueue *last_q = q;
        struct heapNode *temp;
        while (current_q != NULL) {
            temp = current_q->element;
            heapQueue_put(temp, &last_q);
            // printf("the process is %d, with execution time %d\n", temp->data->id, temp->data->execTime);
            current_q = current_q->next;
        }
        free_heapQueue(&q);
        printf("\n");
    }
}
 