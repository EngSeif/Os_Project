#include "headers.h"

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
}

typedef struct process
{
    int id;
    int arrival_time;
    int runtime;
    int priority;

} process;

typedef struct MsgBufferScheduler
{
    long mtype;
    process proc;
} MsgBufferScheduler;

int countFileLines(char *FileName)
{
    int lines = 0;
    char buff[1024];
    FILE *process_file;

    process_file = fopen(FileName, "r");

    if (process_file == NULL)
    {
        printf("Error! Cannot Open File: %s\n", FileName);
        exit(1); // Return -1 for file open failure
    }

    // Read through each line of the file
    while (fgets(buff, sizeof(buff), process_file) != NULL)
    {
        lines++;
    }

    fclose(process_file);
    return lines;
}
void fillProcessArray(char *FileName, process *Process_Array)
{
    int id, arrival_time, runtime, priority, i = 0;
    char buff[1024];
    FILE *process_file;

    process_file = fopen(FileName, "r");

    if (process_file == NULL)
    {
        printf("Error ! Cannot Open File \n");
        return;
    }

    // skip first Line
    fgets(buff, sizeof(buff), process_file);

    while (fgets(buff, sizeof(buff), process_file) != NULL)
    {
        if (sscanf(buff, "%d %d %d %d", &id, &arrival_time, &runtime, &priority) == 4)
        {
            Process_Array[i].id = id;
            Process_Array[i].arrival_time = arrival_time;
            Process_Array[i].runtime = runtime;
            Process_Array[i].priority = priority;
            i++;
        }
        else
        {
            printf("Error extracting numbers from the line.\n");
        }
    }
    fclose(process_file);
}

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

void sendToScheduler(process Process)
{
    key_t key_scheduler;
    int scheudlar_msg_id, send_val;

    key_scheduler = ftok("KeyFileUP", 60);
    if (key_scheduler == -1)
    {
        perror("ftok failed");
        exit(-1);
    }

    scheudlar_msg_id = msgget(key_scheduler, 0666 | IPC_CREAT);
    if (scheudlar_msg_id == -1)
    {
        perror("msgget failed");
        exit(-1);
    }

    MsgBufferScheduler msgSend;
    msgSend.mtype = 1;
    msgSend.proc = Process;

    send_val = msgsnd(scheudlar_msg_id, &msgSend, sizeof(msgSend.proc), !IPC_NOWAIT);
    if (send_val == -1)
    {
        perror("Error in Sending Message From Process Generator to Client");
        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    process *Process_array;
    int AlgNo, Quanta = 1, file_lines, index = 0;
    char *file_name;

    signal(SIGINT, clearResources);

    // TODO Initialization
    // 1. Read the input files.
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    validate_input(argc, argv, &AlgNo, &Quanta);
    file_name = argv[1];
    file_lines = countFileLines(file_name) - 1;
    Process_array = (process *)malloc((file_lines) * sizeof(process));
    if (Process_array == NULL)
    {
        printf("Failed To allocate an array for processes\n");
        return 1;
    }
    fillProcessArray(file_name, Process_array);
    // 3. Initiate and create the scheduler and clock processes.

    pid_t clock_pid = fork();
    if (clock_pid == 0)
    {
        execl("./clock.o", "./clock.o", NULL);
        perror("Error executing Clock");
        exit(1);
    }

    pid_t scheduler_pid = fork();
    if (scheduler_pid == 0)
    {
        execl("./scheduler.o", "testcase.txt", "-sch", AlgNo, NULL);
        perror("Error executing Scheduler");
        exit(1);
    }

    // 4. Use this function after creating the clock process to initialize clock.
    sleep(1);
    initClk();
    int current_time = getClk();
    printf("Current Time is %d\n", current_time);

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    printf("Begin Sending.....\n");
    while (index < file_lines)
    {
        while (index < file_lines && Process_array[index].arrival_time <= getClk())
        {
            printf("Current Time is %d\n", getClk());
            printf("Sendinng Process of Index %d\n\n", index);
            sendToScheduler(Process_array[index]);
            index++;
        }
    }

    // 7. Clear clock resources
    destroyClk(true);

    return 0;
}
