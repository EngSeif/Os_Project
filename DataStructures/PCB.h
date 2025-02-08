#ifndef PCB_S
#define PCB_S

#include <sys/types.h>
#include "../headers.h"

typedef struct PCB
{
    int processID;
    pid_t processPID;
    int processPriority;
    int arrivalTime;
    int remainingTime;
    int waitingTime;
    int turnAroundTime;
    float weightedTurnAroundTime;
    int startTime;
    int finishTime;
    int runtime;
    int executionTime;
    int originalPriority; 
    int LastExecTime;
    int memsize;
    bool allocated;
} PCB;

#endif