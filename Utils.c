#include "Utils.h"
#include <stdlib.h>
#include <string.h>

int GetUserName(char* dest, int len)
{
    char* n = getenv(USERNAME);
    if(n == NULL)
    {
        strncpy(dest, "inconnue", len);
    }else
    {
        strncpy(dest, n, len);
    }
    return n != NULL;
}
