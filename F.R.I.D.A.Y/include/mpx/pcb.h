#include "stdbool.h"
#include "stddef.h"
#ifndef MPX_PCB_H
#define MPX_PCB_H

/**
 * @file pcb.h
 * @brief This file contains all of the structure and functions for a PCB and its context.
 */

///The maximum length of a PCB's name.
#define PCB_MAX_NAME_LEN 8
///The initial size of a PCB's stack.
#define PCB_STACK_SIZE 2048

///The clas of a PCB.
enum pcb_class {
    USER = 0,
    SYSTEM = 1,
};

///The execution state of a PCB.
enum pcb_exec_state {
    READY = 0,
    RUNNING = 1,
    BLOCKED = 2,
};

///An enum of dispatch state for PCBs.
enum pcb_dispatch_state {
    NOT_SUSPENDED = 0,
    SUSPENDED = 1,
};

///The definition of a process control block.
struct pcb {
    ///This exists as an extremely hacky way to use them in the linked list without allocating memory.
    void *_next;
    ///This exists as an extremely hacky way to use them in the linked list without allocating memory.
    void *_item;

    ///The name of the PCB, max length of 8.
    const char *name;
    ///The process class type.
    enum pcb_class process_class;
    ///Integer priority of PCB, 0-9, lower = higher priority;
    int priority;
    ///The execution state of this PCB.
    enum pcb_exec_state exec_state;
    ///The dispatch state of this PCB.
    enum pcb_dispatch_state dispatch_state;
    ///A pointer to the next available byte in the stack.
    void *stack_ptr;
    ///The stack itself.
    unsigned char stack[PCB_STACK_SIZE];
};

///The context to save onto a PCB.
struct context {
    ///The segment registers.
    int gs, fs, es, ds, ss;
    ///The general purpose registers.
    int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    ///The status control registers, ordered as they are added for interrupts.
    int eip, cs, eflags;
};

/**
 *@brief Sets up queue for PCBS
 * @authors Andrew Bowie
 */
void setup_queue(void);

/**
 * @brief Peeks the next available PCB, or returns NULL if it's empty.
 * @return the next PCB or NULL.
 */
struct pcb *peek_next_pcb(void);

/**
 * @brief Polls the next available PCB, or returns NULL if it's empty.
 * @return the next PCB or NULL.
 */
struct pcb *poll_next_pcb(void);

/**
 * @brief Allocates memory for a PCB block.
 *
 * @return A pointer to the allocated PCB.
 * @authors Andrew Bowie, Kolby Eisenhauer
 */
struct pcb *pcb_alloc(void);

/**
 * @brief Frees the memory associated with the given PCB block.
 *
 * @param pcb_ptr the pointer to the pcb.
 * @return 0 on success, non-zero on failure.
 * @authors Andrew Bowie
 */
int pcb_free(struct pcb* pcb_ptr);

/**
 * @brief Sets up a PCB with the given information.
 *
 * @param name the name of the PCB, cannot be longer than @code PCB_MAX_NAME_LEN chars.
 * @param class the class of the PCB.
 * @param priority the priority of the PCB.
 * @return the created PCB, or NULL on error.
 * @authors Andrew Bowie
 */
struct pcb *pcb_setup(const char *name, int class, int priority);

/**
* @brief Inserts a PCB into appropriate queue, based on state and priority
* @param pcb_ptr pointer to pcb
* @authors Kolby Eisenhauers
*/
void pcb_insert(struct pcb* pcb_ptr);

/**
 * @brief Finds the PCB with the given name.
 * @param name the name of the pCB
 * @return the pcb found, or NULL if not found.
 * @authors Jared Crowley
 */
struct pcb *pcb_find(const char *name);

/**
 * @brief Removes a given PCB from the list.
 *
 * @param pcb_ptr the pointer to the PCB.
 * @return true if it was removed, false if not.
 * @authors Jared Crowley, Andrew Bowie
 */
bool pcb_remove(struct pcb *pcb_ptr);

/**
 * @brief Generates a new PCB with the new and the pointer to the given function.
 * @param name the name of the pcb.
 * @param priority the priority of the process.
 * @param class the class of the process.
 * @param begin_ptr the pointer of the function to start.
 * @return true if the PCB was successfully scheduled and started.
 * @authors Andrew Bowie, Zachary Ebert
 */
bool generate_new_pcb(const char *name,
                      int priority,
                      enum pcb_class class,
                      void *begin_ptr,
                      const char *input,
                      size_t input_len,
                      size_t param_ptrs);

/**
 * @brief Runs the PCB command from the given string.
 * @param comm the command.
 * @authors Andrew Bowie
 */
void exec_pcb_cmd(const char *comm);


#endif