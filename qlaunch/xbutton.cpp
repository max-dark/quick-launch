#include "main.h"
#include "xbutton.h"
#include <math.h>
/*/
 + Project name : [Q] Launch
 + File name    : xbutton.cpp
 + Description  : Создание кнопок для главного окна
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/

#define XB_CLASS "XButton"
#define XB_STYLE (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)
#define IDC_HAND MAKEINTRESOURCE(32649)
#define isz 20
#define scl RGB(0x55,0xAA,0xFF)
//(0xB5,0xB2,0xDE)
#define bcl RGB(0x00,0x88,0xCC)
//0x33,0x66,0xCC

ATOM XBClass = 0;
WNDCLASS wc;

LRESULT CALLBACK XButtonProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
HRGN CreateXRegion(lpxbutton lpx);
void Paint(lpxbutton lpx, HDC hdc);
void Calc(int rad, double angle, int xc, int yc, long& x, long& y);
bool PointInButton(lpxbutton lpx, POINT pt);

ATOM RegisterXButton(HINSTANCE hApplication) {
	if (!XBClass) {
		ZeroMemory(&wc, sizeof(WNDCLASS));
		wc.lpszClassName = XB_CLASS;
		wc.hbrBackground = CreateSolidBrush(bcl);
		wc.hCursor = LoadCursor(NULL, IDC_HAND);
		wc.hIcon = LoadIcon(hApplication, MAKEINTRESOURCE(IDI_MAINICON));//IDI_XBICON
		wc.hInstance = hApplication;
		wc.lpfnWndProc = XButtonProc;
		//wc.style = CS_SAVEBITS | CS_PARENTDC;
		//if(OSIsWinowsXP())
		//{
		//	wc.style|=0x20000; // Add shadow style
		//}
		XBClass = RegisterClass(&wc);
	}
	return XBClass;
}

HWND CreateXButton(HWND parent, WPARAM id, HICON icon, int xc, int yc,
				   double alpha, double beta, int r1, int r2) {
	HWND hbtn = 0;
	int width, height;
	int y1,y2,x1,x2;
	POINT pt[4];
	lpxbutton lpx = new xbutton;
	if (lpx) {
		ZeroMemory(lpx, sizeof(xbutton));
		lpx->alpha = alpha;
		lpx->beta = alpha + beta;
		lpx->r1 = r1;
		lpx->r2 = r2;
		lpx->id = id;
		lpx->icon = icon;
		lpx->xc = xc;
		lpx->yc = yc;

		Calc( lpx->r1 , lpx->alpha, xc, yc, pt[0].x, pt[0].y);
		Calc( lpx->r1 , lpx->beta , xc, yc, pt[1].x, pt[1].y);
		Calc( lpx->r2 , lpx->beta , xc, yc, pt[2].x, pt[2].y);
		Calc( lpx->r2 , lpx->alpha, xc, yc, pt[3].x, pt[3].y);

		y1=y2=pt[0].y;
		x1=x2=pt[0].x;
		for (int i=1;i<4;i++) {
			if(pt[i].x<x1) x1 = pt[i].x;
			if(pt[i].y<y1) y1 = pt[i].y;
			if(pt[i].x>x2) x2 = pt[i].x;
			if(pt[i].y>y2) y2 = pt[i].y;
		}
		width = x2-x1;
		height= y2-y1;
		lpx->xc-=x1;
		lpx->yc-=y1;

		hbtn = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_ACCEPTFILES, XB_CLASS, NULL, XB_STYLE,
							  x1, y1, width, height,
							  parent, NULL, hApplication, lpx);
	}
	return hbtn;
}

void UnRegisterXButton() {
	UnregisterClass(XB_CLASS, hApplication);
	DeleteObject(wc.hbrBackground);
	DestroyIcon(wc.hIcon);
}

LRESULT CALLBACK XButtonProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;
	PAINTSTRUCT ps;
	POINT pt;
	HDC hdc;

	lpxbutton lpx = (lpxbutton)GetWindowLong(hwnd, GWL_USERDATA);
	if ((lpx == NULL) && (umsg != WM_CREATE)) return DefWindowProc(hwnd, umsg, wparam, lparam);
	tooltip_msg ttm;
	ZeroMemory(&ttm, sizeof(tooltip_msg));

	switch (umsg) {
		case WM_DROPFILES: {
				result = SendMessage(GetParent(hwnd), umsg, wparam, (LPARAM)lpx);
			}
			break;
		case WM_PAINT: {
				hdc = BeginPaint(hwnd, &ps);
				Paint(lpx, hdc);
				EndPaint(hwnd, &ps);
			}
			break;
		case WM_MOUSEMOVE: {
				pt.x = LOWORD(lparam);
				pt.y = HIWORD(lparam);
				ttm.id = lpx->id;
				if (PointInButton(lpx, pt)) {
					if (!lpx->down) SetCapture(lpx->hwnd);
					if (!lpx->sel) {
						lpx->sel = true;
						hdc = GetDC(lpx->hwnd);
						Paint(lpx, hdc);
						ReleaseDC(lpx->hwnd, hdc);
						pt = lpx->pt;
						ClientToScreen(hwnd, &pt);
						ttm.x = pt.x;ttm.y = pt.y;
						ttm.show = true;
						SendMessage(GetParent(hwnd), XBM_TOOLTIP, 0, (LPARAM)&ttm);
					}
				} else {
					if (lpx->sel) {
						lpx->sel = false;
						hdc = GetDC(lpx->hwnd);
						Paint(lpx, hdc);
						ReleaseDC(lpx->hwnd, hdc);
						SendMessage(GetParent(hwnd), XBM_TOOLTIP, 0, (LPARAM)&ttm);
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
					hwnd = GetParent(lpx->hwnd);
					ReleaseCapture();
					SendMessage(hwnd, XBM_CLICK, lpx->id, (LPARAM)hwnd);
				} else {
					ReleaseCapture();
				}
			}
			break;
		case WM_RBUTTONDOWN: {
				SetCapture(hwnd);
				lpx->down = 2;
				hdc = GetDC(lpx->hwnd);
				Paint(lpx, hdc);
				ReleaseDC(lpx->hwnd, hdc);
			}
			break;
		case WM_RBUTTONUP: {
				if (lpx->sel && (lpx->down == 2)) {
					hwnd = GetParent(lpx->hwnd);
					ReleaseCapture();
					if (SendMessage(hwnd, XBM_SETPARAM, lpx->id, (LPARAM)lpx)) {
						hdc = GetDC(lpx->hwnd);
						Paint(lpx, hdc);
						ReleaseDC(lpx->hwnd, hdc);
					}
				} else {
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
					ttm.id = lpx->id;
					SendMessage(GetParent(hwnd), XBM_TOOLTIP, 0, (LPARAM)&ttm);
				}
			}
			break;
		case WM_CREATE: {
				lpx = (lpxbutton)(((LPCREATESTRUCT)lparam)->lpCreateParams);
				if (lpx == NULL) {
					result = -1;
				} else {
					lpx->hwnd = hwnd;
					HRGN rgn = CreateXRegion(lpx);
					lpx->bg = CreateSolidBrush(bcl);
					lpx->sg = CreateSolidBrush(scl);
					//if (!lpx->icon)lpx->icon = (HICON)GetClassLong(lpx->hwnd, GCL_HICON);
					SetWindowLong(hwnd, GWL_USERDATA, (long)lpx);
					SetWindowLong(hwnd, GWL_ID, (long)lpx->id);
					SetWindowRgn(hwnd, rgn, true);
					DeleteObject(rgn);
				}
			}
			break;
		case WM_DESTROY: {
				SetWindowLong(hwnd, GWL_USERDATA, 0L);
				if (lpx) {
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

HRGN CreateXRegion(lpxbutton lpx) {
	HRGN r1 = NULL, r2 = NULL;
	int r = 3 * lpx->r2 / 2;
	POINT pt[4];

	r1 = CreateEllipticRgn(lpx->xc - lpx->r1, lpx->yc - lpx->r1, lpx->xc + lpx->r1, lpx->yc + lpx->r1);
	r2 = CreateEllipticRgn(lpx->xc - lpx->r2, lpx->yc - lpx->r2, lpx->xc + lpx->r2, lpx->yc + lpx->r2);
	CombineRgn(r1, r2, r1, RGN_DIFF);
	DeleteObject(r2);
	Calc( lpx->r1 , lpx->alpha, lpx->xc, lpx->yc, pt[0].x, pt[0].y);
	Calc( lpx->r1 , lpx->beta , lpx->xc, lpx->yc, pt[1].x, pt[1].y);
	Calc(r, lpx->beta , lpx->xc, lpx->yc, pt[2].x, pt[2].y);
	Calc(r, lpx->alpha, lpx->xc, lpx->yc, pt[3].x, pt[3].y);
	r2 = CreatePolygonRgn(pt, 4, WINDING);
	CombineRgn(r1, r1, r2, RGN_AND);
	DeleteObject(r2);

	Calc(lpx->r1 + (lpx->r2 - lpx->r1) / 2, lpx->alpha + (lpx->beta - lpx->alpha) / 2.0,
		 lpx->xc, lpx->yc, lpx->pt.x, lpx->pt.y);
	lpx->pt.x -= isz / 2;
	lpx->pt.y -= isz / 2;
	return r1;
}

void Paint(lpxbutton lpx, HDC hdc) {
	RECT rc;
	GetClientRect(lpx->hwnd, &rc);
	int x=lpx->pt.x;
	int y=lpx->pt.y;
	int sz=isz;
	if (lpx->sel) {
		FillRect(hdc, &rc, lpx->sg);
		if (lpx->down) {
			x -= 3;
			y -= 3;
			sz += 6;
		}
	}
	else {
		FillRect(hdc, &rc, lpx->bg);
	}
	DrawIconEx(hdc, lpx->pt.x, lpx->pt.y, lpx->icon, sz, sz, 0, NULL, DI_NORMAL);
}

void Calc(int rad, double angle, int xc, int yc, long& x, long& y) {
	//double s,c;
	//SinCos(angle,s,c);
	x = (long)((double)xc + cos(angle) * ((double)rad));
	y = (long)((double)yc - sin(angle) * ((double)rad));
}

bool PointInButton(lpxbutton lpx, POINT pt) {
	pt.x -= lpx->xc;
	pt.y -= lpx->yc;
	long R = pt.x * pt.x + pt.y * pt.y;
	long r1 = lpx->r1 * lpx->r1;
	long r2 = lpx->r2 * lpx->r2;
	if ((R >= r1) && (R <= r2)) {
		pt.y=-pt.y;
		double a = atan2(((double)pt.y) , ((double)pt.x));
		if (pt.y < 0) {
			a += 2 * PI;
		}
		if ((a >= lpx->alpha) && (a <= lpx->beta)) {
			return true;
		}
	}
	return false;
}

void SetXButtonColors(HWND btn,COLORREF fg,COLORREF sg) {
	lpxbutton lpx=(lpxbutton)GetWindowLong(btn,GWL_USERDATA);
	if (lpx!=NULL) {
		DeleteObject(lpx->bg);
		lpx->bg=CreateSolidBrush(fg);
		DeleteObject(lpx->sg);
		lpx->sg=CreateSolidBrush(sg);
		HDC hdc=GetDC(btn);
		Paint(lpx,hdc);
		ReleaseDC(btn,hdc);
	}
}
