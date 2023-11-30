#ifndef MPX_CTYPE_H
#define MPX_CTYPE_H

/**
 @file ctype.h
 @brief A subset of standard C library functions.
*/

/**
 Determine if a character is whitespace.
 @param c Character to check
 @return Non-zero if space, 0 if not space
*/
int isspace(int c);

/**
 Determine if a character is a digit.
 @param c Character to check
 @return Non-zero if digit, 0 if not digit
*/
int isdigit(int c);
/**
 Return int value of character if is digit
 @param c Character to check
 @return Negative not digit, value of digit otherwise
*/
int todigit(int c);

/**
 * Determine if a character is uppercase. If the character is not
 * alphabetical, 0 is returned.
 * @param c Character to check.
 * @return Non-zero if upper, 0 if not upper.
 */
int isupper(int c);

/**
 * Determine if a character is lowercase. If the character is not
 * alphabetical, 0 is returned.
 * @param c Character to check.
 * @return Non-zero if lower, 0 if not lower.
 */
int islower(int c);

/**
 * @brief Converts the given character to lowercase.
 * @param c the character to convert.
 * @return the lowercase character.
 */
int tolower(int c);

/**
 * @brief Converts the given character to uppercase.
 * @param c the character to convert.
 * @return the uppercase character.
 */
int toupper(int c);

#endif
