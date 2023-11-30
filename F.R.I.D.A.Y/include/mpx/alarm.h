#ifndef F_R_I_D_A_Y_ALARM_H
#define F_R_I_D_A_Y_ALARM_H

/**
 * @file alarm.h
 * @brief A header file for alarm functions.
 */

/**
 * @brief Creates a new pcb that will display message at or after given time
 * @param time_array the time to display message
 * @param message message to display
 * @return true if the alarm was created, false if it failed.
 * @author Kolby Eisenhauer, Andrew Bowie
 */
bool create_new_alarm(int *time_array, const char* message);

#endif
