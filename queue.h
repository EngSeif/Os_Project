#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

/**
 ** struct node - Represents a node in a linked list or queue
 * @data: An integer value stored in the node
 * @next: A pointer to the next node in the list
 *
 ** Description: This struct is used to represent a single
 ** node in a linked list or queue. It contains an integer
 ** value and a pointer to the next node, enabling traversal
 ** of the list or queue.
 */

typedef struct node
{
    int data;
    struct node *next;
} node;

/**
 ** createNode - Creates a new node for a linked list or queue
 * @data: The integer value to be stored in the new node
 *
 ** Description: This function dynamically allocates memory for a new node,
 ** initializes its `data` field with the provided value, and sets its `next`
 ** pointer to NULL. If memory allocation fails, the function prints an error
 ** message and returns NULL.
 *
 ** Return: A pointer to the newly created node, or NULL if memory allocation fails
 */

node *createNode(int data)
{
    node *newNode = (node *)malloc(sizeof(node));
    if (!newNode)
    {
        perror("Failed To Create A New Node\n");
        return NULL;
    }
    newNode->data = data;
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

typedef struct queue
{
    node *head;
    node *tail;
} queue;

/**
 ** CreateQueue - Dynamically allocates memory for a queue
 *! Return: A pointer to the newly created queue, or NULL on failure
 */

queue *CreateQueue(void)
{
    queue *newQueue = (queue *)malloc(sizeof(queue));
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
 ** enqueue - Adds an element to the queue
 * @queue: Pointer to the queue
 * @data: The data to be added
 *
 ** Return: 0 on success, -1 on failure
 */

int enqueue(queue *Queue, int data)
{
    node *newNode;

    //! if There is No Queue Passed
    if (!Queue)
        return -1;

    //! Create A New Node
    newNode = createNode(data);

    /*
    ! Checking if the Queue is Not Empty
    * if not, assign next node of tail to the new Node
    * if empty, assign head to be the new node
    */
    if (Queue->tail)
        Queue->tail->next = newNode;
    else
        Queue->head = newNode;

    //! Update Value Of Tail Pointer
    Queue->tail = newNode;

    return 0;
}

/**
 ** dequeue - Removes an element from the queue
 * @queue: Pointer to the queue
 *
 ** Return: The data of the removed element, or -1 if the queue is empty
 */

int dequeue(queue *queue)
{
    node *temp;
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

int destoryQueue(queue *queue)
{
    node *temp;

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
