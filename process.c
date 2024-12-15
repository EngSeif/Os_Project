#include "headers.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>

#define SHM_KEY 1234

int remainingTime;


//this code represents a single process
int main(int argc, char *argv[])
{
    initClk(); // Initialize the clock

    // Creating shared memory and attaching it
    int sharedMemoryId = shmget(SHM_KEY, sizeof(int) * 2, 0666); // Shared memory for remaining time + quantum
    if (sharedMemoryId == -1)
    {
        perror("Failed to get shared memory");
        return -1;
    }

    int *sharedMemory = (int *)shmat(sharedMemoryId, NULL, 0); // Attach shared memory
    if (sharedMemory == (int *)-1)
    {
        perror("Failed to attach shared memory");
        return -1;
    }

    while (*sharedMemory == 0) {
        sleep(1); // Adjust the sleep time as needed
    }

    remainingTime = sharedMemory[0]; // First integer in shared memory is the remaining time
    int quantum = sharedMemory[1]; // Second integer is the quantum value

    // Loop to decrement remaining time in chunks of the quantum size
    while (remainingTime > 0)
    {
        int timeSlice = (remainingTime >= quantum) ? quantum : remainingTime; // Determine time slice to run
        sleep(timeSlice); // Simulate the passage of time
        remainingTime -= timeSlice; // Decrease the remaining time
    }

    // Detach the shared memory segment before exiting
    shmdt(sharedMemory); 

    destroyClk(false); // Clean up and destroy the clock
    return 0; // Exit the program
}
