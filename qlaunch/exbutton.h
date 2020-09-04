/*/
 + Project name : [Q] Launch
 + File name    : exbutton.h
 + Description  : кнопки
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#ifndef __exbutton_h
#define __exbutton_h

#include <windows.h>
#include "resource.h"

typedef struct{
  HWND   hwnd;
  int    w,h;
  WPARAM id;
  char*  title;
  HBRUSH bg,sg;
  HFONT  fnt;
  HICON  icon;
  char   sel,down;
}exbutton,*lpexbutton;

ATOM RegisterExButton(HINSTANCE hApplication);
HWND CreateExButton(HWND parent,WPARAM id,
					char*title,HICON icon,HFONT fnt,
					int x,int y,int w,int h);
void UnRegisterExButton();

#endif
