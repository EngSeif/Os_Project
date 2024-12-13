#include "headers.h"
#include <stdbool.h>
#include <string.h>
#include "CircularQueue.h"
#include <stdlib.h>
#include <stdio.h>     

#define RR_Quantum 3


typedef struct {
    int processID; 
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

void readProcessesFromFile(const char* fileName , PCB **processesArray , int n);
void roundRobinScheduler(PCB *processes, int n, int quantum);
int main(int argc, char *argv[])
{
    initClk();
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
    if(argc<3){
        printf("Error! , You should enter program name , number of processes , scheduling algorithm number and quantum (in case you are choosing round robin) ");
        exit(1);
    }

    int processCount = atoi(argv[1]);
    int schedulingAlgorithm = atoi(argv[2]);
    if(schedulingAlgorithm ==3 && argc<4){
        printf("Error! You can't choose round robin and don't enter the quantum");
        exit(1);
    }

    int quantum;
    if(schedulingAlgorithm==3) quantum = atoi(argv[3]);
    PCB *processes; //array that will hold the processes
    readProcessesFromFile("process.txt", &processes, processCount);

    if(schedulingAlgorithm==3)roundRobinScheduler(processes, processCount, quantum);

    free(processes); // Free dynamically allocated memory
    destroyClk(true);
}


void readProcessesFromFile(const char* fileName , PCB **processesArray , int numberOfProcesses){
    FILE *processFile = fopen(fileName , "r"); //read from file specified
    if (!processFile) {
        printf("Failed to open %s file" , fileName);
        exit(1);
    }

    *processesArray = malloc((numberOfProcesses) * sizeof(PCB));
    if (!(*processesArray)) {
        perror("Failed to allocate memory for processes");
        fclose(fileName);
        exit(1);
    }

    sscanf(processFile , "%s %s %s %s"); //ignoring first line as it has the headers

    for(int i = 0 ; i < numberOfProcesses ; i++){
        sscanf(processFile , "%d %d %d %d" , (*processesArray)[i].processID ,(*processesArray)[i].arrivalTime , (*processesArray)[i].runtime , (*processesArray)[i].processPriority);
        (*processesArray)[i].finishTime=-1;
        (*processesArray)[i].remainingTime=0;
        (*processesArray)[i].startTime=-1;
        (*processesArray)[i].turnAroundTime=0;
        (*processesArray)[i].waitingTime=0;
        (*processesArray)[i].executionTime=0;
    }
    fclose(processFile);
}

// Helper function to write logs to the output file
void logProcessState(FILE *file, int currenTime, PCB process, const char *state) {
    fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d",
            currenTime, process.processID, state, process.arrivalTime, 
            process.turnAroundTime + process.remainingTime, process.remainingTime, process.waitingTime);

    if (strcmp(state, "finished") == 0) {
        float TA = process.turnAroundTime;
        float WTA = TA / (process.turnAroundTime + process.remainingTime);
        fprintf(file, " TA %.2f WTA %.2f", TA, WTA);
    }

    fprintf(file, "\n");
}

void roundRobinScheduler(PCB *processes, int n, int quantum){
    CircularQueue *readyQueue = CreateCircularQueue();
    if(!readyQueue){
        perror("Failed to create the ready queue");
        return;
    }

    int currentTime = getClk(); // Get the initial current time from the clock
    PCB runningProcess;         // Current running process
    bool isRunning = false;     // Flag to indicate if a process is running

    FILE *output = fopen("scheduler.log", "w");
    if (!output) {
        perror("Failed to open output file");
        return;
    }


    while (1) {
        currentTime = getClk(); // Update the current time

        // Add newly arrived processes to the ready queue
        for (int i = 0; i < n; i++) {
            if (processes[i].arrivalTime == currentTime) {
                enqueueCircular(readyQueue, i); // Enqueue process index arriving at the current time
                logProcessState(output, currentTime, processes[i], "started");
            }
        }

        // If no process is running, fetch the next from the ready queue
        if (!isRunning && readyQueue->Front) {
            int nextProcessIndex = dequeueCircular(readyQueue); // Dequeue the next process index
            runningProcess = processes[nextProcessIndex]; // Fetch the full PCB for the dequeued process
            if (runningProcess.startTime == -1) {
                runningProcess.startTime = currentTime; // Record the start time if not already set
            }
            logProcessState(output, currentTime, runningProcess, "resumed");
            isRunning = true; // Mark that a process is running
        }

        if(isRunning){
            int excuteTime = (runningProcess.remainingTime>=quantum)? quantum:runningProcess.remainingTime; //get excute time that process needs
            runningProcess.remainingTime-=excuteTime; //Run the process for the quantum or until completion

            sleep(excuteTime); //time passing
            currentTime += excuteTime;
            //by this logic if the quantum is for example 4 but remainig time was 2 then running process only excutes for 2 units then the other 2 are given to the next process

            if(runningProcess.remainingTime==0){ //process finished after running that quantum
                runningProcess.finishTime = currentTime;
                runningProcess.turnAroundTime = currentTime - runningProcess.arrivalTime;
                //runningProcess.waitingTime //! needs to be calculated
                runningProcess.runtime = runningProcess.finishTime-runningProcess.startTime;

                logProcessState(output, currentTime, runningProcess, "finished");
                processes[runningProcess.processID] = runningProcess; // Update the process in the array
                isRunning=false;
            }
            else {
                // Preempt the process if time quantum is over
                 logProcessState(output, currentTime, runningProcess, "stopped");
                enqueueCircular(readyQueue, runningProcess.processID); // Add the process index back to the queue
                isRunning = false;       // Mark no process is running
            }

        }

        // End simulation if all processes are completed
        bool all_done = true; // Flag to check if all processes are done
        for (int i = 0; i < n; i++) {
            if (processes[i].remainingTime > 0) {
                all_done = false; // At least one process is not done
                break;
            }
        }

        if (all_done && !readyQueue->Front && !isRunning) {
            break; // Exit the loop if all processes are done
        }

        //! not sure if it should be here for now
        //printf("CPU Utilization: %.2f%%\n", (float)(current_time - processes[0].arrival_time) / current_time * 100);



    }

}