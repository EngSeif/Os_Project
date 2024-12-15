#ifndef PCB_S
#define PCB_S

typedef struct PCB
{
    pid_t processID;
    int processPriority;
    int arrivalTime;
    int remainingTime;
    int waitingTime;
    int turnAroundTime;
    int startTime;
    int finishTime;
    int runtime;
    int executionTime;
} PCB;

#endif