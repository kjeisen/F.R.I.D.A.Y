#ifndef MPX_STRING_H
#define MPX_STRING_H

#include <stddef.h>
#include "stdarg.h"
#include "stdbool.h"

/**
 @file string.h
 @brief A subset of standard C library functions.
*/

/**
 * @brief Checks if the given string's first part matches the label.
 * @param str1 the string.
 * @param label the label.
 * @return if the string matches the label.
 */
bool first_label_matches(const char *str1, const char *label);

/**
 Copy a region of memory.
 @param dst The destination memory region
 @param src The source memory region
 @param n The number of bytes to copy
 @return A pointer to the destination memory region
*/
void* memcpy(void * restrict dst, const void * restrict src, size_t n);

/**
 Fill a region of memory.
 @param address The start of the memory region
 @param c The byte to fill memory with
 @param n The number of bytes to fill
 @return A pointer to the filled memory region
*/
void* memset(void *address, int c, size_t n);

/**
 * @brief Copies the data from the string source into the string destination.
 * If maxlen is exceeded, it only copies that amount of chars over.
 * @param str_dest the string destination.
 * @param str_src the string source.
 * @param maxlen the maximum amount of bytes to copy. Note that maxlen does not include the null terminator.
 * @return a pointer to the string, or NULL if there was an error.
 */
char *strcpy(char *str_dest, const char *str_src, size_t maxlen);

/**
 Compares two strings
 @param s1 The first string to compare
 @param s2 The second string to compare
 @return 0 if strings are equal, <0 if s1 is lexicographically before s2, >0 otherwise
*/
int strcmp(const char *s1, const char *s2);

/**
 * @brief Compares two strings, ignoring case
 * @param s1 The first string to compare
 * @param s2 The second string to compare
 * @return 0 if strings are equal, <0 if s1 is lexicographically before s2, >0 otherwise
 */
int strcicmp(const char *s1, const char *s2);

/**
 * @brief Strips leading and trailing whitespace from the given string.
 * @param str the string to strip from.
 * @param buffer the buffer to store the resulting string in, or NULL
 *               if the strip should be done in place.
 * @param buf_len the length of the buffer.
 * @return a pointer to the resulting string, or NULL if it failed.
 */
char *str_strip_whitespace(char *str, char *buffer, size_t buf_len);

/**
 Returns the length of a string.
 @param s A NUL-terminated string
 @return The number of bytes in the string (not counting NUL terminator)
*/
size_t strlen(const char *s);

/**
 * @brief Converts the given string to upper case. If the provided buffer
 * is null, overwrites the original string.
 * @param str     the original string.
 * @param buffer  the buffer to store the string in, or NULL if the original
 *                string should be overwritten.
 * @param buf_len the length of the buffer. If buffer is NULL, can be any
 *                number.
 * @return a pointer to the upper case string, or NULL if the buffer
 *         was too small to store the resulting string.
 */
char *str_to_upper(char *str, char *buffer, int buf_len);

/**
 * @brief Converts the given string to lower case. If the provided buffer
 * is null, overwrites the original string.
 * @param str     the original string.
 * @param buffer  the buffer to store the string in, or NULL if the original
 *                string should be overwritten.
 * @param buf_len the length of the buffer. If buffer is NULL, can be any
 *                number.
 * @return a pointer to the lower case string, or NULL if the buffer
 *         was too small to store the resulting string.
 */
char *str_to_lower(char *str, char *buffer, int buf_len);

/**
 Split string into tokens
 TODO
*/
char* strtok(char * restrict s1, const char * restrict s2);

/**
 * @brief Formats the string with normal C formatting options.
 * @param format the string format.
 * @param str the buffer to store the resulting string in.
 * @param buf_len the length of the provided string buffer.
 * @param ... the formatting values.
 * @return the formatted string.
 */
char *sprintf(const char *format, char *str, size_t buf_len, ...);

/**
 * @brief Formats the string with normal C formatting options.
 * @param format the string format.
 * @param str the buffer to store the resulting string in.
 * @param buf_len the length of the provided string buffer.
 * @param ... the formatting values.
 * @return the formatted string.
 */
char *vsprintf(const char *format, char *str, size_t buf_len, va_list varargs);

/** 
* @brief Returns string located after where to split, orginal string returned if not split
* @param string string to be split
* @param splitAt string that chooses where to split
* @return the string split or not
*/
char split_once_after(const char* string, const char* split_after, char buff[], int buff_len);

/** 
* @brief Returns true if string starts with given string 
* @param string string to be tested
* @param starts_with given string to start with
* @return if string starts with starts_with string
*/
bool starts_with(const char* string, const char* starts_with);

/**
 * @brief Returns true if the string starts with the given prefix.
 *        Case is ignored.
 * @param string the string to be tested.
 * @param prefix the prefix of the string.
 * @return true if the string starts with the prefix.
 */
bool ci_starts_with(const char *string, const char *prefix);

/**
* @brief Splits the given string at character saving into a 2D buffer
* @param string string to be split
* @param split_at character to split at
* @param wordlength length of the column dimension of buffer must match buff dimension
* @param words number of rows (words) available in buff
* @returns error codes 0 is successful, negative if not
*/
int split(const char *string, char split_at, int word_length, char buff[][word_length], int words);

/**
* @brief Splits the given string at character saving into a 2D buffer
* @param string string to be spliced
* @param start index to start at
* @param end index to end at
* @param buff buffer to save result to
* @param buff_size length of buff
* @returns error codes 0 is successful, negative if not
*/
int substring(const char* string, int start, int end, char buff[], int buff_size);
#endif
