#include "headers.h"
#include <stdbool.h>
#include <string.h>
#include "./DataStructures/CircularQueue.h"
#include "./DataStructures/PriorityQueue.h"
#include "./DataStructures/queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include "./DataStructures/PCB.h"

#define SHM_KEY 1234
#define RR_Quantum 3
#define MAX_LEVEL 11

typedef struct
{
    queue_PCB *levels[MAX_LEVEL]; // 11 levels for the MLFQ
} MLFQ;

typedef struct MsgBufferScheduler
{
    long mtype;
    PCB proc;
} MsgBufferScheduler;

void RoundRobinScheduler(int, int);
void ShortestJobFirst(int);
void pHPF(int);
void logProcessState(FILE *, int, PCB, const char *);

int actualEndTimeForScheduler, totalProcessesRunningTime, totalWaitingTime, totalTA;
float totalWTA;

int main(int argc, char *argv[])
{
    initClk();
    int noProcess = atoi(argv[1]);
    printf("noProcess : %d\n", noProcess);
    RoundRobinScheduler(noProcess, 4);
    // ShortestJobFirst(noProcess);
    printf("hello world\n");
    sleep(2);
    destroyClk(false);
}

// Helper function to write logs to the output file
// Helper function to write logs to the output file
void logProcessState(FILE file, int currentTime, PCB process, const charstate)
{
    if (file == NULL)
    {
        fprintf(stderr, "Error: File pointer is NULL\n");
        return;
    }

    // Log common process details
    // If the process is "finished", calculate and log TA and WTA
    if (strcmp(state, "finished") == 0)
    {
        fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d",
                currentTime, process.processID, state, process.arrivalTime,
                process.turnAroundTime + process.remainingTime, process.remainingTime, process.waitingTime);
        process.turnAroundTime = process.finishTime - process.arrivalTime;
        int TA = process.turnAroundTime; // Ensure turnAroundTime is correctly calculated elsewhere
        totalTA += TA;

        // Avoid division by zero
        float WTA = 0;
        if (process.turnAroundTime + process.remainingTime > 0)
        {
            WTA = (float)TA / (process.runtime);
        }
        totalWTA += WTA;

        // Log TA and WTA
        fprintf(file, " TA %d WTA %.2f", TA, WTA);
    }
    else
    {
        fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d",
                currentTime, process.processID, state, process.arrivalTime,
                process.turnAroundTime + process.remainingTime, process.remainingTime, process.waitingTime);
    }

    // Add a new line to separate entries
    fprintf(file, "\n");

    fflush(file);
}

//? ============================================ ROUND ROBIN ALGORITHM ===========================================================

void RoundRobinScheduler(int noProcesses, int quantum)
{

    printf("Round Robin Begin\n");

    PCB *PCB_array = (PCB *)malloc(noProcesses * sizeof(PCB)); // Allocate memory for storing PCB array

    // Declare variables for message queue and shared memory
    key_t schedulerKey;
    int schedulerMessageID, index = 0;

    queue_cir_PCB *readyQueue = createQueue_circular_PCB();

    int sharedMemoryId = shmget(SHM_KEY, sizeof(int), 0666 | IPC_CREAT); // Create shared memory segment for inter-process communication

    if (sharedMemoryId == -1)
    {
        perror("Failed to create shared memory");
        exit(-1);
    }

    schedulerKey = ftok("KeyFileUP", 60); // Generate a unique key for the scheduler message queue
    if (schedulerKey == -1)
    {
        perror("ftok failed");
        exit(-1);
    }

    schedulerMessageID = msgget(schedulerKey, 0666 | IPC_CREAT); // Create the scheduler message queue
    if (schedulerMessageID == -1)
    {
        perror("msgget failed");
        exit(-1);
    }

    MsgBufferScheduler msgReceive; // Define the message buffer for receiving messages

    int *sharedMemory = (int *)shmat(sharedMemoryId, NULL, 0); // Attach the shared memory segment
    if (sharedMemory == (int *)-1)
    {
        perror("Failed to attach shared memory");
        exit(-1);
    }

    printf("Entering main loop\n");

    while (true)
    {
        // Check for newly arrived processes in the message queue
        int receive_Val = msgrcv(schedulerMessageID, &msgReceive, sizeof(msgReceive.proc), 1, IPC_NOWAIT);
        if (receive_Val > 0)
        {
            printf("Received new process\n");
            msgReceive.proc.remainingTime = msgReceive.proc.runtime;
            enqueue_circular_PCB(readyQueue, msgReceive.proc);
        }

        if (!isQueueEmpty_circular_PCB(readyQueue)) // Check if the ready queue has processes to execute
        {

            PCB currentProcess = dequeue_circular_PCB(readyQueue);
            pid_t runningPid; // Fork a child process to execute the current process
            if (currentProcess.processPID == -1)
            {
                printf("Process %d started at time %d\n", currentProcess.processID, getClk());
                runningPid = fork();
                currentProcess.processPID == runningPid;
                if (runningPid == 0)
                {
                    // In child process: set shared memory values for execution
                    // first value in shared memory is remaining time and second value is quantum
                    printf("remaining time: %d\n", currentProcess.remainingTime);
                    *sharedMemory = currentProcess.remainingTime;
                    execl("./process.o", "process.o", NULL); // Execute the process by the child

                    perror("execl failed");
                    exit(-1);
                }
            }
            else
            {
                printf("Process %d resumed at time %d\n", currentProcess.processID, getClk());
                runningPid = currentProcess.processPID;
                kill(runningPid, SIGCONT);
            }
            // In parent process: calculate execution time
            int executeTime = (currentProcess.remainingTime > quantum) ? quantum : currentProcess.remainingTime;
            sleep(executeTime); // Simulate process execution

            currentProcess.remainingTime = *sharedMemory; // Update the process's remaining time from shared memory

            if (currentProcess.remainingTime == 0)
            {
                // If process has finished, wait for it and record finish time
                waitpid(runningPid, NULL, 0);         // to prevent being zoombie
                currentProcess.finishTime = getClk(); // recording finish time
                printf("Process %d finished at time %d\n", currentProcess.processID, currentProcess.finishTime);
                PCB_array[index++] = currentProcess; // storing finished process in array for further calculations
                noProcesses--;
            }
            else
            {
                // If process is not finished, preempt it and re-add to queue
                kill(runningPid, SIGSTOP);
                printf("Process %d stoped at time %d\n", currentProcess.processID, getClk());
                currentProcess.remainingTime = *sharedMemory;
                while (msgrcv(schedulerMessageID, &msgReceive, sizeof(msgReceive.proc), 1, IPC_NOWAIT) != -1)
                {
                    printf("Received new process\n");
                    msgReceive.proc.remainingTime = msgReceive.proc.runtime;
                    printf("added proceess with id : %d\n", msgReceive.proc.processID);
                    enqueue_circular_PCB(readyQueue, msgReceive.proc);
                }
                enqueue_circular_PCB(readyQueue, currentProcess);
            }
        }

        if (noProcesses == 0 && isQueueEmpty_circular_PCB(readyQueue))
        {
            break;
        }
    }

    // Detach the shared memory segment
    if (shmdt(sharedMemory) == -1)
    {
        perror("shmdt failed");
        exit(-1);
    }

    // Remove the shared memory segment
    if (shmctl(sharedMemoryId, IPC_RMID, NULL) == -1)
    {
        perror("shmctl IPC_RMID failed");
        exit(-1);
    }

    // Calculate performance metrics:
    int totalTurnaroundTime = 0;
    int totalWaitingTime = 0;
    int totalExecutionTime = 0;
    int startTime = getClk();
    for (int i = 0; i < index; i++)
    {
        int turnaroundTime = PCB_array[i].finishTime - PCB_array[i].arrivalTime;
        int waitingTime = turnaroundTime - PCB_array[i].executionTime;

        totalTurnaroundTime += turnaroundTime;
        totalWaitingTime += waitingTime;
        totalExecutionTime += PCB_array[i].executionTime;

        printf("Process %d: Turnaround Time = %d, Waiting Time = %d\n", PCB_array[i].processID, turnaroundTime, waitingTime);
    }

    int totalTime = getClk() - startTime;
    float cpuUtilization = ((float)totalExecutionTime / totalTime) * 100;
    float averageTurnaroundTime = (float)totalTurnaroundTime / index;
    float averageWaitingTime = (float)totalWaitingTime / index;

    printf("\nPerformance Metrics:\n");
    printf("Average Turnaround Time: %.2f\n", averageTurnaroundTime);
    printf("Average Waiting Time: %.2f\n", averageWaitingTime);
    printf("CPU Utilization: %.2f%%\n", cpuUtilization);

    printf("All processes completed\n");
    free(PCB_array);
}

//? ============================================ MULTI LEVEL FEEDBACK QUEUE ALGORITHM ===========================================================

// MLFQ *Create_MLQF()
// {
//     MLFQ *mlfq = (MLFQ *)malloc(sizeof(MLFQ));

//     if (!mlfq)
//         perror("Failed To Make A multilevel feedback queue\n");
//     exit(-1);

//     for (int i = 0; i < MAX_LEVEL; i++)
//         mlfq->levels[i] = createQueue_PCB();

//     return mlfq;
// }

// int enqueueProcessMLFQ(MLFQ *mlfq, PCB Process)
// {
//     if (Process.processPriority >= 0 && Process.processPriority < MAX_LEVEL)
//     {
//         enqueue_PCB(mlfq->levels[Process.processPriority], Process);
//         return 0;
//     }
//     else
//     {
//         perror("Process Prioirty Number is invalid");
//         return -1;
//     }
// }

// int moveProcessBetweenLevels(MLFQ *mlfq, int current_level, int next_level)
// {
//     if (current_level >= 0 && current_level < MAX_LEVEL && next_level >= 0 && next_level < MAX_LEVEL)
//     {
//         PCB process = dequeue_PCB(mlfq->levels[current_level]);
//         enqueue_PCB(mlfq->levels[next_level], process);
//     }
//     else
//     {
//         perror("Current Level Or Next Level Number is invalid");
//         return -1;
//     }
// }

// void destroyMLFQ(MLFQ *mlfq)
// {
//     for (int i = 0; i < MAX_LEVEL; i++)
//     {
//         destroyQueue_PCB(mlfq->levels[i]);
//     }
//     free(mlfq);
// }

// void processMLFQ(MLFQ *mlfq)
// {
//     for (int i = 0; i < MAX_LEVEL; i++)
//     {
//         while (mlfq->levels[i]->head)
//         {
//             // simulate process

//             if (i < MAX_LEVEL)
//             {
//                 moveProcessBetweenLevels(mlfq, i, i + i);
//             }
//         }
//     }
// }

// void multiLevelFeedbackScheduler(PCB **processesArray, int size)
// {
//     MLFQ *mlfq = Create_MLQF();

//     for (int i = 0; i < size; i++)
//         enqueueProcessMLFQ(mlfq, *processesArray[i]);
//     processMLFQ(mlfq);
//     destroyMLFQ(mlfq);
// }

//? ============================================ Shortest Job First ALGORITHM ===========================================================

void ShortestJobFirst(int noProcesses)
{
    printf("SJB Begin\n");
    PCB *PCB_array = (PCB *)malloc(noProcesses * sizeof(PCB));
    key_t key_scheduler;
    int receive_Val;
    int scheduler_msg_id, index = 0;
    priority_queue_PCB *pq = createPriorityQueue_PCB();

    int sharedMemoryId = shmget(SHM_KEY, sizeof(int), 0666 | IPC_CREAT);
    if (sharedMemoryId == -1)
    {
        perror("Failed to get shared memory");
        exit(-1);
    }

    key_scheduler = ftok("KeyFileUP", 60);
    if (key_scheduler == -1)
    {
        perror("ftok failed");
        exit(-1);
    }

    scheduler_msg_id = msgget(key_scheduler, 0666 | IPC_CREAT);
    if (scheduler_msg_id == -1)
    {
        perror("msgget failed");
        exit(-1);
    }

    FILE *file = fopen("scheduler.log", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }
    fclose(file);

    file = fopen("scheduler.log", "a");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    MsgBufferScheduler msgReceive;

    while (noProcesses > 0)
    {
        receive_Val = msgrcv(scheduler_msg_id, &msgReceive, sizeof(msgReceive.proc), 1, IPC_NOWAIT);
        if (receive_Val > 0)
        {
            printf("Received new process\n");
            enqueuePriority_PCB(pq, msgReceive.proc, msgReceive.proc.runtime);
        }

        if (!isQueueEmpty_PCB(pq))
        {
            printf("Executing highest priority job\n");

            PCB currentProcess = dequeuePriority_PCB(pq);
            currentProcess.remainingTime = currentProcess.runtime;
            logProcessState(file, getClk(), currentProcess, "started");
            pid_t runningPid = fork();
            if (runningPid == 0)
            {
                int start_time = getClk();
                int *shared_number = (int *)shmat(sharedMemoryId, NULL, 0);
                if (shared_number == (int *)-1)
                {
                    perror("shmat");
                    exit(1);
                }
                *shared_number = currentProcess.runtime;
                execl("./process.o", "process.o", NULL);
                shmdt(shared_number);
                if (shmctl(sharedMemoryId, IPC_RMID, NULL) == -1)
                {
                    perror("shmctl IPC_RMID failed");
                    exit(1);
                }
                sleep(currentProcess.runtime);
                exit(0);
            }
            else
            {
                int status;
                pid_t completedPid = waitpid(runningPid, &status, 0); // Wait for the specific child

                if (completedPid == -1)
                {
                    perror("waitpid failed");
                }
                else if (WIFEXITED(status)) // Check if child terminated normally
                {
                    printf("Child process %d finished with status %d\n", completedPid, WEXITSTATUS(status));
                }

                currentProcess.finishTime = getClk();
                printf("Process %d finished at %d\n", currentProcess.processID, currentProcess.finishTime);
                PCB_array[index] = currentProcess;
                logProcessState(file, getClk(), PCB_array[index], "finished");
                index++;
                noProcesses--;
                printf("No processes remaining: %d\n", noProcesses);
            }
        }
    }
    if (shmctl(sharedMemoryId, IPC_RMID, NULL) == -1)
    {
        perror("shmctl IPC_RMID failed");
        exit(1);
    }
    printf("All processes completed\n");
    printf("PCB_array %d\n", PCB_array[0].finishTime);
    fflush(stdout);
    PCB_array[0].finishTime = 5;
    printf("PCB_array index 0 : %d\n", PCB_array[0].finishTime);
    free(PCB_array);
    fclose(file);
    printf("finished\n");
}

//? ============================================ Preemptive Highest Priority First ALGORITHM ===========================================================

void pHPF(int noProcesses)
{
    printf("Preemptive HPF Begin\n");
    PCB *PCB_array = (PCB *)malloc(noProcesses * sizeof(PCB));
    key_t key_scheduler;
    int receive_Val;
    int scheduler_msg_id, index = 0;
    priority_queue_PCB *pq = createPriorityQueue_PCB();

    // creating shared memory
    int sharedMemoryId = shmget(SHM_KEY, sizeof(int), 0666 | IPC_CREAT);
    if (sharedMemoryId == -1)
    {
        perror("Failed to get shared memory");
        exit(-1);
    }

    // creating message queue
    key_scheduler = ftok("KeyFileUP", 60);
    if (key_scheduler == -1)
    {
        perror("ftok failed");
        exit(-1);
    }

    scheduler_msg_id = msgget(key_scheduler, 0666 | IPC_CREAT);
    if (scheduler_msg_id == -1)
    {
        perror("msgget failed");
        exit(-1);
    }

    // File for logging
    FILE *file = fopen("scheduler.log", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }
    fclose(file);

    file = fopen("scheduler.log", "a");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    MsgBufferScheduler msgReceive;
    PCB currentProcess = {0};
    pid_t runningPid = -1; // Track currently running process

    while (noProcesses > 0)
    {
        // Check for new processes in the message queue
        receive_Val = msgrcv(scheduler_msg_id, &msgReceive, sizeof(msgReceive.proc), 1, IPC_NOWAIT);
        if (receive_Val > 0)
        {
            printf("At time %d: Received new process ID: %d with Priority: %d\n", getClk(), msgReceive.proc.processID, msgReceive.proc.processPriority);
            msgReceive.proc.remainingTime = msgReceive.proc.runtime; // Initialize remaining time
            enqueuePriority_PCB(pq, msgReceive.proc, msgReceive.proc.processPriority);
        }

        // Handle preemption or start a new process if no process is running
        if (!isQueueEmpty_PCB(pq))
        {
            PCB nextProcess = dequeuePriority_PCB(pq); // Get the next process

            if (runningPid == -1 || nextProcess.processPriority < currentProcess.processPriority)
            {
                // Stop current process if running
                if (runningPid != -1)
                {
                    kill(runningPid, SIGSTOP);
                    currentProcess.remainingTime -= (getClk() - currentProcess.startTime);
                    logProcessState(file, getClk(), currentProcess, "stopped");
                    printf("At time %d: Process %d preempted, remaining time: %d\n", getClk(), currentProcess.processID, currentProcess.remainingTime);
                    currentProcess.LastExecTime = getClk();

                    // Re-enqueue if remaining time is > 0
                    if (currentProcess.remainingTime > 0)
                    {
                        enqueuePriority_PCB(pq, currentProcess, currentProcess.processPriority);
                    }
                }

                // Start the next process
                currentProcess = nextProcess;
                currentProcess.startTime = getClk();

                if (currentProcess.remainingTime == currentProcess.runtime)
                {
                    currentProcess.waitingTime = currentProcess.startTime - currentProcess.arrivalTime;
                    logProcessState(file, getClk(), currentProcess, "started");
                    printf("At time %d: Process %d started with Priority: %d\n", getClk(), currentProcess.processID, currentProcess.processPriority);
                }
                else
                {
                    currentProcess.waitingTime += getClk() - currentProcess.LastExecTime;
                    logProcessState(file, getClk(), currentProcess, "resumed");
                    printf("At time %d: Process %d resumed with Priority: %d\n", getClk(), currentProcess.processID, currentProcess.processPriority);
                }

                runningPid = fork();
                if (runningPid == 0)
                {
                    // Child process execution
                    int *shared_number = (int *)shmat(sharedMemoryId, NULL, 0);
                    if (shared_number == (int *)-1)
                    {
                        perror("shmat");
                        exit(1);
                    }
                    *shared_number = currentProcess.remainingTime;

                    execl("./process.o", "process.o", NULL);
                    perror("execl failed");
                    shmdt(shared_number);
                    exit(1);
                }
            }
            else
            {
                // Re-enqueue if not preempting
                enqueuePriority_PCB(pq, nextProcess, nextProcess.processPriority);
            }
        }

        // Check if the current process has finished
        if (runningPid != -1)
        {
            int status;
            pid_t completedPid = waitpid(runningPid, &status, WNOHANG);
            if (completedPid > 0) // Process finished
            {
                printf("At time %d: Process %d finished\n", getClk(), currentProcess.processID);
                currentProcess.finishTime = getClk();
                logProcessState(file, getClk(), currentProcess, "finished");

                PCB_array[index++] = currentProcess;
                runningPid = -1; // Reset runningPid since no process is running
                noProcesses--;
                printf("Processes remaining: %d\n", noProcesses);
            }
        }
    }

    // Cleanup
    if (shmctl(sharedMemoryId, IPC_RMID, NULL) == -1)
    {
        perror("shmctl IPC_RMID failed");
        exit(1);
    }

    fclose(file);
    free(PCB_array);
    printf("Preemptive HPF finished\n");
}