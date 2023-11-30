//
// Created by Andrew Bowie on 1/19/23.
//

#include "math.h"

int abs(int x)
{
    return x < 0 ? -x : x;
}

unsigned int ui_realmod(int x, int mod)
{
    int val = x % mod;
    return val < 0 ? val + mod : val;
}

double pow(double a, double b)
{
    double c = a;

    int i = 1;
    if (b != 0)
    {
        if (i < b)
        {
            int i = 1;
            while (i < b)
            {
                c = c * a;
                i++;
            }
        }
        if (i > b)
        {
            int i = -1;
            while (i > b)
            {
                c = c * a;
                i--;
            }
            return (1 / c);
        }
        return c;
    }
    return 1;
}

///The LCRNG multiplier from C++.
#define SEED_MULTI 25214903917L
///The LCRNG addend from C++.
#define SEED_ADDEND 11L
///The bit mask to apply to the seed
#define SEED_MASK (0xFFFFFFFFFFFFL - 1L)

///The seed for random number generation.
static unsigned long long rand_seed = 0L;

unsigned long long get_seed(void)
{
    return rand_seed;
}

void s_rand(unsigned long long seed)
{
    //Apply the initial scramble of the seed.
    rand_seed = (seed ^ SEED_MULTI) & SEED_MASK;
}

unsigned int next_random(void)
{
    //Find the next seed.
    unsigned long long next_seed = (rand_seed * SEED_MULTI + SEED_ADDEND) & SEED_MASK;

    //Read 32 bits from the seed.
    unsigned int thing = (int) (next_seed >> 16L);
    rand_seed = next_seed;
    return thing;
}

unsigned int next_random_lim(int limit)
{
    return ui_realmod((int) next_random(), limit);
}

bool next_rand_bool(void)
{
    return next_random_lim(2) == 0;
}