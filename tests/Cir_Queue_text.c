#include <stdio.h>
#include <stdlib.h>
#include "../DataStructures/CircularQueue.h"

int main() {
    // Test Case 1: Test Empty Queue Initialization
    CircularQueue *queue = CreateCircularQueue();
    if (queue == NULL) {
        printf("Queue creation failed.\n");
        return -1;
    }
    // The queue should be empty initially
    if (queue->Front == NULL && queue->Rear == NULL) {
        printf("Test 1 Passed: Queue is empty initially.\n");
    } else {
        printf("Test 1 Failed: Queue should be empty initially.\n");
    }

    // Test Case 2: Test Enqueueing One Element
    enqueueCircular(queue, 10);
    if (queue->Front && queue->Rear && queue->Front == queue->Rear && queue->Front->data == 10) {
        printf("Test 2 Passed: One element enqueued successfully.\n");
    } else {
        printf("Test 2 Failed: Queue was not updated correctly.\n");
    }

    // Test Case 3: Test Enqueueing Multiple Elements
    enqueueCircular(queue, 20);
    enqueueCircular(queue, 30);
    if (queue->Front->data == 10 && queue->Rear->data == 30) {
        printf("Test 3 Passed: Multiple elements enqueued successfully.\n");
    } else {
        printf("Test 3 Failed: Queue order or circular link is incorrect.\n");
    }

    // Test Case 4: Test Dequeueing One Element
    int removed = dequeueCircular(queue);
    if (removed == 10 && queue->Front->data == 20) {
        printf("Test 4 Passed: One element dequeued successfully.\n");
    } else {
        printf("Test 4 Failed: Dequeue operation did not work correctly.\n");
    }

    // Test Case 5: Test Dequeueing All Elements
    dequeueCircular(queue);  // Removes 20
    removed = dequeueCircular(queue);  // Removes 30
    if (removed == 30 && queue->Front == NULL && queue->Rear == NULL) {
        printf("Test 5 Passed: All elements dequeued and queue is empty.\n");
    } else {
        printf("Test 5 Failed: Queue is not empty after dequeueing all elements.\n");
    }

    // Test Case 6: Test Dequeue on Empty Queue
    removed = dequeueCircular(queue);
    if (removed == -1) {
        printf("Test 6 Passed: Dequeue on empty queue returns -1.\n");
    } else {
        printf("Test 6 Failed: Dequeue on empty queue did not return -1.\n");
    }

    // Test Case 7: Test Circular Behavior (Wrap Around)
    enqueueCircular(queue, 10);
    enqueueCircular(queue, 20);
    enqueueCircular(queue, 30);
    dequeueCircular(queue);  // Removes 10
    enqueueCircular(queue, 40);  // Adds 40

    if (queue->Front->data == 20 && queue->Rear->data == 40) {
        printf("Test 7 Passed: Circular behavior is maintained correctly.\n");
    } else {
        printf("Test 7 Failed: Circular behavior is not maintained.\n");
    }

    // Free queue
    destroyQueueCircular(queue);

    return 0;
}
