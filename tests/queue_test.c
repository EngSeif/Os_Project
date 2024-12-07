#include "../queue.h"

int main(void)
{
    printf("====== Queue Test Suite ======\n");

    // Test Case 1: Create a queue
    queue *q = CreateQueue();
    if (q)
    {
        printf("Test Case 1: Create Queue - Passed\n");
    }
    else
    {
        printf("Test Case 1: Create Queue - Failed\n");
        return 1; // Exit if the queue creation fails
    }

    // Test Case 2: Enqueue elements
    enqueue(q, 10);
    enqueue(q, 20);
    enqueue(q, 30);
    printf("Test Case 2: Enqueue Elements - Passed\n");

    // Test Case 3: Dequeue elements
    int data1 = dequeue(q); // Should return 10
    int data2 = dequeue(q); // Should return 20
    if (data1 == 10 && data2 == 20)
    {
        printf("Test Case 3: Dequeue Elements - Passed\n");
    }
    else
    {
        printf("Test Case 3: Dequeue Elements - Failed\n");
    }

    // Test Case 4: Enqueue more elements
    enqueue(q, 40);
    enqueue(q, 50);
    printf("Test Case 4: Enqueue More Elements - Passed\n");

    // Test Case 5: Dequeue remaining elements
    int data3 = dequeue(q); // Should return 30
    int data4 = dequeue(q); // Should return 40
    int data5 = dequeue(q); // Should return 50
    if (data3 == 30 && data4 == 40 && data5 == 50)
    {
        printf("Test Case 5: Dequeue Remaining Elements - Passed\n");
    }
    else
    {
        printf("Test Case 5: Dequeue Remaining Elements - Failed\n");
    }

    // Test Case 6: Dequeue from an empty queue
    int emptyResult = dequeue(q); // Should return -1
    if (emptyResult == -1)
    {
        printf("Test Case 6: Dequeue from Empty Queue - Passed\n");
    }
    else
    {
        printf("Test Case 6: Dequeue from Empty Queue - Failed\n");
    }

    // Test Case 7: Destroy the queue
    if (destoryQueue(q) == 0)
    {
        printf("Test Case 7: Destroy Queue - Passed\n");
    }
    else
    {
        printf("Test Case 7: Destroy Queue - Failed\n");
    }

    // Test Case 8: Enqueue/Dequeue on NULL Queue
    if (enqueue(NULL, 10) == -1 && dequeue(NULL) == -1 && destoryQueue(NULL) == -1)
    {
        printf("Test Case 8: Operations on NULL Queue - Passed\n");
    }
    else
    {
        printf("Test Case 8: Operations on NULL Queue - Failed\n");
    }

    // Test Case 9: Destroy an empty queue
    queue *emptyQueue = CreateQueue();
    if (destoryQueue(emptyQueue) == 0)
    {
        printf("Test Case 9: Destroy Empty Queue - Passed\n");
    }
    else
    {
        printf("Test Case 9: Destroy Empty Queue - Failed\n");
    }

    printf("====== Test Suite Complete ======\n");

    return 0;
}
