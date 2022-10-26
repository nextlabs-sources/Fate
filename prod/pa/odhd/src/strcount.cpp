
#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include "log.h"

//
/*
FONT size we are using:
CreateFontW(14,
0,
0,
0,
550,
0,
0,
0,
DEFAULT_CHARSET,
OUT_DEFAULT_PRECIS,
CLIP_DEFAULT_PRECIS,
CLEARTYPE_QUALITY,
DEFAULT_PITCH | FF_DONTCARE,
L"Arial"
);
Char   Width
A   =   8
B   =   7
C   =   8
D   =   7
E   =   6
F   =   6
G   =   8
H   =   7
I   =   2
J   =   5
K   =   7
L   =   6
M   =   8
N   =   7
O   =   8
P   =   7
Q   =   8
R   =   7
S   =   7
T   =   7
U   =   7
V   =   8
W   =   10
X   =   8
Y   =   8
Z   =   7
a   =   6
b   =   6
c   =   6
d   =   6
e   =   6
f   =   5
g   =   6
h   =   6
i   =   2
j   =   2
k   =   5
l   =   3
m   =   9
n   =   6
o   =   6
p   =   6
q   =   6
r   =   4
s   =   6
t   =   4
u   =   6
v   =   6
w   =   10
x   =   6
y   =   6
z   =   6

white space = 3
@   =   11
#   =   6
_   =   7
-   =   4
.   =   2
:   =   3
\   =   4
$   =   6
&   =   7
(   =   4
)   =   4
'   =   2
,   =   3
=   =   6
~   =   6
+   =   6
[   =   3
]   =   3
{   =   4
}   =   4
*/
static int UPCHAR_SIZE[] = {8, 7, 8, 7, 7, 6, 9, 8, 3, 5, 7, 7, 11, 7, 8, 7, 8, 7, 7, 7, 7, 8, 10, 8, 8, 7};
static int LWCHAR_SIZE[] = {7, 7, 7, 7, 7, 5, 8, 7, 3, 4, 8, 3, 11, 7, 7, 7, 7, 5, 7, 5, 7, 6, 10, 6, 7, 6};
static int GetCharWidth(const WCHAR wc)
{
//    int nWidth = 0;
    int nIndex = 0;
    if(L' ' == wc)
    {
        return 3;
    }
    else if (L'A' <= wc && L'Z' >= wc)
    {
        nIndex = (int)(wc-L'A');
        return UPCHAR_SIZE[nIndex];
    }
    else if (L'a' <= wc && L'z' >= wc)
    {
        nIndex = (int)(wc-L'a');
        return LWCHAR_SIZE[nIndex];
    }
    else if (L'@' == wc)
    {
        return 11;
    }
    else if (L'#' == wc)
    {
        return 7;
    }
    else if (L'_' == wc)
    {
        return 7;
    }
    else if (L'-' == wc)
    {
        return 4;
    }
    else if (L'.' == wc)
    {
        return 3;
    }
    else if (L':' == wc)
    {
        return 4;
    }
    else if (L'\\'==wc || L'/'==wc || L'('==wc || L')'==wc)
    {
        return 4;
    }
    else if (L'['==wc || L']'==wc || L'{'==wc || L'}'==wc)
    {
        return 4;
    }
    else if (L'\''==wc || L'\"'==wc)
    {
        return 3;
    }
    else if (L'$' == wc || L'+' == wc || L'=' == wc || L'~' == wc)
    {
        return 6;
    }
    else if (L'&' == wc)
    {
        return 7;
    }
    //else if (L'1' <= wc && L'9'>=wc)
    //{
    //    return 6;
    //}
    else
    {
        return 6;
    }
}

void WrapTextToFitWidth(LPCWSTR pwzText, std::wstring& strText, int nWidthLimit)
{
    int nSize=(int)wcslen(pwzText);
    int nLen = 0;

    strText = L"";
    for(int i=0; i<nSize; i++)
    {
        int   nCharWidth = GetCharWidth(pwzText[i]);
        if((nLen+nCharWidth)>nWidthLimit)
        {
            strText.append(L" \n");
            nLen = 0;
        }
        strText.append(1, pwzText[i]);
        nLen += nCharWidth;
    }

    DP((L"WrapTextToFitWidth!\n%s", strText.c_str()));
}