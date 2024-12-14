#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

/**
 ** struct node - Represents a node in a linked list or queue
 * @data: A pointer to the data stored in the node (specific type)
 * @next: A pointer to the next node in the list
 *
 ** Description: This struct is used to represent a single
 ** node in a linked list or queue. It contains a pointer to the
 ** data, which can be of a specific type, and a pointer to the next node.
 */

typedef struct PCB
{
    int processID;
    int processPriority;
    int arrivalTime;
    int remainingTime;
    int waitingTime;
    int turnAroundTime;
    int startTime;
    int finishTime;
    int runtime;
    int executionTime;
} PCB;



#define CREATE_QUEUE(type)                                                      \
                                                                                \
    typedef struct node_##type                                                  \
    {                                                                           \
        type data;                                                              \
        struct node_##type *next;                                               \
    } node_##type;                                                              \
                                                                                \
    typedef struct queue_##type                                                 \
    {                                                                           \
        node_##type *head;                                                      \
        node_##type *tail;                                                      \
    } queue_##type;                                                             \
                                                                                \
    queue_##type *createQueue_##type(void)                                      \
    {                                                                           \
        queue_##type *newQueue = (queue_##type *)malloc(sizeof(queue_##type));  \
        if (!newQueue)                                                          \
        {                                                                       \
            perror("Failed To Create A New Queue\n");                           \
            return NULL;                                                        \
        }                                                                       \
        newQueue->head = NULL;                                                  \
        newQueue->tail = NULL;                                                  \
        return newQueue;                                                        \
    }                                                                           \
                                                                                \
    void enqueue_##type(queue_##type *Queue, type data)                         \
    {                                                                           \
        node_##type *newNode = (node_##type *)malloc(sizeof(node_##type));      \
        if (!newNode)                                                           \
        {                                                                       \
            perror("Failed to create node\n");                                  \
            return;                                                             \
        }                                                                       \
        newNode->data = data;                                                   \
        newNode->next = NULL;                                                   \
        if (Queue->tail)                                                        \
            Queue->tail->next = newNode;                                        \
        else                                                                    \
            Queue->head = newNode;                                              \
        Queue->tail = newNode;                                                  \
    }                                                                           \
                                                                                \
    type dequeue_##type(queue_##type *Queue)                                    \
    {                                                                           \
        if (!Queue || !Queue->head)                                             \
        {                                                                       \
            return (type){0};                                                   \
        }                                                                       \
        node_##type *temp = Queue->head;                                        \
        type data = temp->data;                                                 \
        Queue->head = temp->next;                                               \
        if (!Queue->head)                                                       \
            Queue->tail = NULL;                                                 \
        free(temp);                                                             \
        return data;                                                            \
    }                                                                           \
                                                                                \
    void destroyQueue_##type(queue_##type *Queue)                               \
    {                                                                           \
        node_##type *temp;                                                      \
        while (Queue->head)                                                     \
        {                                                                       \
            temp = Queue->head;                                                 \
            Queue->head = temp->next;                                           \
            free(temp);                                                         \
        }                                                                       \
        free(Queue);                                                            \
    }

/* Define queues for specific types */
typedef char* CharPtr; 
CREATE_QUEUE(int);
CREATE_QUEUE(float)
CREATE_QUEUE(CharPtr)
CREATE_QUEUE(PCB);

#endif
