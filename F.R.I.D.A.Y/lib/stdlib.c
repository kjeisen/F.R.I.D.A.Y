#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include "stdio.h"

static const char *num_encoding = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int atoi(const char *s)
{
    int res = 0;
    char sign = ' ';

    while (isspace(*s))
    {
        s++;
    }

    if (*s == '-' || *s == '+')
    {
        sign = *s;
        s++;
    }

    while ('0' <= *s && *s <= '9')
    {
        res = res * 10 + (*s - '0');
        s++;

    }

    if (sign == '-')
    {
        res = res * -1;
    }

    return res;
}

double atod(const char *s)
{
    double res = 0.0;
    char sign = ' ';
    int length = 0;
    double decimal = 0.0;

    while (isspace(*s))
    {
        s++;
    }

    if (*s == '-' || *s == '+')
    {
        sign = *s;
        s++;
    }

    while ('0' <= *s && *s <= '9')
    {
        res = res * 10 + (*s - '0');
        s++;

    }


    if (*s == '.')
    {
        s++;
    }
    length = strlen(s);

    while ('0' <= *s && *s <= '9')
    {
        decimal = decimal * 10 + (*s - '0');
        s++;
    }
    if (sign == '-')
    {
        res = res * -1;
        decimal = decimal * -1;
    }
    for (int i = 0; i < length; i++)
    {
        decimal = decimal / 10;
    }

    return res + decimal;
}

char *itoa(int i, char *str_buf, int buf_len)
{
    return itoa_base(i, 10, str_buf, buf_len);
}
//Converts a number to a string
char *itoa_base(int i, int base, char *str_buf, int buf_len)
{
    if (buf_len == 0) {
        return NULL;
    }

    if (i == 0)
    {

        str_buf[0] = '0';
        return str_buf;
    }

    int num_pos = 0;

    //Check for a sign.
    if (i < 0)
    {
        str_buf[0] = '-';
        i *= -1;
        num_pos = 1;
    }

    if (buf_len <= num_pos)
    {
        return NULL;
    }

    //Loop through the number, removing a single digit at a time.
    char swap[10] = {0};
    int num_index = 0;
    while (i > 0)
    {
        int digit = i % base;

        swap[num_index++] = num_encoding[digit];

        i /= base;
    }

    //If this is the case, we can't store the string.
    if (num_index + num_pos >= buf_len)
        return NULL;

    //Put all chars from swap into the buffer.
    for (int index = num_index - 1; index >= 0; --index)
    {
        str_buf[num_pos++] = swap[index];
    }
    return str_buf;
}

int atox(const char *s)
{
    while(isspace(*s)) s++;
    char sign = ' ';
    int value = 0;
    if(*s == '+' || *s == '-')
    {
        sign = *s;
        s++;
    }

    while((*s >= '0' && *s <= '9') || (*s <= 'F' && *s >= 'A'))
    {
        int digit_value = *s - '0';
        // subtract the offset in ascii values
        if(*s >= 'A') digit_value = digit_value - 7;
        value = (value << 4) + digit_value;
        s++;
    }
    if(sign == '-') value *= -1;

    return value;
}
