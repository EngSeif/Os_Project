#ifndef PRI_QUEUE_H
#define PRI_QUEUE_H

#include <stdio.h>
#include <stdlib.h>

#include "./PCB.h"

// Macro to define a priority queue for a specific type
#define PRI_QUEUE(TYPE)                                                                         \
    typedef struct PriNode_##TYPE                                                                      \
    {                                                                                                  \
        TYPE data;                                                                                     \
        int priority;                                                                                  \
        struct PriNode_##TYPE *next;                                                                   \
    } PriNode_##TYPE;                                                                                  \
                                                                                                       \
    typedef struct priorityQueue_##TYPE                                                                \
    {                                                                                                  \
        PriNode_##TYPE *head;                                                                          \
        PriNode_##TYPE *tail;                                                                          \
    } priorityQueue_##TYPE;                                                                            \
                                                                                                       \
    PriNode_##TYPE *createPriNode_##TYPE(TYPE data, int priority)                                      \
    {                                                                                                  \
        PriNode_##TYPE *newNode = (PriNode_##TYPE *)malloc(sizeof(PriNode_##TYPE));                    \
        if (!newNode)                                                                                  \
        {                                                                                              \
            perror("Failed To Create A New Node\n");                                                   \
            return NULL;                                                                               \
        }                                                                                              \
        newNode->data = data;                                                                          \
        newNode->priority = priority;                                                                  \
        newNode->next = NULL;                                                                          \
        return newNode;                                                                                \
    }                                                                                                  \
                                                                                                       \
    priorityQueue_##TYPE *CreatePriQueue_##TYPE(void)                                                  \
    {                                                                                                  \
        priorityQueue_##TYPE *newQueue = (priorityQueue_##TYPE *)malloc(sizeof(priorityQueue_##TYPE)); \
        if (!newQueue)                                                                                 \
        {                                                                                              \
            perror("Failed To Create A New Queue\n");                                                  \
            return NULL;                                                                               \
        }                                                                                              \
        newQueue->head = NULL;                                                                         \
        newQueue->tail = NULL;                                                                         \
        return newQueue;                                                                               \
    }                                                                                                  \
                                                                                                       \
    int priEnqueue_##TYPE(priorityQueue_##TYPE *Queue, TYPE data, int priority)                        \
    {                                                                                                  \
        PriNode_##TYPE *newNode, *temp;                                                                \
                                                                                                       \
        if (!Queue)                                                                                    \
            return -1;                                                                                 \
                                                                                                       \
        newNode = createPriNode_##TYPE(data, priority);                                                \
                                                                                                       \
        if (!Queue->head)                                                                              \
        {                                                                                              \
            Queue->head = newNode;                                                                     \
            Queue->tail = newNode;                                                                     \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            if (Queue->head->priority > priority)                                                      \
            {                                                                                          \
                newNode->next = Queue->head;                                                           \
                Queue->head = newNode;                                                                 \
            }                                                                                          \
            else                                                                                       \
            {                                                                                          \
                temp = Queue->head;                                                                    \
                while (temp->next && temp->next->priority <= priority)                                 \
                    temp = temp->next;                                                                 \
                newNode->next = temp->next;                                                            \
                temp->next = newNode;                                                                  \
                                                                                                       \
                if (!newNode->next)                                                                    \
                    Queue->tail = newNode;                                                             \
            }                                                                                          \
        }                                                                                              \
        return 0;                                                                                      \
    }                                                                                                  \
                                                                                                       \
    TYPE PriDequeue_##TYPE(priorityQueue_##TYPE *queue)                                                \
    {                                                                                                  \
        PriNode_##TYPE *temp;                                                                          \
        TYPE data;                                                                                     \
        if (!queue || !queue->head)                                                                    \
            return (TYPE)(0); /* Return a sentinel value for error */                                  \
                                                                                                       \
        temp = queue->head;                                                                            \
        data = temp->data;                                                                             \
        queue->head = temp->next;                                                                      \
                                                                                                       \
        if (!queue->head)                                                                              \
            queue->tail = NULL;                                                                        \
                                                                                                       \
        free(temp);                                                                                    \
        return data;                                                                                   \
    }                                                                                                  \
                                                                                                       \
    int destroyPriQueue_##TYPE(priorityQueue_##TYPE *queue)                                            \
    {                                                                                                  \
        PriNode_##TYPE *temp;                                                                          \
                                                                                                       \
        if (!queue)                                                                                    \
            return -1;                                                                                 \
                                                                                                       \
        while (queue->head)                                                                            \
        {                                                                                              \
            temp = queue->head;                                                                        \
            queue->head = queue->head->next;                                                           \
            free(temp);                                                                                \
        }                                                                                              \
        free(queue);                                                                                   \
        return 0;                                                                                      \
    }

PRI_QUEUE(int)
typedef char * CharPtr;
PRI_QUEUE(CharPtr)
typedef PCB * PCB_Ptr;
PRI_QUEUE(PCB_Ptr)


#endif
