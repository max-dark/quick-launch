/*/
 + Project name : [Q] Launch
 + File name    : tooltip.h
 + Description  : создание всплывающей подсказки
 + Copiright    : (c) Ma[X]iM Software <max_dark@list.ru>
/*/
#ifndef __tooltip_h
#define __tooltip_h
#include <windows.h>

#define ToolTipClass "QL_ToolTip_Class"

ATOM RegisterToolTips(HINSTANCE hApplication);
HWND CreateTooltip(HWND parent,HFONT fnt,COLORREF bg,COLORREF fg);

#endif //__tooltip_h
