//
// Created by Andrew Bowie on 2/1/23.
//

#include "print_format.h"
#include "stdio.h"

/**
 * Prints an ANSII format code to the standard output.
 *
 * @param code the code to print.
 */
void print_format_code(int code)
{
    printf("%c[%dm", 27, code);
}

///The bold format code.
static const int BOLD_CODE = 1;
///The reset code for bold.
static const int BOLD_RESET_CODE = 22;
///The bold format code.
static const int ITALIC_CODE = 3;
///The reset code for bold.
static const int ITALIC_RESET_CODE = 23;
///The bold format code.
static const int UNDERLINE_CODE = 4;
///The reset code for bold.
static const int UNDERLINE_RESET_CODE = 24;
///The blinking format code.
static const int BLINKING_CODE = 5;
///The reset code for blinking.
static const int BLINKING_RESET_CODE = 25;
///The inverse format code.
static const int INVERSE_CODE = 7;
///The reset code for inverse.
static const int INVERSE_RESET_CODE = 27;
///The invisible format code.
static const int INVISIBLE_CODE = 8;
///The reset code for invisible.
static const int INVISIBLE_RESET_CODE = 28;
///The bold format code.
static const int STRIKETHROUGH_CODE = 9;
///The reset code for bold.
static const int STRIKETHROUGH_RESET_CODE = 29;

///All format codes stored by their enum index. (Code = enum num * 2, Reset Code = enum num * 2 + 1).
static const int FORMAT_CODES[(STRIKETHROUGH + 1) * 2] = {
        BOLD_CODE,
        BOLD_RESET_CODE,
        UNDERLINE_CODE,
        UNDERLINE_RESET_CODE,
        ITALIC_CODE,
        ITALIC_RESET_CODE,
        INVISIBLE_CODE,
        INVISIBLE_RESET_CODE,
        INVERSE_CODE,
        INVERSE_RESET_CODE,
        BLINKING_CODE,
        BLINKING_RESET_CODE,
        STRIKETHROUGH_CODE,
        STRIKETHROUGH_RESET_CODE
};

///Storage for if certain format codes are active.
static const bool ACTIVE_FORMAT_CODES[STRIKETHROUGH + 1] = {0};

bool is_format_code(format_code_t format_code)
{
    if(format_code > STRIKETHROUGH)
        return false;

    return ACTIVE_FORMAT_CODES[format_code];
}

void set_format_code(format_code_t format_code, bool active)
{
    if(active)
        print_format_code(FORMAT_CODES[format_code * 2]);
    else
        print_format_code(FORMAT_CODES[format_code * 2 + 1]);
}

void clear_formats()
{
    for (int i = 0; i <= STRIKETHROUGH; ++i)
    {
        set_format_code(i, false);
    }
}