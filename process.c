#include "headers.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

#define SHM_KEY 1234

int remainingTime;

int main(int argc, char *argv[])
{
    initClk(); // Initialize the clock
    printf("Process : start\n");

    // Creating shared memory and attaching it
    int sharedMemoryId = shmget(SHM_KEY, sizeof(int), 0666 | IPC_CREAT);
    if (sharedMemoryId == -1)
    {
        perror("Failed to get shared memory");
        return -1;
    }

    int *sharedMemory = (int *)shmat(sharedMemoryId, NULL, 0);
    if (sharedMemory == (int *)-1)
    {
        perror("Failed to attach shared memory");
        return -1;
    }

    remainingTime = *sharedMemory;
    printf("Process : Shared memory initial value: %d\n", remainingTime);

    while (remainingTime > 0)
    {
        remainingTime = *sharedMemory;
        if (remainingTime <= 0)
            break; // Safety check
        remainingTime--;
        *sharedMemory = remainingTime;
        printf("Process : Updated shared memory: %d\n", remainingTime);
        sleep(1);
    }

    // Detach the shared memory segment
    printf("Process : end\n");
    if (shmdt(sharedMemory) == -1)
    {
        perror("Failed to detach shared memory");
    }

    destroyClk(false); // Clean up and destroy the clock
    return 0;          // Exit the program
}
