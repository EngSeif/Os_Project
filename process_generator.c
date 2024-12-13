#include "headers.h"

void clearResources(int);

int main(int argc, char *argv[])
{
    int AlgNo, Quanta = 1, file_lines;
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    if (argc < 4)
    {
        printf("<usage>: ./process_generator.o {textFile Path} -sch {ALgorithm Number} -q {Quanta for Round Robin Algorithm} \n");
        return 1;
    }

    AlgNo = atoi(argv[3]);
    if (AlgNo == 0)
    {
        printf("Please Enter a vaild Value for scheduling Algorithm:\n");
        printf("1. Shortest Job First (SJF)\n");
        printf("2. Preemptive Highest Priority First (HPF)\n");
        printf("3. Round Robin (RR)\n");
        printf("4. Multiple level Feedback Loop\n");
        return 1;
    }

    if (argc == 6)
        Quanta = atoi(argv[5]);

    // 3. Initiate and create the scheduler and clock processes.

    // 4. Use this function after creating the clock process to initialize clock.
    initClk();

    // To get time use this function.
    int x = getClk();
    printf("Current Time is %d\n", x);

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.

    // 6. Send the information to the scheduler at the appropriate time.

    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
}

// #include "process_gen.h"

// int main(int argc, char *argv[])
// {
//     process_info *Process_array;
//     int AlgNo, Quanta = 1, file_lines;
//     char *file_name;

//     if (argc < 4)
//     {
//         printf("<usage>: ./process_generator.o {textFile Path} -sch {ALgorithm Number} -q {Quanta for Round Robin Algorithm} \n");
//         return 1;
//     }

//     file_name = argv[1];
//     AlgNo = atoi(argv[3]);
//     if (AlgNo == 0)
//     {
//         printf("Please Enter a vaild Value for scheduling Algorithm:\n");
//         printf("1. Shortest Job First (SJF)\n");
//         printf("2. Preemptive Highest Priority First (HPF)\n");
//         printf("3. Round Robin (RR)\n");
//         printf("4. Multiple level Feedback Loop\n");
//         return 1;
//     }

//     if (argc == 6)
//         Quanta = atoi(argv[5]);

//     file_lines = countFileLines(file_name);
//     printf("%d\n", file_lines);

//     Process_array = (process_info *)malloc((file_lines - 1) * sizeof(process_info));
//     if (Process_array == NULL)
//     {
//         printf("Failed To allocate an array for processes\n");
//         return 1;
//     }

//     fillProcessArray(file_name, Process_array);

//     return 0;
// }