//
// Created by Andrew Bowie on 1/13/23.
//

#include "stdio.h"
#include "stdbool.h"
#include "mpx/comhand.h"
#include "commands.h"
#include "string.h"
#include "cli.h"
#include "print_format.h"
#include "sys_req.h"
#include "mpx/clock.h"
#include "mpx/r3cmd.h"
#include "math.h"
#include "mpx/pcb.h"

///The message to send to the user if a command hasn't been recognized.
#define UNKNOWN_CMD_MSG "Unknown command '%s'. Type 'help' for help!"

/**
 * @brief An array of pointers to command functions. These functions
 * will return true if the command was handled by them, or false if not.
 */
bool (*comm_funcs[])(const char *comm) = {
        &cmd_version,
        &cmd_help,
        &cmd_get_time_menu,
        &cmd_shutdown,
        &cmd_set_date,
        &cmd_set_time,
        &cmd_set_tz,
        &cmd_clear,
        &cmd_color,
        &cmd_pcb,
        &cmd_alarm,
        &cmd_yield,
        &loadr3,
        &cmd_alarm,
        &cmd_free_memory,
        &cmd_allocate_memory,
        &cmd_show_allocate,
        &cmd_show_free,
        &cmd_dragonmaze,
        &cmd_minesweeper
};

/// Used to denote if the comm hand should stop.
static bool sig_shutdown = false;

void signal_shutdown(void)
{
    sig_shutdown = true;

    //Empty the PCB queue.
    while(poll_next_pcb() != NULL);

    sys_req(EXIT);
}

/**
 * @brief Prints a welcome message to the user.
 */
void print_welcome(void)
{
    set_output_color(get_color("bright-red"));
    println("*********      **********      **      ****                  **         **        **");
    println("**             **     **       **      **   **             **  **         **    **");
    println("**             **    **        **      **    **           **     **         ****");
    set_output_color(get_color("white"));
    println("*****          **  **          **      **     **         **       **         **");
    println("**             ****            **      **     **        *************        **");
    println("**             **  **          **      **     **       **           **       ** ");
    set_output_color(get_color("blue"));
    println("**             **    **        **      **    **       **             **      **");
    println("**             **     **       **      **   **       **               **     **");
    println("**             **      **      **      ******       **                 **    **");
    set_output_color(get_color("reset"));

    println("Welcome to MPX. Please select an option");
    println("=> help");
    println("=> get-time-date");
    println("=> alarm HH:mm:SS");
    println("=> set-time HH:mm:SS");
    println("=> set-date MM/DD/YY");
    println("=> set-timezone");
    println("=> load-R3");
    println("=> version");
    println("=> shutdown");
    println("=> color");
    println("=> clear");
    println("=> allocate-memory");
    println("=> free-memory");
    println("=> show-allocate");
    println("=> show-free");
    println("=> dragonmaze");
    println("=> minesweeper");
}

void comhand(void)
{
    //Initialize random seed from clock.
    int *time = get_time(NULL);

    unsigned long long new_seed = 0L;
    for (int i = 0; i < 7; ++i)
    {
        unsigned long long element = time[i];

        new_seed += element * (i + 1) * 31;
    }
    s_rand(new_seed);

    print_welcome();
    bool running = true;
    while (running)
    {
        //100 + 1 for the null terminator.
        char buf[101] = {0};

        set_cli_history(false);
        set_command_formatting(true);
        set_tab_completions(true);
        set_cli_prompt(CMD_PROMPT);
        print(CMD_PROMPT);
        gets(buf, 100);
        set_cli_history(false);
        set_command_formatting(false);
        set_tab_completions(false);
        set_cli_prompt(NULL);

        //Strip whitespace.
        str_strip_whitespace(buf, NULL, 0);

        //Handle all functions.
        int comm_func_count = sizeof(comm_funcs) /
                              sizeof(comm_funcs[0]);

        //Loop over all commands and check if it matches.
        bool found = false;
        for (int i = 0; i < comm_func_count; ++i)
        {
            bool result = comm_funcs[i](buf);
            if (result)
            {
                found = true;
                break;
            }
        }

        //If something wasn't found, print the unknown command message.
        if (!found && strlen(buf) > 0)
        {
            printf(UNKNOWN_CMD_MSG, buf);
            println("");
        }
        sys_req(IDLE);
    }

    sys_req(EXIT); //Exit ourselves as a fail-safe.
}