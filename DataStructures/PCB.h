#ifndef PCB_S
#define PCB_S

#include <sys/types.h>

typedef struct PCB
{
    int processID;
    pid_t processPID;
    int processPriority;
    int arrivalTime;
    int remainingTime;
    int waitingTime;
    int turnAroundTime;
    int startTime;
    int finishTime;
    int runtime;
    int executionTime;
    int LastExecTime;
} PCB;

#endif