#include "mpx/pcb.h"
#include "sys_req.h"
#include "linked_list.h"
#include "mpx/device.h"
#include "mpx/serial.h"

/**
 * @file sys_call.c
 * @brief This file contains the sys_call function which is used to do context switching.
 */

///The currently running PCB.
static struct pcb *active_pcb_ptr = NULL;
///The first context saved when sys_call is called.
static struct context *first_context_ptr = NULL;

/**
 * @brief Gets the next PCB to replace the current one. The PCB can be sourced from one of two locations. They're listed in the order they're checked.
 * 1. The DCB queues. If a process is loaded from there, it means that its IO operation was finished.
 * 2. The PCB queue. If no such PCB is done in the DCBs, a PCB is polled from the PCB queue. If no PCB is available, NULL returns.
 * In either case, the PCB is prepared for running by removing it from wherever it is in the PCB queue and set to a 'RUNNING' state.
 *
 * @return the next PCB to load, or NULL if no such PCB exists.
 */
struct pcb *get_next_pcb()
{
    //First, we need to check for completed IO operations.
    struct pcb *to_load = check_completed();
    if (to_load != NULL)
    {
        //Check that the PCB wasn't suspended while it was in the queue.
        if(to_load->dispatch_state == SUSPENDED)
        {
            to_load->exec_state = READY; //Set it as ready, but not suspended.
        }
        else
        {
            //Otherwise, prepare it for running.
            to_load->exec_state = RUNNING;
            pcb_remove(to_load);
            return to_load;
        }
    }

    //Otherwise, load one from the PCB queues.
    struct pcb *queue_pcb = peek_next_pcb();
    if(queue_pcb == NULL || queue_pcb->exec_state == BLOCKED || queue_pcb->dispatch_state == SUSPENDED)
        return NULL;

    poll_next_pcb();
    queue_pcb->exec_state = RUNNING;
    return queue_pcb;
}

/**
 * @brief Want to check if next PCB is blocked, unblocked, IDLE, NULL, etc
 * @param next_pcb the next PCB to load.
 * @param current_context the current context.
 * @param next_state the new state of the PCB.
 * @return Pointer to the next context struct
 * @author Zachary Ebert
 */
struct context *next_pcb(struct pcb *next_pcb, struct context *current_context, enum pcb_exec_state next_state)
{
    if(next_pcb == NULL)
        return current_context;

    struct pcb *present_pcb = active_pcb_ptr;
    active_pcb_ptr = next_pcb;
    struct context *new_ctx = (struct context *) next_pcb->stack_ptr;
    //Checks to see if the active pointer pcb is null
    if (present_pcb != NULL && current_context != NULL)
    {
        present_pcb->exec_state = next_state;
        pcb_insert(present_pcb);
        //Update where the PCB's context pointer is pointing.
        present_pcb->stack_ptr = current_context;
    }
    return new_ctx;
}

/**
 * @brief The main system call function, implementing the IDLE and EXIT system requests.
 * @param action the action to perform.
 * @param ctx the current PCB context.
 * @return a pointer to the next context to load.
 * @author Andrew Bowie,  Zachary Ebert, Kolby Eisenhauer
 */
struct context *sys_call(op_code action, struct context *ctx)
{
    if (first_context_ptr == NULL)
    {
        first_context_ptr = ctx;
    }

    int ebx = 0, ecx = 0, edx = 0;
    __asm__ volatile("mov %%ebx,%0" : "=r"(ebx));
    __asm__ volatile("mov %%ecx,%0" : "=r"(ecx));
    __asm__ volatile("mov %%edx,%0" : "=r"(edx));

    //Handle different actions in their own way.
    struct pcb *next_to_load = get_next_pcb();
    switch (action)
    {
        case READ:
        {
            device dev = (device) ebx;
            char *buffer = (char *) ecx;
            size_t bytes = (size_t) edx;
            io_req_result result = io_request(active_pcb_ptr, action, dev, buffer, bytes);

            if (result == INVALID_PARAMS || result == SERVICED)
                return next_pcb(next_to_load, ctx, READY);

            //In this case, we need to move this device to a blocked state and CTX switch.
            if (result == PARTIALLY_SERVICED || result == DEVICE_BUSY)
            {
                return next_pcb(next_to_load, ctx, BLOCKED);
            }
            return ctx;
        }
        case WRITE:
        {
            device dev = (device) ebx;
            char *buffer = (char *) ecx;
            size_t bytes = (size_t) edx;
            io_req_result result = io_request(active_pcb_ptr, action, dev, buffer, bytes);

            if (result == INVALID_PARAMS || result == SERVICED)
            {
                return next_pcb(next_to_load, ctx, READY);
            }

            //In this case, we need to move this device to a blocked state and CTX switch.
            if (result == PARTIALLY_SERVICED || result == DEVICE_BUSY)
            {
                return next_pcb(next_to_load, ctx, BLOCKED);
            }
            return ctx;
        }
        case IDLE:
        {
            return next_pcb(next_to_load, ctx, READY);
        }
        case EXIT:
        {
            //Exiting PCB.
            struct pcb *exiting_pcb = active_pcb_ptr;
            if (exiting_pcb == NULL) //We can't exit if there's no PCB.
                return ctx;

            pcb_remove(exiting_pcb);
            if (next_to_load == NULL) //No next process to load? Try loading the global one.
                return first_context_ptr;

            //Free the old one.
            pcb_free(exiting_pcb);
            return next_pcb(next_to_load, NULL, 0);
        }
        default:
            return next_pcb(next_to_load, ctx, READY);
    }
}