#include <stdio.h>
#include "../DataStructures/typedef.h"

int main()
{
    // Create an integer queue
    queue_int *intQueue = createQueue_int();
    
    int a = 10, b = 20, c = 30;
    enqueue_int(intQueue, a);
    enqueue_int(intQueue, b);
    enqueue_int(intQueue, c);
    
    printf("Dequeued: %d\n", dequeue_int(intQueue));
    printf("Dequeued: %d\n", dequeue_int(intQueue));
    printf("Dequeued: %d\n", dequeue_int(intQueue));

    // Create a float queue
    queue_float *floatQueue = createQueue_float();
    
    float x = 10.5, y = 20.5, z = 30.5;
    enqueue_float(floatQueue, x);
    enqueue_float(floatQueue, y);
    enqueue_float(floatQueue, z);
    
    printf("Dequeued: %.2f\n", dequeue_float(floatQueue));
    printf("Dequeued: %.2f\n", dequeue_float(floatQueue));
    printf("Dequeued: %.2f\n", dequeue_float(floatQueue));

    // Create a string (char*) queue
    queue_CharPtr *charQueue = createQueue_CharPtr();
    
    char *str1 = "Hello", *str2 = "World", *str3 = "Queue";
    enqueue_CharPtr(charQueue, str1);
    enqueue_CharPtr(charQueue, str2);
    enqueue_CharPtr(charQueue, str3);

    printf("Dequeued: %s\n", dequeue_CharPtr(charQueue));
    printf("Dequeued: %s\n", dequeue_CharPtr(charQueue));
    printf("Dequeued: %s\n", dequeue_CharPtr(charQueue));

    // Destroy the queues
    destroyQueue_int(intQueue);
    destroyQueue_float(floatQueue);
    destroyQueue_CharPtr(charQueue);

    return 0;
}
