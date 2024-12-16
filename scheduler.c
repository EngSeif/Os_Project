#include "headers.h"
#include <stdbool.h>
#include <string.h>
#include "./DataStructures/CircularQueueInt.h"
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

void readProcessesFromFile(const char *, PCB **, int);
void roundRobinScheduler(PCB *, int, int);
void ShortestJobFirst(PCB *, int);
void logProcessState(FILE *, int, PCB, const char *);

int actualEndTimeForScheduler, totalProcessesRunningTime, totalWaitingTime, totalTA;
float totalWTA;

int main(int argc, char *argv[])
{
    initClk();
    int noProcess = atoi(argv[1]);
    printf("noProcess : %d\n", noProcess);
    PCB *pcb_array = (PCB *)malloc(sizeof(PCB) * noProcess);
    ShortestJobFirst(pcb_array, noProcess);
    printf("PCB_array index 0 : %d", pcb_array[0].finishTime);

    FILE *file = fopen("final.txt", "w");
    logProcessState(file, getClk(), pcb_array[0], "finished");
    fclose(file);
    sleep(2);
    destroyClk(true);
}

// Helper function to write logs to the output file
void logProcessState(FILE *file, int currenTime, PCB process, const char *state)
{
    // fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d",
    //         currenTime, process.processID, state, process.arrivalTime,
    //         process.turnAroundTime + process.remainingTime, process.remainingTime, process.waitingTime);

    if (strcmp(state, "finished") == 0)
    {
        int TA = process.turnAroundTime;
        totalTA += TA;
        float WTA = TA / (process.turnAroundTime + process.remainingTime);
        totalWTA += WTA;
        fprintf(file, " TA %.2d WTA %.2f", TA, WTA);
    }

    fprintf(file, "\n");
}

//? ============================================ ROUND ROBIN ALGORITHM ===========================================================

// void roundRobinScheduler(PCB *processes, int n, int quantum)
// {
//     int sharedMemeoryId = shmget(SHM_KEY, sizeof(int), 0666 | IPC_CREAT); // creating shared memory for communication with process.c
//     if (sharedMemeoryId == -1)
//     {
//         perror("Failed to create shared memory");
//         return;
//     }

//     int *sharedMemory = (int *)shmat(sharedMemeoryId, NULL, 0); // attach the shared memory segment to the process address space
//     if (sharedMemory == (int *)-1)
//     {
//         perror("Failed to attach shared memory");
//         return;
//     }
//     // it will have 2 parameters sharedMemory[0] includes remaining time , sharedMemory[1] includes quantum

//     CircularQueue *readyQueue = CreateCircularQueue();
//     if (!readyQueue)
//     {
//         perror("Failed to create the ready queue");
//         return;
//     }

//     int currentTime = getClk(); // Get the initial current time from the clock
//     PCB runningProcess;         // Current running process
//     bool isRunning = false;     // Flag to indicate if a process is running
//     pid_t runningPid = -1;      // PID of running process

//     FILE *output = fopen("scheduler.log", "w");
//     if (!output)
//     {
//         perror("Failed to open scheduler.log file for output");
//         return;
//     }

//     totalWaitingTime = 0;
//     totalProcessesRunningTime = 0;
//     actualEndTimeForScheduler = 0;
//     while (1)
//     {
//         currentTime = getClk(); // Update the current time

//         // Add newly arrived processes to the ready queue
//         for (int i = 0; i < n; i++)
//         {
//             if (processes[i].arrivalTime == currentTime)
//             {
//                 enqueueCircular(readyQueue, i); // Enqueue process index arriving at the current time
//                 logProcessState(output, currentTime, processes[i], "started");
//             }
//         }

//         // If no process is running, fetch the next from the ready queue
//         if (!isRunning && readyQueue->Front)
//         {
//             int nextProcessIndex = dequeueCircular(readyQueue); // Dequeue the next process index
//             runningProcess = processes[nextProcessIndex];       // Fetch the full PCB for the dequeued process
//             sharedMemory[0] = runningProcess.remainingTime;     // Write the process remaining time to shared memory
//             sharedMemory[1] = quantum;                          // Write the quantum to shared memory

//             runningPid = fork(); // Creating a new process to run the process.c
//             if (runningPid == 0)
//             { // Child

//                 execl("./process.out", "process.out", NULL); // Execute process.c
//             }
//             else if (runningPid < 0)
//             {
//                 perror("Failed to fork");
//                 continue;
//             }
//             if (runningProcess.startTime == -1)
//             {
//                 runningProcess.startTime = currentTime; // Record the start time if not already set
//             }
//             logProcessState(output, currentTime, runningProcess, "resumed");
//             isRunning = true; // Mark that a process is running
//         }

//         if (isRunning)
//         {
//             int excuteTime = (runningProcess.remainingTime >= quantum) ? quantum : runningProcess.remainingTime; // get excute time that process needs
//             runningProcess.remainingTime -= excuteTime;                                                          // Run the process for the quantum or until completion

//             sleep(excuteTime); // time passing
//             currentTime += excuteTime;
//             // by this logic if the quantum is for example 4 but remainig time was 2 then running process only excutes for 2 units then the other 2 are given to the next process
//             runningProcess.remainingTime = *sharedMemory;
//             if (runningProcess.remainingTime == 0)
//             {               // process finished after running that quantum
//                 wait(NULL); // preventing child process to turn zombie so we makes parent wait until child terminates
//                 runningProcess.finishTime = currentTime;
//                 runningProcess.turnAroundTime = currentTime - runningProcess.arrivalTime;
//                 runningProcess.waitingTime = runningProcess.startTime - runningProcess.arrivalTime;
//                 runningProcess.runtime = runningProcess.finishTime - runningProcess.startTime;
//                 totalWaitingTime += runningProcess.waitingTime;

//                 logProcessState(output, currentTime, runningProcess, "finished");
//                 processes[runningProcess.processID] = runningProcess; // Update the process in the array
//                 isRunning = false;
//             }
//             else
//             {
//                 // Preempt the process if time quantum is over
//                 kill(runningPid, SIGSTOP);                    // stop currently running process as its quantum has finished
//                 runningProcess.remainingTime = *sharedMemory; // update the remaining time from shared memory
//                 logProcessState(output, currentTime, runningProcess, "stopped");
//                 enqueueCircular(readyQueue, runningProcess.processID); // Add the process index back to the queue
//                 isRunning = false;                                     // Mark no process is running
//             }
//         }

//         // End simulation if all processes are completed
//         bool all_done = true; // Flag to check if all processes are done
//         for (int i = 0; i < n; i++)
//         {
//             if (processes[i].remainingTime > 0)
//             {
//                 all_done = false; // At least one process is not done
//                 break;
//             }
//         }

//         if (all_done && !readyQueue->Front && !isRunning)
//         {
//             break; // Exit the loop if all processes are done
//         }
//     }
//     actualEndTimeForScheduler = processes[n - 1].finishTime;
//     float avgWTA = totalWTA / n;
//     float avgWaiting = (float)totalWaitingTime / n;
//     totalProcessesRunningTime = actualEndTimeForScheduler - totalWaitingTime;
//     float CPU_Utilization = (float)((totalProcessesRunningTime / actualEndTimeForScheduler) * 100);
//     FILE *stats = fopen("scheduler.perf", "w");
//     if (!stats)
//     {
//         perror("Failed to open scheduler.perf file for output");
//         return;
//     }
//     fprintf(stats, "CPU Utilization: %.2f%%\nAvg WTA: %.2f%%\nAvg Waiting: %.2f%%", CPU_Utilization, avgWTA, avgWaiting);

//     shmdt(sharedMemory);                     // detaching shared memory after we finished using it
//     shmctl(sharedMemeoryId, IPC_RMID, NULL); // marking shared memory for destruction
// }

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

void ShortestJobFirst(PCB *PCB_array, int noProcesses)
{
    printf("SJB Begin\n");

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

    MsgBufferScheduler msgReceive;

    while (noProcesses > 0)
    {
        receive_Val = msgrcv(scheduler_msg_id, &msgReceive, sizeof(msgReceive.proc), 1, IPC_NOWAIT);
        if (receive_Val > 0)
        {
            printf("recieved\n");
            enqueuePriority_PCB(pq, msgReceive.proc, msgReceive.proc.runtime);
        }
        if (!isQueueEmpty_PCB(pq))
        {
            pid_t runningPid = fork();
            PCB currentProcess = dequeuePriority_PCB(pq);
            currentProcess.startTime = getClk();
            printf("At time %d, Executing Process ID: %d\n", currentProcess.startTime, currentProcess.processID);
            printf("current process Runtime %d\n", currentProcess.runtime);
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
            }
            waitpid(runningPid, NULL, 0);
            currentProcess.finishTime = getClk();
            printf("process %d finished at %d\n", currentProcess.processID, currentProcess.finishTime);
            PCB_array[index] = currentProcess;
            index++;
            noProcesses--;
            printf("no Processes %d\n", noProcesses);
        }
    }
    printf("no P %d\n", noProcesses);
    destroyClk(false);
    return;
}

//? ============================================ Preemptive Highest Priority First ALGORITHM ===========================================================

// void pHPF(PCB **processesArray, int size)
// {
//     // size = no of processes
//     priorityQueue_PCB_Ptr *pq = CreatePriQueue_PCB_Ptr(); // ready queue
//     int processed = 0;                                    // number of processes that finished
//     int current_time = 0;                                 // simulates a real time clock

//     while (processed < size)
//     {
//         for (int i = 0; i < size; i++)
//         {
//             if (processesArray[i]->arrivalTime = current_time)
//                 priEnqueue_PCB_Ptr(pq, processesArray[i], processesArray[i]->processPriority);
//         }
//         if (pq->head != NULL)
//         {
//             PCB *currentProcess = PriDequeue_PCB_Ptr(pq);
//             printf("At %d, executing process %d\n", current_time, currentProcess->processID);

//             // memic simulation
//             // SJB is non-preemptive, so we add runtime to current time directly
//             current_time++;
//             currentProcess->remainingTime--;

//             if (currentProcess->remainingTime == 0)
//             {
//                 printf("Process %d finished at %d\n", currentProcess->processID, current_time);
//                 processed++;
//             }
//             else
//             {
//                 // enqueue process again, so that at start of while loop
//                 // check if a new process with higher priority entered or not
//                 priEnqueue_PCB_Ptr(pq, currentProcess, currentProcess->processPriority);
//             }
//         }
//         else
//         {
//             prinf("No process to use pHPF at %d\n", current_time);
//             current_time++;
//         }
//     }
//     destroyPriQueue_PCB_Ptr(pq);
// }

// initClk();
// if (argc < 3)
// {
//     printf("Error! , You should enter program name , number of processes , scheduling algorithm number and quantum (in case you are choosing round robin) ");
//     exit(1);
// }
// int processCount = atoi(argv[1]);
// int schedulingAlgorithm = atoi(argv[2]);
// if (schedulingAlgorithm == 3 && argc < 4)
// {
//     printf("Error! You can't choose round robin and don't enter the quantum");
//     exit(1);
// }
// totalTA=0;
// totalWTA=0.0;

// int quantum;
// if (schedulingAlgorithm == 3)
//     quantum = atoi(argv[3]);
// PCB *processes; // array that will hold the processes
// readProcessesFromFile("process.txt", &processes, processCount);

// if (schedulingAlgorithm == 3)
//     roundRobinScheduler(processes, processCount, quantum);

// free(processes); // Free dynamically allocated memory
// destroyClk(true);

// void readProcessesFromFile(const char *fileName, PCB **processesArray, int numberOfProcesses)
// {
//     FILE *processFile = fopen(fileName, "r");
//     char buffer[1024];

//     if (!processFile)
//     {
//         printf("Failed to open %s file", fileName);
//         exit(1);
//     }

//     *processesArray = malloc((numberOfProcesses) * sizeof(PCB));
//     if (!(*processesArray))
//     {
//         perror("Failed to allocate memory for processes");
//         fclose(processFile);
//         exit(1);
//     }

//     fgets(buffer, sizeof(buffer), processFile); //! Ignore First Line

//     int i = 0;
//     while (fgets(buffer, sizeof(buffer), processFile) != NULL)
//     {
//         sscanf(buffer, "%d %d %d %d", &(*processesArray)[i].processID, &(*processesArray)[i].arrivalTime, &(*processesArray)[i].runtime, &(*processesArray)[i].processPriority);
//         (*processesArray)[i].finishTime = -1;
//         (*processesArray)[i].remainingTime = 0;
//         (*processesArray)[i].startTime = -1;
//         (*processesArray)[i].turnAroundTime = 0;
//         (*processesArray)[i].waitingTime = 0;
//         (*processesArray)[i].executionTime = 0;
//         i++;
//     }
//     fclose(processFile);
// }