#include "headers.h"
#include <stdbool.h>
#include <string.h>
#include "CircularQueue.h"
#include <stdlib.h>
#include <stdio.h>     

#define RR_Quantum 3


typedef struct {
    int processID; 
    int arrivalTime; 
    int remainingTime; 
    int waitingTime; 
    int turnAroundTime;
    int startTime;
    int finishTime;
} PCB;

CircularQueue *readyQueue;

int main(int argc, char *argv[])
{
    initClk();
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    int currentTime = getClk();
    int processCount = 0 ; //initially 0 processes //! total number of proccesses
    PCB currentProcess; //define current running process currently
    bool isCurrentProcessRunning = false ; //initializing that it is not running yet 

    FILE *inputFile = fopen("processes.txt", "r"); //file where processes are present , it reads only from the file 
    if (!inputFile) {
        perror("Error opening processes file"); // Handle file opening error
        return -1;
    }

    readyQueue = CreateCircularQueue();
    if (!readyQueue) {
        perror("Failed to create the ready queue");
        return -1;
    }

    CircularQueue *tempQueue;
    tempQueue = CreateCircularQueue();
    if (!tempQueue) {
        perror("Failed to create the temp queue");
        return -1;
    }
    char line[1024]; //initial size for the number of characters that will be read from processes.txt
    fgets(line, sizeof(line) , inputFile); //skipping header 


    PCB process;
    while (fgets(line, sizeof(line) , inputFile))
    {
        sscanf(line , "%d %d %d" , process.processID , process.arrivalTime , process.remainingTime);
        //enqueueCircular(tempQueue , process) //!to be handled later
        processCount++;
    }
    fclose(inputFile);

    
    while (true)
    {
        currentTime = getClk(); // Update the current time

        // Add newly arrived processes to the ready queue
        for (int i = 0; i < processCount; i++) {
            PCB temp;
            temp = dequeueCircular(tempQueue);
            if (temp.arrivalTime == currentTime) {
                enqueueCircular(readyQueue, temp); 
            }
            else{
                enqueueCircular(tempQueue , temp);
            }
        }
    }
    
    


    destroyClk(true);
}
