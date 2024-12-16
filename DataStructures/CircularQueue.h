#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "PCB.h"

// Macro to create circular queue for any data type
#define CREATE_CIRCULAR_QUEUE(type)                                            \
                                                                               \
    typedef struct node_cir_##type                                             \
    {                                                                          \
        type data;                                                             \
        struct node_cir_##type *next;                                          \
    } node_cir_##type;                                                         \
                                                                               \
    typedef struct queue_cir_##type                                            \
    {                                                                          \
        node_cir_##type *head;                                                 \
        node_cir_##type *tail;                                                 \
    } queue_cir_##type;                                                        \
                                                                               \
    /* Function to create a new queue */                                       \
    queue_cir_##type *createQueue_circular_##type(void)                         \
    {                                                                          \
        queue_cir_##type *newQueue = (queue_cir_##type *)malloc(sizeof(queue_cir_##type)); \
        if (!newQueue)                                                         \
        {                                                                      \
            perror("Failed to create a new queue");                            \
            return NULL;                                                       \
        }                                                                      \
        newQueue->head = NULL;                                                 \
        newQueue->tail = NULL;                                                 \
        return newQueue;                                                       \
    }                                                                          \
                                                                               \
    /* Function to enqueue data into the queue */                              \
    void enqueue_circular_##type(queue_cir_##type *Queue, type data)           \
    {                                                                          \
        node_cir_##type *newNode = (node_cir_##type *)malloc(sizeof(node_cir_##type)); \
        if (!newNode)                                                          \
        {                                                                      \
            perror("Failed to create node");                                   \
            return;                                                            \
        }                                                                      \
        newNode->data = data;                                                  \
        newNode->next = Queue->head;                                           \
        if (Queue->tail)                                                       \
        {                                                                      \
            Queue->tail->next = newNode;                                       \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            Queue->head = newNode;                                             \
        }                                                                      \
        Queue->tail = newNode;                                                 \
    }                                                                          \
                                                                               \
    /* Function to dequeue data from the queue */                              \
    type dequeue_circular_##type(queue_cir_##type *Queue)                      \
    {                                                                          \
        if (!Queue || !Queue->head)                                            \
        {                                                                      \
            fprintf(stderr, "Queue is empty\n");                               \
            return (type){0};                                                  \
        }                                                                      \
        node_cir_##type *temp = Queue->head;                                   \
        type data = temp->data;                                                \
        if (Queue->head == Queue->tail)                                        \
        {                                                                      \
            Queue->head = NULL;                                                \
            Queue->tail = NULL;                                                \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            Queue->head = temp->next;                                          \
            Queue->tail->next = Queue->head;                                   \
        }                                                                      \
        free(temp);                                                            \
        return data;                                                           \
    }                                                                          \
                                                                               \
    /* Function to destroy the queue */                                        \
    void destroyQueue_circular_##type(queue_cir_##type *Queue)                 \
    {                                                                          \
        if (!Queue)                                                            \
            return;                                                            \
        node_cir_##type *temp;                                                 \
        while (Queue->head)                                                    \
        {                                                                      \
            temp = Queue->head;                                                \
            Queue->head = Queue->head->next;                                   \
            free(temp);                                                        \
            if (Queue->head == Queue->tail)                                    \
            {                                                                  \
                free(Queue->head);                                             \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        free(Queue);                                                           \
    }                                                                          \
                                                                               \
    /* Function to check if the queue is empty */                              \
    int isQueueEmpty_circular_##type(queue_cir_##type *Queue)                  \
    {                                                                          \
        return (Queue->head == NULL);                                          \
    }

/* Define queues for specific types */
CREATE_CIRCULAR_QUEUE(PCB);

#endif
