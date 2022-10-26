// NLVLViewPrint.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


#include "NLVLViewPrint.h"
#include "OvelayWndMgr.h"
#include "OverlayWnd.h"
#include "ManageThread.h"

int ViewVisualLabeling(const wstring& strFilePath, const NM_VLObligation::VisualLabelingInfo& theInfo,HWND hParent)
{
	wchar_t szLog[512]={0};
	StringCchPrintf(szLog,512,L"Enter to module of NLVLViewPrint, path is [%s], the parent window handle is :[0x%p]...........\n",
		strFilePath.c_str(),hParent);
	OutputDebugStringW(szLog);

	if(hParent != NULL)
	{
		COvelayWndMgr &OverLayIns = COvelayWndMgr::GetInstance();
		EnterCriticalSection(&OverLayIns.m_GlobalSec);
		if(!OverLayIns.ExistOverLayInfo(hParent))
		{
			COverlayWnd::CreateOverLayWnd(theInfo, hParent);
		}
		LeaveCriticalSection(&OverLayIns.m_GlobalSec);

		CManageThread& theMgrThread = CManageThread::GetInstance();
		theMgrThread.StartManageThread(hParent);

		return NLOverlaySuccess;
	}
	return NLNoViewHandle;
}

bool PrintVL(const HDC& hDC,const NM_VLObligation::IVisualLabelingInfo& theVLInfo)
{
	wchar_t szLog[2048]={0};
	wsprintf(szLog,L" color [%d], size1 [%d], size2 [%d], hor [%d] , leftm [%d],topm [%d], dwTra [%d],verS [%d], Font [%s], place [%s], text [%s]  ........\n",
		theVLInfo.dwFontColor,theVLInfo.dwFontSize1,theVLInfo.dwFontSize2,theVLInfo.dwHorSpace,
		theVLInfo.dwLeftMargin,theVLInfo.dwTopMargin,theVLInfo.dwTransparency,theVLInfo.dwVerSpace,
		theVLInfo.strFontName,theVLInfo.strPlacement,theVLInfo.strTextValue);
	OutputDebugStringW(szLog);

	int dwScreenWidth = ::GetDeviceCaps(hDC,PHYSICALWIDTH);	//the width of the physical page, in device units
	int dwScreenHigh = ::GetDeviceCaps(hDC,PHYSICALHEIGHT);	//the height of the physical page, in device units

	int dwLogicX= ::GetDeviceCaps(hDC,LOGPIXELSX);	
	int dwLogicY = ::GetDeviceCaps(hDC,LOGPIXELSY);

	float fPaperWidth = (float)dwScreenWidth/(float)dwLogicX;
	float fPaperHigh = (float)dwScreenHigh/(float)dwLogicY;


	Graphics graphics(hDC);
	graphics.SetSmoothingMode(SmoothingModeHighQuality);

	FontFamily fontFamily(theVLInfo.strFontName);


	PointF origin((float)theVLInfo.dwLeftMargin, (float)theVLInfo.dwTopMargin);
    PointF origintemp(0, 0);
	CAuxiliary theAU;
	SolidBrush blackBrush(theAU.ChooseFontColor(theVLInfo.dwFontColor,theVLInfo.dwTransparency));
	Pen pen(&blackBrush);

	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	RectF boundRect;
	RectF boundRect2;

	int lLeftPos = theVLInfo.dwLeftMargin;
	int lTopPos = theVLInfo.dwTopMargin;
	wstring strTextValue = theVLInfo.strTextValue;


	size_t pos = strTextValue.find(L"\\n");
	wstring strUser= L"";
	wstring strHostHome = L"";
	if(pos != wstring::npos)
	{
		strUser = strTextValue.substr(0,pos);
		if(pos + 2 <  (int)strTextValue.length())
		{
			strHostHome = strTextValue.substr(pos + 2);
		}
		else
		{
			strHostHome = L"";
		}
	}
	else
	{
		strUser = strTextValue;
		strHostHome = L"";
	}

	if (strUser.empty())
	{
		boundRect.Width = 0;
		boundRect.Height = 0;
	}
	else
	{
        GraphicsPath path;
        path.AddString(strUser.c_str(), -1, &fontFamily, theVLInfo.bFontBold ? FontStyleBold : FontStyleRegular, (Gdiplus::REAL)theVLInfo.dwFontSize1, origintemp, &format);
        path.GetBounds(&boundRect);
	}

	int lHight = (int)(fPaperHigh*96);
	int lWidth = (int)(fPaperWidth*96);
	int lTxtLength = (int)boundRect.Width;	
	if(0 == _wcsicmp(L"repeat",theVLInfo.strPlacement))
	{
        bool measureFlag = true;
		while(lTopPos < (int)lHight)
		{
			int time =0;
			origin.X = (float)lLeftPos;
			origin.Y = (float)lTopPos; 
			while(lLeftPos < (int)lWidth)
			{
				if (!strUser.empty())
				{
					GraphicsPath path;
                    path.AddString(strUser.c_str(),-1,&fontFamily,theVLInfo.bFontBold ? FontStyleBold : FontStyleRegular,(Gdiplus::REAL)theVLInfo.dwFontSize1,origin,&format);
					graphics.FillPath(&blackBrush, &path);
					origin.Y += boundRect.Height;
				}
				// second line
				wstring strTemp = strHostHome;
				wstring strPrint = strHostHome;
				time = 0;
				while(true)
				{
					size_t lNpos = strTemp.find(L"\\n");
					if(lNpos == wstring::npos)
					{
						if (strTemp.empty())
						{
							break;
						}
						strPrint = strTemp;
                        GraphicsPath path;
                        if (measureFlag)
                        {
                            path.AddString(strPrint.c_str(), -1, &fontFamily, theVLInfo.bFontBold ? FontStyleBold : FontStyleRegular, (Gdiplus::REAL)theVLInfo.dwFontSize2, origintemp, &format);
                            path.GetBounds(&boundRect2);
                            if (lTxtLength < boundRect2.Width)
                            {
                                lTxtLength = (int)boundRect2.Width;
                            }
                            path.Reset();
                        }
                        path.AddString(strPrint.c_str(),-1,&fontFamily,theVLInfo.bFontBold ? FontStyleBold : FontStyleRegular,(Gdiplus::REAL)theVLInfo.dwFontSize2,origin,&format);
						graphics.FillPath(&blackBrush, &path);
						time++;
						break;
					}
					strPrint = strTemp.substr(0,lNpos);
					strTemp = strTemp.substr(lNpos + 2);
					if (!strPrint.empty())
					{
                        GraphicsPath path;
                        if (measureFlag)
                        {
                            path.AddString(strPrint.c_str(), -1, &fontFamily, theVLInfo.bFontBold ? FontStyleBold : FontStyleRegular, (Gdiplus::REAL)theVLInfo.dwFontSize2, origintemp, &format);
                            path.GetBounds(&boundRect2);
                            if (lTxtLength < boundRect2.Width)
                            {
                                lTxtLength = (int)boundRect2.Width;
                            }
                            path.Reset();
                        }
                        path.AddString(strPrint.c_str(),-1,&fontFamily,theVLInfo.bFontBold ? FontStyleBold : FontStyleRegular,(Gdiplus::REAL)theVLInfo.dwFontSize2,origin,&format);
						graphics.FillPath(&blackBrush, &path);
					}
					origin.Y +=  boundRect2.Height;
					time++;
				}
                if (measureFlag)
                {
                    measureFlag = false;
                }
				lLeftPos = (int)(lLeftPos + lTxtLength + theVLInfo.dwHorSpace);
				origin.X = (float)lLeftPos;
				origin.Y = (float)lTopPos;
			}
			lLeftPos = theVLInfo.dwLeftMargin;
			origin.X = (float)lLeftPos;
			lTopPos = (int)(lTopPos + boundRect.Height +boundRect2.Height * time + theVLInfo.dwVerSpace);
			origin.Y = (float)lTopPos; 

		}
		graphics.ReleaseHDC(hDC);
	}
	else
	{
		PointF origin1;
		int lFontHight = 0;
		lFontHight += (int)boundRect.Height;

        origin1.X = (float)(lWidth - (int)boundRect.Width)/2;
        origin1.Y = (float)(lHight - (int)boundRect.Height)/2;

        wstring strTemp = strHostHome;
		wstring strPrint = strHostHome;

		if (!strUser.empty())
		{
			GraphicsPath path;
            path.AddString(strUser.c_str(),-1,&fontFamily,theVLInfo.bFontBold?FontStyleBold:FontStyleRegular,(Gdiplus::REAL)theVLInfo.dwFontSize1,origin1,&format);
			graphics.FillPath(&blackBrush, &path);
		}
		origin1.Y +=  boundRect.Height;

		while(true)
		{
			size_t lNpos = strTemp.find(L"\\n");
			if(lNpos == wstring::npos)
			{
				if (strTemp.empty())
				{
					break;
				}
				strPrint = strTemp;

                GraphicsPath path;
                //PointF origin;
                path.AddString(strPrint.c_str(),-1,&fontFamily,theVLInfo.bFontBold?FontStyleBold:FontStyleRegular,(Gdiplus::REAL)theVLInfo.dwFontSize2,origintemp,&format);
		        path.GetBounds(&boundRect);

				origin1.X = (float)(lWidth - boundRect.Width)/2;

                path.Reset();
                path.AddString(strPrint.c_str(),-1,&fontFamily,theVLInfo.bFontBold ? FontStyleBold : FontStyleRegular,(Gdiplus::REAL)theVLInfo.dwFontSize2,origin1,&format);
				graphics.FillPath(&blackBrush, &path);
				break;
			}
			strPrint = strTemp.substr(0,lNpos);
			if (!strPrint.empty())
			{
                GraphicsPath path;
                path.AddString(strPrint.c_str(),-1,&fontFamily,theVLInfo.bFontBold?FontStyleBold:FontStyleRegular,(Gdiplus::REAL)theVLInfo.dwFontSize2,origintemp,&format);
		        path.GetBounds(&boundRect);
				origin1.X = (float)(lWidth - boundRect.Width)/2;

                path.Reset();
                path.AddString(strPrint.c_str(),-1,&fontFamily,theVLInfo.bFontBold ? FontStyleBold : FontStyleRegular,(Gdiplus::REAL)theVLInfo.dwFontSize2,origin1,&format);
				graphics.FillPath(&blackBrush, &path);
			}
			origin1.Y +=  boundRect.Height;
			strTemp = strTemp.substr(lNpos + 2);
		}
		graphics.ReleaseHDC(hDC);
	}
	return true;
}
