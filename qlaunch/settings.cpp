/*/
 + Project name : [Q] Launch
 + File name    : settings.cpp
 + Description  : сохранение/загрузка настроек
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#include "settings.h"
#include "utils.h"
#include <shellapi.h>

//bool changed = false;
bool autohide = true;
bool visible = false;
POINT position={0,0};

typedef void(*lpLoadProc)(FILE* f, lpsettings set , int count,colors &clr);
typedef char verinfo[10];
typedef struct {
	verinfo info;
	lpLoadProc LoadProc;
}
VerToProc;

void LoadV1(FILE* f, lpsettings set , int count,colors &clr);
void LoadV2(FILE* f, lpsettings set , int count,colors &clr);
void LoadV3(FILE* f, lpsettings set , int count,colors &clr);
int LoadFileVersion(FILE* f);
void SaveFileVersion(FILE* f);

const int vercount = 3;
const VerToProc versions[vercount] = {
					{"QLaunchV1", LoadV1},
					{"QLaunchV2", LoadV2},
					{"QLaunchV3", LoadV3}
				};

void InitSettings (lpsettings &set , int count,colors &clr) {
	set = new settings[count];
	ZeroMemory(set , sizeof(settings)*count);
	clr.bg=RGB(0x66, 0xCC, 0xFF);
	clr.fg=RGB(0x55,0xAA,0xFF);
	clr.sg=RGB(0x00,0x88,0xCC);
	/*int len=lstrlen(defttip)+1;
	//for(int i=0;i<count;i++) {
	//	set[i].tooltip=new char[len];
	//	memcpy(set[i].tooltip,defttip,len);
	//}*/
}

void ClearSettings(lpsettings &set , int count,colors &clr) {
	for (int i = 0;i < count;i++) {
		if (set [i].command ) delete [](set [i].command);
		if (set [i].iconfile) delete [](set [i].iconfile);
		if (set [i].tooltip ) delete [](set [i].tooltip);
		if (set [i].workpath) delete [](set [i].workpath);
	}
	delete []set ;
	set = NULL;
}

void StoreSettings(char* file, lpsettings set , int count,colors &clr) {
	FILE* f = fopen(file, "wb");
	long len = 0;
	SaveFileVersion(f);
	fwrite(&count, 1, sizeof(int), f);
	for (int i = 0;i < count;i++) {
		len = lstrlen(set [i].command);
		fwrite(&len, 1, sizeof(long), f);
		if (len > 0) {
			fwrite(set [i].command, 1, len + 1, f);
			len = lstrlen(set [i].workpath);
			fwrite(&len, 1, sizeof(long), f);
			if (len > 0) {
				fwrite(set [i].workpath, 1, len + 1, f);
			}
			len = lstrlen(set [i].iconfile);
			fwrite(&len, 1, sizeof(long), f);
			if (len > 0) {
				fwrite(set [i].iconfile, 1, len + 1, f);
			}
			len = lstrlen(set [i].tooltip);
			fwrite(&len, 1, sizeof(long), f);
			if (len > 0) {
				fwrite(set [i].tooltip, 1, len + 1, f);
			}
		}
	}
	fwrite(&autohide,1,sizeof(bool),f);
	fwrite(&visible,1,sizeof(bool),f);
	fwrite(&position,1,sizeof(POINT),f);
	fclose(f);
}

void LoadSettings (char* file, lpsettings set , int count,colors &clr) {
	FILE* f = fopen(file, "rb");
	int v = LoadFileVersion(f); // Get version of settings
	if ((v >= 0) && (v < vercount)) {   // If version supported,
		versions[v].LoadProc(f, set , count, clr); // Call version specifed LoadProc;
		//changed = (v != (vercount - 1));
	}
	else {
		MessageBox(NULL, "Version of setings not supported.\nProgram will clear settings",
				   "Check for program updates",
				   MB_ICONINFORMATION);
	}
	fclose(f);
}

void LoadV1(FILE* f, lpsettings set , int count,colors &clr) {
	long len = 0;
	int cnt = 0;
	fread(&cnt, 1, sizeof(int), f);
	if (cnt > count) cnt = count;
	for (int i = 0;i < cnt;i++) {
		fread(&len, 1, sizeof(long), f);
		if (len > 0) {
			set [i].command = new char[len + 1];
			fread(set [i].command, 1, len + 1, f);
			if (!FileExists(set [i].command)) {
				delete [] (set [i].command);
				set [i].command = NULL;
			}
			else {
				ExtractFileName(set [i].command, &(set [i].tooltip));
			}
		}
		else {
			set [i].tooltip = new char[lstrlen(defttip) + 1];
			memcpy(set [i].tooltip, defttip, lstrlen(defttip) + 1);
		}
	}
}

void LoadV2(FILE* f, lpsettings set , int count,colors &clr) {
	long len = 0;
	int cnt = 0;
	fread(&cnt, 1, sizeof(int), f);
	if (cnt > count) cnt = count;
	for (int i = 0;i < cnt;i++) {
		fread(&len, 1, sizeof(long), f);
		if (len > 0) {
			set [i].command = new char[len + 1];
			fread(set [i].command, 1, len + 1, f);
			if (!FileExists(set [i].command)) {
				delete [] (set [i].command);
				set [i].command = NULL;
				set [i].tooltip = new char[lstrlen(defttip) + 1];
				memcpy(set [i].tooltip, defttip, lstrlen(defttip) + 1);
			}
			else {
				ExtractFileName(set [i].command, &(set [i].tooltip));
				fread(&len, 1, sizeof(long), f);
				if (len > 0) {
					set [i].workpath = new char[len + 1];
					fread(set [i].workpath, 1, len + 1, f);
				}
				fread(&len, 1, sizeof(long), f);
				if (len > 0) {
					set [i].iconfile = new char[len + 1];
					fread(set [i].iconfile, 1, len + 1, f);
				}
				fread(&len, 1, sizeof(long), f);
				if (len > 0) {
					set [i].tooltip = new char[len + 1];
					fread(set [i].tooltip, 1, len + 1, f);
				} else {
					ExtractFileName(set [i].command, &(set [i].tooltip));
				}
			}
		}
		else {
			set [i].tooltip = new char[lstrlen(defttip) + 1];
			memcpy(set [i].tooltip, defttip, lstrlen(defttip) + 1);
		}
	}
}

void LoadV3(FILE* f, lpsettings set , int count,colors &clr) {
	LoadV2(f,set,count,clr);
	fread(&autohide,1,sizeof(bool),f);
	fread(&visible,1,sizeof(bool),f);
	fread(&position,1,sizeof(POINT),f);
}

int LoadFileVersion(FILE*f) {
	int v = vercount - 1;
	verinfo tmp;
	fread(&tmp, 1, sizeof(verinfo), f);
	tmp[sizeof(verinfo) - 1] = 0;
	while (v >= 0) {
		if (memcmp(versions[v].info, &tmp, sizeof(verinfo)) == 0)break;
		v--;
	}
	return v;
}

void SaveFileVersion(FILE* f) {
	fwrite(versions[vercount - 1].info, 1, sizeof(verinfo), f);
}
