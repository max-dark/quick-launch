/*/
 + Project name : [Q] Launch
 + File name    : settings.h
 + Description  : сохранение/загрузка настроек
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#ifndef __settings_h
#define __settings_h

#include <stdio.h>
#include <windows.h>

//extern bool  changed;
extern bool autohide;
extern bool visible;
extern POINT position;

typedef struct {
  char* command;
  char* iconfile;
  char* tooltip;
  char* workpath;
  int   id;
  HWND  hwnd;
  HICON icon;
}settings,* lpsettings;

typedef struct {
	COLORREF bg,fg,sg;
}colors,*lpcolors;

const char defttip[] = "You mast define action!";

void InitSettings (lpsettings &set,int count,colors &clr);
void ClearSettings(lpsettings &set,int count,colors &clr);
void StoreSettings(char* file,lpsettings set,int count,colors &clr);
void LoadSettings (char* file,lpsettings set,int count,colors &clr);

#endif
