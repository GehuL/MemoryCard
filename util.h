#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

// Redefinition car windows.h entre en collision avec raylib.h
int showMessageBox(void* hwnd, const char* title, const char* msg, unsigned int type);

int GetAsyncKeyState_(int);

void waitMsg();

unsigned int SetTimerEx(void* hwnd, unsigned int id, unsigned int time, void cb(void*c, unsigned int a, unsigned int b, unsigned long cc));

int KillTimerEx(void* hwnd, unsigned int id);

#endif // UTIL_H_INCLUDED
