#include "commands.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include "mpx/pcb.h"
#include "linked_list.h"

///The PCB queue for processes.
static linked_list *running_pcb_queue;

/**
 * @brief Gets the class name from the given enum.
 * @param state the class of the execution.
 * @return the string representation.
 * @authors Andrew Bowie
 */
const char *get_class_name(enum pcb_class class)
{
    switch (class)
    {
        case USER:
            return "User";
        case SYSTEM:
            return "System";
        default:
            return "Unknown";
    }
}



/**
 * @brief Gets the dispatch state name from the given enum.
 *
 * @param state the dispatch state of the execution.
 * @return the string representation.
 * @authors Andrew Bowie
 */
const char *get_dispatch_state(enum pcb_dispatch_state dispatch)
{
    switch (dispatch)
    {
        case SUSPENDED:
            return "Suspended";
        case NOT_SUSPENDED:
            return "Not Suspended";
        default:
            return "Unknown";
    }
}

/**
 * @brief Gets the execution state name from the given enum.
 *
 * @param state the state of the execution.
 * @return the string representation.
 * @authors Andrew Bowie
 */
const char *get_exec_state_name(enum pcb_exec_state state)
{
    switch (state)
    {
        case BLOCKED:
            return "Blocked";
        case RUNNING:
            return "Running";
        case READY:
            return "Ready";
        default:
            return "Unknown";
    }
}
/**
 * @brief Prints the given PCB to standard output.
 *
 * @param pcb_ptr the pointer to the pcb.
 * @authors Andrew Bowie
 */
void print_pcb(struct pcb *pcb_ptr)
{
    printf("PCB \"%s\"\n", pcb_ptr->name);
    printf("  - Priority: %d\n", pcb_ptr->priority);
    printf("  - Class: %s\n", get_class_name(pcb_ptr->process_class));
    printf("  - State: %s\n", get_exec_state_name(pcb_ptr->exec_state));
    printf("  - Suspended: %s\n", get_dispatch_state(pcb_ptr->dispatch_state));
}

/**
 * @brief A pointer for comparing PCBs.
 *
 * @param ptr1 the first pcb.
 * @param ptr2 the second pcb.
 * @return the comparison value of the two pcbs.
 * @authors Andrew Bowie
 */
int pcb_cmpr(void *ptr1, void *ptr2)
{
    struct pcb *pcb_ptr1 = (struct pcb *) ptr1;
    struct pcb *pcb_ptr2 = (struct pcb *) ptr2;

    if(pcb_ptr1->dispatch_state != pcb_ptr2->dispatch_state)
    {
        return (int) pcb_ptr1->dispatch_state - (int) pcb_ptr2->dispatch_state;
    }

    if(pcb_ptr1->exec_state != pcb_ptr2->exec_state)
    {
        return (int) pcb_ptr1->exec_state - (int) pcb_ptr2->exec_state;
    }
    return pcb_ptr1->priority - pcb_ptr2->priority;
}

void setup_queue()
{
    if(running_pcb_queue != NULL)
        return;

    running_pcb_queue = nl_unbounded();
    set_sort_func((linked_list *) running_pcb_queue, &pcb_cmpr);
}

struct pcb *pcb_alloc(void)
{
    setup_queue();

    struct pcb *pcb_ptr = sys_alloc_mem(sizeof (struct pcb));
    if(pcb_ptr == NULL) return NULL;
    memset(pcb_ptr, 0, sizeof (struct pcb));
    pcb_ptr->stack_ptr = (void *) ((int) pcb_ptr->stack) + PCB_STACK_SIZE - 4;
    return pcb_ptr;
}

int pcb_free(struct pcb* pcb_ptr)
{
    setup_queue();

    if(pcb_ptr == NULL)
        return 1;

    if(sys_free_mem((void *) pcb_ptr->name) != 0)
        return 1;

    return sys_free_mem(pcb_ptr);
}

struct pcb *pcb_setup(const char *name, int class, int priority)
{
    setup_queue();

    //Don't allow null names or names that are too long.
    if(name == NULL || strlen(name) > PCB_MAX_NAME_LEN)
        return NULL;

    //Check validity of class.
    if(class < USER || class > SYSTEM)
        return NULL;

    //Check validity of priority.
    if(priority < 0 || priority > 9)
        return NULL;

    struct pcb *pcb_ptr = pcb_alloc();
    if(pcb_ptr == NULL)
        return NULL;

    //We need to malloc the string,
    size_t str_len = strlen(name);
    char *malloc_name = sys_alloc_mem(str_len + 1);
    if(malloc_name == NULL)
        return NULL;
    memcpy(malloc_name, name, str_len + 1);

    pcb_ptr->name = malloc_name;
    pcb_ptr->process_class = class;
    pcb_ptr->_item = pcb_ptr;
    pcb_ptr->priority = priority;
    return pcb_ptr;
}

void pcb_insert(struct pcb* pcb_ptr)
{
    setup_queue();

    if(pcb_ptr == NULL)
        return;
    add_item(running_pcb_queue, pcb_ptr);
}
/**
 *
 * @param name
 * @return
 * @authors Jared Crowley, Andrew Bowie
 */
struct pcb *pcb_find(const char *name)
{
    if(name == NULL)
        return NULL;

    setup_queue();

    //Iterate over and find the item.
    ll_node *first_node = get_first_node(running_pcb_queue);
    while(first_node != NULL)
    {
        struct pcb *item_ptr = (struct pcb *) get_item_node(first_node);
        if(strcmp(item_ptr->name, name) == 0)
            return item_ptr;

        first_node = next_node(first_node);
    }
    return NULL;
}
/**
 *
 * @param pcb_ptr
 * @return
 * @authors Jared Crowley
 */
bool pcb_remove(struct pcb *pcb_ptr)
{
    setup_queue();
    if(pcb_ptr == NULL)
        return -1;

    // get size of linked list
    return remove_item_ptr(running_pcb_queue, pcb_ptr) == 0 ? true : false;
}

///The label for the create label.
#define CMD_CREATE_LABEL "create"
#define CMD_DELETE_LABEL "delete"
#define CMD_SUSPEND_LABEL "suspend"
#define CMD_RESUME_LABEL "resume"
#define CMD_SETPRIORITY_LABEL "priority"
#define CMD_SHOW_LABEL "show"
#define CMD_SHOW_READY "show-ready"
#define CMD_SHOW_BLOCKED "show-blocked"
#define CMD_SHOW_ALL "show-all"

/**
 * The 'create' sub command.
 * @param comm the string command.
 * @return true if it matched, false if not.
 * @authors Andrew Bowie
 */
bool pcb_create_cmd(const char *comm)
{
    if(!first_label_matches(comm, CMD_CREATE_LABEL))
        return false;

    //Copy the string.
    size_t str_len = strlen(comm);
    char comm_cpy[str_len + 1];
    memcpy(comm_cpy, comm, str_len + 1);

    //Tokenize the string.
    char *token = strtok(comm_cpy, " ");
    //Push it forward.
    token = strtok(NULL, " ");

    //Initialize pointers, all null for error checking.
    char *name = NULL;
    int class = -1;
    int priority = -1;

    if(token == NULL)
    {
        println("Missing Arguments! Do it like this: 'pcb create (name) (class) (priority)'");
        return true;
    }

    //Copy the name.
    size_t token_size = strlen(token);
    if(token_size > PCB_MAX_NAME_LEN)
    {
        printf("Invalid Argument! '%s' exceeds maximum name length of %d!\n", token, PCB_MAX_NAME_LEN);
        return true;
    }

    if(pcb_find(token) != NULL)
    {
        printf("Invalid Argument! The PCB '%s' already exists!\n", token);
        return true;
    }

    char name_cpy[token_size + 1];
    memcpy(name_cpy, token, token_size + 1);
    name = name_cpy;

    token = strtok(NULL, " ");
    if(token == NULL)
    {
        println("Missing Arguments! Do it like this: 'pcb create (name) (class) (priority)'");
        return true;
    }

    //Check which type it was.
    if(strcicmp(token, "USER") == 0)
    {
        class = USER;
    }
    else if(strcicmp(token, "SYSTEM") == 0)
    {
        class = SYSTEM;
    }
    else
    {
        printf("Invalid Argument! %s isn't a valid class! Try 'USER' or 'SYSTEM'!\n", token);
        return true;
    }

    token = strtok(NULL, " ");
    if(token == NULL)
    {
        println("Missing Arguments! Do it like this: 'pcb create (name) (class) (priority)'");
        return true;
    }

    priority = atoi(token);
    if(priority < 0 || priority > 9)
    {
        printf("Invalid Argument! %d isn't a valid priority! Try 0-9 instead.\n", priority);
        return true;
    }

    //Alloc the pcb.
    struct pcb *pcb_ptr = pcb_setup(name, class, priority);
    if(pcb_ptr == NULL)
    {
        println("There was an error setting up the PCB!");
        return true;
    }

    //Insert it.
    pcb_insert(pcb_ptr);

    printf("Successfully created a new PCB with the following info.\nName: %s\nClass: %s\nPriority: %d\n",
           name,
           class == 0 ? "USER" : "SYSTEM",
           priority);
    return true;
}

 /*
 * The 'delete' sub command.
 * @param comm the string command.
 * @return true if it matched, false if not.
 * @authors Andrew Bowie
 */
bool pcb_delete_cmd(const char *comm)
{
    if(!first_label_matches(comm, CMD_DELETE_LABEL))
        return false;

    //Copy the command.
    size_t s_len = strlen(comm);
    char comm_cpy[s_len + 1];
    memcpy(comm_cpy, comm, s_len + 1);

    //Tokenize.
    char *token = strtok(comm_cpy, " ");
    token = strtok(NULL, " ");

    if(token == NULL)
    {
        println("Missing Arguments! Do it like this: 'pcb delete (name)'");
        return true;
    }

    //Find the PCB.
    struct pcb *pcb_ptr = pcb_find(token);
    if(pcb_ptr == NULL)
    {
        printf("Could not find PCB named '%s'!\n", token);
        return true;
    }

    if(pcb_ptr->process_class == SYSTEM)
    {
        println("You cannot delete PCBs with the 'SYSTEM' class.");
        return true;
    }

    pcb_remove(pcb_ptr);
    pcb_free(pcb_ptr);
    printf("Removed PCB named '%s'!\n", pcb_ptr->name);
    return true;
}

/**
 * The 'suspend' sub command.
 * @param comm the string command.
 * @return true if it matched, false if not.
 * @authors Kolby Eisenhauer, Zachary Ebert
 */
bool pcb_suspend_cmd(const char* comm)
{
    if(!first_label_matches(comm, CMD_SUSPEND_LABEL))
        return false;
    size_t comm_strlen = strlen(comm);

    char comm_cpy[comm_strlen + 1];
    memcpy(comm_cpy, comm, comm_strlen + 1);
    char *name_token = strtok(comm_cpy, " ");
    name_token = strtok(NULL, " ");
    struct pcb* pcb_ptr = pcb_find(name_token);
    if (name_token == NULL){
        println("There was No Name Given for PCB: Enter pcb suspend name");
        return true;
    }
    if(pcb_ptr == NULL) {
        printf("PCB with name: %s, not found\n",name_token);
        return true;
    }
    if(pcb_ptr->process_class == SYSTEM)
    {
        printf("PCB %s is a system class PCB cannot be suspended by user\n",name_token);
        return true;
    }
    
    pcb_ptr->dispatch_state = SUSPENDED;
    pcb_remove(pcb_ptr);
    pcb_insert(pcb_ptr);
    
    printf("The pcb named: %s was suspended\n", pcb_ptr->name);
    return true;
}
/**
 * The 'resume' sub command.
 * @param comm the string command.
 * @return true if it matched, false if not.
 * @author Zachary Ebert
 */
bool pcb_resume_cmd(const char* comm)
{
    if(!first_label_matches(comm, CMD_RESUME_LABEL))
        return false;
    size_t comm_strlen = strlen(comm);

    char comm_cpy[comm_strlen + 1];
    memcpy(comm_cpy, comm, comm_strlen + 1);
    char *name_value = strtok(comm_cpy, " ");
    name_value = strtok(NULL, " ");

    struct pcb* pcb_ptr = pcb_find(name_value);
    if (name_value == NULL){
        println("There was No Name Given for PCB: Enter pcb resume name");
        return true;
    }
    if(pcb_ptr == NULL) {
        printf("PCB with name: %s, cannot be found \n",name_value);
        return true;
    }
    if(pcb_ptr->process_class == SYSTEM)
    {
        printf("PCB %s is a system class PCB cannot be suspended nor resumed by user\n",name_value);
        return true;
    }
   
    pcb_ptr->dispatch_state = NOT_SUSPENDED;
    
    pcb_remove(pcb_ptr);
    pcb_insert(pcb_ptr);
    printf("The pcb named: %s was resumed\n", pcb_ptr->name);
    return true;
}
/**
 * The 'Priority' sub command.
 * @param comm the string command.
 * @return true if it matched, false if not.
 * @author Zachary Ebert
 */
bool pcb_priority_cmd(const char* comm){
    if(!first_label_matches(comm, CMD_SETPRIORITY_LABEL))
        return false;
    size_t comm_strlen = strlen(comm);

    char comm_cpy[comm_strlen + 1];
    memcpy(comm_cpy, comm, comm_strlen + 1);
    char *parameters = strtok(comm_cpy, " ");
    parameters = strtok(NULL, " ");

    struct pcb* pcb_ptr = pcb_find(parameters);
    if (parameters == NULL){
        println("There was No Name Given for PCB: Enter pcb priority name #");
        return true;
    }
    if(pcb_ptr == NULL) {
        printf("PCB with name: %s, cannot be found \n", parameters);
        return true;
    }

    parameters = strtok(NULL, " ");
    int priority;
    if(parameters != NULL && parameters[0] <= '9'  && parameters[0] >= '0') {
        priority = atoi(parameters);
    }else{
        println("Priority is Invalid: Priority must be a number");
        return true;
    }

    if(priority > 9 || priority < 0){
        println("The Number is Out of Range. Enter a Number between 0-9");
        return true;
    }
    pcb_ptr->priority = priority;

    pcb_remove(pcb_ptr);
    pcb_insert(pcb_ptr);

    printf("The pcb named: %s was changed to priority %d\n", pcb_ptr->name, pcb_ptr->priority);
    return true;
}
/**
 * @brief The 'show' sub command.
 * @param comm the string command.
 * @return true if it matched, false if not.
 * @author Zachary Ebert
 */
bool pcb_show_cmd(const char* comm){

    if(!first_label_matches(comm, CMD_SHOW_LABEL))
        return false;
    size_t comm_strlen = strlen(comm);

    char comm_cpy[comm_strlen + 1];
    memcpy(comm_cpy, comm, comm_strlen + 1);
    char *name_value = strtok(comm_cpy, " ");
    name_value = strtok(NULL, " ");

    struct pcb* pcb_ptr = pcb_find(name_value);
    if (name_value == NULL){
        println("There was No Name Given for PCB: Enter pcb show name");
        return true;
    }
    if(pcb_ptr == NULL) {
        printf("PCB with name: %s, cannot be found \n",name_value);
        return true;
    }

    print_pcb(pcb_ptr);
    return true;
}
/**
 * @brief The 'Show Ready' User Command
 * @param comm the command to handle.
 * @return true if the command was handled
 * @authors Jared Crowley
 */
bool pcb_show_ready(const char *comm)
{
    if(!first_label_matches(comm, CMD_SHOW_READY))
        return false;

    setup_queue();

    //Iterate over and find the item.
    ll_node *first_node = get_first_node(running_pcb_queue);
    int printed = 0;
    while(first_node != NULL)
    {
        struct pcb *item_ptr = (struct pcb *) get_item_node(first_node);
        //Check if the dispatch state is equal to not suspended and the exec state is equal to ready
        // indicating the pcb is in the ready state
        if(item_ptr->dispatch_state == NOT_SUSPENDED && item_ptr->exec_state == READY)
        {
            //Print the pcb
            print_pcb(item_ptr);
            printed++;
        }
        first_node = next_node(first_node);
    }

    if(printed == 0)
    {
        printf("Could not find any PCB's in the ready state\n");
    }
    return true;
}

/**
 * @brief The 'Show Blocked' User Command
 * @param comm the command to handle.
 * @return true if the command was handled
 * @authors Jared Crowley
 */
bool pcb_show_blocked(const char *comm)
{
    if(!first_label_matches(comm, CMD_SHOW_BLOCKED))
        return false;
    setup_queue();

    //Iterate over and find the item.
    ll_node *first_node = get_first_node(running_pcb_queue);
    int printed = 0;
    while(first_node != NULL)
    {
        struct pcb *item_ptr = (struct pcb *) get_item_node(first_node);
        //Check if the dispatch state is equal to suspended and the exec state is equal to blocked
        // indicating the pcb is in the blocked state
        if(item_ptr->dispatch_state == SUSPENDED || item_ptr->exec_state == BLOCKED)
        {
            //Print the pcb
            print_pcb(item_ptr);
            printed++;
        }
        first_node = next_node(first_node);
    }

    if(printed == 0)
    {
        printf("Could not find any PCBs in the blocked state\n");
    }
    return true;
}
/**
 * @brief The 'Show All' User Command
 * @param comm the command to handle.
 * @return true if the command was handled
 * @authors Jared Crowley
 */
bool pcb_show_all(const char *comm)
{
    if(!first_label_matches(comm, CMD_SHOW_ALL))
        return false;
    setup_queue();

    //Iterate over and find the item.
    int printed = 0;
    ll_node *first_node = get_first_node(running_pcb_queue);
    while(first_node != NULL)
    {
        struct pcb *item_ptr = (struct pcb *) get_item_node(first_node);

        //Print the pcb
        print_pcb(item_ptr);

        first_node = next_node(first_node);
        printed++;
    }

    if(printed == 0)
    {
        println("Could not find any PCBs!");
    }

    return true;
}


///All commands within this file, terminated with NULL.
static bool (*command[])(const char *) = {
        &pcb_delete_cmd,
        &pcb_suspend_cmd,
        &pcb_resume_cmd,
        &pcb_priority_cmd,
        &pcb_show_cmd,
        &pcb_show_ready,
        &pcb_show_blocked,
        &pcb_show_all,
        NULL,
};

bool generate_new_pcb(const char *name,
                      int priority,
                      enum pcb_class class,
                      void *begin_ptr,
                      const char *input,
                      size_t input_len,
                      size_t param_ptrs)
{
    if(priority < 0 || priority > 9)
        return false;

    if(class != USER && class != SYSTEM)
        return false;

    //Can't duplicate names.
    if(pcb_find(name) != NULL)
    {
        return false;
    }

    struct pcb *new_pcb = pcb_setup(name, class, priority);
    if(new_pcb == NULL)
    {
        return false;
    }

    //Save the context into pcb.
    if(input != NULL)
    {
        //Adjust the stack pointer to account for the input.
        new_pcb->stack_ptr = (void *) (new_pcb->stack_ptr - input_len - 1);

        //Copy the input into the stack.
        for (size_t i = 0; i < input_len; ++i)
        {
            ((char *) new_pcb->stack_ptr)[i] = input[i];
        }

        //Adjust pointer addresses to account for new offset.
        for (size_t i = 0; i < param_ptrs; ++i)
        {
            size_t offset = i * sizeof (void *);
            void **ptr = new_pcb->stack_ptr + offset;
            size_t other_offset = (int) *ptr - (int) input;
            *ptr = new_pcb->stack_ptr + other_offset;
        }
    }

    //Adjust the stack pointer to account for initial context.
    new_pcb->stack_ptr = (void *) (new_pcb->stack_ptr - sizeof (struct context) - sizeof(int));
    struct context *pcb_context = (struct context *)new_pcb->stack_ptr;

    //Initialize the context to its appropriate values.
    pcb_context->cs = 0x08;
    pcb_context->ds = 0x10;
    pcb_context->fs = 0x10;
    pcb_context->ds = 0x10;
    pcb_context->es = 0x10;
    pcb_context->gs = 0x10;
    pcb_context->ss = 0x10;
    pcb_context->ebp = (int) (new_pcb->stack + PCB_STACK_SIZE - sizeof(struct context));
    pcb_context->esp = (int) (new_pcb->stack + PCB_STACK_SIZE - sizeof(struct context));
    pcb_context->eip = (int) begin_ptr;
    pcb_context->eflags = 0x0202;

    pcb_insert(new_pcb);
    return true;
}

struct pcb *peek_next_pcb(void)
{
    if(list_size(running_pcb_queue) == 0)
        return NULL;

    return get_item(running_pcb_queue, 0);
}

struct pcb *poll_next_pcb(void)
{
    if(list_size(running_pcb_queue) == 0)
        return NULL;

    return remove_item_unsafe(running_pcb_queue, 0);
}

void exec_pcb_cmd(const char *comm)
{
    size_t str_len = strlen(comm);
    char comm_cpy[str_len + 1];
    memcpy(comm_cpy, comm, str_len + 1);

    str_strip_whitespace(comm_cpy, NULL, 0);

    int index = 0;
    while(command[index] != NULL)
    {
        bool result = command[index](comm_cpy);
        if(result)
            return;
        index++;
    }

    //Inform the user that there wasn't any matches.
    if(strlen(comm) > 0)
        printf("PCB sub command '%s' does not exist! Type 'help pcb' for more info!\n", comm_cpy);
    else
        println("Please provide a PCB sub command!");
}
