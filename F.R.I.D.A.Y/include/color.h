//
// Created by Andrew Bowie on 1/27/23.
//

#ifndef F_R_I_D_A_Y_COLOR_H
#define F_R_I_D_A_Y_COLOR_H

/**
 * @file color.h
 * @brief Contains definitions for the @code color_t struct and useful functions for it.
 */

///A definition for a color, used for terminal output.
typedef struct {
    ///The label for the color.
    const char *color_label;
    ///The ansi number for the color.
    const int color_num;
} color_t;

/**
 * Sets the output color for the terminal.
 * @param color the color to set.
 */
void set_output_color(const color_t *color);

/**
 * Gets the output color for the terminal.
 * @return the color.
 */
const color_t *get_output_color(void);

/**
 * @brief Gets a pointer to a color from the label provided.
 * @param label the label of the color.
 * @return a pointer to the color, or NULL.
 */
const color_t *get_color(const char *label);

/**
 * @brief Gets all registered colors. Terminated with 'NULL' for iteration.
 * @return the array of character pointers.
 */
const color_t **get_colors(void);

#endif //F_R_I_D_A_Y_COLOR_H
