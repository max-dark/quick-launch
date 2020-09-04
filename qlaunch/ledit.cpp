/*/
 + Project name : [Q] Launch
 + File name    : ledit.cpp
 + Description  : однострочный текстовый редактор
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#include "main.h"
#include "ledit.h"

ATOM CEClass = 0;
HMENU pmenu = 0;
COLORREF fg = RGB(0x00, 0x00, 0x00);
COLORREF bg = RGB(0xff, 0xff, 0xff);
COLORREF sg = RGB(181, 178, 222);

struct uritem;
typedef uritem* lpuritem;

struct uritem {
	char *udata;
	char *rdata;
	long upos, rpos;
	long ulen, rlen;
	long ucnt, rcnt;
	lpuritem next, prev;
};

typedef struct {
	lpuritem first, last;
	long count;
}urbuff;

typedef struct {
	char* str;
	long len, cnt;
	long pos, vpos, cps, se;
	//long ss;
	HWND hwnd;
	HFONT fnt;
	int ch, cw;
	bool owr;
	urbuff undo, redo;
	long maxur;
}line;

struct PMItem {
	char title[30];
	UINT id;
};

#define NewCnt(len) (32*(len/32+1))

#define MPC_UNDO  100
#define MPC_REDO  101
#define MPC_COPY  102
#define MPC_CUT   103
#define MPC_PASTE 104
#define MPC_DEL   105
#define MPC_SALL  106
#define MPC_SEP   0x0
#define MPE_CNT   0x9
PMItem menu[9] = {
		{"Отмена\tCtrl+Z", MPC_UNDO},
		{"Повторить\tCtrl+Shift+Z", MPC_REDO},
		{"", MPC_SEP},
		{"Вырезать\tCtrl+X", MPC_CUT},
		{"Копировать\tCtrl+C", MPC_COPY},
		{"Вставить\tCtrl+V", MPC_PASTE},
		{"Удалить\tDel", MPC_DEL},
		{"", MPC_SEP},
		{"Выделить все\tCtrl+A", MPC_SALL}};

typedef line* lpline;

static inline bool Selected(lpline l) {
	return (l->pos != l->se);
}

//LRESULT CALLBACK
LONG APIENTRY LEditProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

ATOM RegisterEditor(HINSTANCE hApplication) {
	WNDCLASS wc;
	MENUITEMINFO mii;
	if (CEClass == 0) {
		ZeroMemory(&wc, sizeof(WNDCLASS));
		wc.style = CS_DBLCLKS;
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.hInstance = hApplication;
		wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
		wc.lpfnWndProc = LEditProc;
		wc.lpszClassName = LE_CLASS;
		CEClass = RegisterClass(&wc);
	}
	if (pmenu == 0) {
		pmenu = CreatePopupMenu();
		ZeroMemory(&mii, sizeof(MENUITEMINFO));
		mii.cbSize = sizeof(MENUITEMINFO);
		for (int i = 0;i < MPE_CNT;i++) {
			mii.dwItemData = menu[i].id;
			mii.cch = strlen(menu[i].title);
			mii.wID = menu[i].id;
			if (menu[i].id == MPC_SEP) {
				//Add Separator to pmenu
				mii.fMask = MIIM_TYPE;
				mii.fType = MFT_SEPARATOR;
				mii.dwTypeData = NULL;
			}
			else {
				//Add command item to pmenu
				mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_DATA;
				mii.fType = MFT_STRING;
				mii.dwTypeData = menu[i].title;
			}
			InsertMenuItem(pmenu, i, true, &mii);
		}
	}
	return CEClass;
}
HWND CreateLEdit(HWND parent, int x, int y, int w, int h) {
	return CreateWindowEx(0, LE_CLASS, NULL, LE_STYLE,
				x, y, w, h, parent, NULL, hApplication, NULL);
}
//LE_EX_STYLE
void DoneEdit() {
	DestroyMenu(pmenu);
	UnregisterClass(LE_CLASS, hApplication);
}

void Init (lpline& l, HWND hwnd);
void Done (lpline& l);
void PutChar(lpline l, char c);
void KeyDown(lpline l, WPARAM key);
void Paint (lpline l, HDC hdc);
void Command(lpline l, UINT id);
void ContextMenu(lpline l);

bool dwn = false;
void SelectWord(lpline l, int x) {
	long tpos;
	tpos = x / l->cw;
	if (x % l->cw > l->cw / 2) tpos++;
	tpos += l->vpos;
	if (tpos >= l->len) return ;
	l->se = tpos;
	while (l->se > 0) {
		if (!IsCharAlphaNumeric(l->str[l->se - 1])) break;
		l->se--;
	}
	l->pos = tpos;
	while (l->pos < l->len) {
		if (!IsCharAlphaNumeric(l->str[l->pos])) break;
		l->pos++;
	}
	//l->pos = l->ss;
	if (l->pos >= (l->vpos + l->cps)) l->vpos = l->pos - l->cps + 1;
	HideCaret(l->hwnd);
	HDC hdc = GetDC(l->hwnd);
	Paint(l, hdc);
	ReleaseDC(l->hwnd, hdc);
	SetCaretPos(((l->pos) - (l->vpos))*(l->cw), 0);
	ShowCaret(l->hwnd);
}

static inline bool CanRedo(lpline l) {
	return (l->redo.count > 0);
}
static inline bool CanUndo(lpline l) {
	return (l->undo.count > 0);
}

void DoRedo(lpline l) {
	lpuritem tmp;
	if (CanRedo(l)) {
		tmp = l->redo.first;
		l->redo.first = tmp->next;
		l->redo.count--;
		if (l->redo.count == 0) {
			l->redo.last = l->redo.first = NULL;
		}
		tmp->next = l->undo.first;
		if (l->undo.first) l->undo.first->prev = tmp;
		l->undo.first = tmp;
		if (l->str) delete [](l->str);
		l->pos = tmp->rpos;
		l->len = tmp->rlen;
		l->cnt = tmp->rcnt;
		l->str = new char[l->cnt];
		CopyMemory(l->str, tmp->rdata, l->len);
		if (l->undo.count >= l->maxur) {
			tmp = l->undo.last;
			l->undo.last = tmp->prev;
			if (tmp->rdata) delete [](tmp->rdata);
			if (tmp->udata) delete [](tmp->udata);
			delete tmp;
		}
		else {
			l->undo.count++;
		}
		//l->ss =
		l->se = l->pos;
		if (l->pos < l->vpos) {
			l->vpos = l->pos;
		}
		else if (l->pos >= (l->vpos + l->cps)) {
			l->vpos = l->pos - l->cps + 1;
		}
	}
}
void DoUndo(lpline l) {
	lpuritem tmp;
	if (CanUndo(l)) {
		tmp = l->undo.first;
		l->undo.first = tmp->next;
		l->undo.count--;
		if (l->undo.count == 0) {
			l->undo.last = l->undo.first = NULL;
		}
		tmp->next = l->redo.first;
		if (l->redo.first) l->redo.first->prev = tmp;
		l->redo.first = tmp;
		if (l->str) delete [](l->str);
		l->pos = tmp->upos;
		l->len = tmp->ulen;
		l->cnt = tmp->ucnt;
		l->str = new char[l->cnt];
		CopyMemory(l->str, tmp->udata, l->len);
		if (l->redo.count >= l->maxur) {
			tmp = l->redo.last;
			l->redo.last = tmp->prev;
			if (tmp->rdata) delete [](tmp->rdata);
			if (tmp->udata) delete [](tmp->udata);
			delete tmp;
		}
		else {
			l->redo.count++;
		}
		//l->ss =
		l->se = l->pos;
		if (l->pos < l->vpos) {
			l->vpos = l->pos;
		}
		else if (l->pos >= (l->vpos + l->cps)) {
			l->vpos = l->pos - l->cps + 1;
		}
	}
}
void ClearRedo(lpline l) {
	lpuritem tmp;
	while (l->redo.count > 0) {
		tmp = l->redo.first;
		l->redo.first = tmp->next;
		if (tmp->rdata) delete [](tmp->rdata);
		if (tmp->udata) delete [](tmp->udata);
		delete tmp;
		l->redo.count--;
	}
	ZeroMemory(&l->redo, sizeof(urbuff));
}
void ClearUndo(lpline l) {
	lpuritem tmp;
	while (l->undo.count > 0) {
		tmp = l->undo.first;
		l->undo.first = tmp->next;
		if (tmp->rdata) delete [](tmp->rdata);
		if (tmp->udata) delete [](tmp->udata);
		delete tmp;
		l->undo.count--;
	}
	ZeroMemory(&l->undo, sizeof(urbuff));
}
void PrepareUndo(lpline l) {
	lpuritem tmp;
	char *str;
	ClearRedo(l);
	if (l->undo.count >= l->maxur) {
		tmp = l->undo.last;
		l->undo.last = tmp->prev;
		if (tmp->rdata) delete [](tmp->rdata);
		if (tmp->udata) delete [](tmp->udata);
		delete tmp;
		l->undo.count--;
	}
	tmp = new uritem;
	ZeroMemory(tmp, sizeof(uritem));
	str = new char[l->cnt];
	CopyMemory(str, l->str, l->len);
	tmp->udata = str;
	tmp->upos = l->pos;
	tmp->ulen = l->len;
	tmp->ucnt = l->cnt;
	if (l->undo.count == 0) {
		l->undo.first = l->undo.last = tmp;
	}
	else {
		l->undo.first->prev = tmp;
		tmp->next = l->undo.first;
		l->undo.first = tmp;
	}
	l->undo.count++;
}
void PrepareRedo(lpline l) {
	char *str;
	if (l->undo.first->rdata) delete [](l->undo.first->rdata);
	str = new char[l->cnt];
	CopyMemory(str, l->str, l->len);
	l->undo.first->rdata = str;
	l->undo.first->rlen = l->len;
	l->undo.first->rpos = l->pos;
	l->undo.first->rcnt = l->cnt;
}
void Init(lpline& l, HWND hwnd) {
	LOGFONT lf;
	TEXTMETRIC tm;
	HDC hdc;
	l = new line;
	ZeroMemory(l, sizeof(line));
	l->hwnd = hwnd;
	l->maxur = 256;

	ZeroMemory(&lf, sizeof(LOGFONT));
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfHeight = -13;
	lf.lfWeight = FW_NORMAL;
	lf.lfOutPrecision = OUT_TT_PRECIS;
	strcpy(lf.lfFaceName, "Courier New");

	l->fnt = CreateFontIndirect(&lf);
	hdc = GetDC(hwnd);
	SelectObject(hdc, l->fnt);
	GetTextMetrics(hdc, &tm);
	l->ch = tm.tmHeight;
	l->cw = tm.tmAveCharWidth;
	ReleaseDC(hwnd, hdc);
	SetWindowLong(hwnd, GWL_USERDATA, (long)l);
}
void Done(lpline& l) {
	SetWindowLong(l->hwnd, GWL_USERDATA, 0L);
	if (l->str) delete [](l->str);
	if (l->fnt) DeleteObject(l->fnt);
	ClearRedo(l);
	ClearUndo(l);
	delete l;
	l = NULL;
}
void FindWordStart(lpline l) {
	while (l->pos > 0) {
		l->pos--;
		if (l->pos == 0) return ;
		if (IsCharAlphaNumeric(l->str[l->pos]) && (!IsCharAlphaNumeric(l->str[l->pos - 1]))) {
			return ;
		}
	}
}
void FindWordEnd(lpline l) {
	while (l->pos < l->len) {
		l->pos++;
		if (l->pos == l->len) return ;
		if (IsCharAlphaNumeric(l->str[l->pos - 1]) && (!IsCharAlphaNumeric(l->str[l->pos]))) {
			return ;
		}
	}
}
void Delete(lpline l) {
	char *tmp = NULL;
	long nlen, ncnt;
	long ss, se;
	if (Selected(l)) {
		PrepareUndo(l);
		ss = l->pos;
		se = l->se;
		if (ss > se) {
			nlen = ss;
			ss = se;
			se = nlen;
		}
		l->pos = l->se = ss;
	}
	else {
		if (l->pos >= l->len) return ;
		PrepareUndo(l);
		ss = l->pos;
		se = ss + 1;
	}
	nlen = l->len - (se - ss);
	ncnt = NewCnt(nlen);
	if (ncnt != l->cnt) {
		tmp = new char[ncnt];
		if (ss > 0) CopyMemory(tmp, l->str, ss);
		if (se < l->len) CopyMemory(&tmp[ss], &l->str[se], l->len - se);
		l->cnt = ncnt;
		if (l->str) delete [] (l->str);
		l->str = tmp;
	}
	else {
		if (se < l->len) MoveMemory(&l->str[ss], &l->str[se], l->len - se);
	}
	l->len = nlen;
	if (l->pos < l->vpos) {
		l->vpos = l->pos;
	}
	else if (l->pos >= (l->vpos + l->cps)) {
		l->vpos = l->pos - l->cps + 1;
	}
	PrepareRedo(l);
}
void Backspace(lpline l) {
	char *tmp = NULL;
	long nlen, ncnt;
	long ss, se;
	if (Selected(l)) {
		PrepareUndo(l);
		ss = l->pos;
		se = l->se;
		if (ss > se) {
			nlen = ss;
			ss = se;
			se = nlen;
		}
		l->pos = l->se = ss;
	}
	else {
		if (l->pos < 1) return ;
		PrepareUndo(l);
		ss = l->pos - 1;
		se = l->pos;
		l->pos = l->se = ss;
	}
	nlen = l->len - (se - ss);
	ncnt = NewCnt(nlen);
	if (ncnt != l->cnt) {
		tmp = new char[ncnt];
		if (ss > 0) CopyMemory(tmp, l->str, ss);
		if (se < l->len) CopyMemory(&tmp[ss], &l->str[se], l->len - se);
		l->cnt = ncnt;
		if (l->str) delete [] (l->str);
		l->str = tmp;
	}
	else {
		if (se < l->len) MoveMemory(&l->str[ss], &l->str[se], l->len - se);
	}
	l->len = nlen;
	PrepareRedo(l);
}
void Copy(lpline l) {
	char *tmp = NULL;
	long ss, se, len;
	HGLOBAL hmem = NULL;
	if (Selected(l)) {
		if (!OpenClipboard(l->hwnd)) return ;
		EmptyClipboard();
		if (l->pos > l->se) {
			ss = l->se;
			se = l->pos;
		}
		else {
			ss = l->pos;
			se = l->se;
		}
		len = se - ss;
		hmem = GlobalAlloc(GMEM_DDESHARE, (len + 1) * sizeof(char));
		if (!hmem) {
			CloseClipboard();
			return ;
		}
		tmp = (char*)GlobalLock(hmem);
		CopyMemory(tmp, &l->str[ss], len);
		tmp[len] = 0;
		GlobalUnlock(hmem);
		SetClipboardData(CF_TEXT, hmem);
		CloseClipboard();
	}
}
void Cut(lpline l) {
	if (Selected(l)) {
		Copy(l);
		Delete(l);
	}
}
void Paste(lpline l) {
	char *tmp;
	char *str;
	long len, nlen, ncnt, ss, se, sz;
	HGLOBAL hmem;
	if (IsClipboardFormatAvailable(CF_TEXT)) {
		if (!OpenClipboard(l->hwnd)) return ;
		hmem = GetClipboardData(CF_TEXT);
		if (hmem) {
			tmp = (char*)GlobalLock(hmem);
			if (tmp) {
				PrepareUndo(l);
				len = strlen(tmp);
				if (Selected(l)) {
					if (l->pos > l->se) {
						ss = l->se;
						se = l->pos;
					}
					else {
						ss = l->pos;
						se = l->se;
					}
					sz = ss + len;
					nlen = l->len - (se - ss) + len;
					ncnt = NewCnt(nlen);
					if (ncnt != l->cnt) {
						str = new char[ncnt];
						if (ss > 0) CopyMemory(str, l->str, ss);
						if (se < l->len) CopyMemory(&str[sz], &l->str[se], l->len - se);
						if (l->str) delete [](l->str);
						l->str = str;
						l->cnt = ncnt;
					}
					else {
						if (se < l->len) MoveMemory(&l->str[sz], &l->str[se], l->len - ss);
					}
					l->len = nlen;
					CopyMemory(&l->str[ss], tmp, len);
					l->se = l->pos = sz;
				}
				else {
					nlen = l->len + len;
					ncnt = NewCnt(nlen);
					ss = l->pos;
					se = ss + len;
					if (ncnt != l->cnt) {
						str = new char[ncnt];
						if (ss > 0) CopyMemory(str, l->str, ss);
						if (ss < l->len) CopyMemory(&str[se], &l->str[ss], l->len - ss);
						if (l->str) delete [](l->str);
						l->str = str;
						l->cnt = ncnt;
					}
					else {
						if (ss < l->len) MoveMemory(&l->str[se], &l->str[ss], l->len - ss);
					}
					l->len = nlen;
					CopyMemory(&l->str[ss], tmp, len);
					l->pos += len;
					l->se = l->pos;
				}
				if (l->pos < l->vpos) {
					l->vpos = l->pos;
				}
				else if (l->pos >= (l->vpos + l->cps)) {
					l->vpos = l->pos - l->cps + 1;
				}
				PrepareRedo(l);
				GlobalUnlock(hmem);
			}
		}
		CloseClipboard();
	}
}
void KeyDown(lpline l, WPARAM key) {
	if (KeyPressed(VK_MENU)) return ;
	bool sft = KeyPressed(VK_SHIFT);
	bool ctr = KeyPressed(VK_CONTROL);
	switch (key) {
		case VK_BACK: Backspace(l); break;
		case VK_DELETE:{
			if (sft) {
				Cut(l);
			}
			else {
				Delete(l);
			}
			}break;
		case VK_LEFT:{
			if (sft) {
				if (l->pos == 0) return ;
				if (ctr) {
					FindWordStart(l);
				}
				else {
					l->pos--;
				}
				//l->ss = l->pos;
			}
			else {
				if (l->pos > 0) {
					if (ctr) {
						FindWordStart(l);
					}
					else {
						l->pos--;
					}
				}
				l->se = l->pos;
			}
			if (l->pos < l->vpos) l->vpos = l->pos;
			}break;
		case VK_RIGHT:{
			if (sft) {
				if (l->pos == l->len) return ;
				if (ctr) {
					FindWordEnd(l);
				}
				else {
					l->pos++;
				}
				//l->ss = l->pos;
			}
			else {
				if (l->pos < l->len) {
					if (ctr) {
						FindWordEnd(l);
					}
					else {
						l->pos++;
					}
				}
				l->se = l->pos;
			}
			if (l->pos >= (l->vpos + l->cps)) l->vpos = l->pos - l->cps + 1;
			}break;
		case VK_HOME:{
			l->pos = 0;
			if (sft) {
				;//l->ss = l->pos;
			}
			else {
				l->se = l->pos;
			}
			l->vpos = 0;
			}break;
		case VK_END:{
			l->pos = l->len;
			if (sft) {
				;//l->ss = l->pos;
			}
			else {
				l->se = l->pos;
			}
			if (l->pos >= (l->vpos + l->cps)) l->vpos = l->pos - l->cps + 1;
			}break;
		case VK_INSERT:{
			if (ctr) {
				Copy(l);
			}
			else if (sft) {
				Paste(l);
			}
			else {
				l->owr = !l->owr;
			}
			}return ;
		case 'A':{
			if (ctr) {
				SendMessage(l->hwnd, WM_COMMAND, MPC_SALL, 0);
				return ;
			}
			}break;
		case 'Z':{
			if (ctr) {
				if (sft) {
					DoRedo(l);
				}
				else {
					DoUndo(l);
				}
			}
			}break;
		case 'X':{
			if (ctr) {
				Cut(l);
			}
			}break;
		case 'C':{
			if (ctr) {
				Copy(l);
			}
			}break;
		case 'V':{
			if (ctr) {
				Paste(l);
			}
			}break;
		default: return ;
	}
	HideCaret(l->hwnd);
	HDC hdc = GetDC(l->hwnd);
	Paint(l, hdc);
	ReleaseDC(l->hwnd, hdc);
	SetCaretPos(((l->pos) - (l->vpos))*(l->cw), 0);
	ShowCaret(l->hwnd);
}
void PutChar(lpline l, char c) {
	if (KeyPressed(VK_MENU) || KeyPressed(VK_CONTROL)) return ;
	char *tmp = NULL;
	long ss, se, sz;
	long nlen, ncnt;
	switch (c) {
		case 0x08:
		case 0x0A:
		case 0x0D:
		case 0x1B:
		case 0x09:
			return ;
		default: {
				PrepareUndo(l);
				if (Selected(l)) {
					ss = (l->pos < l->se ? l->pos : l->se);
					sz = (l->pos > l->se ? l->pos : l->se);
					se = ss + 1;
					nlen = l->len - (sz - ss) + 1;
					ncnt = NewCnt(nlen);
					if (ncnt != l->cnt) {
						tmp = new char[ncnt];
						if (ss > 0) CopyMemory(tmp, l->str, ss);
						if (sz < l->len) CopyMemory(&tmp[se], &l->str[sz], l->len - sz);
						if (l->str) delete [](l->str);
						l->str = tmp;
						l->cnt = ncnt;
					}
					else {
						if (sz < l->len) MoveMemory(&l->str[se], &l->str[sz], l->len - sz);
					}
					l->str[ss] = c;
					l->len = nlen;
					l->pos = se;
				}
				else {
					if (l->owr && (l->pos < l->len)) {
						l->str[l->pos] = c;
					}
					else {
						nlen = l->len + 1;
						ncnt = NewCnt(nlen);
						ss = l->pos;
						se = ss + 1;
						if (ncnt != l->cnt) {
							tmp = new char[ncnt];
							if (ss > 0) CopyMemory(tmp, l->str, ss);
							if (ss < l->len) CopyMemory(&tmp[se], &l->str[ss], l->len - ss);
							if (l->str) delete [](l->str);
							l->str = tmp;
							l->cnt = ncnt;
						}
						else {
							if (ss < l->len) MoveMemory(&l->str[se], &l->str[ss], l->len - ss);
						}
						l->str[l->pos] = c;
						l->len = nlen;
					}
					l->pos++;
				}
				l->se = l->pos;
				if (l->pos < l->vpos) {
					l->vpos = l->pos;
				}
				else if (l->pos >= (l->vpos + l->cps)) {
					l->vpos = l->pos - l->cps + 1;
				}
				PrepareRedo(l);
			};
	}
	HideCaret(l->hwnd);
	HDC hdc = GetDC(l->hwnd);
	Paint(l, hdc);
	ReleaseDC(l->hwnd, hdc);
	SetCaretPos((l->pos - l->vpos)*l->cw, 0);
	ShowCaret(l->hwnd);
}
void Paint(lpline l, HDC hdc) {
	long len, pos, tmp, ss, se;
	RECT rc;
	GetClientRect(l->hwnd, &rc);
	HDC mdc = CreateCompatibleDC(hdc);
	HBITMAP mbm = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	SelectObject(mdc, mbm);
	PatBlt(mdc, 0, 0, rc.right, rc.bottom, WHITENESS);
	//RoundRect(mdc,0,0,rc.right,rc.bottom,3,3);
	SelectObject(mdc, l->fnt);
	if (Selected(l)) {
		ss = l->pos;se = l->se;
		if (se < ss) {
			tmp = ss;
			ss = se;
			se = tmp;
		}
		pos = l->vpos;
		if (ss > pos) {
			len = ss - pos;
			SetTextColor(mdc, fg);
			SetBkColor(mdc, bg);
			TextOut(mdc, 0, 0, &(l->str[pos]), len);
			pos += len;
		}
		len = se - pos;
		//SetTextColor(mdc,bg);
		SetBkColor(mdc, sg);
		TextOut(mdc, l->cw*(pos - l->vpos), 0, &(l->str[pos]), len);
		pos += len;
		if ((l->len - 1 >= pos) && (pos < l->vpos + l->cps)) {
			len = l->len - pos;
			SetTextColor(mdc, fg);
			SetBkColor(mdc, bg);
			TextOut(mdc, l->cw*(pos - l->vpos), 0, &(l->str[pos]), len);
		}
	}
	else {
		SetTextColor(mdc, fg);
		SetBkColor(mdc, bg);
		TextOut(mdc, 0, 0, &(l->str[l->vpos]), (l->cps <= (l->len - l->vpos) ? l->cps : (l->len - l->vpos)));
	}
	BitBlt(hdc, 0, 0, rc.right, rc.bottom, mdc, 0, 0, SRCCOPY);
	DeleteDC(mdc);
	DeleteObject(mbm);
}
void Command(lpline l, UINT id) {
	switch (id) {
		case MPC_UNDO:
			DoUndo(l);
			break;
		case MPC_REDO:
			DoRedo(l);
			break;
		case MPC_COPY:
			Copy(l);
			break;
		case MPC_CUT:
			Cut(l);
			break;
		case MPC_PASTE:
			Paste(l);
			break;
		case MPC_DEL:
			Delete(l);
			break;
		case MPC_SALL:
			l->pos = l->len;
			l->se = 0;
			if (l->pos >= (l->vpos + l->cps)) l->vpos = l->pos - l->cps + 1;
			break;
		default:
			MessageBeep(IDOK);
			return ;
	}
	HideCaret(l->hwnd);
	HDC hdc = GetDC(l->hwnd);
	Paint(l, hdc);
	ReleaseDC(l->hwnd, hdc);
	SetCaretPos((l->pos - l->vpos)*l->cw, 0);
	ShowCaret(l->hwnd);
}
void ContextMenu(lpline l) {
	POINT pt;
	MENUITEMINFO mii;

	pt.y = 0;
	pt.x = (l->pos - l->vpos) * l->cw;
	ClientToScreen(l->hwnd, &pt);
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fState = (CanUndo(l) ? MFS_ENABLED : MFS_DISABLED);
	SetMenuItemInfo(pmenu, MPC_UNDO, false, &mii);
	mii.fState = (CanRedo(l) ? MFS_ENABLED : MFS_DISABLED);
	SetMenuItemInfo(pmenu, MPC_REDO, false, &mii);
	mii.fState = (Selected(l) ? MFS_ENABLED : MFS_DISABLED);
	SetMenuItemInfo(pmenu, MPC_COPY, false, &mii);
	SetMenuItemInfo(pmenu, MPC_CUT , false, &mii);
	SetMenuItemInfo(pmenu, MPC_DEL , false, &mii);
	mii.fState = (IsClipboardFormatAvailable(CF_TEXT) ? MFS_ENABLED : MFS_DISABLED);
	SetMenuItemInfo(pmenu, MPC_PASTE, false, &mii);
	TrackPopupMenuEx(pmenu, 0, pt.x, pt.y, l->hwnd, NULL);
}

LONG APIENTRY LEditProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	lpline l = (lpline)GetWindowLong(hwnd, GWL_USERDATA);
	if ((l == NULL) && (umsg != WM_CREATE)) return DefWindowProc(hwnd, umsg, wparam, lparam);
	PAINTSTRUCT ps;
	HDC hdc;
	long tpos;
	int ox;
	switch (umsg) {
		case WM_SIZE:
			l->cps = LOWORD(lparam) / l->cw;
			break;
		case WM_GETTEXT: {
				char* buff = (char*)lparam;
				int len = (int)wparam;
				len = (len <= l->len + 1 ? len : l->len);
				CopyMemory(buff, l->str, len);
				buff[len] = 0;
				//Beep(200,300);
				return len;
			}
			break;
		case WM_SETTEXT: {
				ClearUndo(l);
				ClearRedo(l);
				if (l->str != NULL) {
					delete [](l->str);
					l->str = NULL;
					l->len = 0;
					l->cnt = 0;
				}
				l->vpos = l->se = l->pos = 0;
				LPCTSTR lpsz = (LPCTSTR)lparam;
				long len = lstrlen(lpsz);
				if (len > 0) {
					l->len = len;
					long nlen = NewCnt(len);
					l->str = new char[nlen];
					CopyMemory(l->str, lpsz, len);
					l->cnt=nlen;
				}
				HideCaret(l->hwnd);
				hdc = GetDC(l->hwnd);
				Paint(l, hdc);
				ReleaseDC(l->hwnd, hdc);
				SetCaretPos(((l->pos) - (l->vpos))*(l->cw), 0);
				ShowCaret(l->hwnd);
				return true;
			}
			break;
		case WM_GETTEXTLENGTH:
			return l->len;
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			SetFocus(hwnd);
			break;
		case WM_LBUTTONDBLCLK:
			//MessageBeep(IDOK);
			dwn = false;
			SelectWord(l, LOWORD(lparam));
			break;
		case WM_LBUTTONDOWN:
			SetCapture(hwnd);
			SetFocus(hwnd);
			ox = LOWORD(lparam);
			tpos = ox / l->cw;
			if (ox % l->cw > l->cw / 2) tpos++;
			tpos += l->vpos;
			if (tpos > l->len) tpos = l->len;
			dwn = true;
			l->pos = tpos;
			if (!KeyPressed(VK_SHIFT)) l->se = tpos;
			HideCaret(l->hwnd);
			hdc = GetDC(l->hwnd);
			Paint(l, hdc);
			ReleaseDC(l->hwnd, hdc);
			SetCaretPos(((l->pos) - (l->vpos))*(l->cw), 0);
			ShowCaret(l->hwnd);
			break;
		case WM_LBUTTONUP:
			ReleaseCapture();
			if (dwn) {
				ox = LOWORD(lparam);
				tpos = ox / l->cw;
				if (ox % l->cw > l->cw / 2) tpos++;
				tpos += l->vpos;
				if (tpos > l->len) tpos = l->len;
				dwn = false;
				l->pos = tpos;
				HideCaret(l->hwnd);
				hdc = GetDC(l->hwnd);
				Paint(l, hdc);
				ReleaseDC(l->hwnd, hdc);
				SetCaretPos(((l->pos) - (l->vpos))*(l->cw), 0);
				ShowCaret(l->hwnd);
			}
			break;
		case WM_MOUSEMOVE:
			if (dwn) {
				ox = LOWORD(lparam);
				tpos = ox / l->cw;
				if (tpos <= 1) {
					if (l->vpos > 0) {
						//MessageBeep(MB_OK);
						l->vpos--;
					}
				}
				if (ox % l->cw > l->cw / 2) tpos++;
				tpos += l->vpos;
				if (tpos > l->len) tpos = l->len;
				l->pos = tpos;
				if (l->pos < l->vpos) {
					l->vpos = l->pos;
				}
				else if (l->pos >= (l->vpos + l->cps)) {
					l->vpos = l->pos - l->cps + 1;
				}
				HideCaret(l->hwnd);
				hdc = GetDC(l->hwnd);
				Paint(l, hdc);
				ReleaseDC(l->hwnd, hdc);
				SetCaretPos(((l->pos) - (l->vpos))*(l->cw), 0);
				ShowCaret(l->hwnd);
			}
			break;
		case WM_PAINT:
			HideCaret(hwnd);
			hdc = BeginPaint(hwnd, &ps);
			Paint(l, hdc);
			EndPaint(hwnd, &ps);
			ShowCaret(hwnd);
			break;
		case WM_KEYDOWN:
			KeyDown(l, wparam);
			break;
		case WM_COMMAND:
			Command(l, LOWORD(wparam));
			break;
		case WM_CONTEXTMENU:
			SetFocus(hwnd);
			ContextMenu(l);
			break;
		case WM_CHAR:
			PutChar(l, (char)wparam);
			break;
		case WM_SETFOCUS:
			DestroyCaret();
			CreateCaret(hwnd, (HBITMAP)NULL, 2, l->ch);
			SetCaretPos((l->pos - l->vpos)*l->cw, 0);
			ShowCaret(hwnd);
			break;
		case WM_KILLFOCUS:
			DestroyCaret();
			break;
		case WM_CREATE: Init(l, hwnd); break;
		case WM_DESTROY: Done(l); break;
		default: return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	return 0;
}
