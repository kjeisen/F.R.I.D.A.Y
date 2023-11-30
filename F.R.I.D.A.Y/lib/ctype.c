#include <ctype.h>

int isspace(int c)
{
	return (c == ' ' || c == '\n' || c == '\r' || c == '\f' || c == '\t' || c == '\v');
}
int isdigit(int c)
{
        return ('0' <= c && c <= '9');
}
int todigit(int c)
{
        if(isdigit(c)){
                return c - '0';
        }
        return -1;
        
}

int isupper(int c)
{
        return 'A' <= c && c <= 'Z';
}

int islower(int c)
{
        return 'a' <= c && c <= 'z';
}

int tolower(int c)
{
        const int char_diff = 'a' - 'A';
        if(islower(c))
                return c - char_diff;
        return c;
}

int toupper(int c)
{
        const int char_diff = 'a' - 'A';
        if(islower(c))
                return c + char_diff;
        return c;
}
