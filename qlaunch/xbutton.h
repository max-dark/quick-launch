/*/
 + Project name : [Q] Launch
 + File name    : xbutton.h
 + Description  : Создание кнопок для главного окна
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#ifndef __xbutton_h
#define __xbutton_h

#include <windows.h>
#include "resource.h"

#define PI 3.1415926535897932384626433832795
#define XBM_SETPARAM WM_USER+11
#define XBM_CLICK    WM_USER+12
#define XBM_TOOLTIP  WM_USER+13

typedef struct {
	bool show; // show/hide tool tip
	int  id;   // ID of XButton
	int  x,y;  // Mouse position
}tooltip_msg,*lptooltip_msg;

typedef struct {
  HWND   hwnd;
  int    xc,yc;
  double alpha,beta;
  int    r1,r2;
  HBRUSH bg,sg;
  HICON  icon;
  char   sel,down;
  WPARAM id;
  POINT  pt;
}xbutton,*lpxbutton;

ATOM RegisterXButton(HINSTANCE hApplication);
HWND CreateXButton(HWND parent,WPARAM id,HICON icon,int xc,int yc,double alpha,double beta,int r1,int r2);
void SetXButtonColors(HWND btn,COLORREF fg,COLORREF sg);
void UnRegisterXButton();

#endif
