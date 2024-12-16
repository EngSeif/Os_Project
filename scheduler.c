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
    queue_PCB **levels;
} MLFQ;

typedef struct MsgBufferScheduler
{
    long mtype;
    PCB proc;
} MsgBufferScheduler;

typedef struct msgBuff
{
    long mType;
    char mText[256];
} msgBuff;

void RoundRobinScheduler(int, int);
void ShortestJobFirst(int);
void pHPF(int);
void logProcessState(FILE *, int, PCB *, const char *);
void multiLevelFeedbackScheduler(int, int);

int actualEndTimeForScheduler, totalProcessesRunningTime, totalWaitingTime, totalTA;
float totalWTA;

int main(int argc, char *argv[])
{
    initClk();
    key_t key_scheduler;
    int send_val, scheduler_msg_id;
    int noProcess = atoi(argv[1]);
    int algNO = atoi(argv[3]);
    int quantum = 1;
    if (argv[5] != "0")
    {
        quantum = atoi(argv[5]);
    }

    if (algNO == 1)
        ShortestJobFirst(noProcess);
    else if (algNO == 2)
        pHPF(noProcess);
    else if (algNO == 3)
        RoundRobinScheduler(noProcess, quantum);
    else if (algNO == 4)
        multiLevelFeedbackScheduler(noProcess, quantum);

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

    msgBuff msgSend;
    msgSend.mType = 2;
    strcpy(msgSend.mText, "Scheduler said he is finished");

    send_val = msgsnd(scheduler_msg_id, &msgSend, sizeof(msgSend.mText), !IPC_NOWAIT);
    if (send_val == -1)
    {
        perror("Error in Sending Message From Scheduler to Process Generator");
        exit(-1);
    }
    destroyClk(false);
}

// Helper function to write logs to the output file
void logProcessState(FILE *file, int currentTime, PCB *process, const char *state)
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
                currentTime, process->processID, state, process->arrivalTime,
                process->runtime, process->remainingTime, process->waitingTime);
        process->turnAroundTime = process->finishTime - process->arrivalTime;
        int TA = process->turnAroundTime; // Ensure turnAroundTime is correctly calculated elsewhere
        totalTA += TA;

        // Avoid division by zero
        float WTA = 0;
        if (process->turnAroundTime + process->remainingTime > 0)
        {
            WTA = (float)TA / (process->runtime);
        }
        totalWTA += WTA;

        // Log TA and WTA
        process->weightedTurnAroundTime = totalWTA;
        fprintf(file, " TA %d WTA %.2f", TA, WTA);
    }
    else
    {
        fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d",
                currentTime, process->processID, state, process->arrivalTime,
                process->runtime, process->remainingTime, process->waitingTime);
    }

    // Add a new line to separate entries
    fprintf(file, "\n");

    fflush(file);
}

void perfLogFile(PCB *process, int noProcess)
{
    FILE *file = fopen("scheduler.perf", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }
    fclose(file);
    int SumTotalTime = 0, SumWTA = 0, SumWaiting = 0;

    file = fopen("scheduler.perf", "a");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    for (int i = 0; i < noProcess; i++)
    {
        SumTotalTime += process[i].runtime;
        SumWTA += process[i].weightedTurnAroundTime;
        SumWaiting += process[i].runtime;
    }
    fprintf(file, "CPU utilization = %.2f %% \n", ((float)(SumTotalTime + 1) / getClk()) * 100);
    fprintf(file, "Avg WTA = %.2f\n", (float)(SumWTA / noProcess));
    fprintf(file, "Avg Waiting = %.2f\n", (float)(SumWaiting / noProcess));
    fflush(file);
}

//? ============================================ ROUND ROBIN ALGORITHM ===========================================================

void RoundRobinScheduler(int noProcesses, int quantum)
{

    printf("Scheduler : Round Robin Begin\n");
    int TotalP = noProcesses;
    PCB *PCB_array = (PCB *)malloc(noProcesses * sizeof(PCB)); // Allocate memory for storing PCB array

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

    printf("Scheduler : Entering main loop\n");

    while (true)
    {
        // Check for newly arrived processes in the message queue
        int receive_Val = msgrcv(schedulerMessageID, &msgReceive, sizeof(msgReceive.proc), 1, IPC_NOWAIT);
        if (receive_Val > 0)
        {
            printf("Scheduler : Received new process\n");
            msgReceive.proc.remainingTime = msgReceive.proc.runtime;
            enqueue_circular_PCB(readyQueue, msgReceive.proc);
        }

        if (!isQueueEmpty_circular_PCB(readyQueue)) // Check if the ready queue has processes to execute
        {

            PCB currentProcess = dequeue_circular_PCB(readyQueue);
            pid_t runningPid; // Fork a child process to execute the current process
            if (currentProcess.processPID == -1)
            {
                logProcessState(file, getClk(), &currentProcess, "started");
                printf("Scheduler : Process %d started at time %d\n", currentProcess.processID, getClk());
                runningPid = fork();
                currentProcess.processPID == runningPid;
                if (runningPid == 0)
                {
                    // In child process: set shared memory values for execution
                    // first value in shared memory is remaining time and second value is quantum
                    printf("Scheduler : remaining time: %d\n", currentProcess.remainingTime);
                    *sharedMemory = currentProcess.remainingTime;
                    execl("./process.out", "process.out", NULL); // Execute the process by the child

                    perror("execl failed");
                    exit(-1);
                }
            }
            else
            {
                logProcessState(file, getClk(), &currentProcess, "resumed");
                printf("Scheduler : Process %d resumed at time %d\n", currentProcess.processID, getClk());
                runningPid = currentProcess.processPID;
                *sharedMemory = currentProcess.remainingTime;
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
                logProcessState(file, getClk(), &currentProcess, "finished");
                printf("Scheduler : Process %d finished at time %d\n", currentProcess.processID, currentProcess.finishTime);
                PCB_array[index++] = currentProcess; // storing finished process in array for further calculations
                noProcesses--;
            }
            else
            {
                // If process is not finished, preempt it and re-add to queue
                kill(runningPid, SIGSTOP);
                logProcessState(file, getClk(), &currentProcess, "stopped");
                printf("Scheduler : Process %d stoped at time %d\n", currentProcess.processID, getClk());
                currentProcess.remainingTime = *sharedMemory;
                while (msgrcv(schedulerMessageID, &msgReceive, sizeof(msgReceive.proc), 1, IPC_NOWAIT) != -1)
                {
                    printf("Scheduler : Received new process\n");
                    msgReceive.proc.remainingTime = msgReceive.proc.runtime;
                    printf("Scheduler : added proceess with id : %d\n", msgReceive.proc.processID);
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

    perfLogFile(PCB_array, TotalP);
    free(PCB_array);
}

//? ============================================ MULTI LEVEL FEEDBACK QUEUE ALGORITHM ===========================================================
MLFQ *Create_MLFQ()
{
    MLFQ *mlfq = malloc(sizeof(MLFQ)); // Dynamically allocate memory for MLFQ
    if (mlfq == NULL)
    {
        return NULL; // Handle memory allocation failure
    }

    mlfq->levels = malloc(sizeof(queue_PCB *) * MAX_LEVEL); // Allocate memory for the levels array
    if (mlfq->levels == NULL)
    {
        free(mlfq);  // Free the previously allocated memory for MLFQ
        return NULL; // Handle memory allocation failure
    }

    for (int i = 0; i < MAX_LEVEL; i++)
    {
        mlfq->levels[i] = malloc(sizeof(queue_PCB)); // Allocate memory for each queue at each level
        if (mlfq->levels[i] == NULL)
        {
            // Handle partial allocation failure (free previously allocated memory)
            for (int j = 0; j < i; j++)
            {
                free(mlfq->levels[j]);
            }
            free(mlfq->levels);
            free(mlfq);
            return NULL;
        }
    }

    return mlfq;
}

void Destroy_MLFQ(MLFQ *mlfq)
{
    if (mlfq == NULL)
    {
        return; // If the MLFQ pointer is NULL, there's nothing to free
    }

    // Free each queue at each level
    for (int i = 0; i < MAX_LEVEL; i++)
    {
        free(mlfq->levels[i]); // Free memory allocated for each level's queue
    }

    // Free the memory allocated for the levels array
    free(mlfq->levels);

    // Finally, free the memory allocated for the MLFQ struct itself
    free(mlfq);
}

int enqueueProcessMLFQ(MLFQ *mlfq, PCB Process)
{
    if (Process.processPriority >= 0 && Process.processPriority < MAX_LEVEL)
    {
        enqueue_PCB(mlfq->levels[Process.processPriority], Process);
        return 0;
    }
    else
    {
        perror("Process Prioirty Number is invalid");
        return -1;
    }
}

void multiLevelFeedbackScheduler(int noProcesses, int Quantom)
{
    PCB *PCB_array = (PCB *)malloc(noProcesses * sizeof(PCB)); // Allocate memory for PCB array
    MLFQ *mlfq = Create_MLFQ();
    int TotalP = noProcesses;

    // Declare variables for message queue and shared memory
    key_t schedulerKey;
    int schedulerMessageID, processCount = 0, index = 0;

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

    // Create shared memory
    int sharedMemoryId = shmget(SHM_KEY, sizeof(int), 0666 | IPC_CREAT);
    if (sharedMemoryId == -1)
    {
        perror("Failed to create shared memory");
        exit(-1);
    }

    schedulerKey = ftok("KeyFileUP", 60);
    if (schedulerKey == -1)
    {
        perror("ftok failed");
        exit(-1);
    }

    schedulerMessageID = msgget(schedulerKey, 0666 | IPC_CREAT);
    if (schedulerMessageID == -1)
    {
        perror("msgget failed");
        exit(-1);
    }

    MsgBufferScheduler msgReceive;
    int *sharedMemory = (int *)shmat(sharedMemoryId, NULL, 0);
    if (sharedMemory == (int *)-1)
    {
        perror("Failed to attach shared memory");
        exit(-1);
    }

    printf("Scheduler : Entering main loop\n");

    while (true)
    {
        // Check for newly arrived processes in the message queue
        int receive_Val = msgrcv(schedulerMessageID, &msgReceive, sizeof(msgReceive.proc), 1, IPC_NOWAIT);
        if (receive_Val > 0)
        {
            printf("Scheduler : Received new process\n");
            msgReceive.proc.remainingTime = msgReceive.proc.runtime;
            printf("Scheduler : processPriority %d\n", msgReceive.proc.processPriority);
            enqueueProcessMLFQ(mlfq, msgReceive.proc);
            printf("Scheduler : level %d\n", msgReceive.proc.processPriority);
        }

        // Check each level of the MLFQ (from highest priority to lowest)
        for (int i = 0; i < MAX_LEVEL; i++)
        {
            if (!isQueueEmpty_NormalPCB(mlfq->levels[i])) // If there are processes in the current level's queue
            {
                PCB currentProcess = dequeue_PCB(mlfq->levels[i]);
                printf("Scheduler : level %d\n", i);
                pid_t runningPid; // Fork a child process to execute the current process

                if (currentProcess.processPID == -1)
                {
                    printf("Scheduler :Process %d started at time %d\n", currentProcess.processID, getClk());
                    currentProcess.waitingTime = getClk() - currentProcess.arrivalTime;
                    logProcessState(file, getClk(), &currentProcess, "started");
                    runningPid = fork();
                    currentProcess.processPID = runningPid;

                    if (runningPid == 0)
                    {
                        // In child process: set shared memory values for execution
                        printf("Scheduler : process id %d is working now with remaining time %d\n", currentProcess.processID, currentProcess.remainingTime);
                        *sharedMemory = currentProcess.remainingTime;
                        execl("./process.out", "process.out", NULL); // Execute the process by the child
                        perror("execl failed");
                        exit(-1);
                    }
                }
                else
                {
                    printf("Scheduler : Process %d resumed at time %d\n", currentProcess.processID, getClk());
                    currentProcess.waitingTime += getClk() - currentProcess.LastExecTime;
                    logProcessState(file, getClk(), &currentProcess, "resumed");
                    *sharedMemory = currentProcess.remainingTime;
                    runningPid = currentProcess.processPID;
                    kill(runningPid, SIGCONT);
                }

                // In parent process: calculate execution time
                int executeTime = (currentProcess.remainingTime > Quantom) ? Quantom : currentProcess.remainingTime;
                sleep(executeTime);
                currentProcess.remainingTime = *sharedMemory;
                if (currentProcess.remainingTime == 0)
                {
                    // If process has finished, wait for it and record finish time
                    waitpid(runningPid, NULL, 0);         // to prevent being zombie
                    currentProcess.finishTime = getClk(); // recording finish time
                    logProcessState(file, getClk(), &currentProcess, "finished");
                    printf("Scheduler : Process %d finished at time %d in level %d\n", currentProcess.processID, currentProcess.finishTime, i);
                    PCB_array[index++] = currentProcess; // storing finished process in array for further calculations
                    noProcesses--;
                }
                else
                {
                    // If process is not finished, preempt it and move to the next level
                    kill(runningPid, SIGSTOP);
                    logProcessState(file, getClk(), &currentProcess, "stopped");
                    printf("Scheduler : Process %d stopped at time %d\n", currentProcess.processID, getClk());
                    currentProcess.LastExecTime = getClk();
                    currentProcess.remainingTime = *sharedMemory;
                    if (currentProcess.processPriority == MAX_LEVEL - 1 && currentProcess.remainingTime > 0)
                    {
                        currentProcess.processPriority = currentProcess.originalPriority; // Reset priority to original
                    }
                    else
                    {
                        currentProcess.processPriority = (currentProcess.processPriority + 1 < MAX_LEVEL) ? currentProcess.processPriority + 1 : currentProcess.processPriority;
                    }

                    while (msgrcv(schedulerMessageID, &msgReceive, sizeof(msgReceive.proc), 1, IPC_NOWAIT) != -1)
                    {
                        printf("Scheduler : Received new process\n");
                        msgReceive.proc.remainingTime = msgReceive.proc.runtime;
                        enqueueProcessMLFQ(mlfq, msgReceive.proc);
                    }
                    enqueueProcessMLFQ(mlfq, currentProcess); // Re-enqueue the process into the next level
                }
                break;
            }
        }

        // Exit condition: No more processes to schedule
        if (noProcesses == 0)
            break;
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
    perfLogFile(PCB_array, TotalP);
    Destroy_MLFQ(mlfq);
    free(PCB_array);
}

//? ============================================ Shortest Job First ALGORITHM ===========================================================

void ShortestJobFirst(int noProcesses)
{
    int TotalP = noProcesses;
    printf("Scheduler : SJF Begin\n");
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
            printf("Scheduler : Received new process\n");
            enqueuePriority_PCB(pq, msgReceive.proc, msgReceive.proc.runtime);
        }

        if (!isQueueEmpty_PCB(pq))
        {
            printf("Scheduler : Executing highest priority job\n");

            PCB currentProcess = dequeuePriority_PCB(pq);
            currentProcess.waitingTime = getClk() - currentProcess.arrivalTime;
            currentProcess.remainingTime = currentProcess.runtime;
            logProcessState(file, getClk(), &currentProcess, "started");
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
                execl("./process.out", "process.out", NULL);
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
                    printf("Scheduler : Child process %d finished with status %d\n", completedPid, WEXITSTATUS(status));
                }

                currentProcess.finishTime = getClk();
                printf("Scheduler : Process %d finished at %d\n", currentProcess.processID, currentProcess.finishTime);
                PCB_array[index] = currentProcess;
                logProcessState(file, getClk(), &PCB_array[index], "finished");
                index++;
                noProcesses--;
                printf("Scheduler : No processes remaining: %d\n", noProcesses);
            }
        }
    }
    if (shmctl(sharedMemoryId, IPC_RMID, NULL) == -1)
    {
        perror("shmctl IPC_RMID failed");
        exit(1);
    }
    fclose(file);
    printf("Scheduler : All processes completed\n");
    perfLogFile(PCB_array, TotalP);
    free(PCB_array);
}

//? ============================================ Preemptive Highest Priority First ALGORITHM ===========================================================

void pHPF(int noProcesses)
{
    int TotalP = noProcesses;
    printf("Scheduler : Preemptive HPF Begin\n");
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
            printf("Scheduler : At time %d: Received new process ID: %d with Priority: %d\n", getClk(), msgReceive.proc.processID, msgReceive.proc.processPriority);
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
                    logProcessState(file, getClk(), &currentProcess, "stopped");
                    printf("Scheduler : At time %d: Process %d preempted, remaining time: %d\n", getClk(), currentProcess.processID, currentProcess.remainingTime);
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
                    logProcessState(file, getClk(), &currentProcess, "started");
                    printf("Scheduler : At time %d: Process %d started with Priority: %d\n", getClk(), currentProcess.processID, currentProcess.processPriority);
                }
                else
                {
                    currentProcess.waitingTime += getClk() - currentProcess.LastExecTime;
                    logProcessState(file, getClk(), &currentProcess, "resumed");
                    printf("Scheduler : At time %d: Process %d resumed with Priority: %d\n", getClk(), currentProcess.processID, currentProcess.processPriority);
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

                    execl("./process.out", "process.out", NULL);
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
                printf("Scheduler : At time %d: Process %d finished\n", getClk(), currentProcess.processID);
                currentProcess.finishTime = getClk();
                logProcessState(file, getClk(), &currentProcess, "finished");

                PCB_array[index++] = currentProcess;
                runningPid = -1; // Reset runningPid since no process is running
                noProcesses--;
                printf("Scheduler : Processes remaining: %d\n", noProcesses);
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
    perfLogFile(PCB_array, TotalP);
    free(PCB_array);
    printf("Scheduler : Preemptive HPF finished\n");
}