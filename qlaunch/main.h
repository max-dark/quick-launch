/*/
 + Project name : [Q] Launch
 + File name    : main.h
 + Description  : основная программа
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#ifndef __main_h
#define __main_h

#include <windows.h>

extern HINSTANCE hApplication;
#define Ask(hwnd,ask) (MessageBox(hwnd,ask,"[Q]Launch", \
						MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2 \
						)==IDOK)
#endif
