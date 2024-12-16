#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "PCB.h"

/**
 ** struct node - Represents a node in a linked list or queue
 * @data: A pointer to the data stored in the node (specific type)
 * @next: A pointer to the next node in the list
 *
 ** Description: This struct is used to represent a single
 ** node in a linked list or queue. It contains a pointer to the
 ** data, which can be of a specific type, and a pointer to the next node.
 */

// Macro to create queue for any data type
#define CREATE_CIRCULAR_QUEUE(type)                                            \
                                                                               \
    typedef struct node_circular_##type                                        \
    {                                                                          \
        type data;                                                             \
        struct node_##type *next;                                              \
    } node_##type;                                                             \
                                                                               \
    typedef struct queue_circular_##type                                       \
    {                                                                          \
        node_##type *head;                                                     \
        node_##type *tail;                                                     \
    } queue_##type;                                                            \
                                                                               \
    /* Function to create a new queue */                                       \
    queue_##type *createQueue_Cirular_##type(void)                             \
    {                                                                          \
        queue_##type *newQueue = (queue_##type *)malloc(sizeof(queue_##type)); \
        if (!newQueue)                                                         \
        {                                                                      \
            perror("Failed To Create A New Queue\n");                          \
            return NULL;                                                       \
        }                                                                      \
        newQueue->head = NULL;                                                 \
        newQueue->tail = NULL;                                                 \
        return newQueue;                                                       \
    }                                                                          \
                                                                               \
    /* Function to enqueue data into the queue */                              \
    void enqueue_circular##type(queue_##type *Queue, type data)                \
    {                                                                          \
        node_##type *newNode = (node_##type *)malloc(sizeof(node_##type));     \
        if (!newNode)                                                          \
        {                                                                      \
            perror("Failed to create node\n");                                 \
            return;                                                            \
        }                                                                      \
        newNode->data = data;                                                  \
        newNode->next = NULL;                                                  \
        if (Queue->tail)                                                       \
            Queue->tail->next = newNode;                                       \
        else                                                                   \
            Queue->head = newNode;                                             \
        Queue->tail = newNode;                                                 \
    }                                                                          \
                                                                               \
    /* Function to dequeue data from the queue */                              \
    type dequeue_circular##type(queue_##type *Queue)                           \
    {                                                                          \
        if (!Queue || !Queue->head)                                            \
        {                                                                      \
            return (type){0};                                                  \
        }                                                                      \
        node_##type *temp = Queue->head;                                       \
        type data = temp->data;                                                \
        Queue->head = temp->next;                                              \
        if (!Queue->head)                                                      \
            Queue->tail = NULL;                                                \
        free(temp);                                                            \
        return data;                                                           \
    }                                                                          \
                                                                               \
    /* Function to destroy the queue */                                        \
    void destroyQueue_circular_##type(queue_##type *Queue)                     \
    {                                                                          \
        node_##type *temp;                                                     \
        while (Queue->head)                                                    \
        {                                                                      \
            temp = Queue->head;                                                \
            Queue->head = temp->next;                                          \
            free(temp);                                                        \
        }                                                                      \
        free(Queue);                                                           \
    }

    bool isQueueEmpty_circular##type(queue_##type *Queue)                       \
    {                                                                          \
        return (Queue->head == NULL);                                          \
    } 

/* Define queues for specific types */
typedef char *CharPtr;

CREATE_CIRCULAR_QUEUE(int);
CREATE_CIRCULAR_QUEUE(float);
CREATE_CIRCULAR_QUEUE(CharPtr);
CREATE_CIRCULAR_QUEUE(PCB);

#endif
