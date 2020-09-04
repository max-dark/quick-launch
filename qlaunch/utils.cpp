/*/
 + Project name : [Q] Launch
 + File name    : utils.cpp
 + Description  : вспомогательные функции
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#include "utils.h"
#include "..\\shared\\shared.h"
#include <shellapi.h>
//#include <direct.h>
#include <commdlg.h>
#include <shlobj.h>

#define makename(nameid) (((ULONG)nameid & 0xFFFF0000)?nameid:MAKEINTRESOURCE(nameid))
extern HINSTANCE hApplication;

bool OSIsWinowsXP() {
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&vi) || vi.dwPlatformId != VER_PLATFORM_WIN32_NT ||
			vi.dwMajorVersion < 5 || vi.dwMinorVersion < 1) {
		return false;
	}
	return true;
}

void Launch(char *mod, char *modpath) {
	//char modpath[MAX_PATH];
	int len = strlen(mod);
	if (len > 0) {
		if (modpath == NULL) {
			modpath = new char[len + 2];
			CopyMemory(modpath, mod, len + 1);
			for (long i = len - 1;i >= 0;i--) {
				if (modpath[i] != '\\') {
					modpath[i] = 0;
				}
				else {
					//chdir(modpath);
					break;
				}
			}
			len = -1;
		}
		ShellExecute(0, NULL, mod, NULL, modpath, SW_SHOW);
		if (len < 0) delete []modpath;
	}
}
void OutError() {
	LPVOID lpMsgBuf;
	DWORD err = GetLastError();
	if (!err) return ;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL);
	MessageBox( NULL, (LPTSTR)lpMsgBuf, "Error", MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
	SetLastError(0);
}

HICON GetModuleIcon(char* modpath, int id) {
	if (modpath == NULL) return NULL;
	WORD bt=0;
	return ExtractAssociatedIcon(NULL, modpath, &bt);//(hApplication, modpath, &bt);
}

//BOOL GetIconCountProc(HINSTANCE hmod, LPCTSTR type, LPTSTR name, LONG lparam) {
//	((lpIconList)lparam)->count++;
//	return true;
//}
//BOOL FillIconListProc(HINSTANCE hmod, LPCTSTR type, LPTSTR name, LONG lparam) {
//	lpIconList il = (lpIconList)lparam;
//	il->icons[il->index] = LoadIcon(hmod, name);
//	il->index++;
//	return true;
//}
//
//void GetModuleIconList(char* modpath, lpIconList il) {
//	ZeroMemory(il, sizeof(IconList));
//	DWORD bt;
//	if (GetBinaryType(modpath, &bt)) {
//		if (bt == SCS_32BIT_BINARY) {
//			HINSTANCE hmod;
//			hmod = LoadLibraryEx(modpath, NULL, DONT_RESOLVE_DLL_REFERENCES);
//			if (hmod) {
//				EnumResourceNames(hmod, RT_GROUP_ICON,
//								  (ENUMRESNAMEPROC)GetIconCountProc, (LONG)il);
//				if (il->count > 0) {
//					il->icons = new HICON[il->count];
//					ZeroMemory(il->icons, il->count*sizeof(HICON));
//					EnumResourceNames(hmod, RT_GROUP_ICON,
//									(ENUMRESNAMEPROC)FillIconListProc, (LONG)il);
//				}
//				FreeLibrary(hmod);
//			}
//			else {
//				SetLastError(0);
//			}
//		}
//	}
//}

bool OpenModule(HWND hwnd, char* modpath, long count, char* filter) {
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = modpath;
	ofn.nMaxFile = count;
	ofn.lpstrFilter = (filter == NULL ? "Exe\0*.exe\0All Files\0*.*\0" : filter);
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	return (GetOpenFileName(&ofn) == TRUE);
}

void GetModulePath(HINSTANCE hInstance, char* modpath, long count) {
	modpath[0] = 0;
	DWORD len = GetModuleFileName(hInstance, modpath, count);
	if (len > 0) {
		for (long i = len - 1;i >= 0;i--) {
			if (modpath[i] != '\\') {
				modpath[i] = 0;
			}
			else {
				return ;
			}
		}
	}
}

void ExtractFileName(const char* path, char** name) {
	int len = lstrlen(path);
	int j = len - 1;
	int i = j;
	while (j >= 0) {
		if (path[j] == '\\') break;
		j--;
	}
	j++;
	while (i > j) {
		if (path[i] == '.') break;
		i--;
	}
	if (i<=j) i=len-1;
	len = i - j;
	(*name) = new char[len + 1];
	CopyMemory((*name), &(path[j]), len);
	(*name)[len]=0x0;
}

bool hooked = false;
HMODULE lib = 0;
NOTIFYICONDATA nid;

proc Init = NULL;
proc Release = NULL;

#ifdef hwnd_to_file
const char tmpset[] = "c:\\tmpset.qls";
void StoreHWND(HWND hwnd) {
	HANDLE f = CreateFile(tmpset, GENERIC_WRITE, 0, NULL,
						  CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
	DWORD sw;
	WriteFile(f, &parent, sizeof(HWND), &sw, NULL);
	CloseHandle(f);
}
#define FreeHWND() DeleteFile(tmpset)
#else
#define StoreHWND(xhwnd) do{;}while(0)
#define FreeHWND() do{;}while(0)
#endif

void Hook(HWND parent) {
	char modpath[MAX_PATH];
	if (!hooked) {
		GetModulePath(hApplication, modpath, MAX_PATH);
		lstrcat(modpath, "xcontrols.dll");
		lib = LoadLibrary(modpath);
		if (!lib) goto eExit;
		Init = (proc)GetProcAddress(lib, "Init");
		if (!Init) goto eQuit;
		Release = (proc)GetProcAddress(lib, "Release");
		if (!Release) goto eQuit;
		StoreHWND(parent);
		Init(lib);
		hooked = true;
	}
	return ;
eQuit:
	FreeLibrary(lib);
	Init = NULL;
	Release = NULL;
	hooked = false;
eExit:
	MessageBeep(MB_ICONEXCLAMATION);
	OutError();
}

void UnHook() {
	if (hooked) {
		Release(lib);
		FreeHWND();
		FreeLibrary(lib);
		Init = NULL;
		Release = NULL;
		hooked = false;
	}
}

void TrayIcon(HWND parent, HICON icon, const char* sztip) {
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = parent;
	nid.uID = 1;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_SYSTRAYICON;
	nid.hIcon = icon;
	lstrcpy(nid.szTip, sztip);
	Shell_NotifyIconA(NIM_ADD, &nid);
}
void DelTrayIcon() {
	Shell_NotifyIconA(NIM_DELETE, &nid);
}
bool ChooseFolder(HWND hwnd, LPSTR lpszTitle, LPSTR lpBuffer) {
	BROWSEINFO bi;
	bool res = false;
	LPITEMIDLIST pidlBrowse;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = hwnd;
	bi.pszDisplayName = lpBuffer;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN;
	pidlBrowse = SHBrowseForFolder(&bi);
	if (pidlBrowse != NULL) {
		if (SHGetPathFromIDList(pidlBrowse, lpBuffer))
			res = true;
	}
	return res;
}

bool AutoRun() {
	HKEY run = NULL;
	bool res = false;
	RegOpenKeyEx(HKEY_CURRENT_USER, reg_run, 0, KEY_READ, &run);
	if (run) {
		res = (RegQueryValueEx(run, title,
						0, NULL, NULL, NULL) == ERROR_SUCCESS);
		RegCloseKey(run);
	}
	return res;
}

bool AutoRun(bool set ) {
	HKEY run = NULL;
	bool res = false;
	RegOpenKeyEx(HKEY_CURRENT_USER, reg_run, 0, KEY_WRITE, &run);
	if (run) {
		if (set ) {
			char buff[MAX_PATH];
			DWORD len = GetModuleFileName(hApplication, buff, MAX_PATH);
			res = (RegSetValueEx(run, title,
							0, REG_SZ, (PBYTE)buff, len) == ERROR_SUCCESS);
		}
		else {
			res = (RegDeleteValue(run, title) == ERROR_SUCCESS);
		}
		RegCloseKey(run);
	}
	return res;
}

#define clWhite RGB(0xff,0xff,0xff)

COLORREF CustomColors[16]={
		clWhite,clWhite,clWhite,clWhite,
		clWhite,clWhite,clWhite,clWhite,
		clWhite,clWhite,clWhite,clWhite,
		clWhite,clWhite,clWhite,clWhite
	};

bool SelectColor(HWND parent,COLORREF &clr) {
	CHOOSECOLOR cc;
	ZeroMemory(&cc,sizeof(CHOOSECOLOR));
	cc.lStructSize=sizeof(CHOOSECOLOR);
	cc.hwndOwner=parent;
	cc.Flags=CC_FULLOPEN | CC_ANYCOLOR | CC_RGBINIT;
	cc.rgbResult=clr;
	cc.lpCustColors=CustomColors;
	if(ChooseColor(&cc)) {
		clr=cc.rgbResult;
		return true;
	}
	return false;
}
