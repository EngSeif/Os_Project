#ifndef PROCESS_GENERATOR
#define PROCESS_GENERATOR

#include <stdio.h>
#include <stdlib.h>

typedef struct process_info
{
    int id;
    int arrival_time;
    int runtime;
    int priority;

} process_info;

int countFileLines(char *FileName)
{
    int lines = 0;
    char buff[1024];
    FILE * process_file;

    process_file = fopen(FileName, "r");

    if (process_file == NULL) {
        printf("Error ! Cannot Open File \n");
        return 1;
    }

    while (fgets(buff, sizeof(buff), process_file) != NULL)
        lines++;

    fclose(process_file);
    return lines;
}

void fillProcessArray(char *FileName, process_info* Process_Array) {
    int id, arrival_time, runtime, priority, i = 0;
    char buff[1024];
    FILE * process_file;

    process_file = fopen(FileName, "r");

    if (process_file == NULL) {
        printf("Error ! Cannot Open File \n");
        return;
    }

    //skip first Line
    fgets(buff, sizeof(buff), process_file);

    while (fgets(buff, sizeof(buff), process_file) != NULL)
    {
        if (sscanf(buff, "%d %d %d %d", &id, &arrival_time, &runtime, &priority) == 4) {
            Process_Array[i].id = id;
            Process_Array[i].arrival_time = arrival_time;
            Process_Array[i].runtime = runtime;
            Process_Array[i].priority = priority;
            i++;
        } else {
            printf("Error extracting numbers from the line.\n");
        }
    }
    fclose(process_file);
}

#endif