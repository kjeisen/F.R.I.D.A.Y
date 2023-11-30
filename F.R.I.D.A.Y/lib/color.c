//
// Created by Andrew Bowie on 1/27/23.
//

#include "color.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"

///Information for the black ansii color.
static const color_t BLACK = {.color_label = "black", .color_num = 30};
///Information for the red ansii color.
static const color_t RED = {.color_label = "red", .color_num = 31};
///Information for the green ansii color.
static const color_t GREEN = {.color_label = "green", .color_num = 32};
///Information for the yellow ansii color.
static const color_t YELLOW = {.color_label = "yellow", .color_num = 33};
///Information for the blue ansii color.
static const color_t BLUE = {.color_label = "blue", .color_num = 34};
///Information for the magenta ansii color.
static const color_t MAGENTA = {.color_label = "magenta", .color_num = 35};
///Information for the cyan ansii color.
static const color_t CYAN = {.color_label = "cyan", .color_num = 36};
///Information for the white ansii color.
static const color_t WHITE = {.color_label = "white", .color_num = 37};

///Information for the bright-black ansii color.
static const color_t BRIGHT_BLACK = {.color_label = "bright-black", .color_num = 90};
///Information for the bright-red ansii color.
static const color_t BRIGHT_RED = {.color_label = "bright-red", .color_num = 91};
///Information for the bright-green ansii color.
static const color_t BRIGHT_GREEN = {.color_label = "bright-green", .color_num = 92};
///Information for the bright-yellow ansii color.
static const color_t BRIGHT_YELLOW = {.color_label = "bright-yellow", .color_num = 93};
///Information for the bright-blue ansii color.
static const color_t BRIGHT_BLUE = {.color_label = "bright-blue", .color_num = 94};
///Information for the bright-magenta ansii color.
static const color_t BRIGHT_MAGENTA = {.color_label = "bright-magenta", .color_num = 95};
///Information for the bright-cyan ansii color.
static const color_t BRIGHT_CYAN = {.color_label = "bright-cyan", .color_num = 96};
///Information for the bright-white ansii color.
static const color_t BRIGHT_WHITE = {.color_label = "bright-white", .color_num = 97};

///Information for the reset ansii color.
static const color_t RESET = {.color_label = "reset", .color_num = 0};

///The current color of output.
static const color_t *current_color = &RESET;

///An array of all colors registered in this file, terminated with null.
static const color_t *COLORS[] = {
        &BLACK,
        &BRIGHT_BLACK,
        &RED,
        &BRIGHT_RED,
        &GREEN,
        &BRIGHT_GREEN,
        &YELLOW,
        &BRIGHT_YELLOW,
        &BLUE,
        &BRIGHT_BLUE,
        &MAGENTA,
        &BRIGHT_MAGENTA,
        &CYAN,
        &BRIGHT_CYAN,
        &WHITE,
        &BRIGHT_WHITE,
        &RESET,
        NULL
};

void set_output_color(const color_t *color)
{
    if(color == NULL)
    {
        return;
    }
    current_color = color;

    //Print the ANSII escape sequence.
    printf("%c[%dm", 27, color->color_num);
}

const color_t *get_output_color(void)
{
    return current_color;
}

const color_t *get_color(const char *label)
{
    //Iterate over all timezones.
    int index = 0;
    while(COLORS[index] != NULL)
    {
        //Get the color
        const color_t *color = COLORS[index];
        if(strcicmp(label, color->color_label) == 0)
            return color;
        index++;
    }
    return NULL;
}

const color_t **get_colors(void)
{
    return COLORS;
}