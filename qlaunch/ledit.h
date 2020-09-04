/*/
 + Project name : [Q] Launch
 + File name    : ledit.h
 + Description  : однострочный текстовый редактор
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#ifndef __ledit_h
#define __ledit_h
#include <windows.h>

#define LE_STYLE (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP | WS_BORDER)
#define LE_EX_STYLE (WS_EX_CLIENTEDGE)
#define LE_CLASS "LEditor"
#define KeyPressed(key) (bool)(GetAsyncKeyState(key)!=0)

ATOM RegisterEditor(HINSTANCE hApplication);
HWND CreateLEdit(HWND parent,int x,int y,int w,int h);
void DoneEdit();

#endif
