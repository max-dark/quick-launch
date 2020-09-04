/*/
 + Project name : [Q] Launch
 + File name    : exbutton.cpp
 + Description  : кнопки
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#include "main.h"
#include "exbutton.h"

#define EXB_CLASS "EXButton"
#define EXB_STYLE (WS_CHILD | WS_VISIBLE | \
			WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)
#define IDC_HAND MAKEINTRESOURCE(32649)
#define isz 20
#define scl RGB(0x55,0xAA,0xFF)
#define bcl RGB(0x00,0x88,0xCC)

ATOM EXBClass = 0;

LRESULT CALLBACK ExButtonProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
void Paint(lpexbutton lpx, HDC hdc);

ATOM RegisterExButton(HINSTANCE hApplication) {
	if (!EXBClass) {
		WNDCLASS wc;
		ZeroMemory(&wc, sizeof(WNDCLASS));
		wc.lpszClassName = EXB_CLASS;
		wc.hbrBackground = CreateSolidBrush(bcl);
		wc.hCursor = LoadCursor(NULL, IDC_HAND);
		wc.hIcon = LoadIcon(hApplication, MAKEINTRESOURCE(IDI_MAINICON));//IDI_XBICON
		wc.hInstance = hApplication;
		wc.lpfnWndProc = ExButtonProc;
		//wc.style=CS_SAVEBITS | CS_PARENTDC;
		EXBClass = RegisterClass(&wc);
	}
	return EXBClass;
}

HWND CreateExButton(HWND parent,WPARAM id,
					char*title,HICON icon,HFONT fnt,
					int x,int y,int w,int h) {
	HWND hbtn = 0;
	lpexbutton lpx = new exbutton;
	if (lpx) {
		ZeroMemory(lpx, sizeof(exbutton));
		lpx->w = w;
		lpx->h = h;
		lpx->id = id;
		lpx->fnt=fnt;
		lpx->icon = icon;
		int len = lstrlen(title);
		lpx->title = new char[len + 1];
		CopyMemory(lpx->title, title, len);
		lpx->title[len] = 0;
		hbtn = CreateWindowEx(0, EXB_CLASS, title, EXB_STYLE,
							  x, y, w, h,
							  parent, NULL, hApplication, lpx);
	}
	return hbtn;
}

void UnRegisterExButton() {
	UnregisterClass(EXB_CLASS, hApplication);
	//DeleteObject(wc.hbrBackground);
	//DestroyIcon(wc.hIcon);
}

LRESULT CALLBACK ExButtonProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;
	PAINTSTRUCT ps;
	POINT pt;
	HDC hdc;

	lpexbutton lpx = (lpexbutton)GetWindowLong(hwnd, GWL_USERDATA);
	if ((lpx == NULL) && (umsg != WM_CREATE))
		return DefWindowProc(hwnd, umsg, wparam, lparam);

	switch (umsg) {
		case WM_PAINT: {
				hdc = BeginPaint(hwnd, &ps);
				Paint(lpx, hdc);
				EndPaint(hwnd, &ps);
			}
			break;
		case WM_MOUSEMOVE: {
				pt.x = LOWORD(lparam);
				pt.y = HIWORD(lparam);
				RECT rc;
				GetClientRect(hwnd, &rc);
				if (PtInRect(&rc, pt)) {
					if (!lpx->down) SetCapture(lpx->hwnd);
					if (!lpx->sel) {
						lpx->sel = true;
						hdc = GetDC(lpx->hwnd);
						Paint(lpx, hdc);
						ReleaseDC(lpx->hwnd, hdc);
					}
				}
				else {
					if (lpx->sel) {
						lpx->sel = false;
						hdc = GetDC(lpx->hwnd);
						Paint(lpx, hdc);
						ReleaseDC(lpx->hwnd, hdc);
					}
					if (!lpx->down) ReleaseCapture();
				}
			}
			break;
		case WM_LBUTTONDOWN: {
				SetCapture(hwnd);
				lpx->down = 1;
				hdc = GetDC(lpx->hwnd);
				Paint(lpx, hdc);
				ReleaseDC(lpx->hwnd, hdc);
			}
			break;
		case WM_LBUTTONUP: {
				if (lpx->down && lpx->sel) {
					{
						hwnd = GetParent(lpx->hwnd);
						ReleaseCapture();
						SendMessage(hwnd, WM_COMMAND, lpx->id, 0);
					}
				}
				else {
					ReleaseCapture();
				}
			}
			break;
		case WM_CAPTURECHANGED: {
				if (((HWND)lparam) != hwnd) {
					lpx->sel = false;
					lpx->down = false;
					hdc = GetDC(lpx->hwnd);
					Paint(lpx, hdc);
					ReleaseDC(lpx->hwnd, hdc);
				}
			}
			break;
		case WM_CREATE: {
				lpx = (lpexbutton)(((LPCREATESTRUCT)lparam)->lpCreateParams);
				if (lpx == NULL) {
					result = -1;
				}
				else {
					lpx->hwnd = hwnd;
					lpx->bg = CreateSolidBrush(bcl);
					lpx->sg = CreateSolidBrush(scl);
					SetWindowLong(hwnd, GWL_USERDATA, (long)lpx);
					SetWindowLong(hwnd, GWL_ID, (long)lpx->id);
				}
			}
			break;
		case WM_DESTROY: {
				SetWindowLong(hwnd, GWL_USERDATA, 0L);
				if (lpx) {
					if (lpx->title) {
						delete [](lpx->title);
					}
					DestroyIcon(lpx->icon);
					DeleteObject(lpx->bg);
					DeleteObject(lpx->sg);
					delete lpx;
				}
			}
			break;
		default: result = DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	return result;
}

void Paint(lpexbutton lpx, HDC hdc) {
	RECT rc;
	GetClientRect(lpx->hwnd, &rc);
	int x = 5;
	int y = (lpx->h - isz) / 2;
	int sz= isz;
	HPEN p=NULL,op=NULL;
	FillRect(hdc, &rc,
			 (HBRUSH)GetClassLong(GetParent(lpx->hwnd), GCL_HBRBACKGROUND));
	if (lpx->sel) {
		SelectObject(hdc, lpx->sg);
		p=CreatePen(PS_SOLID,2,bcl);
		if (lpx->down) {
			x -= 3; y -= 3,
			sz+=6;
		}
	}
	else {
		SelectObject(hdc, lpx->bg);
		p=CreatePen(PS_SOLID,2,scl);
	}
	op=(HPEN)SelectObject(hdc,p);
	int r = lpx->h / 2;
	RoundRect(hdc, 1, 1, lpx->w-2, lpx->h-2, r, r);
	DrawIconEx(hdc, x, y, lpx->icon, sz, sz, 0, NULL, DI_NORMAL);
	rc.left=x+sz;
	rc.right-=2;
	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,0xffffff);
	SelectObject(hdc,lpx->fnt);
	DrawText(hdc,lpx->title,lstrlen(lpx->title),&rc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	SelectObject(hdc,op);
	DeleteObject(p);
}
