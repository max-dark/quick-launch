/*/
 + Project name : [Q] Launch
 + File name    : tooltip.cpp
 + Description  : создание вспдавающей подсказки
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#include "main.h"
#include "tooltip.h"
#include "utils.h"
#include <windows.h>

ATOM CAtom = (ATOM)NULL;

LRESULT CALLBACK ToolTipProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

ATOM RegisterToolTips(HINSTANCE hApplication) {
	if (CAtom == (ATOM)NULL) {
		WNDCLASS wc;
		ZeroMemory(&wc, sizeof(WNDCLASS));
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hInstance = hApplication;
		wc.lpfnWndProc = ToolTipProc;
		wc.lpszClassName = ToolTipClass;
		if (OSIsWinowsXP()) {
			wc.style = 0x20000; // Add shadow style
		}
		CAtom = RegisterClass(&wc);
	}
	return CAtom;
}

typedef struct {
	HFONT fnt;
	COLORREF fg, bg;
	char *text;
	HWND hwnd;
} tooltip, *lptooltip;

//typedef struct {
//	HFONT fnt;
//	COLORREF fg,bg;
//} ttset,*lpttset;

HWND CreateTooltip(HWND parent, HFONT fnt, COLORREF bg, COLORREF fg) {
	tooltip t = {fnt, fg, bg, NULL, NULL};
	return CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
						  ToolTipClass, NULL, WS_POPUP | WS_DISABLED,
						  0, 0, 100, 20, parent, NULL, hApplication, &t);
}
int dd = 2;
int dd2 = dd*2;
const int dy = 2;
const int dy2 = dd*2;
void SizeByText(lptooltip lpt) {
	HDC hdc = GetDC(lpt->hwnd);
	TEXTMETRIC tm;
	SelectObject(hdc, lpt->fnt);
	GetTextMetrics(hdc, &tm);
	dd = tm.tmAveCharWidth;
	dd2 = dd * 2;
	long h, w, len;
	SIZE sz;
	len = lstrlen(lpt->text);
	GetTextExtentPoint32(hdc, lpt->text, len, &sz);
	h = sz.cy + dy2;
	w = sz.cx + dd2;
	ReleaseDC(lpt->hwnd, hdc);
	SetWindowPos(lpt->hwnd, NULL,
				 0, 0, w, h,
				 SWP_NOACTIVATE | SWP_NOOWNERZORDER |
				 SWP_NOMOVE | SWP_NOZORDER);
	InvalidateRect(lpt->hwnd, NULL, true);
}

void Paint(lptooltip lpt, HDC hdc) {
	RECT rc;
	GetClientRect(lpt->hwnd, &rc);
	HBRUSH bg = CreateSolidBrush(lpt->bg);
	HBRUSH ob = (HBRUSH)SelectObject(hdc, bg);
	HPEN fg = CreatePen(PS_SOLID, 1, lpt->fg);
	HPEN op = (HPEN)SelectObject(hdc, fg);
	HFONT of = (HFONT)SelectObject(hdc, lpt->fnt);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, lpt->fg);
	//FillRect(hdc,&rc,bg);
	Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
	TextOut(hdc, dd, dy, lpt->text, lstrlen(lpt->text));
	SelectObject(hdc, ob);
	SelectObject(hdc, op);
	SelectObject(hdc, of);
	DeleteObject(bg);
	DeleteObject(fg);
}

LRESULT CALLBACK ToolTipProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	lptooltip lpt = (lptooltip)GetWindowLong(hwnd, GWL_USERDATA);
	if ((lpt == NULL) && (umsg != WM_CREATE))
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	LRESULT result = 0;
	switch (umsg) {
		case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				Paint(lpt, hdc);
				EndPaint(hwnd, &ps);
			}
			break;
		case WM_NCHITTEST: result = HTNOWHERE;break;
		case WM_SETTEXT: {
				if (lpt->text != NULL) {
					delete [](lpt->text);
					lpt->text = NULL;
				}
				LPCTSTR lpsz = (LPCTSTR)lparam;
				long len = lstrlen(lpsz);
				if (len > 0) {
					len++;
					lpt->text = new char[len];
					memcpy(lpt->text, lpsz, len);
				}
				SizeByText(lpt);
			}
			break;
		case WM_GETTEXTLENGTH: break;
		case WM_GETTEXT: break;
		case WM_CREATE: {
				lptooltip set = (lptooltip)(((LPCREATESTRUCT)lparam)->lpCreateParams);
				if (set == NULL) return -1;
				lpt = new tooltip;
				CopyMemory(lpt, set , sizeof(tooltip));
				lpt->hwnd = hwnd;
				lpt->text = NULL;
				SetWindowLong(hwnd, GWL_USERDATA, (LONG)lpt);
			}
			break;
		case WM_DESTROY: {
				SetWindowLong(hwnd, GWL_USERDATA, 0);
				if (lpt->text) delete [](lpt->text);
				delete lpt;
			}
			break;
		default: result = DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	return result;
}
