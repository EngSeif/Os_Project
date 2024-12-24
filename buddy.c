#include "headers.h"
#include "PCB.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct ProcessMemoryBlock {
    int size;
    PCB *process;
    bool isFree;
    int startIndex;
    int endIndex;
    struct ProcessMemoryBlock *left;
    struct ProcessMemoryBlock *right;
} ProcessMemoryBlock;

typedef struct Buddy {
    int size;
    ProcessMemoryBlock *root;
} Buddy;

//!Helper functions:
//1- Create a memory block for the buddy system
ProcessMemoryBlock* createMemoryBlock(int memorySize, int startIndex) {
    // Allocate memory for the new block
    ProcessMemoryBlock *block = (ProcessMemoryBlock *)malloc(sizeof(ProcessMemoryBlock));
    if (!block) {
        perror("Memory allocation failed for ProcessMemoryBlock");
        return NULL;
    }

    // Initialize the block's attributes
    block->size = memorySize;      // Size of the memory block
    block->process = NULL;      // No process assigned initially
    block->isFree = true;           // Block starts as free
    block->startIndex = startIndex;     // Starting index of the block
    block->endIndex = startIndex + memorySize - 1; // Ending index of the block
    block->left = NULL;          // No left child (yet)
    block->right = NULL;         // No right child (yet)

    return block;
}

//2- Create the buddy system with a specified size
Buddy* createBuddy(int size) {
    // Allocate memory for the buddy system
    Buddy *b = (Buddy *)malloc(sizeof(Buddy));
    if (!b) {
        perror("Memory allocation failed for Buddy");
        return NULL;
    }

    // Initialize the buddy system's attributes
    b->size = size;         // Total size of the buddy system
    b->root = createMemoryBlock(size, 0);  // Root block starts with the full size

    if (!b->root) {
        free(b);    // Cleanup if root creation fails
        return NULL;
    }

    return b;
}

//3- Print Buddy:
void printBuddy(ProcessMemoryBlock *block) {
    if (block == NULL) return;

    // Print the current block's process information if not free
    if (!block->isFree && block->process != NULL) {
        printf("Process %d of size %d starts at %d ends at %d\n",
               block->process->id, block->size, block->startIndex, block->endIndex);
    }

    // Recursively print left and right child blocks
    printBuddy(block->left);
    printBuddy(block->right);
}

//4- Find the closest power of 2 for memory allocation 
int getClosestPower(int size) {
    if (size <= 0) return 1; // Handle invalid input gracefully
    double exponent = ceil(log2(size)); // Find the smallest exponent for power of 2
    return (int)pow(2, exponent); // Calculate 2 raised to the power of the exponent
}

//5- Dividing the block of the buddy into two smaller blocks:
bool divideMemoryBlock(ProcessMemoryBlock *block) {
    // Check if the block is indivisible or already divided
    if (block == NULL || block->size == 1 || block->left != NULL || block->right != NULL) {
        return false;
    }

    // Calculate the size of the two new blocks
    int newSize = block->size / 2;

    // Create the left and right child blocks
    block->left = createMemoryBlock(newSize, block->start);
    block->right = createMemoryBlock(newSize, block->start + newSize);

    return true; // Indicate successful division
}


//6- Find the first free block of the specified size
ProcessMemoryBlock *getBlockBySize(ProcessMemoryBlock *block, int size) {
    if (!block) {
        return NULL;
    }

    // Check if the block is suitable
    if (!block->left && !block->right && block->size == size && block->isFree) {
        return block;
    }

    // Recursively search in the left and right children
    ProcessMemoryBlock *foundBlock = getBlockBySize(block->left, size);
    if (foundBlock) {
        return foundBlock;
    }
    return getBlockBySize(block->right, size);
}

//7- Find the block containing the specified process
ProcessMemoryBlock *getBlockByProcess(ProcessMemoryBlock *block, PCB *process) {
    if (!block) {
        return NULL;
    }

    // Check if the block contains the process
    if (block->process == process) {
        return block;
    }

    // Recursively search in the left and right children
    ProcessMemoryBlock *foundBlock = getBlockByProcess(block->left, process);
    if (foundBlock) {
        return foundBlock;
    }
    return getBlockByProcess(block->right, process);
}

//8- Combine free memory blocks back into their parent
void combineMemoryBlocks(ProcessMemoryBlock *block) {
    if (!block) {
        return; // Base case for recursion
    }

    // Post-order traversal: process children first
    combineMemoryBlocks(block->left);
    combineMemoryBlocks(block->right);

    // If both children are free and indivisible, merge them
    if (block->left && block->right && block->left->isFree && block->right->isFree) {
        if (!block->left->left && !block->right->left) {
            free(block->left);
            free(block->right);
            block->left = NULL;
            block->right = NULL;
            block->isFree = true; // Mark parent as free
        }
    }
}

//!Buddy Functionalities:
//1- Insert a process into the buddy system
bool insertBuddy(Buddy *b, PCB *process, int time) {
    int closestSize = getClosestPower(process->memsize);
    ProcessMemoryBlock *suitableBlock = getBlockBySize(b->root, closestSize); //get most suitable block

    if (suitableBlock) {
        // Assign the process to the found block
        suitableBlock->process = process;
        suitableBlock->isFree = false;
        process->allocated = true;

        // Log the allocation
        FILE *outputFile = fopen(memory.log, "a");
        fprintf(outputFile, "At time %d allocated %d bytes for process %d from %d to %d\n",
                time, process->memsize, process->id, suitableBlock->start, suitableBlock->end);
        fclose(outputFile);

        return true;
    }

    // If no suitable block is found, attempt insertion by dividing blocks
    return insertProcess(b->root, process, closestSize, time);
}

//2- Recursively find or create a block for the process
bool insertProcess(ProcessMemoryBlock *block, PCB *process, int closestSize, int time) {
    if (!block || block->size < process->memsize || !block->isFree) {
        return false; // Block is invalid or unsuitable
    }

    // If the block is the exact size and not divided, allocate it
    if (!block->left && !block->right && block->size == closestSize) {
        block->process = process;
        block->isFree = false;
        process->allocated = true;

        // Log the allocation
        FILE *memoryFile = fopen(memory.log, "a");
        fprintf(memoryFile, "At time %d allocated %d bytes for process %d from %d to %d\n",
                time, process->memsize, process->id, block->start, block->end);
        fclose(memoryFile);

        return true;
    }

    // If the block is larger, divide it and try to insert into the left child
    if (!block->left && !block->right && block->size > process->memsize) {
        if (!divideMemoryBlock(block)) {
            return false; // Division failed
        }
        return insertProcess(block->left, process, closestSize, time);
    }

    // Recursively try to insert into the left or right child
    if (insertProcess(block->left, process, closestSize, time)) {
        return true;
    }
    return insertProcess(block->right, process, closestSize, time);
}

//3- Remove a process from the buddy system
bool removeProcess(Buddy *b, PCB *process) {
    // Find the memory block associated with the given process
    ProcessMemoryBlock *block = getBlockByProcess(b->root, process);
    if (!block) {
        return false; // Process not found
    }

    // Log the memory release
    FILE *memoryFile = fopen(memory.log, "a");
    fprintf(memoryFile,
            "At time %d freed %d bytes for process %d from %d to %d\n",
            process->finishTime, process->memsize, process->id,
            block->start, block->end);
    fclose(memoryFile);

    // Mark the block as free and remove the process
    block->process = NULL;
    block->isFree = true;

    // Attempt to combine free memory blocks
    combineMemoryBlocks(b->root);

    return true;
}


//!Cleaning up the rescources:
//1- Free all memory blocks in the buddy system
void freeBlock(ProcessMemoryBlock *block) {
    if (!block) {
        return; // Base case for recursion
    }

    // Free child blocks first
    freeBlock(block->left);
    freeBlock(block->right);

    // Free the current block
    free(block);
}

//2- Free the entire buddy system
void freeBuddy(Buddy *buddy) {
    if (!buddy) {
        return; // Ensure the buddy system exists
    }

    // Free the root block and all its children
    freeBlock(b->root);
}
