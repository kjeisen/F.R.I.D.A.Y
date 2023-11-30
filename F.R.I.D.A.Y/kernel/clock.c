#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "math.h"
#include "mpx/clock.h"
#include <mpx/io.h>
#include <ctype.h>
#include <mpx/interrupts.h>

//Defines all the time variables
#define YEAR 0x09
#define MONTH 0x08
#define DATE 0x07
#define DAY 0x06
#define HOURS 0x04
#define MINUTES 0x02
#define SECONDS 0x00

#define TIME_OUT_FORMAT "%s, %02d/%02d/%02d @ %02d:%02d:%02d %s"

/**
 * The timezone hour offset. Initialized by the @code get_clock_timezone function.
 * This pointer should not be used directly, all access should be through @code get_block_timezone.
 */
static const time_zone_t *clock_tz = NULL;

/**
 * @brief Gets the clock value at the specific address index and converts it from BCD to a normal number.
 * Should use one of the compiler constant time addresses in this file.
 * @param a the address to read from.
 * @return the normal number read from the specific index.
 */
int get_index(int a);

const time_zone_t *get_clock_timezone(void) {
    //Check if the clock timezone has been initialized.
    if(clock_tz == NULL)
        clock_tz = get_timezone("UTC");

    return clock_tz;
}

void set_timezone(const time_zone_t *tz){
    //Can't set timezone to null.
    if(tz == NULL)
        return;

    clock_tz = tz;
}

int *adj_timezone(int time[6], int tz_offset_hr, int tz_offset_min)
{
    int *year = time;
    int *month = time + 1;
    int *date = time + 2;
    int *day = time + 3;
    int *hours = time + 4;
    int *mins = time + 5;

    //Apply TZ offset.
    *mins += tz_offset_min;
    *hours += tz_offset_hr;

    //Adjust minutes.
    if (*mins < 0)
    {
        (*hours)--;
    } else if (*mins >= 60)
    {
        (*hours)++;
    }

    //Adjust hours.
    if (*hours < 0)
    {
        (*date)--;
        (*day)--;
    } else if (*hours > 23)
    {
        (*date)++;
        (*day)++;
    } else
    {
        return time;
    }
    *hours = (int) ui_realmod(*hours, 24);

    //Adjust day.
    *day = (int) ui_realmod((*day) - 1, 7) + 1;

    //Adjust date.
    unsigned int bcd_dim = get_days_in_month(decimal_to_bcd(*month), decimal_to_bcd(*year));
    int dim = (int) ((((bcd_dim >> 4) & 0xF) * 10) + bcd_dim & 0xF);
    if (*date < 1)
    {
        (*month)--;
    } else if (*date > dim)
    {
        (*month)++;
    } else
    {
        return time;
    }

    //Adjust month.
    if (*month < 1)
    {
        (*year)--;
    } else if (*month > 12)
    {
        (*year)++;
    } else
    {
        return time;
    }
    *month = (int) ui_realmod(*month - 1, 12) + 1;
    return time;
}

int *get_time(int time[7])
{
    //Set up the storage array.
    static int time_arr[7] = {0};
    int *storage = NULL;
    if(time == NULL)
    {
        storage = time_arr;
    }
    else
    {
        storage = time;
    }

    int year = get_index(YEAR);
    int month = get_index(MONTH);
    int date = get_index(DATE);
    int day_of_week = get_index(DAY);
    int hours = get_index(HOURS);
    int minutes = get_index(MINUTES);
    int seconds = get_index(SECONDS);

    storage[0] = year;
    storage[1] = month;
    storage[2] = date;
    storage[3] = day_of_week;
    storage[4] = hours;
    storage[5] = minutes;
    storage[6] = seconds;
    return storage;
}

//Tuesday, 1/17/23  @ 09:08:04
int print_time(void)
{
    int year = get_index(YEAR);
    int month = get_index(MONTH);
    int date = get_index(DATE);
    int day_of_week = get_index(DAY);
    int hours = get_index(HOURS);
    int minutes = get_index(MINUTES);
    int seconds = get_index(SECONDS);

    int time_arr[6] = {year, month, date, day_of_week, hours, minutes};
    adj_timezone(time_arr,
                 get_clock_timezone()->tz_hour_offset,
                 get_clock_timezone()->tz_minute_offset);

    year = time_arr[0];
    month = time_arr[1];
    date = time_arr[2];
    day_of_week = time_arr[3];
    hours = time_arr[4];
    minutes = time_arr[5];

    char *week;
    if (day_of_week == 1)
    {
        week = "Sunday";
    }
    else if (day_of_week == 2)
    {
        week = "Monday";
    }
    else if (day_of_week == 3)
    {
        week = "Tuesday";
    }
    else if (day_of_week == 4)
    {
        week = "Wednesday";
    }
    else if (day_of_week == 5)
    {
        week = "Thursday";
    }
    else if (day_of_week == 6)
    {
        week = "Friday";
    }
    else
    {
        week = "Saturday";
    }

    printf(TIME_OUT_FORMAT, week, month, date, year, hours, minutes, seconds, get_clock_timezone()->tz_label);
    println("");
    return 0;
}


int get_index(int a)
{
    outb(0x70, a);
    int bits = inb(0x71);

    int fixed = ((bits >> 4) & 0xF) * 10;
    fixed = fixed + (bits & 0xF);

    return fixed;
}

unsigned int get_days_in_month(int month, int year)
{
    int decimal_year = ((year >> 4) & 0xF) * 10;
    decimal_year += year & 0xF;

    switch (month)
    {
        case 1:
            return 0x31;
        case 2:
            if (decimal_year % 4 == 0)
                return 0x29;
            return 0x28;
        case 3:
            return 0x31;
        case 4:
            return 0x30;
        case 5:
            return 0x31;
        case 6:
            return 0x30;
        case 7:
            return 0x31;
        case 8:
            return 0x31;
        case 9:
            return 0x30;
        case 0x10:
            return 0x31;
        case 0x11:
            return 0x30;
        case 0x12:
            return 0x31;
    }
    return 0x00;
}

bool is_valid_date_or_time(int word_len, char buf[][word_len], int buff_len)
{
    int num_digs = 0;
    for(int i = 0; i < buff_len; i++){
        for(int j = 0; j < word_len-1;j++){
            if(buf[i][j] == '\0' && num_digs > 0) {
                num_digs = 0;
                continue;
            }
            if(!isdigit(buf[i][j]))
                return false;
            num_digs++;
        }
    }
    return true;
}

bool set_time_clock(unsigned int hr, unsigned int min, unsigned int sec)
{
    //Check the values.
    if(hr > 0x23 || min > 0x59 || sec > 0x59)
        return false;

    cli();
    outb(0x70, HOURS);
    outb(0x71, hr);
    outb(0x70, MINUTES);
    outb(0x71, min);
    outb(0x70, SECONDS);
    outb(0x71, sec);
    sti();
    return true;
}

bool set_date_clock(unsigned int month, unsigned int day, unsigned int year)
{
    if(month < 0x01 || month > 0x12 || year > 0x99)
        return false;

    //Check the days in the month.
    unsigned int days_in_month = get_days_in_month(month, year);
    if(day > days_in_month)
        return false;

    //It appears that it is necessary to 'refresh' the times in the clock.
    //If this is not done, the hours, minutes, and seconds reset to whatever they were
    //at the time of the previous read.
    get_index(YEAR);
    get_index(MONTH);
    get_index(DATE);
    get_index(DAY);
    get_index(HOURS);
    get_index(MINUTES);
    get_index(SECONDS);

    cli();
    outb(0x70, DATE);
    outb(0x71, day);
    outb(0x70, MONTH);
    outb(0x71, month);
    outb(0x70, YEAR);
    outb(0x71, year);
    sti();
    return true;
}


unsigned char decimal_to_bcd(unsigned int decimal)
{
    unsigned int first_half = decimal / 10;
    unsigned int second_half = decimal % 10;
    return ((first_half << 4) + second_half);
}
int bcd_to_decimal(unsigned char bcd)
{
    int first_half = (bcd & 0xF0) >> 4;
    int  second_half = bcd & 0x0F;
    return ((first_half*10) + second_half);
}