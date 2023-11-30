//
// Created by Andrew Bowie on 3/24/23.
//

#include "mpx/heap.h"
#include "stddef.h"
#include "mpx/vm.h"
#include "stdbool.h"
#include "stdio.h"

/**
 * @file heap.c
 * @brief The implementation file for heap.h. Contains the definition of the memory block and some other useful functions.
 */

///A structure that contains memory.
typedef struct mem_block {
    ///The previous memory block.
    struct mem_block *prev;
    ///The next memory block.
    struct mem_block *next;

    ///The start address of this block
    int start_address;
    ///The size of this block
    size_t size;
} mem_block_t;

///The beginning of the free list of memory blocks.
mem_block_t *free_list;
///The beginning of the allocated list of memory blocks.
mem_block_t *alloc_list;

/**
 * Prints the block and its given data to std output.
 *
 * @param block the block to print.
 * @authors Andrew Bowie
 */
void print_block(mem_block_t *block)
{
    println("Memory Control Block");
    printf("Physical Start: %x\n", block);
    printf("Physical End: %x\n", (block->start_address + block->size));
    printf("Memory Start: %x\n", block->start_address);
    printf("Size: %d\n", block->size);
}

void print_partial_block(mem_block_t *block){
    printf("Memory Start: 0x%x\n", block->start_address);
    printf("Size: %d\n", block->size);
    print("\n");
}

void print_partial_list(bool list){
    printf("Memory Control Block List %s\n", list ? "Free" : "Allocated");
    printf("\n");
    mem_block_t *block = list ? free_list : alloc_list;
    int count = 0;
    while(block != NULL)
    {
        printf("Memory Block #%d\n", count++);
        print_partial_block(block);
        block = block->next;
    }
}
void print_list(bool list)
{
    printf("Memory Control Block List %s\n", list ? "Free" : "Allocated");
    mem_block_t *block = list ? free_list : alloc_list;
    int count = 0;
    while(block != NULL)
    {
        printf("Memory Block #%d\n", count++);
        print_block(block);
        block = block->next;
    }
}

/**
 * Removes a memory control block from its respective list.
 *
 * @param block the block to remove.
 * @authors Andrew Bowie
 */
void rem_mcb_free(mem_block_t *block)
{
    if(block->prev != NULL)
    {
        block->prev->next = block->next;
    }
    else
    {
        //In this case, we're removing the head.
        if(free_list == block)
            free_list = block->next;
        else if(alloc_list == block)
            alloc_list = block->next;
    }

    if(block->next != NULL)
    {
        block->next->prev = block->prev;
    }

    //Remove self from list.
    block->next = block->prev = NULL;
}

/**
 * Merges the newly freed block with neighboring free blocks.
 *
 * @param freed_block the freed block.
 * @authors Andrew Bowie
 */
void merge_blocks(mem_block_t *freed_block)
{
    mem_block_t *previous = freed_block;
    mem_block_t *first_block_found = freed_block->prev;
    if(first_block_found != NULL) {
        int max_address = (int) ((int) first_block_found->start_address + first_block_found->size);
        if(max_address == (int) previous)
        {
            //Merge the two blocks.
            first_block_found->size += previous->size + sizeof (struct mem_block);

            rem_mcb_free(previous);
            previous = first_block_found; //Update the previous for the merge forward.
        }
    }

    //Start working our way forward.
    //Note that freed_block at this point may be unusable as it may have already been removed from its list.
    first_block_found = previous->next;
    int max_address = (int) ((int) previous->start_address + previous->size);
    if(first_block_found != NULL && max_address == (int) first_block_found)
    {
        //Merge the two blocks.
        previous->size += first_block_found->size + sizeof (struct mem_block);

        rem_mcb_free(first_block_found);
    }
}

/**
 * Inserts a memory block into its respective list.
 *
 * @param mblock the block to insert.
 * @param list the list in which to insert the block, true if free, false if allocated.
 * @authors Andrew Bowie
 */
void insert_block(mem_block_t *mblock, bool list)
{
    mem_block_t *previous_block = list ? free_list : alloc_list;
    if(previous_block == NULL)
    {
        if(list)
            free_list = mblock;
        else
            alloc_list = mblock;
        return;
    }

    //Check if immediate insertion is necessary.
    if(previous_block->start_address > mblock->start_address)
    {
        if(list)
            free_list = mblock;
        else
            alloc_list = mblock;

        mblock->next = previous_block;
        previous_block->prev = mblock;
        return;
    }

    //Otherwise, iteration is necessary.
    while(previous_block->next != NULL && previous_block->next->start_address < mblock->start_address)
    {
        previous_block = previous_block->next;
    }

    //We've found the immediate predecessor to our block that we're inserting.
    mblock->next = previous_block->next;
    mblock->prev = previous_block;
    previous_block->next = mblock;
    if(mblock->next != NULL)
        mblock->next->prev = mblock;
}

void *allocate_memory(size_t size)
{
    if(size <= 0)
        return NULL;

    mem_block_t *walk = free_list;
    //This while loop stops when we first find a block that can fit the size.
    while(walk != NULL)
    {
        if(walk->size >= size)
        {
            break;
        }
        walk = walk->next;
    }

    //In this case, we couldn't find memory large enough for the size.
    if(walk == NULL)
        return NULL;

    //Now at this point, walk is the MCB that can contain our new memory.
    if(walk->size - size <= sizeof (struct mem_block))
    {
        rem_mcb_free(walk);
        insert_block(walk, false);
        return (void *) walk->start_address;
    }

    // start an extra free block to be used as a remainder
    mem_block_t *extra_free_block = (mem_block_t *) (size + walk->start_address);
    // find remaining size of extra free block once walk is allocated
    extra_free_block->size = walk->size - size - sizeof(struct mem_block);
    // find new start address in extra free block
    extra_free_block->start_address = (int) ((int) extra_free_block + sizeof (mem_block_t));

    // add the extra free block back to the free list
    insert_block(extra_free_block, true);
    //Set the size of walk.
    walk->size = (int) extra_free_block - walk->start_address;

    // remove walk from the free list
    rem_mcb_free(walk);
    // add walk to the alloc list
    insert_block(walk, false);
    //return a pointer to the new starting address
    return (void *) walk->start_address;
}

void initialize_heap(size_t size)
{
    //Malloc the full free block.
    mem_block_t *block = kmalloc(size + sizeof(mem_block_t), 0, NULL);
    insert_block(block, true);

    //Initialize the values of the block.
    block->size = size - sizeof (mem_block_t);
    block->start_address = (int) (((int) block) + sizeof (mem_block_t));
}

/**
 * Checks if the given block exists in the allocated memory linked list.
 *
 * @param mcb_address the beginning address of the MCB.
 * @return true if it does, false if not.
 * @authors Kolby Eisenhauer
 */
bool block_exists(void * mcb_address)
{
    mem_block_t *walk = alloc_list;
    while(walk != NULL)
    {
        if(mcb_address == walk) return true;
        walk = walk->next;
    }
    return false;
}

int free_memory(void * free){
    void * mcb_address =  (free - sizeof(struct mem_block));
    if(!block_exists(mcb_address)) return -1;

    rem_mcb_free((mem_block_t *) mcb_address);
    insert_block((mem_block_t *) mcb_address, true);
    merge_blocks((mem_block_t *) mcb_address);

    return 0;
}