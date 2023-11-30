//
// Created by Andrew Bowie on 1/13/23.
//

#define FUNNY_MODE false

#include "stdio.h"
#include "stdarg.h"
#include "sys_req.h"
#include "string.h"
#include "memory.h"
#include "cli.h"
#include "print_format.h"
#include "math.h"

#define PRINTF_BUF_LEN 500

char *gets(char *str_buf, size_t buf_len)
{
    sys_req(READ, COM1, str_buf, buf_len);
    return str_buf;
}

char getc(void)
{
    char buf[2] = {0};
    sys_req(READ, COM1, buf, 1);
    return buf[0];
}

char pollc(void)
{
    int rc = sys_req(READ, COM1, NULL, 1);
    return (char) rc;
}

/**
 * An internal method to mess with the user. It will randomly inject formatting codes to change things like bolding or blinking.
 *
 * @param s the text to print.
 */
void print_funny(const char *s)
{
    int str_len = (int) strlen(s);
    const color_t *color = get_output_color();
    for (int i = 0; i < str_len; ++i)
    {
        char c = s[i];
        unsigned int next_rand = next_random();
        if(next_rand % 250 == 0) {
            set_format_code(BLINKING, true);
            sys_req(WRITE, COM1, "\0", 1);
        }else if (next_rand % 30 == 0 ){
            set_format_code(ITALIC, true);
            sys_req(WRITE, COM1, "\0", 1);
        }else if (next_rand % 15 == 0 ){
            set_format_code(BOLD, true);
            sys_req(WRITE, COM1, "\0", 1);
        }else {
            set_output_color(color);
            clear_formats();
        }

        sys_req(WRITE, COM1, &c, 1);
    }
    set_output_color(color);
    clear_formats();
}

void print(const char *s)
{
    int str_len = (int) strlen(s);

    sys_req(WRITE, COM1, s, str_len);
}

int printf(const char *s, ...)
{
    char buffer[PRINTF_BUF_LEN] = {0};

    //Format the string.
    va_list va;
    va_start(va, s);
    char *result = vsprintf(s, buffer, PRINTF_BUF_LEN, va);
    va_end(va);
    if (result == NULL)
        return -1;

    if(FUNNY_MODE)
    {
        bool should_print = true;
        size_t len = sizeof (result);
        for (size_t i = 0; i < len; ++i)
        {
            if(result[i] == 27)
                should_print = false;
        }

        if(should_print)
            print_funny(result);
        else
            print(result);
    }
    else
    {
        print(result);
    }
    return 0;
}

void clearscr(void)
{
    //The 'Clear Screen' action
    char clear_screen[5] = {
            27,
            '[',
            '2',
            'J',
            0
    };
    //The 'reset cursor' action
    char reset_cursor[7] = {
            27,
            '[',
            '1',
            ';',
            '1',
            'H',
            0
    };
    print(clear_screen);
    print(reset_cursor);
}

void println(const char *s)
{
    size_t str_len = strlen(s);
    char buffer[str_len + 2];
    memset(buffer, 0, str_len + 2);
    buffer[str_len] = '\n';
    memcpy(buffer, s, str_len);
    buffer[str_len + 1] = '\0';

    if(FUNNY_MODE)
        print_funny(buffer);
    else
        print(buffer);
}