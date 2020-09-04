/*/
 + Project name : [Q] Launch
 + File name    : utils.h
 + Description  : вспомогательные функции
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#ifndef __utils_h
#define __utils_h

#include <windows.h>

#define WM_SYSTRAYICON WM_USER+10
#define reg_run "Software\\Microsoft\\Windows\\CurrentVersion\\Run"

typedef void (*proc)(HINSTANCE xmod);
extern bool hooked;

#ifdef hwnd_to_file
extern const char tmpset[];
#endif

//typedef struct
//{
//  HICON* icons;
//  long   count;
//  long   index;
//}IconList,*lpIconList;
static inline bool FileExists(const char* FNAME) {
	return (GetFileAttributes(FNAME) != 0xFFFFFFFF);
}
bool OSIsWinowsXP();
void Launch(char *mod,char *modpath=NULL);
HICON GetModuleIcon(char* modpath,int id=0);
//void GetModuleIconList(char* modpath,lpIconList il);

bool OpenModule(HWND hwnd,char* modpath,long count,char* filter=NULL);
void GetModulePath(HINSTANCE hInstance,char* modpath,long count);
void ExtractFileName(const char* path,char** name);
bool ChooseFolder(HWND hwnd,LPSTR lpszTitle,LPSTR lpBuffer);
bool SelectColor(HWND parent,COLORREF &clr);

void DelTrayIcon();
void TrayIcon(HWND parent,HICON icon,const char* sztip);

bool AutoRun();
bool AutoRun(bool set);

void Hook(HWND parent);
void UnHook();

void OutError();

#endif
