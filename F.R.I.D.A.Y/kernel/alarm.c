#include "stdio.h"
#include "stddef.h"
#include "mpx/pcb.h"
#include "string.h"
#include "mpx/clock.h"
#include "sys_req.h"
#include "mpx/pcb.h"
#include "stdlib.h"

/**
 * @file alarm.c
 * @brief Contains logic to create alarms for the OS.
 */

///The parameters used to pass into the alarm function.
typedef struct alarm_params
{
    ///A pointer to where the time is stored.
    int *time_ptr;
    ///A pointer to where the message is stored.
    char *str_ptr;
    ///The timezone used to create the alarm.
    time_zone_t *time_zone;
    ///The data to store.
    unsigned char buffer[100];
} alarm_structure;

/**
 * Check if the given time array of hours, minutes, seconds is after the other.
 *
 * @param now the time array considered to be 'now'
 * @param check the time to check at.
 * @return true if it is after.
 */
bool is_time_after(const int *now, const int *check)
{
    //Hours.
    if(now[0] < check[0])
        return false;
    else if(now[0] > check[0])
        return true;

    //Minutes.
    if(now[1] < check[1])
        return false;
    else if(now[1] > check[1])
        return true;

    //Seconds.
    if(now[2] < check[2])
        return false;
    else if(now[2] > check[2])
        return true;
    return true;
}

bool shouldAlarm(const int *time_array, time_zone_t *tz)
{
    // get current time
    int time_buf[7];
    get_time(time_buf);
    adj_timezone(time_buf, tz->tz_hour_offset, tz->tz_minute_offset);
    //Check the years.
    if(time_array[0] > time_buf[0])
        return false;

    //Check the month.
    if(time_array[1] > time_buf[1])
        return false;

    //Check day.
    if(time_array[2] > time_buf[2])
        return false;

    // index 4-6 is hours - seconds
    if (time_array[4] < time_buf[4])
        return true;
    // printf("alarm time is %d:%d:%d current time is %d:%d:%d\n", time_array[0], time_array[1], time_array[2], time_buf[4], time_buf[5], time_buf[6]);
    if (time_array[4] == time_buf[4] && time_array[5] < time_buf[5])
        return true;
    if (time_array[4] == time_buf[4] && time_array[5] == time_buf[5] && time_array[6] <= time_buf[6])
        return true;

    return false;
}

/**
 * @brief The alarm function used by the alarm processes.
 * @param time_array the time array to go off at.
 * @param message the message to send to the user.
 * @param time_zone the timezone to use for the alarm.
 * @authors Kolby Eisenhauer
 */
void alarm_function(int *time_array, const char *message, time_zone_t *time_zone)
{
    while (!shouldAlarm(time_array, time_zone))
    {
        sys_req(IDLE);
    }

    println(message);
    sys_req(EXIT);
}

///The alarm count used to make sure names are unique.
static int alarms = 0;


bool create_new_alarm(int *time_array, const char *message)
{
    char name[20] = {'a', 'l', 'a', 'r', 'm', '\0'};

    //Prepare the parameters for the function.
    alarm_structure parameters = {0};
    parameters.str_ptr = (char *) parameters.buffer;
    size_t len = strlen(message);
    strcpy(parameters.str_ptr, message, len);
    parameters.time_zone = (time_zone_t *) get_clock_timezone();

    int time_buf[7] = {0};
    get_time(time_buf);
    adj_timezone(time_buf, parameters.time_zone->tz_hour_offset, parameters.time_zone->tz_minute_offset);

    //Check if we need to adjust the days.
    if(is_time_after(time_buf + 4, time_array + 4))
    {
        adj_timezone(time_array, 24, 0);
    }

    //Copy in the time pointer.
    parameters.time_ptr = (int *) (parameters.buffer + len + 2);
    memcpy(parameters.time_ptr, time_array, 7 * sizeof(int));

    //Prepare the process' name.
    size_t name_len = 15;
    char process_name[name_len];
    do {
        memset(process_name, 0, name_len);
        strcpy(process_name, name, -1);
        char num_buf[10] = {0};
        itoa(alarms, num_buf, 9);
        strcpy(process_name + strlen(name), num_buf, -1);
        alarms++;
    }while(pcb_find(process_name) != NULL);

    //Generate the actual PCB.
    bool generated = generate_new_pcb(process_name, 1, USER, &alarm_function, (char *) &parameters, sizeof(alarm_structure), 2);
    return generated;
}

