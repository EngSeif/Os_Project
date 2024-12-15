#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "./PCB.h"
/**
 * Macro to define a priority queue for any data type.
 * Lower priority numbers represent higher priority.
 */

#define CREATE_PRIORITY_QUEUE(type)                                             \
                                                                                 \
    typedef struct node_priority_##type                                         \
    {                                                                           \
        type data;                                                              \
        int priority;                                                           \
        struct node_priority_##type *next;                                      \
    } node_priority_##type;                                                    \
                                                                                 \
    typedef struct priority_queue_##type                                        \
    {                                                                           \
        node_priority_##type *head;                                             \
    } priority_queue_##type;                                                   \
                                                                                 \
    priority_queue_##type *createPriorityQueue_##type(void)                     \
    {                                                                           \
        priority_queue_##type *newQueue =                                       \
            (priority_queue_##type *)malloc(sizeof(priority_queue_##type));     \
        if (!newQueue)                                                          \
        {                                                                       \
            perror("Failed to create a new priority queue\n");                  \
            return NULL;                                                        \
        }                                                                       \
        newQueue->head = NULL;                                                  \
        return newQueue;                                                        \
    }                                                                           \
                                                                                 \
    void enqueuePriority_##type(priority_queue_##type *Queue, type data, int priority) \
    {                                                                           \
        node_priority_##type *newNode =                                         \
            (node_priority_##type *)malloc(sizeof(node_priority_##type));       \
        if (!newNode)                                                           \
        {                                                                       \
            perror("Failed to create node\n");                                  \
            return;                                                             \
        }                                                                       \
        newNode->data = data;                                                   \
        newNode->priority = priority;                                           \
        newNode->next = NULL;                                                   \
                                                                                \
        if (!Queue->head || priority < Queue->head->priority)                   \
        {                                                                       \
            newNode->next = Queue->head;                                        \
            Queue->head = newNode;                                              \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            node_priority_##type *current = Queue->head;                        \
            while (current->next && current->next->priority <= priority)        \
            {                                                                   \
                current = current->next;                                        \
            }                                                                   \
            newNode->next = current->next;                                      \
            current->next = newNode;                                            \
        }                                                                       \
    }                                                                           \
                                                                                \
    type dequeuePriority_##type(priority_queue_##type *Queue)                   \
    {                                                                           \
        if (!Queue || !Queue->head)                                             \
        {                                                                       \
            perror("Queue is empty\n");                                         \
            return (type){0};                                                   \
        }                                                                       \
        node_priority_##type *temp = Queue->head;                               \
        type data = temp->data;                                                 \
        Queue->head = temp->next;                                               \
        free(temp);                                                             \
        return data;                                                            \
    }                                                                           \
                                                                                \
    void destroyPriorityQueue_##type(priority_queue_##type *Queue)              \
    {                                                                           \
        node_priority_##type *temp;                                             \
        while (Queue->head)                                                     \
        {                                                                       \
            temp = Queue->head;                                                 \
            Queue->head = temp->next;                                           \
            free(temp);                                                         \
        }                                                                       \
        free(Queue);                                                            \
    }                                                                           \
                                                                                \
    /* Function to check if the queue is empty */                               \
    int isQueueEmpty_##type(priority_queue_##type *Queue)                       \
    {                                                                           \
        return (Queue == NULL || Queue->head == NULL);                          \
    }

    
/* Define priority queues for specific types */
typedef char* CharPtr;
CREATE_PRIORITY_QUEUE(int);
CREATE_PRIORITY_QUEUE(float);
CREATE_PRIORITY_QUEUE(CharPtr);
CREATE_PRIORITY_QUEUE(PCB);

#endif
