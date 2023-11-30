//
// Created by Andrew Bowie on 1/21/23.
//

#include "time_zone.h"
#include "stdio.h"
#include "stddef.h"
#include "string.h"

///The eastern time timezone
static const time_zone_t ET = {.tz_label = "ET",
        .tz_longformat = "Eastern Time",
        .tz_hour_offset = -5,
        .tz_minute_offset = 0,
        .tz_city = "New York"};
///The central time timezone
static const time_zone_t CT = {.tz_label = "CT",
        .tz_longformat = "Central Time",
        .tz_hour_offset = -6,
        .tz_minute_offset = 0,
        .tz_city = "Chicago"};
///The mountain time timezone
static const time_zone_t MT = {.tz_label = "MT",
        .tz_longformat = "Mountain Time",
        .tz_hour_offset = -7,
        .tz_minute_offset = 0,
        .tz_city = "Denver"};
///The pacific time timezone
static const time_zone_t PT = {.tz_label = "PT",
        .tz_longformat = "Pacific Time",
        .tz_hour_offset = -8,
        .tz_minute_offset = 0,
        .tz_city = "Los Angeles"};
static const time_zone_t AKT = {.tz_label = "AKT",
        .tz_longformat = "Alaskan Time",
        .tz_hour_offset = -9,
        .tz_minute_offset = 0,
        .tz_city = "Anchorage"};
static const time_zone_t HAT = {.tz_label = "HAT",
        .tz_longformat = "Hawaii-Aleutian Time",
        .tz_hour_offset = -10,
        .tz_minute_offset = 0,
        .tz_city = "Honolulu"};
///The universal time timezone
static const time_zone_t UTC = {.tz_label = "UTC",
        .tz_longformat = "Universal Time Code",
        .tz_hour_offset = 0,
        .tz_minute_offset = 0,
        .tz_city = "London"};

///Central european timezone.
static const time_zone_t CET = {.tz_label = "CET",
        .tz_longformat = "Central European Time",
        .tz_hour_offset = +1,
        .tz_minute_offset = 0,
        .tz_city = "Paris"};
///Arabian timezone.
static const time_zone_t AT = {.tz_label = "AT",
        .tz_longformat = "Arabian Time",
        .tz_hour_offset = +2,
        .tz_minute_offset = 0,
        .tz_city = "Moscow"};

///Contains all timezones, in an array, for easier iteration
static const time_zone_t *TIMEZONES[] = {
    &ET,
    &CT,
    &MT,
    &PT,
    &AKT,
    &HAT,
    &UTC,
    &CET,
    &AT,
    NULL //For iteration
};

const time_zone_t **get_all_timezones(void)
{
    return TIMEZONES;
}

const time_zone_t *get_timezone(const char *tz_label) {
    //Iterate over all timezones.
    int index = 0;
    while(TIMEZONES[index] != NULL)
    {
        const time_zone_t *tz = TIMEZONES[index];
        if(strcicmp(tz_label, tz->tz_label) == 0)
            return tz;
        index++;
    }
    return NULL;
}