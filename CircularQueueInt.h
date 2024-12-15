#ifndef CIR_QUEUE_INT
#define CIR_QUEUE_INT

#include <stdio.h>
#include <stdlib.h>

/**
 ** struct nodeCircular - Represents a node in a linked list or queue
 * @data: An integer value stored in the node
 * @next: A pointer to the next node in the list
 *
 ** Description: This struct is used to represent a single
 ** node in a linked list or queue. It contains an integer
 ** value and a pointer to the next node, enabling traversal
 ** of the list or queue.
 */

typedef struct nodeCircular
{
    int data;
    struct nodeCircular *next;
} nodeCircular;

/**
 ** createNode - Creates a new node for a linked list or queue
 * @data: The int value to be stored in the new node
 *
 ** Description: This function dynamically allocates memory for a new node,
 ** initializes its `data` field with the provided value, and sets its `next`
 ** pointer to NULL. If memory allocation fails, the function prints an error
 ** message and returns NULL.
 *
 ** Return: A pointer to the newly created node, or NULL if memory allocation fails
 */

nodeCircular *createNodeCircular(int data)
{
    nodeCircular *newNode = (nodeCircular *)malloc(sizeof(nodeCircular));
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

typedef struct CircularQueue
{
    nodeCircular *Front;
    nodeCircular *Rear;
} CircularQueue;

/**
 ** CreateQueue - Dynamically allocates memory for a queue
 *! Return: A pointer to the newly created queue, or NULL on failure
 */

CircularQueue *CreateCircularQueue(void)
{
    CircularQueue *newQueue = (CircularQueue *)malloc(sizeof(CircularQueue));
    if (!newQueue)
    {
        perror("Failed To Create A New Queue\n");
        return NULL;
    }
    newQueue->Front = NULL;
    newQueue->Rear = NULL;
    return newQueue;
}

/**
 ** enqueueCircular - Adds an element to the queue
 * @queue: Pointer to the queue
 * @data: The data to be added
 *
 ** Return: 0 on success, -1 on failure
 */

int enqueueCircular(CircularQueue *Queue, int data)
{
    nodeCircular *newNode;

    //! if There is No Queue Passed
    if (!Queue)
        return -1;

    //! Create A New Node
    newNode = createNodeCircular(data);

    if (Queue->Front)
    {
        //? if Queue is not empty:-

        //! make last node point to newNode
        Queue->Rear->next = newNode;

        //! make Rear the new node
        Queue->Rear = newNode;

        //! make the rear point to front (Maintain Circular Queue Behaviour)
        Queue->Rear->next = Queue->Front;
    }
    else
    {
        //? if Queue is Empty:

        //! make front the newNode
        Queue->Front = newNode;

        //! make rear the newNode
        Queue->Rear = newNode;

        //! make the rear point to front (Maintain Circular Queue Behaviour)
        newNode->next = Queue->Front;
    }

    return 0;
}

/**
 ** dequeueCircular - Removes an element from the queue
 * @queue: Pointer to the queue
 *
 ** Return: The data of the removed element, or -1 if the queue is empty
 */

int dequeueCircular(CircularQueue *queue)
{
    nodeCircular *temp;
    int data ;  // Default int in case of an empty queue

    //? Check if Queue is Empty Or Not
    if (!queue || !queue->Front)
        return -1;

    temp = queue->Front;
    data = temp->data;

    //? If there is one node in the Queue
    if (queue->Front == queue->Rear)
    {
        queue->Front = NULL;
        queue->Rear = NULL;
    }
    else
    {
        //! Make the Next Node to front Is the New Front
        queue->Front = queue->Front->next;

        //! make the rear point to front (Maintain Circular Queue Behaviour)
        queue->Rear->next = queue->Front;
    }

    free(temp);
    return data;
}

/**
 ** destoryQueueCircular - Frees all memory associated with a queue
 * @queue: Pointer to the queue to be destroyed
 *
 ** Description: This function iterates through all nodes in the queue,
 ** freeing each node and then the queue structure itself. It safely
 ** handles an empty queue or a NULL pointer.
 *
 ** Return: 0 on success, -1 if the queue pointer is NULL
 */

int destroyQueueCircular(CircularQueue *queue)
{
    if (!queue)
        return -1;

    nodeCircular *temp = queue->Front;
    nodeCircular *nextNode;

    // Traverse the queue and free all nodes
    while (temp != NULL && temp->next != queue->Front)
    {
        nextNode = temp->next;
        free(temp);
        temp = nextNode;
    }

    if (temp != NULL)
    {
        free(temp);
    }

    free(queue);
    return 0;
}
#endif
