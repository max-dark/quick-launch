/*/
 + Project name : [Q] Launch
 + File name    : editdlg.cpp
 + Description  : окно диалога редактирования подсказки
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#include "main.h"
#include "editdlg.h"
#include "utils.h"
#include "ledit.h"
#include "exbutton.h"

ATOM cAtom = (ATOM)NULL;

LRESULT CALLBACK EditDlgProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

WNDCLASS wec;

ATOM RegisterEditDlg(HINSTANCE hApplication) {
	if (cAtom == (ATOM)NULL) {
		RegisterEditor(hApplication);
		RegisterExButton(hApplication);
		ZeroMemory(&wec, sizeof(WNDCLASS));
		wec.hbrBackground = CreateSolidBrush(RGB(0x66, 0xCC, 0xFF));
		wec.hCursor = LoadCursor(NULL, IDC_ARROW);
		wec.hInstance = hApplication;
		wec.lpfnWndProc = EditDlgProc;
		wec.lpszClassName = "EditDlg_Class";
		if(OSIsWinowsXP()) {
			wec.style=0x20000; // Add shadow style
		}
		cAtom = RegisterClass(&wec);
	}
	return cAtom;
}

HWND CreateEditDlg(HWND parent) {
	return CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
						  wec.lpszClassName, NULL, WS_POPUP,// | DS_SETFONT,
						  0, 0, 230, 80, parent, NULL, hApplication, NULL);
}

int ExecuteEditDlg(HWND edit, lpsettings set) {
	return SendMessage(edit, EDLG_RETURN, 0, (LONG)set);
}
HWND edit, ok, cancel;
lpsettings lps=NULL;
LRESULT CALLBACK EditDlgProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;
	switch (umsg) {
			case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				DrawIconEx(hdc, 15, 24, lps->icon, 32, 32, 0, NULL, DI_NORMAL);
				EndPaint(hwnd, &ps);
			}
			break;
			case WM_SHOWWINDOW:{
					if (wparam==0) {
						lps=NULL;
					}
				}
				break;
			case WM_NCHITTEST: result = HTCAPTION;break;
			case WM_GETTEXTLENGTH: break;
			case WM_GETTEXT: break;
			case EDLG_RETURN: {
				if (lparam == 0) return false;
				lps = (lpsettings)lparam;
				SetWindowText(edit, lps->tooltip);
				POINT p;
				GetCursorPos(&p);
				MoveWindow(hwnd, p.x - 115, p.y - 40, 230, 80, true);
				SetFocus(edit);
				ShowWindow(hwnd, SW_SHOW);
				InvalidateRect(hwnd,NULL,true);
				result = 1;
			}
			break;
			case WM_COMMAND: {
				switch (LOWORD(wparam)) {
						case IDOK: {
							if (lps->tooltip) {
								delete [](lps->tooltip);
								lps->tooltip = NULL;
							}
							int len = GetWindowTextLength(edit);
							if (len > 0) {
								lps->tooltip = new char[len + 1];
								GetWindowText(edit, lps->tooltip, len + 1);
								lps->tooltip[len] = 0;
							}
							SendMessage(GetParent(hwnd), EDLG_RETURN, 0, (LONG)lps);
							ShowWindow(hwnd, SW_HIDE);
						} break;
						case IDCANCEL: {
							ShowWindow(hwnd, SW_HIDE);
						} break;
				}
			}
			break;
			case WM_CREATE: {
// TODO (max#1#): [ Доработать диалог ]
//				ZeroMemory(&er, sizeof(edlg_return));
//				er.id = -1;
				lps=NULL;
				LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lparam;
				HRGN r = CreateRoundRectRgn(0, 0, lpcs->cx, lpcs->cy, 15, 15);
				SetWindowRgn(hwnd, r, true);
				DeleteObject(r);

				LOGFONT lf;
				ZeroMemory(&lf, sizeof(LOGFONT));
				lf.lfHeight = -12;
				lf.lfCharSet = DEFAULT_CHARSET;
				lf.lfWeight  = FW_ULTRABOLD;
				lstrcpy(lf.lfFaceName, "Comic Sans MS"); //Courier New
				HFONT fnt = CreateFontIndirect(&lf);

				edit = CreateLEdit(hwnd, 55, 15, 160, 23);
				ok = CreateExButton(hwnd, IDOK, "Ok",
							LoadIcon(hApplication, (char*)IDI_OKICON),fnt,
									55, 45, 80, 30);
				cancel = CreateExButton(hwnd, IDCANCEL, "Отмена",
							LoadIcon(hApplication, (char*)IDI_CANCELICON),fnt,
								135, 45, 80, 30);
			}
			break;
			case WM_CLOSE: {
				ShowWindow(hwnd, SW_HIDE);
			}
			break;
			default: result = DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	return result;
}
