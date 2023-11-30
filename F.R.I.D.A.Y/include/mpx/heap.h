//
// Created by Andrew Bowie on 3/24/23.
//

#ifndef F_R_I_D_A_Y_HEAP_H
#define F_R_I_D_A_Y_HEAP_H

#include "stddef.h"
#include "stdbool.h"

/**
 * @file heap.h
 * @brief the heap file contains functions useful for allocating and freeing memory.
 */

/**
 * Prints one of the given list based upon the bool.
 *
 * @param list the list to print, free if true, alloc if false.
 * @authors Andrew Bowie
 */
void print_list(bool list);
/**
 * Prints memory and size of list based upon the bool.
 *
 * @param list the list to print, free if true, alloc if false.
 * @authors Zachary Ebert
 */
void print_partial_list(bool list);
/**
 * Initializes the heap with the given size.
 *
 * @param size the size of the new heap.
 * @authors Andrew Bowie
 */
void initialize_heap(size_t size);

/**
 * Allocates memory to the heap, returns NULL if it can't find enough room for the memory.
 *
 * @param size the amount of bytes to allocate.
 * @return the pointer to the allocated memory, or NULL.
 * @authors Andrew Bowie, Jared Crowley
 */
void *allocate_memory(size_t size);

/**
 * Frees the Memory Block at the given pointer.
 * @param pointer the address of the MB.
 * @return 0 on success
 * @authors Kolby Eisenhauer
 */
int free_memory(void* pointer);


#endif //F_R_I_D_A_Y_HEAP_H
