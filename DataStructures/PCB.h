#ifndef PCB_S
#define PCB_S

typedef struct PCB
{
    int processID;
    pid_t procssPID;
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