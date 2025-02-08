#pragma once

#include <sys/types.h>
#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers.h"
#include "./DataStructures/PCB.h"

// Memory Block Like A Tree Node
typedef struct MemoryBlock
{
    int size;
    PCB process;
    bool isFree;
    int start;
    int end;
    struct MemoryBlock *left;
    struct MemoryBlock *right;
} MemoryBlock;

typedef struct BuddySystemSystem
{
    int size;
    MemoryBlock *root;
} BuddySystem;

// creates a memory block to be stored in the BuddySystem tree
MemoryBlock *createBlock(int size, int start)
{
    MemoryBlock *newBlock = (MemoryBlock *)malloc(sizeof(MemoryBlock));
    newBlock->size = size;
    newBlock->start = start;
    newBlock->end = start + size - 1;
    newBlock->isFree = true;
    PCB empty = {0};
    newBlock->process = empty;
    newBlock->left = NULL;
    newBlock->right = NULL;
    return newBlock;
}

// init a body system with a specific size
BuddySystem *createBuddySystemSystem(int size)
{
    BuddySystem *b = (BuddySystem *)malloc(sizeof(BuddySystem));
    b->size = size;
    b->root = createBlock(size, 0);
    return b;
}

void printBuddySystem(MemoryBlock *block, int depth, const char *prefix)
{
    if (block == NULL)
        return;

    //%*s: This specifies a dynamic-width string
    // indentation for string

    printf("%*s%s%d [%d][%d-%d]\n", depth * 4, "", prefix, block->size,
           block->isFree, block->start, block->end);

    if (block->left != NULL || block->right != NULL)
    {
        printBuddySystem(block->right, depth + 1, "└──R:");
        printBuddySystem(block->left, depth + 1, "└──L:");
    }
}

int getClosestPowerOfTwo(int size) { return (int)pow(2, ceil(log2(size))); }

// a function to divde the block size into two
bool divideBlock(MemoryBlock *block)
{
    if (block->size == 1 || block->left != NULL || block->right != NULL)
    {
        return false;
    }
    block->left = createBlock(block->size / 2, block->start);
    block->right = createBlock(block->size / 2, block->start + block->size / 2);
    return true;
}

MemoryBlock *findBlockBySize(MemoryBlock *block, int size)
{
    if (!block)
        return NULL;

    // if node does not have childs
    if (!block->left && !block->right)
    {
        if (block->size == size && block->isFree)
            return block;
    }

    // if it has childs begin seraching in left
    MemoryBlock *leftFind = findBlockBySize(block->left, size);
    if (leftFind)
        return leftFind;

    // if theres is nothing in the left serach in the right
    return findBlockBySize(block->right, size);
}

MemoryBlock *findBlockByProcess(MemoryBlock *block, PCB *process)
{
    if (!block)
        return NULL;

    // check if the node has my process
    if (block->process.processID == process->processID)
    {
        printf("Matching\n");
        return block;
    }

    // check if the
    MemoryBlock *leftFind = findBlockByProcess(block->left, process);
    if (leftFind)
        return leftFind;

    return findBlockByProcess(block->right, process);
}

bool insertProcess(MemoryBlock *block, PCB *process, int closest_size, int time, FILE *memoryFile)
{
    if (!block)
        return false;
    if (block->size < process->memsize)
        return false;
    if (!block->isFree)
        return false;

    // without dividing if node is the same size assign the procss to it
    if (!block->left && !block->right && block->size == closest_size)
    {
        block->process = *process;
        block->isFree = false;
        process->allocated = true;
        printf("process id %d\n", process->processID);
            fprintf(memoryFile,
                    "At time %d allocated %d bytes for process %d from %d "
                    "to %d\n",
                    time, process->memsize, process->processID, block->start, block->end);
        fflush(memoryFile);
        printf("written\n");
        return true;
    }

    // if no division for node and in the same time block size is bigger
    if (!block->left && !block->right && block->size > process->memsize)
    {
        // divide the block and if it cannot be divided means that it cannot be allocated
        if (!divideBlock(block))
            return false;
        // after dividing block insert the process
        return insertProcess(block->left, process, closest_size, time, memoryFile);
    }
    else
    {
        // if process has childs try to insert in the left child if not try the right child
        if (insertProcess(block->left, process, closest_size, time, memoryFile))
            return true;
        return insertProcess(block->right, process, closest_size, time, memoryFile);
    }

    return false;
}

bool insertIntoBuddySystem(BuddySystem *b, PCB *process, int time, FILE *memoryFile)
{
    int closest_size = getClosestPowerOfTwo(process->memsize);
    MemoryBlock *closest = findBlockBySize(b->root, closest_size);

    // if a block of the size is already found
    if (closest)
    {
        closest->process = *process;
        closest->isFree = false;
        process->allocated = true;
        fprintf(memoryFile,
                "At time %d allocated %d bytes for process %d from %d "
                "to %d\n",
                time, process->memsize, process->processID, closest->start,
                closest->end);
        fflush(memoryFile);
        return true;
    }

    // if not call the insert procees to divide and insert
    return insertProcess(b->root, process, closest_size, time, memoryFile);
}

// post-order traversal for tree in order to combine memory blocks nodes

void combineMemoryBlocks(MemoryBlock *block)
{
    if (!block)
        return;

    combineMemoryBlocks(block->left);
    combineMemoryBlocks(block->right);

    // if node has no childs it needs no merging return

    if (!block->left && !block->right)
        return;

    // if there is a node to be merged check first if both childs are free
    if (block->left->isFree && block->right->isFree)
    {
        if (block->left->left || block->right->left)
            return;
        free(block->left);
        free(block->right);
        block->left = NULL;
        block->right = NULL;
    }
}

bool removeProcess(BuddySystem *b, PCB *process, FILE *memoryFile)
{
    MemoryBlock *block = findBlockByProcess(b->root, process);

    if (!block)
        return false;

    printf("Process Removed\n");

    PCB empty = {0};
    block->process = empty;
    block->isFree = true;

    fprintf(memoryFile,
            "At time %d freed %d bytes for process %d from %d "
            "to %d\n",
            process->finishTime, process->memsize, process->processID, block->start,
            block->end);
    fflush(memoryFile);
    combineMemoryBlocks(b->root);

    return true;
}

void freeBlock(MemoryBlock *block)
{
    if (!block)
        return;
    freeBlock(block->left);
    freeBlock(block->right);
    free(block);
}

void freeBuddySystem(BuddySystem *b) { freeBlock(b->root); }
