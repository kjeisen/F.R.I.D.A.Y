
#ifndef F_R_I_D_A_Y_TIME_ZONE_H
#define F_R_I_D_A_Y_TIME_ZONE_H

/**
 * @file time_zone.h
 * @brief Contains the definition of the @code time_zone_t type.
 */

///The definition of a time zone, contains its label and offsets.
typedef struct {
    ///The label of the timezone.
    const char *tz_label;
    ///The long format of the timezone, (i.e. ET -> Eastern Time)
    const char *tz_longformat;
    ///The hour offset of this timezone.
    const int tz_hour_offset;
    ///The minute offset of this timezone.
    const int tz_minute_offset;
    ///The main/most populated city this timezone belongs to.
    const char* tz_city;
} time_zone_t;

/**
 * @brief Returns an array to all time zones defined in this file.
 * Terminated with a NULL pointer at the end of the array.
 * @return the array of pointers for timezones.
 */
const time_zone_t **get_all_timezones(void);

/**
 * @brief Gets a timezone from its label case insensitive
 * @param tz_label the label of the timezone
 * @return a pointer to the found timezone, or NULL.
 */
const time_zone_t *get_timezone(const char *tz_label);

#endif //F_R_I_D_A_Y_TIME_ZONE_H
