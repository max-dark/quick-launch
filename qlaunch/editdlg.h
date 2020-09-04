/*/
 + Project name : [Q] Launch
 + File name    : editdlg.h
 + Description  : окно диалога редактирования подсказки
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#ifndef __editdlg_h
#define __editdlg_h
#include <windows.h>
#include "settings.h"

#define EDLG_RETURN WM_USER+14

//typedef struct {
//	char*str;
//	int len;
//	int id;
//	HICON i;
//}
//edlg_return, *lpedlg_return;

ATOM RegisterEditDlg(HINSTANCE hApplication);
HWND CreateEditDlg(HWND parent);
int ExecuteEditDlg(HWND edit,lpsettings set);
#endif
