#include <windows.h>
#include "..\\shared\\shared.h"
#pragma comment(linker,"/entry:DllMain")

#define XHOOK_API __declspec(dllexport)

#ifndef UINT_MAX
#define UINT_MAX 0xffffffff
#endif

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES 104
#endif

#ifndef WHEEL_PAGESCROLL
#define WHEEL_PAGESCROLL (UINT_MAX)
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

extern "C"
{
	XHOOK_API void Release(HINSTANCE xmod);
	//XHOOK_API LRESULT CALLBACK HookProc(int nCode,WPARAM wParam,LPARAM lParam);
	XHOOK_API void Init(HINSTANCE xmod);
};

HINSTANCE xmod;
HHOOK xhook;


void OutError()
{
	LPVOID lpMsgBuf;
    DWORD err=GetLastError();
    //if(!err)return;
	FormatMessage(
   		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
   		NULL,
   		err,
   		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
   		(LPTSTR) &lpMsgBuf,
   		0,NULL);
	MessageBox( NULL,(LPTSTR)lpMsgBuf, "Error", MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
	SetLastError(0);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  if(fdwReason==DLL_PROCESS_ATTACH)
  {
    xmod=hinstDLL;
    xhook=0;
    if(!xmod)OutError();
  }
  return TRUE;
}

XHOOK_API void Release(HINSTANCE xmod)
{
  UnhookWindowsHookEx(xhook);
}
#ifdef hwnd_to_file
HWND GetMainWindow()
{
	const char tmpset[]="c:\\tmpset.qls";
	HWND main=NULL;
	HANDLE f=CreateFile(tmpset,GENERIC_READ,0,NULL,
						OPEN_EXISTING,0,NULL);
	if(f)
	{
		DWORD sr;
		ReadFile(f,&main,sizeof(HWND),&sr,NULL);
		CloseHandle(f);
	}
	return main;
}
#else
#define GetMainWindow() FindWindow(ql_class,title)
#endif

#ifdef _WHEELTOSCROLL
void WheelToScroll(PMSG msg)
{
	short zDelta;
	UINT wlines;
	WORD umsg;

	if(!SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,&wlines,0)) wlines=3;
	zDelta = (short) HIWORD(msg->wParam);
	msg->message=WM_VSCROLL;
	if(wlines==WHEEL_PAGESCROLL)
	{
		if(zDelta>0)
		{
			umsg=SB_PAGEUP;
		}
		else
		{
			umsg=SB_PAGEDOWN;
		}
	}
	else
	{
		if(zDelta>0)
		{
			umsg=SB_LINEUP;
		}
		else
		{
			umsg=SB_LINEDOWN;
		}
	}
	msg->wParam=MAKEWPARAM(umsg,0);
	msg->lParam=NULL;
}
#else
#define WheelToScroll(msg) do{;}while(0)
#endif
LRESULT CALLBACK HookProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	short zDelta;
	UINT wlines;
	WORD umsg;
	HWND main=NULL;
	if(nCode==HC_ACTION)
	{
	PMSG msg=(PMSG)lParam;
	POINT pt;
	switch(msg->message)
	{
	  case WM_MBUTTONDOWN:
	  case WM_NCMBUTTONDOWN:
		msg->message=WM_NULL;
		return 0;
	  case WM_MBUTTONUP:
	  case WM_NCMBUTTONUP:
		GetCursorPos(&pt);
		main=GetMainWindow();
		if(IsWindow(main))
		{
			SendMessage(main,msg->message,0,MAKELPARAM(pt.x,pt.y));
			msg->message=WM_NULL;
		}
		return 0;
	  case WM_MOUSEWHEEL:
		if(InSendMessage()) return 0;
		if(GetAsyncKeyState(VK_MENU)!=0) return 0;
		umsg=LOWORD(msg->wParam);
		if(umsg==MK_CONTROL)
		{
		  if(!SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,&wlines,0)) wlines=3;
		  zDelta = (short) HIWORD(msg->wParam);
		  msg->message=WM_HSCROLL;
		  if(wlines==WHEEL_PAGESCROLL)
		  {
			if(zDelta>0)
			{
			  umsg=SB_PAGEUP;
			}
			else
			{
			  umsg=SB_PAGEDOWN;
			}
		  }
		  else
		  {
			if(zDelta>0)
			{
			  umsg=SB_LINEUP;
			}
			else
			{
			  umsg=SB_LINEDOWN;
			}
		  }
		  msg->wParam=MAKEWPARAM(umsg,0);
		  msg->lParam=0;//NULL;
		}
		else if(umsg!=0)
		{
		  return 0;
		}
		else WheelToScroll(msg);

		POINT pt;
		GetCursorPos(&pt);
		main=WindowFromPoint(pt);
		if(main!=msg->hwnd)
		{
		  SetForegroundWindow(main);
		  SetActiveWindow(main);
		  SendMessage(main,msg->message,msg->wParam,msg->lParam);
		  msg->message=WM_NULL;
		}
		return 0;
	  default: break;
	}
	}
	return CallNextHookEx(xhook,nCode,wParam,lParam);
}

XHOOK_API void Init(HINSTANCE xmod)
{
  xhook=SetWindowsHookEx(WH_GETMESSAGE,HookProc,xmod,0);
  if(!xhook)
  {
  	OutError();
    MessageBeep(IDOK);
  }
}
