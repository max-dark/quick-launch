/*/
 + Project name : [Q] Launch
 + File name    : main.cpp
 + Description  : основная программа
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#include "main.h"
#include "editdlg.h"
#include "utils.h"
#include "settings.h"
#include "tooltip.h"
#include "xbutton.h"
#include "..\\shared\\shared.h"
#include "resource.h"


void DoExit(HWND hwnd);

HINSTANCE hApplication = NULL;

namespace Main {

lpsettings set = NULL;
char buff[MAX_PATH + 1];
int id = -1;
bool arun = false;
const int r1 = 30;
const int d1 = 30;
const int d2 = 5;
const int r2 = r1 + d1 + d2;
const int mr = r2 + d1 + d2;

HMENU hmenu = NULL;
HMENU xbmenu = NULL;
HWND ttip = NULL;
HWND edlg = NULL;
HFONT fnt = NULL;
WNDCLASS wc;
colors clr;
LRESULT CALLBACK MainProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;
	char buff[MAX_PATH];
	lpxbutton lpx = NULL;
	HICON icon = NULL;
	RECT rc;
	long len;

	switch (umsg) {
		case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc=BeginPaint(hwnd,&ps);
				HBRUSH bg=CreateSolidBrush(clr.bg);
				RECT rc;
				GetClientRect(hwnd,&rc);
				FillRect(hdc,&rc,bg);
				DeleteObject(bg);
				EndPaint(hwnd,&ps);
			}break;
		case WM_DROPFILES: {
				lpx = (lpxbutton)lparam;
				id = lpx->id;
				HDROP drop = (HDROP)wparam;
				int cnt = DragQueryFile(drop, 0xFFFF, (LPSTR)NULL, 0);
				if (cnt > 1) return -1;
				buff[0] = 0;
				DragQueryFile(drop, 0, buff, MAX_PATH);
				if (set [id].command) {
					if (lstrcmpi(set [id].command, buff) == 0) return -2;
				}
				lpx->icon = GetModuleIcon(buff);
				set[id].icon=lpx->icon;
				if (set [id].command) delete [](set [id].command);
				len = lstrlen(buff) + 1;
				set [id].command = new char[len];
				memcpy(set [id].command, buff, len);
				ExtractFileName(set [id].command, &(set [id].tooltip));
				StoreSettings(Main::buff, set , 24, clr);//changed = true;
				InvalidateRect(lpx->hwnd, NULL, true);
				DragFinish(drop);
				id = -1;
			} break;
		case WM_SHOWWINDOW: {
				if (wparam == 0) {
					ShowWindow(ttip, SW_HIDE);
					ShowWindow(edlg, SW_HIDE);
				}
			} break;
		case WM_NCHITTEST: result = HTCAPTION;break;
		case XBM_TOOLTIP: {
				lptooltip_msg lptm = (lptooltip_msg)lparam;
				if (lptm == NULL) return -1;
				if (lptm->show) {
					RECT rc;
					SetWindowText(ttip, set [lptm->id].tooltip);
					GetClientRect(ttip, &rc);
					MoveWindow(ttip, lptm->x + 10, lptm->y - 10 - rc.bottom,
						rc.right, rc.bottom, true);
					ShowWindow(ttip, SW_SHOWNOACTIVATE);
				}
				else {
					ShowWindow(ttip, SW_HIDE);
				}
			} break;
		case XBM_CLICK: {
				id = wparam;
				if (set [id].command) {
					if(autohide)
						ShowWindow(hwnd, SW_HIDE);
					Launch(set [id].command, set [id].workpath);
				}
				else {
					MessageBox(hwnd, "You mast define action for this button.",
									"XButton Clicked!",
									MB_ICONINFORMATION);
				}
			} break;
		case EDLG_RETURN: {
				if (lparam == 0) return -1;
				StoreSettings(Main::buff, set , 24, clr);
				result = 1;
			} break;
		case XBM_SETPARAM: {
				id = wparam;
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hwnd);
				int flag = MF_BYPOSITION | (
					set [id].command != NULL ? MF_ENABLED : MF_GRAYED
				);
				int cnt=GetMenuItemCount(xbmenu);
				for (int i = 1;i < cnt;i++)
					EnableMenuItem(xbmenu, i, flag);
				TrackPopupMenu(xbmenu, 0, pt.x, pt.y, 0, hwnd, NULL);
				result = true;
			} break;
		case WM_NCMBUTTONUP:
		case WM_MBUTTONUP: {
				if (!IsWindowVisible(hwnd)) // if window is not visible ...
				{
					GetClientRect(hwnd, &rc);
					// Set window center to cursor position
					MoveWindow( hwnd, LOWORD(lparam) - rc.right / 2,
								HIWORD(lparam) - rc.bottom / 2,
								rc.right, rc.bottom, true);
					// And show it
					ShowWindow(hwnd, SW_SHOW);
					SetForegroundWindow(hwnd);
				} else  {
					if (autohide)
						ShowWindow(hwnd, SW_HIDE); // hide window
				}
			} break;
		case WM_COMMAND: { // Process popup menu commands
				switch (wparam) {
					case ID_MAIN_SHOWHIDE: {
							if (IsWindowVisible(hwnd)) {
								ShowWindow(hwnd, SW_HIDE);
							} else {
								ShowWindow(hwnd, SW_SHOW);
							}
						}
						break;
					case ID_MAIN_ABOUT: {
							GetModulePath(hApplication, buff, MAX_PATH);
							lstrcat(buff, "readme.txt");
							Launch(buff);
						}
						break;
					case ID_MAIN_AUTORUN: {
							arun = !arun;
							AutoRun(arun);
							CheckMenuItem(hmenu, ID_MAIN_AUTORUN,
										  MF_BYCOMMAND | (arun ? MF_CHECKED : MF_UNCHECKED));
						}
						break;
					case ID_MAIN_AUTOHIDE: {
							autohide = !autohide;
							CheckMenuItem(hmenu, ID_MAIN_AUTOHIDE,
										  MF_BYCOMMAND | (autohide == false ? MF_CHECKED : MF_UNCHECKED));
							StoreSettings(Main::buff,set,24,clr);
						}
						break;
					case ID_MAIN_COLORS:{
						if(SelectColor(hwnd,clr.bg)) {
							InvalidateRect(hwnd,NULL,true);
						}
						}break;
					case ID_MAIN_EXIT: DoExit(hwnd); break;
					case ID_XBUTTON_SELECT: {
						if (id < 0) return 0;
						lpx = (lpxbutton)GetWindowLong(set [id].hwnd, GWL_USERDATA);
						buff[0] = 0;
						if (OpenModule(hwnd, buff, MAX_PATH - 1)) {
							if (set [id].command) {
								if (lstrcmpi(set [id].command, buff) == 0) return false;
							}
							icon = GetModuleIcon(buff);
							lpx->icon = icon;
							set[id].icon=icon;
							if (set [id].command) delete [](set [id].command);
							len = lstrlen(buff) + 1;
							set [id].command = new char[len];
							memcpy(set [id].command, buff, len);
							ExtractFileName(set [id].command, &(set [id].tooltip));
							StoreSettings(Main::buff, set , 24, clr);//changed = true;
							InvalidateRect(lpx->hwnd, NULL, true);
						}
						id = -1;
						}
						break;
					case ID_XBUTTON_WORKPATH: {
						if (id < 0) return 0;
						buff[0] = 0;
						if (ChooseFolder(hwnd, set [id].tooltip, buff)) {
							if (set [id].workpath != NULL) delete [](set [id].workpath);
							int len = lstrlen(buff);
							set [id].workpath = new char[len + 1];
							memcpy(set [id].workpath, buff, len + 1);
							StoreSettings(Main::buff, set , 24, clr);//changed = true;
						}
						id = -1;
						} break;
					case ID_XBUTTON_CLEAR: {
						if (id < 0) return 0;
						if (set [id].command==NULL) return 0;
						lpx = (lpxbutton)GetWindowLong(set [id].hwnd, GWL_USERDATA);
						if (Ask(hwnd, "Clear button?")) {
							lpx->icon = NULL;
							set[id].icon=NULL;
							if (set [id].command ) {
								delete [](set [id].command);
								set [id].command=NULL;
							}
							if (set [id].iconfile) {
								delete [](set [id].iconfile);
								set [id].iconfile=NULL;
							}
							if (set [id].tooltip ) {
								delete [](set [id].tooltip);
								set[id].tooltip = new char[lstrlen(defttip) + 1];
								CopyMemory(set[id].tooltip,
									defttip, lstrlen(defttip) + 1);
							}
							if (set [id].workpath) {
								delete [](set [id].workpath);
								set [id].workpath=NULL;
							}
							StoreSettings(Main::buff, set , 24, clr);//changed = true;
							InvalidateRect(lpx->hwnd, NULL, true);
						}
						id = -1;
						} break;
					case ID_XBUTTON_ICON: break;
					case ID_XBUTTON_TOOLTIP: {
							ExecuteEditDlg(edlg, &set [id]);
						}
						break;
				}
			} break;
		case WM_NCRBUTTONUP: { // Show popup menu
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hwnd);
				TrackPopupMenu(hmenu, 0, pt.x, pt.y, 0, hwnd, NULL);
			} break;
		case WM_SYSTRAYICON: { // Process tray icon messages
				switch (lparam) {
					case WM_RBUTTONUP: {
							POINT pt;
							GetCursorPos(&pt);
							SetForegroundWindow(hwnd);
							TrackPopupMenu(hmenu, 0, pt.x, pt.y, 0, hwnd, NULL);
						} break;
					case WM_LBUTTONUP: {
							MainProc(hwnd, WM_COMMAND, ID_MAIN_SHOWHIDE, 0);
						} break;
				}
			} break;
		case WM_CREATE: {
				LOGFONT lf;
				ZeroMemory(&lf, sizeof(LOGFONT));
				lf.lfHeight = -12;
				lf.lfCharSet = DEFAULT_CHARSET;
				lstrcpy(lf.lfFaceName, "Comic Sans MS"); //Courier New
				fnt = CreateFontIndirect(&lf);
				ttip = CreateTooltip(hwnd, fnt, clr.bg, RGB(0, 0, 0));
				edlg = CreateEditDlg(hwnd);
				if (visible) ShowWindow(hwnd,SW_SHOW);
			} break;
		case WM_CLOSE: DoExit(hwnd); break;
		case WM_DESTROY: {
				DelTrayIcon();
				DeleteObject(fnt);
				for (id = 0;id < 24;id++) {
					DestroyWindow(set[id].hwnd);
				}
				PostQuitMessage(0);
			} break;
		default: result = DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	return result;
}

}// namespase Main

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nShowCmd) {
	using namespace Main;
	MSG msg;
	HWND hwnd;
	RECT rc;

	hApplication = hInstance;
	hwnd = FindWindow(ql_class, title);
	if (IsWindow(hwnd)) {
		ShowWindow(hwnd, SW_SHOW);
		return -0x13;
	}
	if (!RegisterXButton(hInstance)) return -1;
	RegisterEditDlg(hInstance);
	RegisterToolTips(hInstance);
	InitSettings(set , 24, clr);
	if (!set ) return -0x10;
	GetModulePath(hInstance, buff, MAX_PATH);
	if (buff[0] == 0) {
		ClearSettings(set , 24, clr);
		return -0x11;
	}
	lstrcat(buff, "settings.dat");
	if (FileExists(buff)) {
		LoadSettings(buff, set , 24, clr);
	}
	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.hbrBackground = CreateSolidBrush(clr.bg); //0x00,0x88,0xCC
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	wc.lpfnWndProc = MainProc;
	wc.lpszClassName = ql_class;
	if (OSIsWinowsXP()) { wc.style |= 0x20000; /* Add shadow style*/ }
	if (!RegisterClass(&wc)) return -2;
	DWORD exstyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
	hwnd = CreateWindowEx(exstyle,
						  wc.lpszClassName, title,
						  WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
						  position.x, position.y, mr * 2, mr * 2,
						  NULL, NULL, hInstance, NULL);
	if (!hwnd) return -3;
	GetClientRect(hwnd, &rc);
	HRGN r = CreateEllipticRgnIndirect(&rc);
	SetWindowRgn(hwnd, r, true);
	DeleteObject(r);
	double alp = 0.0;
	int i, id = 0;
	// Create buttons
	for (i = 0;i < 8;i++) {
		if (set [id].command != NULL) {
			set [id].icon = GetModuleIcon(set [id].command);
		}
		set [id].hwnd = CreateXButton(hwnd, id, set [id].icon, mr, mr,
							alp + PI / 72, PI / 4 - PI / 36, r1, r1 + d1);
		set [id].id = id;

		id++;

		if (set [id].command != NULL) {
			set [id].icon = GetModuleIcon(set [id].command);
		}
		set [id].hwnd = CreateXButton(hwnd, id, set [id].icon, mr, mr,
							alp + PI / 96, PI / 8 - PI / 48, r2, r2 + d1);
		set [id].id = id;

		alp += PI / 8;id++;

		if (set [id].command != NULL) {
			set [id].icon = GetModuleIcon(set [id].command);
		}
		set [id].hwnd = CreateXButton(hwnd, id, set [id].icon, mr, mr,
							alp + PI / 96, PI / 8 - PI / 48, r2, r2 + d1);
		set [id].id = id;

		alp += PI / 8;id++;
	}
	//hwnd=parent;
	HMENU hm1 = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
	hmenu = GetSubMenu(hm1, 0);

	arun = AutoRun();
	if (arun)
		CheckMenuItem(hmenu, ID_MAIN_AUTORUN, MF_BYCOMMAND | MF_CHECKED);
	if (autohide == false) {
		CheckMenuItem(hmenu, ID_MAIN_AUTOHIDE, MF_BYCOMMAND | MF_CHECKED);
	}

	HMENU hm2 = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_XBMENU));
	xbmenu = GetSubMenu(hm2, 0);

	TrayIcon(hwnd, wc.hIcon, title);
	Hook(hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//if (changed)
		StoreSettings(Main::buff, set , 24, clr);
	//
	DestroyMenu(hm1);
	DestroyMenu(hm2);
	ClearSettings(set , 24, clr);
	UnRegisterXButton();
	UnregisterClass(wc.lpszClassName, hInstance);
	UnregisterClass(ToolTipClass, hInstance);
	DeleteObject(wc.hbrBackground);
	DeleteObject(wc.hIcon);
	UnHook();
	return msg.wParam;
}

void DoExit(HWND hwnd) {

	RECT rw;
	GetWindowRect(hwnd,&rw);
	position.x=rw.left;
	position.y=rw.top;
	visible=(IsWindowVisible(hwnd)!=0);
	DestroyWindow(hwnd);
}
