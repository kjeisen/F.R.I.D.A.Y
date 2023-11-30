#include <string.h>
#include "stdarg.h"
#include "stdlib.h"
#include "ctype.h"
#include "stdio.h"

#define UPPER_CASE 1
#define LOWER_CASE 0

/**
 * @brief An internal method used to change the case of a string.
 * @param str the original string.
 * @param buffer the buffer to store the resulting string in, or NULL if the
 *               change should be done in place.
 * @param buf_len the buffer's length.
 * @param letter_case the case of the new string. 1 for upper, 0 for lower.
 * @return
 */
char *s_changecase(char *str, char *buffer, int buf_len, int letter_case);

char *s_changecase(char *str, char *buffer, int buf_len, int letter_case)
{
    const int upper_diff = 'a' - 'A';

    //Check if we should do the work in place or on the buffer.
    char *str_buf = NULL;
    int str_len = (int) strlen(str);
    if (buffer == NULL)
    {
        str_buf = str;
    } else
    {
        //Check bounds.
        if (str_len + 1 > buf_len)
            return NULL;

        str_buf = buffer;
    }

    for (int i = 0; i < str_len; ++i)
    {
        int uppercase = isupper(str[i]);
        int lowercase = islower(str[i]);

        //Change case depending on the letter_case value.
        if (uppercase && UPPER_CASE == letter_case)
            str_buf[i] = (char) (str[i] + upper_diff);
        else if (lowercase && LOWER_CASE == letter_case)
            str_buf[i] = (char) (str[i] - upper_diff);
        else
            str_buf[i] = str[i];
    }
    return str_buf;
}

bool first_label_matches(const char *str1, const char *label)
{
    //Create a copy.
    size_t comm_len = strlen(str1);
    char comm_cpy[comm_len + 1];
    memcpy(comm_cpy, str1, comm_len + 1);

    //Token the command.
    char *str_token = strtok(comm_cpy, " ");
    if(str_token == NULL)
        return false;

    return strcicmp(str_token, label) == 0;
}

void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
    unsigned char *dst = s1;
    const unsigned char *src = s2;

    //Keep a temporary buffer to store the bytes.
    //Doing this allows us to do 'in place' copies.
    //i.e. An array {'a', 'b', 'c', '0'} copied to itself one step forward
    //goes to {'a', 'a', 'b', 'c'}
    unsigned char tmpbuf[n];
    memset(tmpbuf, 0, n);
    for (size_t i = 0; i < n; ++i)
    {
        tmpbuf[i] = src[i];
    }

    //Then copy from the temporary buffer.
    for (size_t i = 0; i < n; i++)
    {
        dst[i] = tmpbuf[i];
    }
    return s1;
}

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    for (size_t i = 0; i < n; i++)
    {
        p[i] = (unsigned char) c;
    }
    return s;
}

char *strcpy(char *str_dest, const char *str_src, size_t maxlen)
{
    if (str_dest == NULL || str_src == NULL)
        return NULL;

    size_t src_len = strlen(str_src);
    size_t copy_len = maxlen < src_len && maxlen > 0 ? maxlen : src_len;

    //Copy the data.
    memcpy(str_dest, str_src, copy_len + 1);
    return str_dest;
}

int strcmp(const char *s1, const char *s2)
{

    // Remarks:
    // 1) If we made it to the end of both strings (i. e. our pointer points to a
    //    '\0' character), the function will return 0
    // 2) If we didn't make it to the end of both strings, the function will
    //    return the difference of the characters at the first index of
    //    indifference.
    while ((*s1) && (*s1 == *s2))
    {
        ++s1;
        ++s2;
    }
    return (*(unsigned char *) s1 - *(unsigned char *) s2);
}

int strcicmp(const char *s1, const char *s2)
{

    // Remarks:
    // 1) If we made it to the end of both strings (i. e. our pointer points to a
    //    '\0' character), the function will return 0
    // 2) If we didn't make it to the end of both strings, the function will
    //    return the difference of the characters at the first index of
    //    indifference.
    while ((*s1) && (tolower(*s1) == tolower(*s2)))
    {
        ++s1;
        ++s2;
    }
    return (tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2));
}

char *str_strip_whitespace(char *str, char *buffer, size_t buf_len)
{
    //Load the buffer to store it in.
    char *str_buf = NULL;
    size_t str_len = strlen(str);
    if (buffer == NULL)
    {
        str_buf = str;
    } else
    {
        if (str_len + 1 > buf_len)
            return NULL;

        str_buf = buffer;
    }

    //Find beginning and ending index.
    int starting_pos = 0;
    while (isspace(str[starting_pos]))
        starting_pos++;

    int end_pos = (int) str_len - 1;
    while (isspace(str[end_pos]))
        end_pos--;

    //Copy memory.
    int index = 0;
    for (int i = starting_pos; i <= end_pos; ++i)
    {
        str_buf[index++] = str[i];
    }
    str_buf[index] = '\0';
    return str_buf;
}

size_t strlen(const char *s)
{
    size_t len = 0;
    while (*s++)
    {
        len++;
    }
    return len;
}

char *str_to_upper(char *str, char *buffer, int buf_len)
{
    return s_changecase(str, buffer, buf_len, UPPER_CASE);
}

char *str_to_lower(char *str, char *buffer, int buf_len)
{
    return s_changecase(str, buffer, buf_len, LOWER_CASE);
}

char *strtok(char *restrict s1, const char *restrict s2)
{
    static char *tok_tmp = NULL;
    const char *p = s2;

    //new string
    if (s1 != NULL)
    {
        tok_tmp = s1;
    }
        //old string cont'd
    else
    {
        if (tok_tmp == NULL)
        {
            return NULL;
        }
        s1 = tok_tmp;
    }

    //skip leading s2 characters
    while (*p && *s1)
    {
        if (*s1 == *p)
        {
            ++s1;
            p = s2;
            continue;
        }
        ++p;
    }

    //no more to parse
    if (!*s1)
    {
        return (tok_tmp = NULL);
    }
    //skip non-s2 characters
    tok_tmp = s1;
    while (*tok_tmp)
    {
        p = s2;
        while (*p)
        {
            if (*tok_tmp == *p++)
            {
                *tok_tmp++ = '\0';
                return s1;
            }
        }
        ++tok_tmp;
    }

    //end of string
    tok_tmp = NULL;
    return s1;
}

/**
 * @brief Formats a decimal for a string. Returns a pointer to the formatted number,
 *        or NULL if the formatting was invalid.
 *        TODO When dynamically freeing RAM works, use that instead of passed buffers.
 * @param num the number to format.
 * @param args the arguments of formatting.
 * @param arg_len the length of the arguments.
 * @param buf the buffer to store the resulting string.
 * @param buf_len the length of this buffer.
 * @param base the base of the number
 * @return a pointer to the resulting string, or NULL.
 */
char *f_decimal(int num, const char *args, size_t arg_len, char *buf, size_t buf_len, int base)
{
    //Check the extra data.
    bool fill_zeros = arg_len > 1 ? args[0] == '0' : false;
    int arg_start_pos = fill_zeros ? 1 : 0;
    int fill_count = fill_zeros && arg_len > 1 ? atoi(args + arg_start_pos) : 0;
    if (fill_count < 0)
        return NULL;

    //Convert the argument.
    size_t total_possible_count = fill_count >= 34 ? fill_count : 34;

    //Check bounds.
    if (total_possible_count + 1 > buf_len)
        return NULL;

    //Convert the number.
    memset(buf, 0, total_possible_count + 1);
    itoa_base(num, base, buf, 34);
    int len = (int) strlen(buf);
    int total_fill_amount = fill_count - len;

    //Check if we need to fill zeros.
    if (fill_zeros && total_fill_amount > 0)
    {
        bool negative = buf[0] == '-';

        int start_pos = negative ? 1 : 0;
        memcpy(buf + start_pos + total_fill_amount, buf, 34);

        for (int k = 0; k < total_fill_amount; ++k)
        {
            buf[k + start_pos] = '0';
        }
    }
    return buf;
}

char *sprintf(const char *s, char *str, size_t buf_len, ...)
{
    va_list va;
    va_start(va, buf_len);
    char *result = vsprintf(s, str, buf_len, va);
    va_end(va);

    return result;
}

char *vsprintf(const char *s, char *str, size_t buf_len, va_list va)
{
    //Get the string length and try to format it.
    int str_len = (int) strlen(s);

    int str_ind = 0;
    for (int i = 0; i < str_len; ++i)
    {
        char at = s[i];

        //If we've found the formatting symbol, try to format.
        if (at != '%')
        {
            str[str_ind++] = at;
            continue;
        }

        //Find the appropriate formatting
        char arguments[5] = {0};
        int arg_count = 0;
        for (int j = i + 1; j < str_len; ++j)
        {
            char f_code = s[j];
            if (f_code == 's')
            {
                //Arguments not supported for strings.
                if (arg_count > 0)
                    return NULL;

                i = j;

                char *arg = va_arg(va, char *);

                size_t len = strlen(arg);
                if(len + str_ind >= buf_len)
                {
                    break;
                }
                for (size_t k = 0; k < len; ++k)
                {
                    str[str_ind++] = arg[k];
                }
                break;
            }
            else if (f_code == 'x')
            {
                i = j;

                //Check the extra data.
                char fbuffer[50] = {0};
                char *result = f_decimal(va_arg(va, int),
                                         arguments,
                                         arg_count,
                                         fbuffer,
                                         50,
                                         16);
                size_t len = strlen(fbuffer);
                if (result == NULL)
                {
                    return NULL;
                }

                //Insert string.
                for (size_t k = 0; k < len; ++k)
                {
                    str[str_ind++] = fbuffer[k];
                }
                break;
            }
            else if (f_code == 'd')
            {
                i = j;

                //Check the extra data.
                char fbuffer[50] = {0};
                char *result = f_decimal(va_arg(va, int),
                                         arguments,
                                         arg_count,
                                         fbuffer,
                                         50,
                                         10);
                size_t len = strlen(fbuffer);
                if (result == NULL)
                {
                    return NULL;
                }

                //Insert string.
                for (size_t k = 0; k < len; ++k)
                {
                    str[str_ind++] = fbuffer[k];
                }
                break;
            }
            else if (f_code == 'c')
            {
                //Multi args not supported.
                if (arg_count > 0)
                    return NULL;

                i = j;
                char val = va_arg(va, int);

                str[str_ind++] = val;
                break;
            } else if (f_code == '%')
            {
                //Multiple arguments not supported.
                if (arg_count > 0)
                    return NULL;

                i = j;
                str[str_ind++] = '%';
                break;
            }

            //If the argument was improperly defined, return.
            if (arg_count >= 5)
                return NULL;

            arguments[arg_count++] = f_code;
        }
    }
    str[str_ind] = '\0';
    return str;
}

bool startsWith(const char *string, const char *startingString)
{
    while (*string == *startingString)
    {
        if (*string == '\0' && *startingString == '\0')
            return true;

        string++;
        startingString++;
    }
    //Check if the prefix reached its end.
    if (*startingString)
        return false;
    return true;
}

bool ci_starts_with(const char *string, const char *prefix)
{
    while (tolower(*string) == tolower(*prefix))
    {
        if (*string == '\0' && *prefix == '\0')
            return true;

        string++;
        prefix++;
    }
    //Check if the prefix reached its end.
    if (*prefix)
        return false;
    return true;
}

char split_once_after(const char *string, const char *split_after, char *buff, int buff_len)
{
    int index = 0;
    while (*string && *split_after)
    {
        if (*string == *split_after)
        {
            string++;
            split_after++;
        } else
        {
            return -1;
        }
    }
    while (*string)
    {
        if (index >= buff_len - 1) return -1;
        buff[index++] = *string++;

    }
    buff[index] = '\0';
    return 0;
}

int split(const char *string, char split_at, int word_length, char buff[][word_length], int words)
{
    int length = 0;
    int num_words = 0;
    int start = 0;
    int end = 0;
    const char *temp = string;
    while (*string)
    {
        if (*string == split_at)
        {
            if (num_words == words) return -1;
            if (length > word_length) return -2;
            if (substring(temp, start, end, buff[num_words], word_length)) return -1;
            start = ++end;
            num_words++;
            string++;
            length = 0;
            continue;
        }
        end++;
        length++;
        string++;
    }
    if (*string == '\0' && length > 0)
    {
        if (substring(temp, start, end, buff[num_words], word_length)) return -1;
    }
    return 0;
}

int substring(const char *string, int start, int end, char buff[], int buff_size)
{
    int length = end - start;
    int index = 0;
    if (buff_size <= length) return -1;
    for (int i = 0; i < start; i++)
    {
        string++;
    }
    while (index < length)
    {
        buff[index++] = *string++;
    }
    buff[index] = '\0';

    return 0;
} 