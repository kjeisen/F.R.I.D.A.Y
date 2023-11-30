//
// Created by Andrew Bowie on 2/1/23.
//

#ifndef F_R_I_D_A_Y_PRINT_FORMAT_H
#define F_R_I_D_A_Y_PRINT_FORMAT_H

#include "color.h"
#include "stdbool.h"

///All format codes available for this file.
typedef enum {
    BOLD = 0,
    UNDERLINE = 1,
    ITALIC = 2,
    INVISIBLE = 3,
    INVERSE = 4,
    BLINKING = 5,
    STRIKETHROUGH = 6,
} format_code_t;

/**
 * Checks if the given format code is active.
 *
 * @param format_code the format code.
 * @return true if the format code is active.
 */
bool is_format_code(format_code_t format_code);

/**
 * Sets the format code to active or not.
 *
 * @param format_code the format code.
 * @param active if it should be active or not.
 */
void set_format_code(format_code_t format_code, bool active);

/**
 * Clears all formatting from the output. Does NOT clear color!
 */
void clear_formats();

#endif //F_R_I_D_A_Y_PRINT_FORMAT_H
