#include "headers.h"
#include "./DataStructures/PCB.h"
//? scheduler_msg_id : ID of Message Queue Between Process Generator and Scheduler
//? Global To be Accessed by Signal Killer Function

int scheduler_msg_id;

//? msgBuff : Message Received by the process Generator from Scheduler when it ends

typedef struct msgBuff
{
    long mType;
    char mText[256];
} msgBuff;

//? MsgBufferScheduler : Message Send by the process Generator from Scheduler About Processes sent

typedef struct MsgBufferScheduler
{
    long mtype;
    PCB proc;
} MsgBufferScheduler;

/**
 ** clearResources - Signal handler to release resources and terminate the process.
 *
 ** This function is triggered when a termination signal (e.g., SIGINT) is received.
 ** It performs the following actions:
 ** 1. Destroys the message queue associated with the scheduler to free up system resources.
 ** 2. Destroys the clock by calling the `destroyClk` function to ensure proper cleanup.
 ** 3. Exits the process gracefully to prevent any further execution.
 *
 *! Parameters:
 * @signum: The signal number triggering the handler, such as SIGINT.
 *
 *! Note:
 ** It ensures that system resources like message queues and clocks are not left
 ** orphaned after the process terminates.
 */

void clearResources(int signum)
{
    //? Clears all resources in case of interruption

    printf("Process Generator: Destroying Message Queue...\n");
    msgctl(scheduler_msg_id, IPC_RMID, (struct msqid_ds *)0);

    printf("Process Generator: Destroying Clock...\n");
    destroyClk(true);

    exit(0);
}

/**
 ** countFileLines - Counts the number of lines in a file.
 *
 ** @FileName: Path to the file to be read.
 *
 ** Return: Number of lines in the file.
 **         Exits with an error message if the file cannot be opened.
 *
 ** Description: Reads the file line by line and counts lines. Closes the file
 ** properly after reading.
 */

int countFileLines(char *FileName)
{
    int lines = 0;
    char buff[1024];
    FILE *process_file;

    process_file = fopen(FileName, "r");

    if (process_file == NULL)
    {
        printf("Process Generator: Error! Cannot Open File: %s\n", FileName);
        exit(1); // Return -1 for file open failure
    }

    // Read through each line of the file
    while (fgets(buff, sizeof(buff), process_file) != NULL)
        lines++;

    fclose(process_file);
    return lines;
}

/**
 ** fillProcessArray - Populates an array of processes from a file.
 *
 ** @FileName: Path to the input file containing process data.
 ** @Process_Array: Array of process structures to be filled with data.
 *
 ** Description:
 *? - Opens the file specified by FileName for reading.
 *? - Skips the first line of the file (assumed to be headers or irrelevant data).
 *? - Reads each subsequent line, extracting process information (id, arrival_time, runtime, priority).
 *? - Populates the corresponding fields in the Process_Array.
 *? - Prints an error message if a line's data cannot be extracted or if the file cannot be opened.
 *? - Closes the file after processing.
 */

void fillProcessArray(char *FileName, PCB *Process_Array)
{
    int id, arrival_time, runtime, priority, i = 0;
    char buff[1024];
    FILE *process_file;

    process_file = fopen(FileName, "r");

    if (process_file == NULL)
    {
        printf("Process Generator : Error ! Cannot Open File \n");
        return;
    }

    // skip first Line
    fgets(buff, sizeof(buff), process_file);

    while (fgets(buff, sizeof(buff), process_file) != NULL)
    {
        sscanf(buff, "%d %d %d %d", &Process_Array[i].processID, &Process_Array[i].arrivalTime, &Process_Array[i].runtime, &Process_Array[i].originalPriority);
        Process_Array[i].processPriority = Process_Array[i].originalPriority;
        Process_Array[i].finishTime = -1;
        Process_Array[i].processPID = -1;
        Process_Array[i].remainingTime = 0;
        Process_Array[i].startTime = -1;
        Process_Array[i].turnAroundTime = 0;
        Process_Array[i].waitingTime = 0;
        Process_Array[i].executionTime = 0;
        Process_Array[i].LastExecTime = 0;
        Process_Array[i].weightedTurnAroundTime = 0;
        i++;
    }
    fclose(process_file);
}

/**
 ** validate_input - Validates the command-line arguments provided to the program.
 *
 * @argc: Number of command-line arguments.
 * @argv: Array of command-line arguments.
 * @algno: Pointer to an integer where the selected scheduling algorithm will be stored.
 * @quanta: Pointer to an integer where the quantum value (for Round Robin) will be stored.
 *
 *! Description:
 *?  - Ensures that the minimum required arguments are provided (at least 4 arguments).
 *?  - Displays usage instructions and exits the program if arguments are insufficient.
 *?  - Validates the scheduling algorithm number (*AlgNo) provided in argv[3]:
 *?  - Must be a valid integer between 1 and 4.
 *?  - Prints an error message and exits if the value is invalid.
 *? - If a quantum value (*Quanta) is provided (expected at argv[5]), it is parsed and stored.
 */

void validate_input(int argc, char *argv[], int *AlgNo, int *Quanta)
{
    if (argc < 4)
    {
        printf("<usage>: ./process_generator.o {textFile Path} -sch {ALgorithm Number} -q {Quanta for Round Robin Algorithm} \n");
        exit(1);
    }

    *AlgNo = atoi(argv[3]);
    if (*AlgNo == 0 || *AlgNo < 1 || *AlgNo > 4)
    {
        printf("Please Enter a vaild Value for scheduling Algorithm:\n");
        printf("1. Shortest Job First (SJF)\n");
        printf("2. Preemptive Highest Priority First (HPF)\n");
        printf("3. Round Robin (RR)\n");
        printf("4. Multiple level Feedback Loop\n");
        exit(1);
    }

    if (argc == 6)
        *Quanta = atoi(argv[5]);
}

/**
 ** sendToScheduler - Sends a process to the scheduler via a message queue.
 *
 * @process: The process struct to send.
 *
 *! Description:
 *? - Creates a unique key using `ftok` and retrieves the scheduler's message queue ID.
 *? - Prepares a message with type `1` and the given process data.
 *? - Sends the message to the scheduler, exiting on failure.
 */

void sendToScheduler(PCB Process)
{
    key_t key_scheduler;
    int send_val;

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

    MsgBufferScheduler msgSend;
    msgSend.mtype = 1;
    msgSend.proc = Process;

    send_val = msgsnd(scheduler_msg_id, &msgSend, sizeof(msgSend.proc), !IPC_NOWAIT);
    if (send_val == -1)
    {
        perror("Error in Sending Message From Process Generator to Client");
        exit(-1);
    }
}

/**
 ** wait_to_finish - Waits for a signal from the scheduler to terminate.
 *
 *! Description:
 *? - Generates a unique key using `ftok` and retrieves the message queue ID.
 *? - Waits to receive a message of type `1` from the scheduler.
 *? - On successful reception, sends a `SIGINT` signal to terminate processes.
 *? - Exits on any failure in message queue operations.
 */

void wait_to_finish()
{

    key_t key_scheduler;
    int receive_Val;

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

    msgBuff msgReceive;
    receive_Val = msgrcv(scheduler_msg_id, &msgReceive, sizeof(msgReceive.mText), 2, !IPC_NOWAIT);

    if (receive_Val == -1)
    {
        printf("Process Generator : Error in Sending Message From Scheduler To Process Generator\n");
        exit(-1);
    }

    kill(0, SIGINT);
}

int main(int argc, char *argv[])
{
    PCB *Process_array;
    int AlgNo, Quanta = 1, file_lines, index = 0;
    char *file_name;

    signal(SIGINT, clearResources);

    // TODO Initialization
    // 1. Read the input files.
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    validate_input(argc, argv, &AlgNo, &Quanta);
    file_name = argv[1];
    file_lines = countFileLines(file_name) - 1;
    Process_array = (PCB *)malloc((file_lines) * sizeof(PCB));
    if (Process_array == NULL)
    {
        printf("Process Generator : Failed To allocate an array for processes\n");
        return 1;
    }
    fillProcessArray(file_name, Process_array);
    // 3. Initiate and create the scheduler and clock processes.

    pid_t clock_pid = fork();
    if (clock_pid == 0)
    {
        execl("./clk.out", "./clk.out", NULL);
        perror("Error executing Clock");
        exit(1);
    }
    else
    {
        pid_t scheduler_pid = fork();
        if (scheduler_pid == 0)
        {
            if (Quanta <= 0)
            {
                char file_lines_str[100];
                sprintf(file_lines_str, "%d", file_lines);
                execl("./scheduler.out", "./scheduler.out", file_lines_str, "-sch", argv[3], "-q", "0", NULL);
                perror("Error executing Scheduler");
                exit(1);
            }
            else
            {
                char file_lines_str[100];
                sprintf(file_lines_str, "%d", file_lines);
                execl("./scheduler.out", "./scheduler.out", file_lines_str, "-sch", argv[3], "-q", argv[5], NULL);
                perror("Error executing Scheduler");
                exit(1);
            }
        }
        else
        {
            // 4. Use this function after creating the clock process to initialize clock.
            initClk();
            int current_time = getClk();
            printf("Process Generator : Current Time is %d\n", current_time);

            // TODO Generation Main Loop
            // 5. Create a data structure for processes and provide it with its parameters.
            // 6. Send the information to the scheduler at the appropriate time.
            printf("Process Generator : Begin Sending.....\n");
            while (index < file_lines)
            {
                while (index < file_lines && Process_array[index].arrivalTime <= getClk())
                {
                    printf("Process Generator : Current Time is %d\n", getClk());
                    printf("Process Generator : Sendinng Process of Index %d\n", index);
                    sendToScheduler(Process_array[index]);
                    index++;
                }
            }

            free(Process_array);
            wait_to_finish();
        }
    }

    return 0;
}
