#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#ifdef __APPLE__
#define USERNAME "USER"
#elif __MINGW32__
#define USERNAME "USERNAME"
#elif __linux__
#define USERNAME ""
#endif

int GetUserName(char* dest, int len);

#endif