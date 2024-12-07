#ifndef PRI_QUEUE_H
#define PRI_QUEUE_H

#include <stdio.h>
#include <stdlib.h>

/**
 ** struct node - Represents a node in a priority queue
 * @data: An integer value stored in the node
 * @priority: An integer value that determines the priority of the node
 * @next: A pointer to the next node in the list
 *
 ** Description: This struct is used to represent a single node in a
 ** priority queue, where each node contains an integer value (`data`)
 ** and a priority value (`priority`). The priority determines the order
 ** in which the nodes are dequeued, with higher priority nodes being dequeued
 ** before lower priority nodes. The `next` pointer links each node to the
 ** next node in the queue, enabling traversal of the list.
 */

typedef struct PriNode
{
    int data;
    int priority;
    struct PriNode *next;
} PriNode;

/**
 ** createNode - Creates a new node for a linked list or priority queue
 * @data: The integer value to be stored in the new node
 * @priority: The integer priority value associated with the node
 *
 ** Description: This function dynamically allocates memory for a new node,
 ** initializes its `data` and `priority` fields with the provided values,
 ** and sets its `next` pointer to NULL. If memory allocation fails, the
 ** function prints an error message and returns NULL.
 *
 ** Return: A pointer to the newly created node, or NULL if memory allocation fails
 */

PriNode *createPriNode(int data, int priority)
{
    PriNode *newNode = (PriNode *)malloc(sizeof(PriNode));
    if (!newNode)
    {
        perror("Failed To Create A New Node\n");
        return NULL;
    }
    newNode->data = data;
    newNode->priority = priority;
    newNode->next = NULL;
    return newNode;
}

/**
 ** struct queue - Represents a queue data structure
 * @head: Pointer to the first node in the queue
 * @tail: Pointer to the last node in the queue
 *
 ** Description: This struct represents a queue, a linear data structure
 ** that follows the FIFO (First In, First Out) principle. The `head`
 ** pointer is used to dequeue elements, and the `tail` pointer is used
 ** to enqueue elements.
 */

typedef struct priorityQueue
{
    PriNode *head;
    PriNode *tail;
} priorityQueue;

/**
 ** CreateQueue - Dynamically allocates memory for a queue
 *! Return: A pointer to the newly created queue, or NULL on failure
 */

priorityQueue *CreatePriQueue(void)
{
    priorityQueue *newQueue = (priorityQueue *)malloc(sizeof(priorityQueue));
    if (!newQueue)
    {
        perror("Failed To Create A New Queue\n");
        return NULL;
    }
    newQueue->head = NULL;
    newQueue->tail = NULL;
    return newQueue;
}

/**
 ** priEnqueue - Adds an element to the priority queue
 * @queue: Pointer to the priority queue
 * @data: The data value to be added
 * @priority: The priority of the data value (lower number = higher priority)
 *
 ** Description: This function inserts a new node with the given data and priority
 ** into the priority queue. The nodes are ordered based on priority, with the
 ** lower priority numbers placed at the front of the queue. If two nodes have
 ** the same priority, the new node is added after the existing nodes with the
 ** same priority.
 *
 ** Return: 0 on success, -1 on failure
 */

int priEnqueue(priorityQueue *Queue, int data, int priority)
{
    PriNode *newNode, *temp, prev;

    //! if There is No Queue Passed
    if (!Queue)
        return -1;

    //! Create A New Node
    newNode = createPriNode(data, priority);

    if (!Queue->head)
    {
        Queue->head = newNode;
        Queue->tail = newNode;
    }
    else
    {
        if (Queue->head->priority > priority)
        {
            newNode->next = Queue->head;
            Queue->head = newNode;
        }
        else
        {
            temp = Queue->head;
            while (temp->next && temp->next->priority <= priority)
            {
                temp = temp->next;
            }

            newNode->next = temp->next;
            temp->next = newNode;

            if (!newNode->next)
                Queue->tail = newNode;
        }
    }

    return 0;
}

/**
 ** PriDequeue - Removes an element from the queue
 * @queue: Pointer to the queue
 *
 ** Return: The data of the removed element, or -1 if the queue is empty
 */

int PriDequeue(priorityQueue *queue)
{
    PriNode *temp;
    int data;

    if (!queue || !queue->head)
        return -1;

    temp = queue->head;
    data = temp->data;

    queue->head = temp->next;

    if (!queue->head)
        queue->tail = NULL;

    free(temp);
    return data;
}

/**
 ** destroyQueue - Frees all memory associated with a queue
 * @queue: Pointer to the queue to be destroyed
 *
 ** Description: This function iterates through all nodes in the queue,
 ** freeing each node and then the queue structure itself. It safely
 ** handles an empty queue or a NULL pointer.
 *
 ** Return: 0 on success, -1 if the queue pointer is NULL
 */

int destoryPriQueue(priorityQueue *queue)
{
    PriNode *temp;

    if (!queue)
        return -1;

    while (queue->head)
    {
        queue->head = temp->next;
        free(temp);
        temp = queue->head;
    }
    free(queue);
    return 0;
}

#endif
