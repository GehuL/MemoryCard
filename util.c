#ifndef UTIL_C_INCLUDED
#define UTIL_C_INCLUDED

#include "util.h"
#include "windows.h"

int showMessageBox(void* hwnd, const char* title, const char* msg, unsigned int type)
{
    return MessageBox(hwnd, msg, title, type);
}

int GetAsyncKeyState_(int i)
{
    return GetAsyncKeyState(i);
}

void waitMsg()
{
    WaitMessage();
}

unsigned int SetTimerEx(void* hwnd, unsigned int id, unsigned int time, void cb(void*c, unsigned int a, unsigned int b, unsigned long cc))
{
    return SetTimer((HWND)hwnd, id, time, (TIMERPROC)cb);
}

int KillTimerEx(void* hwnd, unsigned int id)
{
    return KillTimer(hwnd, id);
}


#endif // UTIL_C_INCLUDED
