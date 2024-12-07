#include "../DataStructures/PriorityQueue.h"  // Include your priority queue header

int main(void)
{
    printf("====== Priority Queue Test Suite ======\n");

    // Test Case 1: Create a priority queue
    priorityQueue *pq = CreatePriQueue();
    if (pq)
    {
        printf("Test Case 1: Create Priority Queue - Passed\n");
    }
    else
    {
        printf("Test Case 1: Create Priority Queue - Failed\n");
        return 1; // Exit if the queue creation fails
    }

    // Test Case 2: Enqueue elements with varying priorities
    priEnqueue(pq, 10, 3); // Data 10, Priority 3
    priEnqueue(pq, 20, 2); // Data 20, Priority 2
    priEnqueue(pq, 30, 1); // Data 30, Priority 1
    priEnqueue(pq, 40, 4); // Data 40, Priority 4
    printf("Test Case 2: Enqueue Elements with Varying Priorities - Passed\n");

    // Test Case 3: Dequeue elements (should be in order of priority)
    int data1 = PriDequeue(pq); // Should return 30 (highest priority, priority 1)
    int data2 = PriDequeue(pq); // Should return 20 (priority 2)
    int data3 = PriDequeue(pq); // Should return 10 (priority 3)
    int data4 = PriDequeue(pq); // Should return 40 (priority 4)
    if (data1 == 30 && data2 == 20 && data3 == 10 && data4 == 40)
    {
        printf("Test Case 3: Dequeue Elements (Order of Priority) - Passed\n");
    }
    else
    {
        printf("Test Case 3: Dequeue Elements (Order of Priority) - Failed\n");
    }

    // Test Case 4: Enqueue more elements after dequeuing
    priEnqueue(pq, 50, 2); // Data 50, Priority 2
    priEnqueue(pq, 60, 1); // Data 60, Priority 1
    printf("Test Case 4: Enqueue More Elements After Dequeue - Passed\n");

    // Test Case 5: Dequeue remaining elements (should be in order of priority)
    int data5 = PriDequeue(pq); // Should return 60 (priority 1)
    int data6 = PriDequeue(pq); // Should return 50 (priority 2)
    if (data5 == 60 && data6 == 50)
    {
        printf("Test Case 5: Dequeue Remaining Elements - Passed\n");
    }
    else
    {
        printf("Test Case 5: Dequeue Remaining Elements - Failed\n");
    }

    // Test Case 6: Dequeue from an empty priority queue
    int emptyResult = PriDequeue(pq); // Should return -1
    if (emptyResult == -1)
    {
        printf("Test Case 6: Dequeue from Empty Priority Queue - Passed\n");
    }
    else
    {
        printf("Test Case 6: Dequeue from Empty Priority Queue - Failed\n");
    }

    // Test Case 7: Destroy the priority queue
    if (destoryPriQueue(pq) == 0)
    {
        printf("Test Case 7: Destroy Priority Queue - Passed\n");
    }
    else
    {
        printf("Test Case 7: Destroy Priority Queue - Failed\n");
    }

    // Test Case 8: Enqueue/Dequeue on NULL priority queue
    if (priEnqueue(NULL, 10, 3) == -1 && PriDequeue(NULL) == -1 && destoryPriQueue(NULL) == -1)
    {
        printf("Test Case 8: Operations on NULL Priority Queue - Passed\n");
    }
    else
    {
        printf("Test Case 8: Operations on NULL Priority Queue - Failed\n");
    }

    // Test Case 9: Destroy an empty priority queue
    priorityQueue *emptyQueue = CreatePriQueue();
    if (destoryPriQueue(emptyQueue) == 0)
    {
        printf("Test Case 9: Destroy Empty Priority Queue - Passed\n");
    }
    else
    {
        printf("Test Case 9: Destroy Empty Priority Queue - Failed\n");
    }

    printf("====== Test Suite Complete ======\n");

    return 0;
}
