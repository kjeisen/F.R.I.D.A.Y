#include "commands.h"
#include "string.h"
#include "stdio.h"
#include "dragon_maze.h"
#include "mine_sweeper.h"
#include "sys_req.h"
#include "bomb_catcher.h"
#include "mpx/comhand.h"
#include "mpx/clock.h"
#include "stdlib.h"
#include "color.h"
#include "cli.h"
#include "mpx/pcb.h"
#include "mpx/io.h"
#include "mpx/alarm.h"
#include "mpx/heap.h"
#include "math.h"

#define CMD_HELP_LABEL "help"
#define CMD_VERSION_LABEL "version"
#define CMD_SHUTDOWN_LABEL "shutdown"
#define CMD_GET_TIME_LABEL "get-time-date"
#define CMD_SET_TIMEZONE_LABEL "set-timezone"
#define CMD_SET_TIME_LABEL "set-time"
#define CMD_SET_DATE_LABEL "set-date"
#define CMD_CLEAR_LABEL "clear"
#define CMD_COLOR_LABEL "color"
#define CMD_YIELD "yield"
#define CMD_LOADR3 "load-r3"
#define CMD_SET_ALARM "alarm"
//PCB Command Header
#define CMD_PCB_LABEL "pcb"
//PCB Commands
#define CMD_ALLOCATE_MEMORY "allocate-memory"
#define CMD_FREE_MEMORY "free-memory"
#define CMD_SHOW_ALLOCATE "show-allocate"
#define CMD_SHOW_FREE "show-free"

#define CMD_DRAGONMAZE "dragonmaze"
#define CMD_MINESWEEPER "minesweeper"


///An array of all command labels, terminated with null.
static const char *CMD_LABELS[] = {
        CMD_HELP_LABEL,
        CMD_VERSION_LABEL,
        CMD_SHUTDOWN_LABEL,
        CMD_GET_TIME_LABEL,
        CMD_SET_TIMEZONE_LABEL,
        CMD_SET_TIME_LABEL,
        CMD_SET_DATE_LABEL,
        CMD_CLEAR_LABEL,
        CMD_COLOR_LABEL,
        CMD_PCB_LABEL,
        CMD_LOADR3,
        CMD_SET_ALARM,
        CMD_ALLOCATE_MEMORY,
        CMD_FREE_MEMORY,
        CMD_SHOW_ALLOCATE,
        CMD_SHOW_FREE,
        CMD_DRAGONMAZE,
        CMD_MINESWEEPER,
        NULL,
};

const char *find_best_match(const char *cmd)
{
    const char *best_match = NULL;
    int index = 0;
    while(CMD_LABELS[index] != NULL)
    {
        const char *potential = CMD_LABELS[index++];

        if(ci_starts_with(potential, cmd))
        {
            if(best_match != NULL)
                return NULL;
            best_match = potential;
        }
    }
    return best_match;
}

bool command_exists(const char *cmd)
{
    int index = 0;
    while(CMD_LABELS[index] != NULL)
    {
        if(first_label_matches(cmd, CMD_LABELS[index]))
            return true;
        index++;
    }
    return false;
}

bool cmd_version(const char *comm)
{
    //The command's label.
    const char *label = CMD_VERSION_LABEL;

    //Check if it matched.
    if (!first_label_matches(comm, label))
        return false;

    println("Module: R6");
    println(__DATE__);
    println(__TIME__);
    return true;
}

bool cmd_shutdown(const char *comm)
{
    const char *label = CMD_SHUTDOWN_LABEL;

    if (!first_label_matches(comm, label))
        return false;

    print("Are you sure you want to shutdown? (y/N): ");
    char confirm_buf[6] = {0};
    set_cli_history(0);
    gets(confirm_buf, 5);
    set_cli_history(1);

    //Check confirmation.
    if (strcicmp(confirm_buf, "y") == 0 ||
        strcicmp(confirm_buf, "yes") == 0)
    {
        signal_shutdown();
    }
    return true;
}

bool cmd_get_time_menu(const char *comm)
{
    //The command's label.
    const char *label = CMD_GET_TIME_LABEL;

    //Check if it matched.
    if (!first_label_matches(comm, label))
        return false;

    print_time();
    //println(__TIME__);
    return true;
}

bool cmd_set_date(const char *comm)
{
    const char *label = CMD_SET_DATE_LABEL;
    // Means that it did not start with label therefore it is not a valid input
    if (!first_label_matches(comm, label))
    {
        return false;
    }
    size_t comm_strlen = strlen(comm);
    //Get the time.
    char comm_cpy[comm_strlen + 1];
    memcpy(comm_cpy, comm, comm_strlen + 1);
    char *date_token = strtok(comm_cpy, " ");
    date_token = strtok(NULL, " ");

    // Date is provided
    if (date_token == NULL)
    {
        println("Date must be provided. enter 'set-date MM/DD/YY'");
        return true;
    }
    // buffer to save numbers
    char date_array[3][3] = {{0}};
    str_strip_whitespace(date_token, NULL, 0);
    // if part after set time is not valid with form hh:mm:ss returns with invalid date
    if (split(date_token, '/', 3, date_array, 3) || !is_valid_date_or_time(3, date_array, 3))
    {
        printf("Invalid date! You entered: %s, expecting format: MM/DD/YY!\n", date_token);
        return true;
    }
    unsigned char month_dec = atoi(date_array[0]);
    unsigned char day_dec = atoi(date_array[1]);
    unsigned char year_dec = atoi(date_array[2]);

    //Check pre-emptively.
    if (month_dec < 1 || month_dec > 12)
    {
        println("Month is out of range 1-12!");
        return true;
    }

    if (day_dec < 1)
    {
        println("Day cannot be negative!");
        return true;
    }

    if (year_dec < 0 || year_dec > 99)
    {
        println("Year is out of range 0-100!");
        return true;
    }

    //Get the current time.
    int current_time[7] = {0};
    get_time(current_time);

    //Update the hours and minutes.
    current_time[0] = year_dec;
    current_time[1] = month_dec;
    current_time[2] = day_dec;
    adj_timezone(current_time,
                 -get_clock_timezone()->tz_hour_offset,
                 -get_clock_timezone()->tz_minute_offset);
    year_dec = current_time[0];
    month_dec = current_time[1];
    day_dec = current_time[2];

    unsigned int month = decimal_to_bcd(month_dec);
    unsigned int day = decimal_to_bcd(day_dec);
    unsigned int year = decimal_to_bcd(year_dec);
    if (month < 0x01 || month > 0x12)
    {
        println("Month is out of range 1-12!");
        return true;
    }

    if (year < 0x00 || year > 0x99)
    {
        println("Year is out of range 0-100!");
        return true;
    }

    if (day < 0x01 | day > get_days_in_month(month, (int) year_dec))
    {
        println("Day is out of range for that month!");
        return true;
    }
    if (!set_date_clock(month, day, year))
    {
        println("Failed to set date, please try again!");
        return true;
    }

    printf("Set the date to: %s\n", date_token);
    return true;
}


bool cmd_set_time(const char *comm)
{
    const char *label = CMD_SET_TIME_LABEL;
    // Means that it did not start with label therefore it is not a valid input
    if (!first_label_matches(comm, label))
    {
        return false;
    }
    size_t comm_strlen = strlen(comm);
    //Get the time.
    char comm_cpy[comm_strlen + 1];
    memcpy(comm_cpy, comm, comm_strlen + 1);
    char *time_token = strtok(comm_cpy, " ");
    time_token = strtok(NULL, " ");

    // Date is provided
    if (time_token == NULL)
    {
        println("Time value must be provided. Try 'set-time HH:mm:SS'");
        return true;
    }
    str_strip_whitespace(time_token, NULL, 0);
    // buffer to save numbers
    char time_array[3][3] = {{0}};
    // if part after set time is not valid with form hh:mm:ss returns with invalid date
    if (split(time_token, ':', 3, time_array, 3) < 0 ||
        !is_valid_date_or_time(3, time_array, 3))
    {
        printf("Invalid time. You entered: %s, expecting format: HH:mm:SS\n", time_token);
        return true;
    }
    unsigned char hour_dec = atoi(time_array[0]);
    unsigned char minute_dec = atoi(time_array[1]);
    unsigned char second_dec = atoi(time_array[2]);

    //Do some pre error checking.
    if (hour_dec < 0 || hour_dec > 23)
    {
        println("Hour is out of range 0-23!");
        return true;
    }
    if (minute_dec < 0 || minute_dec > 59)
    {
        println("Minutes is out of range 0-59!");
        return true;
    }
    if (second_dec < 0 || second_dec > 59)
    {
        println("Seconds is out of range 0-59!");
        return true;
    }

    //Get the current time.
    int current_time[7] = {0};
    get_time(current_time);

    //Update the hours and minutes.
    current_time[4] = hour_dec;
    current_time[5] = minute_dec;
    adj_timezone(current_time,
                 -get_clock_timezone()->tz_hour_offset,
                 -get_clock_timezone()->tz_minute_offset);
    hour_dec = current_time[4];
    minute_dec = current_time[5];

    unsigned char hour = decimal_to_bcd(hour_dec);
    unsigned char minute = decimal_to_bcd(minute_dec);
    unsigned char second = decimal_to_bcd(second_dec);

    //Do some error checking
    if (hour < 0x00 || hour > 0x23)
    {
        println("Hour is out of range 0-23!");
        return true;
    }
    if (minute < 0x00 || minute > 0x59)
    {
        println("Minutes is out of range 0-59!");
        return true;
    }
    if (second < 0x00 || second > 0x59)
    {
        println("Seconds is out of range 0-59!");
        return true;
    }


    if (!set_time_clock(hour, minute, second))
    {
        println("Failed to set time, please try again!");
        return true;
    }

    printf("Set the time to: %s\n", time_token);
    return true;
}

///Used to store information on a specific label of the 'help' command.
struct help_info
{
    /**
     * @brief The label of the command for the help message.
     */
    char* str_label[15];
    /**
     * @brief The help message to send for this struct.
     */
    char *help_message;
};

/**
 * @brief An array of all help info messages.
 */
struct help_info help_messages[] = {
        {.str_label = {CMD_HELP_LABEL},
                .help_message = "The '%s' command gives information about specific aspects of the system.\n=> If you need more general command help type 'help'\n=> If you still dont understand what help is, type 'help help help'"},
        {.str_label = {CMD_HELP_LABEL, CMD_HELP_LABEL},
                .help_message = "The Oxford Definition: 'make it easier for someone to do something'\n=> If you need help with actual commmands type 'help'\n=> if you are still confused, type 'help help help help'"},
        {.str_label = {CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL},
                .help_message = "You only need to specify 'help' once! There isn't any more help we can give you!\n=>For Regular Help, type 'help'\n if you are still having issues with your life, type 'help help help help help'"},
        {.str_label = {CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL},
                .help_message = "Only. One. Help. Argument.\n=> to go back to help, type 'help'\n=> If you got nothing better to do, type 'help help help help help help'"},
        {.str_label = {CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL},
                .help_message = "Orgin: Old English helpan (verb), help (noun), of Germanic origin; related to Dutch helpen and German helfen.'\n=> Please go back to 'help'\n=> Go Ahead, 'help help help help help help help'"},
        {.str_label = {CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL, CMD_HELP_LABEL},
                .help_message = "Nope."},
        {.str_label = {CMD_VERSION_LABEL},
                .help_message = "The '%s' command gives you the version of the OS and the date it was compiled.\nType 'version', to get the version!"},
        {.str_label = {CMD_SHUTDOWN_LABEL},
                .help_message = "The '%s' command prompts the user to shut down the OS.\nType 'shutdown' to turn off the machine!"},
        {.str_label = {CMD_GET_TIME_LABEL},
                .help_message = "The '%s' command gets the current system time and date in the OS.\nType 'get-time-date' to get the time and date!"},
        {.str_label = {CMD_SET_TIME_LABEL},
                .help_message = "The '%s' command allows the use to set the time on the system.\nThe time should follow the format HH:mm:SS.\nTo start changing the time, enter 'set-time HH:mm:SS'"},
        {.str_label = {CMD_SET_DATE_LABEL},
                .help_message = "The '%s' command allows the use to set the date on the system.\nThe date should follow the format MM/DD/YY.\nTo start changing the date, enter 'set-date MM/DD/YY'"},
        {.str_label = {CMD_SET_TIMEZONE_LABEL},
                .help_message = "The '%s' command allows the user to set the timezone for the system.\nMost North American and European Time Zones Are provided\nTo fix the timezone, type 'set-timezone'"},
        {.str_label = {CMD_SET_ALARM},
                .help_message = "The '%s' command allows the user to set an alarm for the system. \nThe alarm command should follow the format 'alarm 12:00:00' after this is entered you will be prompted for the message."},
        {.str_label = {CMD_LOADR3},
                .help_message = "The '%s' command creates all the test processes for R3.\nto load R3, enter 'load-r3'"},
        {.str_label = {CMD_ALLOCATE_MEMORY},
                .help_message = "The '%s' command Allocates a certain number of bytes.\nto Allocate Memory, enter 'allocate-memory'"},
        {.str_label = {CMD_FREE_MEMORY},
                .help_message = "The '%s' command frees the memory in the heap .\nto free memory, enter 'free-memory'"},
        {.str_label = {CMD_SHOW_ALLOCATE},
                .help_message = "The '%s' command prints through everything in the list.\nto show allocated memory, enter 'show-allocate'"},
        {.str_label = {CMD_SHOW_FREE},
                .help_message = "The '%s' command prints through the free list.\nto show free memory, enter 'show-free'"},
        {.str_label = {CMD_CLEAR_LABEL},
                .help_message = "The '%s' command clears the screen.\nto clear your terminal, enter 'clear'"},
        {.str_label = {CMD_COLOR_LABEL},
                .help_message = "The '%s' command sets the color of text output.\nto change your color, enter 'color'"},
        {.str_label = {CMD_PCB_LABEL},
                .help_message = "The '%s' command shows all the pcb commands available to the user. the help commands are listed below\n=> enter 'help pcb delete'\n=> enter 'help pcb suspend'\n=> enter 'help pcb resume'\n=> enter 'help pcb priority'\n=> enter 'help pcb show'\n=> enter 'help pcb show-ready'\n=> enter 'help pcb show-blocked'\n=> enter 'help pcb show-all'"},
        {.str_label = {CMD_PCB_LABEL, "delete"},
                .help_message = "The '%s' Command Deletes the process and frees all associated memory"},
        {.str_label = {CMD_PCB_LABEL, "suspend"},
                .help_message = "The '%s' Command puts the process in a suspended state"},
        {.str_label = {CMD_PCB_LABEL, "resume"},
                .help_message = "The '%s' Command resumes the process after its been suspended"},
        {.str_label = {CMD_PCB_LABEL, "priority"},
                .help_message = "The '%s' Command changes the process's priority"},
        {.str_label = {CMD_PCB_LABEL, "show"},
                .help_message = "The '%s' Command displays the process's info including name, class, state, status, and priority"},
        {.str_label = {CMD_PCB_LABEL, "show-ready"},
                .help_message = "The '%s' Command displays the process's info including name, class, state, status, and priority when in the ready state"},
        {.str_label = {CMD_PCB_LABEL, "show-blocked"},
                .help_message = "The '%s' Command displays the process's info including name, class, state, status, and priority when in the blocked state"},
        {.str_label = {CMD_PCB_LABEL, "show-all"},
                .help_message = "The '%s' Command displays the process's info including name, class, state, status, and priority no matter what state its in"},
        {.str_label = {CMD_DRAGONMAZE},
            .help_message = "The '%s' Command will start up the dragonmaze game. Using W A S D you can manuver the character to try and save the princess, but beware of the dragon."},
        {.str_label = {CMD_MINESWEEPER},
            .help_message = "The '%s' Command will start up a fresh game of classic minesweeper. \nUsing W A S D to move, you can use the spacebar to blow up squares, and [f] to flag potential mines."},

};

bool cmd_help(const char *comm)
{
    //The command's label.
    const char *label = CMD_HELP_LABEL;

    //Check if it matched.
    if (!first_label_matches(comm, label))
        return false;

    const char *split_label = " ";
    size_t str_len = strlen(comm);

    //Create a copy.
    char comm_cpy[str_len + 1];
    memcpy(comm_cpy, comm, str_len + 1);

    char *spl_token = strtok(comm_cpy, split_label); //First Time
    //Bump the token forward.
    int help_m_len = sizeof(help_messages) / sizeof(help_messages[0]);
    if ((spl_token = strtok(NULL, split_label)) != NULL)
    {
        int param_index = 0;
        char param2D[10][50] = {{0}};

        //Tokenize the parameters.
        do {
            strcpy(param2D[param_index], spl_token, 49);
            param_index++;
        } while((spl_token = strtok(NULL, split_label)) != NULL);

        //Get the actual string they entered.
        size_t label_len = strlen(label);
        const char *str_help_labels = comm + label_len;
        while(str_help_labels[0] == ' ')
            str_help_labels++;

        //Try to find help for the specific command.
        for (int i = 0; i < help_m_len; ++i)
        {
            struct help_info *specific_help = help_messages + i;
            int help_index = 0;

            while (help_index <= param_index && specific_help->str_label[help_index] != NULL){
                if (strcicmp(param2D[help_index], specific_help->str_label[help_index]) != 0){
                    break;
                }

                help_index++;
            }

            //Check if the label matches.
            if (help_index == -1 || help_index < param_index || specific_help->str_label[help_index] != NULL)
            {
                continue;
            }
            printf(specific_help->help_message, str_help_labels);
            println("");
            return true;
        }
        

        //At this point, we didn't find any valid strings.
        printf("Couldn't find any help!\n", spl_token);
        return true;
    }
    //All the help function, and the possible functions that are associated to it
    println("If you need help, enter one of the Statements below!");
    println("=> enter 'help set-time'");
    println("=> enter 'help set-date'");
    println("=> enter 'help get-time-date'");
    println("=> enter 'help help'");
    println("=> enter 'help set-timezone'");
    println("=> enter 'help alarm'");
    println("=> enter 'help load-r3'");
    println("=> enter 'help version'");
    println("=> enter 'help shutdown'");
    println("=> enter 'help color'");
    println("=> enter 'help clear'");
    println("=> enter 'help pcb'");
    println("=> enter 'help allocate-memory'");
    println("=> enter 'help free-memory'");
    println("=> enter 'help show-allocate'");
    println("=> enter 'help show-free");
    println("=> enter 'help dragonmaze'");
    println("=> enter 'help minesweeper'");
    return true;
}

bool cmd_set_tz(const char *comm)
{
    const char *label = CMD_SET_TIMEZONE_LABEL;
    if (!first_label_matches(comm, label))
    {
        return false;
    }

    //Create a copy.
    size_t str_len = strlen(comm);
    char comm_cpy[str_len + 1];
    memcpy(comm_cpy, comm, str_len + 1);

    char tz_buf[10] = {0};
    char *tz_token = strtok(comm_cpy, " ");

    //Advance the token forward.
    tz_token = strtok(NULL, " ");
    if(tz_token == NULL)
    {
        //Prompt the user again.
        println("What time zone do you want to be in?");

        //Iterate over the timezone.
        const time_zone_t **all_tzs = get_all_timezones();
        int printed = 0;
        while(all_tzs[printed] != NULL)
        {
            const time_zone_t *tz_ptr = all_tzs[printed];
            printf("=> %s (%s - %s)\n", tz_ptr->tz_label, tz_ptr->tz_longformat, tz_ptr->tz_city);
            printed++;
        }
        set_cli_history(0);
        print(": ");
        gets(tz_buf, 9);
        set_cli_history(1);
    }
    else
    {
        strcpy(tz_buf, tz_token, 9);
    }

    //Check it against all other timezones.
    const time_zone_t *tz_ptr = get_timezone(tz_buf);
    if(tz_ptr == NULL)
    {
        printf("Timezone '%s' not recognized!\n", tz_buf);
        return true;
    }
    

    set_timezone(tz_ptr);
    printf("Set the timezone to '%s'!\n", tz_ptr->tz_longformat);
    return true;
}

bool cmd_clear(const char *comm)
{
    //Check if the label matches.
    if(!first_label_matches(comm, CMD_CLEAR_LABEL))
        return false;

    clearscr();
    return true;
}

bool cmd_color(const char *comm)
{
    if(!first_label_matches(comm, CMD_COLOR_LABEL))
        return false;

    size_t comm_len = strlen(comm);
    char comm_cpy[comm_len + 1];
    memcpy(comm_cpy, comm, comm_len + 1);
    char *token = strtok(comm_cpy, " ");
    //Advance the token
    token = strtok(NULL, " ");

    char input[20] = {0};
    if(token == NULL)
    {
        //Print all the available colors.
        const color_t **colors = get_colors();
        const color_t *current_color = get_output_color();

        int index = 0;
        while(colors[index] != NULL)
        {
            print("=> ");

            //Print the color.
            set_output_color(colors[index]);
            print(colors[index]->color_label);
            set_output_color(current_color);

            println("");
            index++;
        }
        print(": ");

        gets(input, 19);
    }
    else
    {
        strcpy(input, token, 19);
    }

    const color_t *color = get_color(input);
    if(color == NULL)
    {
        printf("The color '%s' is not defined!\n", input);
        return true;
    }

    set_output_color(color);
    printf("Set the color to '%s'!\n", color->color_label);
    return true;
}

bool cmd_yield(const char *comm)
{
    if(!first_label_matches(comm, CMD_YIELD))
        return false;

    sys_req(IDLE);
    return true;
}

bool cmd_pcb(const char *comm)
{
    if(!first_label_matches(comm, CMD_PCB_LABEL))
        return false;

    //Pass the command to the pcb.
    size_t label_len = strlen(CMD_PCB_LABEL);
    exec_pcb_cmd(comm + label_len);
    return true;
}

/**
 *
 * @param comm
 * @param message
 * @return
 * @authors Jared Crowley
 */
bool cmd_alarm(const char *comm)
{
    const char *label = CMD_SET_ALARM;
    // Means that it did not start with label therefore it is not a valid input
    if (!first_label_matches(comm, label))
    {
        return false;
    }

    size_t comm_strlen = strlen(comm);
    //Get the time.
    char comm_cpy[comm_strlen + 1];
    memcpy(comm_cpy, comm, comm_strlen + 1);
    char *time_token = strtok(comm_cpy, " ");
    time_token = strtok(NULL, " ");

    // time is provided
    if (time_token == NULL)
    {
        println("Time value must be provided. Try 'alarm HH:mm:SS'");
        return true;
    }

    //add buffer to accept user input
    char message_buf[51] = {0};
    str_strip_whitespace(time_token, NULL, 0);

    // buffer to save numbers
    char time_array[3][3] = {{0}};
    // if part after set time is not valid with form hh:mm:ss returns with invalid date
    if (split(time_token, ':', 3, time_array, 3) < 0 ||
        !is_valid_date_or_time(3, time_array, 3))
    {
        printf("Invalid time. You entered: %s, expecting format: HH:mm:SS\n", time_token);
        return true;
    }

    unsigned char hour_dec = atoi(time_array[0]);
    unsigned char minute_dec = atoi(time_array[1]);
    unsigned char second_dec = atoi(time_array[2]);

    //Do some pre error checking.
    if (hour_dec < 0 || hour_dec > 23)
    {
        println("Hour is out of range 0-23!");
        return true;
    }
    if (minute_dec < 0 || minute_dec > 59)
    {
        println("Minutes is out of range 0-59!");
        return true;
    }
    if (second_dec < 0 || second_dec > 59)
    {
        println("Seconds is out of range 0-59!");
        return true;
    }

    //prompt user for message to be saved
    printf("Enter the message to be saved in the alarm (Maximum 50 chars):\n");
    print("> ");
    //save message into the buffer
    gets(message_buf, 50);

    //print users alarm with attached message
    int now_time[7];
    get_time(now_time);
    adj_timezone(now_time, get_clock_timezone()->tz_hour_offset, get_clock_timezone()->tz_minute_offset);
    now_time[4] = hour_dec;
    now_time[5] = minute_dec;
    now_time[6] = second_dec;
    bool result = create_new_alarm(now_time, message_buf);
    if(!result)
    {
        println("Failed to create a new alarm! (Heap may be full!)");
        return true;
    }

    printf("Set an alarm for: %s with the message %s\n", time_token, message_buf);
    return true;

}

bool cmd_allocate_memory(const char* comm){
     const char *label = CMD_ALLOCATE_MEMORY; 
    // Means that it did not start with label therefore it is not a valid input
    if (!first_label_matches(comm, label))
    {
        return false;
    }
    print("How many bytes do you want to allocate?\n");
    char msg_buf[12] = {0};
    set_cli_history(0);
    gets(msg_buf, 12);
    set_cli_history(1);

    //Check confirmation.
        size_t byte_size = atoi(msg_buf);
        void* allocate_size = allocate_memory(byte_size);
        if (allocate_size == NULL){
            printf("Not able to Allocate the Appropriate amount of bytes\n");
        }
        // else if (allocate_size > 50000){
        //     printf("The Byte Size is too Big!");
        // }
         else {
            printf("Successfully Allocated The %d Number of Bytes at 0x%x!\n", byte_size, allocate_size);
        }
        //printf(sizeof(MCB));
    return true;
}
bool cmd_show_allocate(const char* comm){
     const char *label = CMD_SHOW_ALLOCATE;
    // Means that it did not start with label therefore it is not a valid input
    
    if (!first_label_matches(comm, label))
    {
        //printf("%x", 0b00111);
        return false;
    }

    print_partial_list(false);
    return true;
}

bool cmd_show_free(const char* comm){
     const char *label = CMD_SHOW_FREE;
    // Means that it did not start with label therefore it is not a valid input
    
    if (!first_label_matches(comm, label))
    {
        //printf("%x", 0b00111);
        return false;
    }
    print_partial_list(true);
    //print("\n");

    return true;
}
bool cmd_free_memory(const char* comm){
     const char *label = CMD_FREE_MEMORY;
    // Means that it did not start with label therefore it is not a valid input
    if (!first_label_matches(comm, label))
    {
        return false;
    }

    size_t comm_strlen = strlen(comm);
    //Get the time.
    char comm_cpy[comm_strlen + 1];
    memcpy(comm_cpy, comm, comm_strlen + 1);
    char *hex = strtok(comm_cpy, " ");
    hex = strtok(NULL, " ");

    // time is provided
    if (hex == NULL)
    {
        println("Hex value not provided please input value as 54 not 0x54");
        return true;
    }
    if(*hex == '0' && *(hex+1) == 'x'){
        hex+=2;
    } 
    int address = atox(hex);
    int err = free_memory((void *) address);
    if(err != 0)
    {
        printf("Failed to free memory error code: %d\n", err);
    }
    else 
    {
        println("Successfully freed memory");
    }

    return true;
}

bool cmd_minesweeper(const char *comm)
{
    const char *label = CMD_MINESWEEPER;
    if(!first_label_matches(comm, label))
        return false;

    start_minesweeper_game(next_random());
    return true;
}

bool cmd_dragonmaze(const char *comm)
{
    const char *label = CMD_DRAGONMAZE;
    if(!first_label_matches(comm, label))
        return false;

    start_dragonmaze_game();
    return true;
}