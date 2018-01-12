#include "Setting.h" 
#include "DefOrg.h"
#include "Scroll.h"
#include "OrgData.h"
#include <stdio.h>
#include "resource.h"
#include "util.h"
//Movement value of scroll bar
#define MAXHORZRANGE	256
#define MAXVERTRANGE	(72-12)//8octave

#define BUF_SIZE 256
#define MAIN_WINDOW "WINDOW"


extern HWND hDlgPlayer;
extern char timer_sw;

extern CHAR app_path[BUF_SIZE];
extern CHAR num_buf[BUF_SIZE];

BOOL ScrollData::InitScroll(void)
{
	scr_info.cbSize = sizeof(SCROLLINFO);
	scr_info.fMask = SIF_RANGE | SIF_PAGE;
	scr_info.nMin = 0;scr_info.nMax = MAXHORZRANGE;
	scr_info.nPage = 4;
	SetScrollInfo(hWnd,SB_HORZ,&scr_info,1);//side
	scr_info.cbSize = sizeof(SCROLLINFO);
	scr_info.fMask = SIF_RANGE | SIF_PAGE;
	scr_info.nMax = MAXVERTRANGE;
	scr_info.nPage = 4;
	SetScrollInfo(hWnd,SB_VERT,&scr_info,1);//Vertical
	hpos = 0;//Horizontal scroll value
	//vpos = MAXVERTRANGE-27;//Vertical initial value
	//vpos = GetPrivateProfileInt(MAIN_WINDOW,"VPOS",MAXVERTRANGE-27,app_path);
	vpos = GetPrivateProfileInt(MAIN_WINDOW,"VPOS",MAXVERTRANGE-27,app_path);

	//Below isvposProcess to enable
	scr_info.fMask = SIF_POS;
	scr_info.nPos = vpos;
	SetScrollInfo(hWnd,SB_VERT,&scr_info,1);

	return TRUE;
}
void ScrollData::ChangeVerticalRange(int WindowHeight){ //Scrollable area is set based on window size
	if(WindowHeight>0){
		int ap;
		ap = (WindowHeight - 158)/12;

		scr_info.nMax = 100 - ap;
		vScrollMax = scr_info.nMax;
	}else{
		scr_info.nMax = MAXVERTRANGE;

	}
	scr_info.cbSize = sizeof(SCROLLINFO);
	scr_info.fMask = SIF_RANGE;
	SetScrollInfo(hWnd, SB_VERT, &scr_info, TRUE);//Vertical
	return;
}
void ScrollData::AttachScroll(void)
{
	
}
void ScrollData::SetIniFile()
{
	wsprintf(num_buf,"%d",vpos);
	WritePrivateProfileString(MAIN_WINDOW,"VPOS",num_buf,app_path);

}

void ScrollData::SetHorzScroll(long x)
{
	RECT rect = {0,0,WWidth,WHeight};//Area to update
	hpos = x;
	if(hpos > MAXHORZRANGE)hpos = MAXHORZRANGE;
	scr_info.fMask = SIF_POS;//nPosEnable
	scr_info.nPos = hpos;
	SetScrollInfo(hWnd,SB_HORZ,&scr_info,1);
	org_data.PutMusic();
	RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
}

void ScrollData::PrintHorzPosition(void)
{
	char str[10];
	itoa(hpos,str,10);
	SetDlgItemText(hDlgPlayer,IDE_VIEWMEAS,str);
	SetDlgItemText(hDlgPlayer,IDE_VIEWXPOS,"0");

}

void ScrollData::HorzScrollProc(WPARAM wParam){
	RECT rect = {0,0,WWidth,WHeight};//Area to update
	MUSICINFO mi;
	switch(LOWORD(wParam)){
	case SB_LINERIGHT://To the right
		hpos++;
		if(hpos > MAXHORZRANGE)hpos = MAXHORZRANGE;
		break;
	case SB_LINELEFT://To the left
		hpos--;
		if(hpos < 0)hpos = 0;
		break;
	case SB_THUMBPOSITION:
		hpos = HIWORD(wParam);//Get current position
		break;
	case SB_THUMBTRACK:
		hpos = HIWORD(wParam);//Get current position
		break;
	case SB_PAGERIGHT://To the right
		hpos += 1;
		if(hpos > MAXHORZRANGE)hpos = MAXHORZRANGE;
		break;
	case SB_PAGELEFT://To the left
		hpos -= 1;
		if(hpos < 0)hpos = 0;
		break;
	}
	//Reflect on player
	PrintHorzPosition();
	if(timer_sw == 0){
		org_data.GetMusicInfo(&mi);
		org_data.SetPlayPointer(hpos*mi.dot*mi.line);
	}
	scr_info.fMask = SIF_POS;//nPosEnable
	scr_info.nPos = hpos;
	SetScrollInfo(hWnd,SB_HORZ,&scr_info,1);
	org_data.PutMusic();
	RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
	//Below is a test
//	char str[80];
//	HDC hdc;
//	hdc = GetDC(hWnd);
//	sprintf(str,"Horizontal:%4d",hpos);
//	TextOut(hdc,200,1,str,strlen(str));
//	ReleaseDC(hWnd,hdc);
}
void ScrollData::VertScrollProc(WPARAM wParam){
	RECT rect = {0,0,WWidth,WHeight};//Area to update
	switch(LOWORD(wParam)){
	case SB_LINEDOWN://Downwards
		vpos++;
		if(vpos > vScrollMax)vpos = vScrollMax;
		break;
	case SB_LINEUP://Up
		vpos--;
		if(vpos < 0)vpos = 0;
		break;
	case SB_THUMBPOSITION:
		vpos = HIWORD(wParam);//Get current position
		break;
	case SB_THUMBTRACK:
		vpos = HIWORD(wParam);//Get current position
		break;
	case SB_PAGEDOWN://Downwards
		vpos += 6;
		if(vpos > vScrollMax)vpos = vScrollMax;
		break;
	case SB_PAGEUP://Up
		vpos -= 6;
		if(vpos < 0)vpos = 0;
		break;
	}
	PrintHorzPosition();
	scr_info.fMask = SIF_POS;//nPosEnable
	scr_info.nPos = vpos;
	SetScrollInfo(hWnd,SB_VERT,&scr_info,1);
	org_data.PutMusic();
	RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
	//Below is a test
//	char str[80];
//	HDC hdc;
//	hdc = GetDC(hWnd);
//	sprintf(str,"Vertical:%4d",vpos);
//	TextOut(hdc,100,1,str,strlen(str));
//	ReleaseDC(hWnd,hdc);
}
//Acquire scroll position
void ScrollData::GetScrollPosition(long *hp,long *vp)
{
	*hp = hpos;
	*vp = vpos;
}

void ScrollData::WheelScrollProc(LPARAM lParam, WPARAM wParam){
	RECT rect = {0,0,WWidth,WHeight};//Area to update

	int fwKeys, zDelta, xPos,yPos;

	fwKeys = LOWORD(wParam);    // key flags  MK_CONTROL
	zDelta = (short) HIWORD(wParam);    // wheel rotation
	xPos = (short) LOWORD(lParam);    // horizontal position of pointer
	yPos = (short) HIWORD(lParam);    // vertical position of pointer

	/*
	switch(LOWORD(wParam)){
	case SB_LINEDOWN://Downwards
		vpos++;
		if(vpos > MAXVERTRANGE)vpos = MAXVERTRANGE;
		break;
	case SB_LINEUP://Up
		vpos--;
		if(vpos < 0)vpos = 0;
		break;
	case SB_THUMBPOSITION:
		vpos = HIWORD(wParam);//Get current position
		break;
	case SB_THUMBTRACK:
		vpos = HIWORD(wParam);//Get current position
		break;
	case SB_PAGEDOWN://Downwards
		vpos += 6;
		if(vpos > MAXVERTRANGE)vpos = MAXVERTRANGE;
		break;
	case SB_PAGEUP://Up
		vpos -= 6;
		if(vpos < 0)vpos = 0;
		break;
	}
	*/
	
	if(zDelta<0){
		if(fwKeys && MK_CONTROL){
			hpos -= 1;
			if(hpos < 0)hpos = 0;
			
		}else{
			vpos+=4;
			if(vpos > vScrollMax)vpos = vScrollMax;
		}
	}else{
		if(fwKeys && MK_CONTROL){
			hpos += 1;
			if(hpos > MAXHORZRANGE)hpos = MAXHORZRANGE;
			
		}else{
			vpos-=4;
			if(vpos < 0)vpos = 0;
		}
	}
	PrintHorzPosition();
	scr_info.fMask = SIF_POS;//nPosEnable
	scr_info.nPos = vpos;
	SetScrollInfo(hWnd,SB_VERT,&scr_info,1);
	scr_info.fMask = SIF_POS;//nPosEnable
	scr_info.nPos = hpos;
	SetScrollInfo(hWnd,SB_HORZ,&scr_info,1);

	org_data.PutMusic();
	RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);


}

void ScrollData::KeyScroll(int iDirection)
{
	RECT rect = {0,0,WWidth,WHeight};//Area to update
	switch(	iDirection ){
	case DIRECTION_UP:
		vpos-=4;
		break;
	case DIRECTION_DOWN:
		vpos+=4;
		break;
	case DIRECTION_LEFT:
		hpos -= 1;
		break;
	case DIRECTION_RIGHT:
		hpos += 1;
		break;
	}
	if(hpos < 0)hpos = 0;
	if(vpos > vScrollMax)vpos = vScrollMax;
	if(hpos > MAXHORZRANGE)hpos = MAXHORZRANGE;
	if(vpos < 0)vpos = 0;

	PrintHorzPosition();
	scr_info.fMask = SIF_POS;//nPosEnable
	scr_info.nPos = vpos;
	SetScrollInfo(hWnd,SB_VERT,&scr_info,1);
	scr_info.fMask = SIF_POS;//nPosEnable
	scr_info.nPos = hpos;
	SetScrollInfo(hWnd,SB_HORZ,&scr_info,1);

	org_data.PutMusic();
	RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);

}

