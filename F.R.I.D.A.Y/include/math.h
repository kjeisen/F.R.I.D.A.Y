//
// Created by Andrew Bowie on 1/19/23.
//

#ifndef F_R_I_D_A_Y_MATH_H
#define F_R_I_D_A_Y_MATH_H

#include "stdbool.h"

/**
 * @file math.h
 * @brief A header full of useful math type functions
 */

/**
 * @brief Gets the absolute value of the number.
 * @param x the number.
 * @return the absolute value.
 */
int abs(int x);

/**
 * @brief Calculates the real modulo value of X modulo 'mod'.
 * @param x the value.
 * @param mod the modulo.
 * @return the modulo value of x modulo 'mod'
 */
unsigned int ui_realmod(int x, int mod);

/**
* @brief Calculates the Answer from a variable and a exponent
* @param a is the variable
* @param b is the exponent
* @return The new value from the a^b
*/
double pow(double a, double b);

/**
 * @brief Gets the current seed for the random.
 *
 * @return the seed.
 * @authors Andrew Bowie
 */
unsigned long long get_seed(void);

/**
 * @brief Seeds the random number generator.
 * @param seed the seed.
 */
void s_rand(unsigned long long seed);

/**
 * @brief Returns the next random 30 bits from the LCRNG.
 * @return the next random number.
 */
unsigned int next_random(void);
/**
 * @brief Generates the next random with the given limit.
 *
 * @param limit the limit.
 * @return the next random number.
 */
unsigned int next_random_lim(int limit);
/**
 * @brief Generates the next random boolean.
 * @return the next bool.
 */
bool next_rand_bool(void);
#endif //F_R_I_D_A_Y_MATH_H
