#include "Setting.h"
#include "DefOrg.h"
#include "OrgData.h"
#include "Mouse.h"
#include "Scroll.h"
#include <stdio.h>//sprintffor test
#include <string.h>//sprintffor test
#include "rxoFunction.h"
#include "resource.h"

//Processing when clicked
extern char timer_sw;//Reclaim switch
extern void SetUndo();
extern void ResetLastUndo(); //Take

extern NOTECOPY nc_Select; //Selection range
extern int tra, ful ,haba; 
extern void SetEZCWindowMessage(char *Mess);

long Last_mouse_x = -99999;	//Multiple clicks at the same position are not recorded as undo.
long Last_mouse_y = -99999;

long Last_VOL_Drag_mouse_x = -99999;	//Multiple clicks at the same position are not recorded as undo.
long Last_VOL_Drag_mouse_y = -99999;


long DragStartx = -99999;	//Drag starting point
long StartNotex = -99999;	//That note

long PutStartX = -99999;	//Drag start point of note

extern char TrackN[];
extern int sGrid;	//Range selection is on a grid level
extern int MinimumGrid(int x);
extern int MaximumGrid(int x);
void ShowStatusMessage(void);

extern void ChangeTrack(HWND hdwnd, int iTrack);

int iDragMode = 0;	//Extend notes with drag
int alt_down = 0;
int shift_down = -1;
long AdjustX = 0; //   Make note heads stand out at reduced display(&O)Correction for


void ClearDrag()
{
	DragStartx = -99999;
	StartNotex = -99999;
}

void RedrawClick()
{
	RECT rect = {64,0,WWidth,WHeight};
	org_data.PutMusic();//Redraw music
	RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);

}

void SelectReset()
{
	tra=-256;
	DragStartx = -99999;
	RedrawClick();
	SetEZCWindowMessage("");
}

void PrintStatusMessage(int Zensentaku){
	char wk[256],wk2[256];
	if(Zensentaku != 0){
		if(ful==0){
			wsprintf(wk2,MessageString[IDS_STRING104], TrackN[tra]); //"truck%c of"
			strcpy(wk,wk2);
			wsprintf(wk2,MessageString[IDS_STRING115]); // " From the beginning to the end"
			strcat(wk,wk2);
			SetEZCWindowMessage(wk);
		}
		else if(ful==1){
			wsprintf(wk2,MessageString[IDS_STRING116]); //"All tracks From the beginning to the end"
			strcpy(wk,wk2);
			SetEZCWindowMessage(wk);
		}
	}else{
		if(ful==0){
			wsprintf(wk2,MessageString[IDS_STRING104], TrackN[tra]); //"truck%c of"
			strcpy(wk,wk2);
			wsprintf(wk2,MessageString[IDS_STRING105]); // "Select the selected range"
			strcat(wk,wk2);
			SetEZCWindowMessage(wk);
		}
		else if(ful==1){
			wsprintf(wk2,MessageString[IDS_STRING106]); // "Select the selected range of all tracks"
			strcpy(wk,wk2);
			SetEZCWindowMessage(wk);
		}
	}

}

void SelectAll(int FullTrack)
{
	unsigned char line,dot;
	MUSICINFO mi;	org_data.GetMusicInfo(&mi);
	dot = mi.dot; line = mi.line;
	nc_Select.x1_1=0;
	nc_Select.x1_2=999*dot*line;
	tra = org_data.track;
	ful = FullTrack;
	PrintStatusMessage(1);
	RedrawClick();

}

void LButtonUP(WPARAM wParam, LPARAM lParam)
{
	RECT rect = {64,0,WWidth,WHeight};
	if(PutStartX != -99999){
		org_data.CheckNoteTail(org_data.track);
		PutStartX = -99999;
		org_data.PutMusic();//Redraw music
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);

	}
	if(DragStartx<=-99999)return;

	PrintStatusMessage(0);
	ClearDrag();
	org_data.PutMusic();//Redraw music
	ShowStatusMessage();
	RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
	Last_VOL_Drag_mouse_x = -99999; //2014.05.02 A
	Last_VOL_Drag_mouse_y = -99999;
	AdjustX = 0;
}
void RButtonUP(WPARAM wParam, LPARAM lParam)
{
	RECT rect = {64,0,WWidth,WHeight};
	Last_VOL_Drag_mouse_x = -99999; //2014.05.02 A
	Last_VOL_Drag_mouse_y = -99999;
	org_data.PutMusic();//Redraw music
	ShowStatusMessage();
	RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
	alt_down = 0; shift_down = -1;

}

void MouseDrag(WPARAM wParam, LPARAM lParam)
{
	RECT rect = {64,WHeight-16,WWidth,WHeight};
	RECT rect2 = {64,0,WWidth,WHeight};
	RECT rectPAN = {64, WHeight+292-WHNM - 3, WWidth, WHeight+(351+7)-WHNM + 3};	// 2010.08.14 

	if(timer_sw)return;
	long mouse_x;
	long mouse_y;
	long ptx;
	long scr_h,scr_v;
	long Note_x;
	unsigned char line,dot;
	scr_data.GetScrollPosition(&scr_h,&scr_v);
	MUSICINFO mi;
	org_data.GetMusicInfo(&mi);
	dot = mi.dot;
	line = mi.line;
	//Get mouse coordinates
	mouse_data.GetMousePosition(&mouse_x,&mouse_y);
	ptx = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
	
	long Ry;	// 2010.08.14 
	Ry = -12*(Last_mouse_y - 95 + scr_v);	// 2010.08.14 
	RECT rectNOTE = {64, Ry, WWidth, Ry + 12};	//Only near the current note	// 2010.08.14 

	if(DragStartx<=-99999){
		if(wParam & MK_LBUTTON){

			//if(PutStartX>-99999+1){ //Note drag 2010.09.23 D
			if(PutStartX>-99999+1 && ( 95 - (mouse_y/12 + scr_v)!=Last_mouse_y || (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot != Last_mouse_x)){ //Note drag //2010.09.23 A
				if(ptx < PutStartX){	// 2010.08.14 C
					ptx = PutStartX;	// 2010.08.14 C
				}
				if(NoteWidth != 16){
					AdjustX = org_data.SearchNoteC(PutStartX, (unsigned char)(95 - (mouse_y/12 + scr_v)), 16 / NoteWidth, 16 % NoteWidth); //2014.05.28
					if(AdjustX > 0) ptx -= AdjustX;
				}else AdjustX = 0;
				if( 95 - (mouse_y/12 + scr_v)!=Last_mouse_y && (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot == Last_mouse_x ){ // 2010.09.23 A yWith the axis only movementnoteI do not change the length of
					org_data.PutMusic();//Redraw music
					RedrawWindow(hWnd,&rectNOTE,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				}else{
					if(org_data.SetNote_onlyLength(PutStartX, ptx - PutStartX  + 1) == TRUE){ // 2010.08.14 C
						org_data.PutMusic();//Redraw music
						RedrawWindow(hWnd,&rectNOTE,NULL,RDW_INVALIDATE|RDW_ERASENOW);
						iLastEditNoteLength = ptx - PutStartX + 1;
					
					}
				}

			}

			if(mouse_y >= WHeight+292-WHNM && mouse_y < WHeight+(351+7)-WHNM){//Pan arrangement
				mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
				mouse_y = (WHeight+(351+5)-WHNM - mouse_y)/5;//96*12Is the vertical size of the score(Pixel)

				if(org_data.SetPan2(mouse_x,(unsigned char)mouse_y)==TRUE){ //Only existing notes are changed	// 2010.08.14 C
					org_data.PutMusic();//Redraw music
					RedrawWindow(hWnd,&rectPAN,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				}
			}else if(mouse_y >= WHeight+365-WHNM && mouse_y < WHeight+428-WHNM){//Volume placement
				mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
				mouse_y = (WHeight+428-WHNM - mouse_y)*4;//96*12Is the vertical size of the score(Pixel)
				if(Last_VOL_Drag_mouse_x > -99999){
					long xxx, xxxplusvalue, xxxredraw = 0;
					xxxplusvalue = (mouse_x >= Last_VOL_Drag_mouse_x) ? 1 : -1;
					for(xxx = Last_VOL_Drag_mouse_x; xxx != mouse_x; xxx += xxxplusvalue){ //Completely erase while mouse pointer is fast
						long yyy;
						if(mouse_x > Last_VOL_Drag_mouse_x){
							yyy = (mouse_y - Last_VOL_Drag_mouse_y) / (mouse_x - Last_VOL_Drag_mouse_x) * (xxx - Last_VOL_Drag_mouse_x) + Last_VOL_Drag_mouse_y;
						}else if(mouse_x < Last_VOL_Drag_mouse_x){
							yyy = (Last_VOL_Drag_mouse_y - mouse_y) / (Last_VOL_Drag_mouse_x - mouse_x) * (xxx - mouse_x) + mouse_y;
						}else yyy = mouse_y;
						if(org_data.SetVolume(xxx,(unsigned char)yyy)==TRUE){ 
							//org_data.PutMusic();//Redraw music
							xxxredraw++; //RedrawWindow(hWnd,&rectVOL,NULL,RDW_INVALIDATE|RDW_ERASENOW);
						}
					}
					Last_VOL_Drag_mouse_x = mouse_x; //2014.05.02 A
					Last_VOL_Drag_mouse_y = mouse_y;
					if(org_data.SetVolume(mouse_x,(unsigned char)mouse_y)==TRUE){	// 2010.08.14 C
						//org_data.PutMusic();//Redraw music
						xxxredraw++; //RedrawWindow(hWnd,&rectVOL,NULL,RDW_INVALIDATE|RDW_ERASENOW);
					}
					//if(xxxredraw){org_data.PutMusic(); RedrawWindow(hWnd,&rectVOL,NULL,RDW_INVALIDATE|RDW_ERASENOW);} //The redrawing system is put together.	// 2014.05.31 D
					if(xxxredraw){org_data.PutMusic(); RedrawWindow(hWnd,&rect2,NULL,RDW_INVALIDATE|RDW_ERASENOW);} //The redrawing system is put together.	// 2014.05.31 A
				}
			}
		}
		else if(wParam & MK_RBUTTON){
			if(mouse_y >= WHeight+365-WHNM && mouse_y < WHeight+428-WHNM){//Volume placement
				mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
				mouse_y = (WHeight+428-WHNM - mouse_y)*4;//96*12Is the vertical size of the score(Pixel)
				if(Last_VOL_Drag_mouse_x > -99999){ //2014.05.03 A //Drag starting point isVOLIf it is notSetUndoThere is also a possibility of being canceled.
					long xxx, xxxplusvalue, xxxredraw = 0;
					xxxplusvalue = (mouse_x >= Last_VOL_Drag_mouse_x) ? 1 : -1;
					for(xxx = Last_VOL_Drag_mouse_x; xxx != mouse_x; xxx += xxxplusvalue){ //Completely erase while mouse pointer is fast
						if(org_data.CutVolume(xxx,(unsigned char)mouse_y)==TRUE){ 
							//org_data.PutMusic();//Redraw music
							xxxredraw++; //RedrawWindow(hWnd,&rectVOL,NULL,RDW_INVALIDATE|RDW_ERASENOW);
						}
					}
					Last_VOL_Drag_mouse_x = mouse_x; //2014.05.02 A
					Last_VOL_Drag_mouse_y = mouse_y;
					if(org_data.CutVolume(mouse_x,(unsigned char)mouse_y)==TRUE){	// 2010.08.14 C
						//org_data.PutMusic();//Redraw music
						xxxredraw++; //RedrawWindow(hWnd,&rectVOL,NULL,RDW_INVALIDATE|RDW_ERASENOW);
					}
					//if(xxxredraw){org_data.PutMusic(); RedrawWindow(hWnd,&rectVOL,NULL,RDW_INVALIDATE|RDW_ERASENOW);} //The redrawing system is put together.	// 2014.05.31 D
					if(xxxredraw){org_data.PutMusic(); RedrawWindow(hWnd,&rect2,NULL,RDW_INVALIDATE|RDW_ERASENOW);} //The redrawing system is put together.	// 2014.05.31 A
				}
			}
		}
		if((!(wParam & MK_LBUTTON)) && (PutStartX != -99999)){
			//org_data.SetNote_afterSetLength(PutStartX);
			org_data.CheckNoteTail(org_data.track);
			PutStartX = -99999;
			org_data.PutMusic();//Redraw music
			RedrawWindow(hWnd,&rectNOTE,NULL,RDW_INVALIDATE|RDW_ERASENOW);

		}
		return;
	}
	if(wParam & MK_LBUTTON){
		//Left button drag
		Note_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ; //NotexCoordinate
		if(Note_x>StartNotex){
			if(sGrid || alt_down == 1){
				nc_Select.x1_1=MinimumGrid(StartNotex);
				nc_Select.x1_2=MaximumGrid(Note_x);
			}else if(alt_down == 2){
				nc_Select.x1_1=MinimumGridLine(StartNotex);
				nc_Select.x1_2=MaximumGridLine(Note_x);
			}else{
				nc_Select.x1_1=StartNotex;
				nc_Select.x1_2=Note_x;
			}
		}else if(Note_x<=StartNotex){ //turn over
			if(sGrid || alt_down == 1){
				nc_Select.x1_1=MinimumGrid(Note_x);
				nc_Select.x1_2=MaximumGrid(StartNotex);
			}else if(alt_down == 2){
				nc_Select.x1_1=MinimumGridLine(Note_x);
				nc_Select.x1_2=MaximumGridLine(StartNotex);
			}else{
				nc_Select.x1_1=Note_x;
				nc_Select.x1_2=StartNotex;
			}
		}else{
			nc_Select.x1_1=Note_x;
			nc_Select.x1_2=StartNotex;
		}
		org_data.RedrawSelectArea();
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);

	}



}

void ClickProcL(WPARAM wParam, LPARAM lParam)
{
	RECT rect = {64,0,WWidth,WHeight};
	long mouse_x;
	long mouse_y;
	long scr_h,scr_v;
	long Note_x;
	unsigned char line,dot;
	if(timer_sw)return;

	scr_data.GetScrollPosition(&scr_h,&scr_v);
	MUSICINFO mi;
	org_data.GetMusicInfo(&mi);
	dot = mi.dot;
	line = mi.line;
	PutStartX = -99999;
	Last_VOL_Drag_mouse_x = -99999;
	alt_down = (GetKeyState(VK_MENU)<0)? 1: ((GetKeyState(VK_APPS)<0)? 2: 0) ;
	shift_down = (GetKeyState(VK_TAB)<0)? 1: 0;

	//Get mouse coordinates
	mouse_data.GetMousePosition(&mouse_x,&mouse_y);

	//ShiftIf it is pressedSelect(Range selection)Treatment
	if(shift_down == 1 && mouse_y < WHeight+288-WHNM+144){
		mouse_y = WHeight+288-WHNM+144 + 8;
	}

	if(mouse_x < 64){
//		if(mouse_x >= 0 && mouse_y >= 0 && mouse_y < 288+WDWHEIGHTPLUS){//keyboard
		if(mouse_x >= 0 && mouse_y >= 0 && mouse_y < WHeight+288-WHNM){//keyboard
			org_data.TouchKeyboard(static_cast<unsigned char>(95 - (mouse_y/12 + scr_v)));//96*12Is the vertical size of the score
		}
		if(mouse_x >= 0 && mouse_y >= WHeight+288-WHNM+144 && mouse_y < WHeight+288-WHNM+144+16){//SelectSite 2014.05.01
			ChangeSelAlwaysCurrent();
			org_data.PutMusic();//Show score
			rect.left = 0; rect.right=64; rect.top = WHeight+288-WHNM+144; rect.bottom = WHeight+288-WHNM+144+16;
			RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
			return;			
		}
		if(mouse_x >= 0 && mouse_y >= WHeight+288-WHNM && mouse_y < WHeight+288-WHNM+72){//PAN Site 2014.05.01
			iActivatePAN = -iActivatePAN + 1;
			org_data.PutMusic();//Show score
			rect.left = 0; rect.right=64; rect.top = WHeight+288-WHNM; rect.bottom = WHeight+288-WHNM+72;
			RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
			return;			
		}
		if(mouse_x >= 0 && mouse_y >= WHeight+288-WHNM+72 && mouse_y < WHeight+288-WHNM+144){//VOL Site 2014.05.01
			iActivateVOL = -iActivateVOL + 1;
			org_data.PutMusic();//Show score
			rect.left = 0; rect.right=64; rect.top = WHeight+288-WHNM+72; rect.bottom = WHeight+288-WHNM+144 ;
			RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
			return;			
		}

		return;
	}
	
//	if(mouse_y >= 0 && mouse_y < 288+WDWHEIGHTPLUS){//Musical note placement
	if(mouse_y >= 0 && mouse_y < WHeight+288-WHNM){//Musical note placement
		//Convert acquired coordinates to score coordinates
		mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
		mouse_y = 95 - (mouse_y/12 + scr_v);//96*12Is the vertical size of the score(Pixel)
		if(mouse_y<0)return; //Out of range
		//Place notes
		SetUndo();
		if(iDragMode && (org_data.track<MAXMELODY))PutStartX = mouse_x; //Initial position memory
		if(NoteWidth != 16){
			AdjustX = org_data.SearchNoteC(mouse_x, (unsigned char)mouse_y, 16 / NoteWidth, 16 % NoteWidth); //2014.05.28
			if(AdjustX > 0) mouse_x -= AdjustX;
		}else AdjustX = 0;
		if(org_data.SetNote(mouse_x,(unsigned char)mouse_y,iDragMode)==FALSE)ResetLastUndo(); //Return it if you lose too long
		org_data.PutMusic();//Redraw music
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		ClearDrag();
//	}else if(mouse_y >= 292+WDWHEIGHTPLUS && mouse_y < 351+7+WDWHEIGHTPLUS){//Pan arrangement
	}else if(mouse_y >= WHeight+292-WHNM && mouse_y < WHeight+(351+7)-WHNM){//Pan arrangement
		mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
//		mouse_y = (351+5+WDWHEIGHTPLUS - mouse_y)/5;//96*12Is the vertical size of the score(Pixel)
		mouse_y = (WHeight+(351+5)-WHNM - mouse_y)/5;//96*12Is the vertical size of the score(Pixel)
		if((mouse_x != Last_mouse_x) ||(mouse_y != Last_mouse_y))SetUndo();
		org_data.SetPan(mouse_x,(unsigned char)mouse_y);
		org_data.PutMusic();//Redraw music
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		ClearDrag();
//	}else if(mouse_y >= 365+WDWHEIGHTPLUS && mouse_y < 428+WDWHEIGHTPLUS){//Volume placement
	}else if(mouse_y >= WHeight+365-WHNM && mouse_y < WHeight+428-WHNM){//Volume placement
		mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
//		mouse_y = (428+WDWHEIGHTPLUS - mouse_y)*4;//96*12Is the vertical size of the score(Pixel)
		mouse_y = (WHeight+428-WHNM - mouse_y)*4;//96*12Is the vertical size of the score(Pixel)
		if((mouse_x != Last_mouse_x) ||(mouse_y != Last_mouse_y))SetUndo();
		org_data.SetVolume(mouse_x,(unsigned char)mouse_y);
//		org_data.SetVolume2(mouse_x,(unsigned char)mouse_y,0); //Going out
		org_data.PutMusic();//Redraw music
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		Last_VOL_Drag_mouse_x = mouse_x; //2014.05.02 A
		Last_VOL_Drag_mouse_y = mouse_y;
		ClearDrag();
	}else if(mouse_y >=WHeight-13 && mouse_y<=WHeight-2){ //Selection range
		if((wParam & MK_SHIFT)&&(tra>=0)){ //Additional mode
			Note_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ; //NotexCoordinate
			if(Note_x>nc_Select.x1_1){
				if(sGrid || alt_down == 1){
					nc_Select.x1_2=MaximumGrid(Note_x);
				}else if(alt_down == 2){
					nc_Select.x1_2=MaximumGridLine(Note_x);
				}else{
					nc_Select.x1_2=Note_x;
				}
			}else{ //turn over
				if(sGrid || alt_down == 1){
					nc_Select.x1_1=MinimumGrid(Note_x);
				}else if(alt_down == 2){
					nc_Select.x1_1=MinimumGridLine(Note_x);
				}else{
					nc_Select.x1_1=Note_x;
				}
			}
		}else{
			//If shifting is not pressed, it is regarded as a starting point
			DragStartx = mouse_x;
			StartNotex = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ; //NotexCoordinate
			if(sGrid || alt_down == 1){ //Use grid
				nc_Select.x1_1 = MinimumGrid(StartNotex);
				nc_Select.x1_2 = MaximumGrid(StartNotex);
			}else if(alt_down == 2){
				nc_Select.x1_1 = MinimumGridLine(StartNotex);
				nc_Select.x1_2 = MaximumGridLine(StartNotex);
			}else{
				nc_Select.x1_1 = StartNotex;
				nc_Select.x1_2 = StartNotex;
			}
			tra = org_data.track;
			if(wParam & MK_CONTROL){ //If you press control all tracks
				ful=1;
			}else ful=0;
		}

		org_data.PutMusic();//Redraw music
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		ShowStatusMessage();
	}
	Last_mouse_x = mouse_x;
	Last_mouse_y = mouse_y;
	//Last_VOL_Drag_mouse_x = mouse_x; //2014.05.02 A
	//Last_VOL_Drag_mouse_y = mouse_y;

	//Below is a test
//	char str[80];
//	HDC hdc;
//	hdc = GetDC(hWnd);
//	sprintf(str,"ypos:%4d",mouse_y);
//	TextOut(hdc,200,100,str,strlen(str));
//	ReleaseDC(hWnd,hdc);
}
void ClickProcR(WPARAM wParam, LPARAM lParam)
{
	RECT rect = {64,0,WWidth,WHeight};
	long mouse_x;
	long mouse_y;
	long scr_h,scr_v;
	long AdjustX;
	unsigned char line,dot;
	int NextTrack;
	if(timer_sw)return;
	
	scr_data.GetScrollPosition(&scr_h,&scr_v);
	MUSICINFO mi;
	org_data.GetMusicInfo(&mi);
	dot = mi.dot;
	line = mi.line;
	//Get mouse coordinates
	mouse_data.GetMousePosition(&mouse_x,&mouse_y);

	if(GetKeyState(VK_TAB)<0){
		mouse_y = WHeight+288-WHNM+144 + 8;
	}

	SetUndo();

	if(mouse_x < 64)return;
	//Musical note cutting
//	if(mouse_y >= 0 && mouse_y < 288+WDWHEIGHTPLUS){
	if(mouse_y >= 0 && mouse_y < WHeight+288-WHNM){
		//Convert acquired coordinates to score coordinates
		mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
		mouse_y = 95  - (mouse_y/12 + scr_v);//96*12Is the vertical size of the score(Pixel)
		if(NoteWidth != 16){
			AdjustX = org_data.SearchNoteC(mouse_x, (unsigned char)mouse_y, 16 / NoteWidth, 16 % NoteWidth); //2014.05.28
			if(AdjustX > 0) mouse_x -= AdjustX;
		}

		//Place notes
		if(org_data.CutNote(mouse_x,(unsigned char)mouse_y)==FALSE){
			//There were no notes to delete.
			ResetLastUndo();
			NextTrack = org_data.SearchNote(mouse_x,(unsigned char)mouse_y,org_data.track);
			//MessageBox(NULL,"l",NULL,NULL);
			if(NextTrack>=0 && NextTrack!=org_data.track){
				//Track change
				ChangeTrack(hWnd, NextTrack);
			}
		}
		org_data.PutMusic();//Redraw music
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
//	}else if(mouse_y >= 292+WDWHEIGHTPLUS && mouse_y < 351+7+WDWHEIGHTPLUS){//Pan arrangement
	}else if(mouse_y >= WHeight+292-WHNM && mouse_y < WHeight+351+7-WHNM){//Pan arrangement
		mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
//		mouse_y = (351+5+WDWHEIGHTPLUS - mouse_y)/5;//96*12Is the vertical size of the score(Pixel)
		mouse_y = (WHeight+351+5-WHNM - mouse_y)/5;//96*12Is the vertical size of the score(Pixel)
		if(org_data.CutPan(mouse_x,(unsigned char)mouse_y)==FALSE)ResetLastUndo();
		org_data.PutMusic();//Redraw music
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		ClearDrag();
//	}else if(mouse_y >= 365+WDWHEIGHTPLUS && mouse_y < 428+WDWHEIGHTPLUS){//Volume placement
	}else if(mouse_y >= WHeight+365-WHNM && mouse_y < WHeight+428-WHNM){//Volume placement
		mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
//		mouse_y = (428+WDWHEIGHTPLUS - mouse_y)*4;//96*12Is the vertical size of the score(Pixel)
		mouse_y = (WHeight+428-WHNM - mouse_y)*4;//96*12Is the vertical size of the score(Pixel)
		//if(org_data.CutVolume(mouse_x,(unsigned char)mouse_y)==FALSE)ResetLastUndo(); //2014.05.03 D //It can not be undone if dragged as it is
		org_data.PutMusic();//Redraw music
		ClearDrag();
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		Last_VOL_Drag_mouse_x = mouse_x; //2014.05.02 A //this isVOLIt is used only.
		Last_VOL_Drag_mouse_y = mouse_y;
	}else if(mouse_y >=WHeight-13 && mouse_y<=WHeight-2){ //Selection range
		//DragStartx = mouse_x; //Drag starting point
		tra=-256;
		org_data.PutMusic();//Redraw music
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		SetEZCWindowMessage("");
		ClearDrag();
		ShowStatusMessage();
	}

	//Below is a test
//	char str[80];
//	HDC hdc;
//	hdc = GetDC(hWnd);
//	sprintf(str,"ypos:%4d",mouse_y);
//	TextOut(hdc,200,100,str,strlen(str));
//	ReleaseDC(hWnd,hdc);
}

void ClickProcM(WPARAM wParam, LPARAM lParam)
{
	RECT rect = {64,0,WWidth,WHeight};
	long mouse_x;
	long mouse_y;
	long scr_h,scr_v;
	unsigned char line,dot;
	int NextTrack, NextTrackB, iTrc;
	long x1, x2;
	if(timer_sw)return;
	
	scr_data.GetScrollPosition(&scr_h,&scr_v);
	MUSICINFO mi;
	org_data.GetMusicInfo(&mi);
	dot = mi.dot;
	line = mi.line;
	//Get mouse coordinates
	mouse_data.GetMousePosition(&mouse_x,&mouse_y);
	if(mouse_x < 64)return;

	if(mouse_y >= 0 && mouse_y < WHeight+288-WHNM){
		//Convert acquired coordinates to score coordinates
		mouse_x = (mouse_x - KEYWIDTH)/NoteWidth + scr_h*line*dot ;
		mouse_y = 95  - (mouse_y/12 + scr_v);//96*12Is the vertical size of the score(Pixel)


		NextTrack = org_data.SearchNote(mouse_x,(unsigned char)mouse_y,org_data.track);
		iTrc = (NextTrack >=0) ? NextTrack : org_data.track; 
		NextTrackB = org_data.SearchNoteB(mouse_x, (unsigned char)mouse_y , &x1, &x2, iTrc); 

		//MessageBox(NULL,"l",NULL,NULL);
		if(NextTrack>=0 && NextTrack!=org_data.track){
			//Track change
			//ChangeTrack(hWnd, NextTrack);
		}
		if(NextTrack<0 && NextTrackB>=0){
			NextTrack = NextTrackB;
		}
		if(NextTrack>=0){
			//Retrieve the start and end of a note
			
			//hereGridIgnore.
			nc_Select.x1_1 = x1;
			nc_Select.x1_2 = x2;
			tra = NextTrack;
			ful = (wParam & MK_CONTROL)? 1 : 0;
			PrintStatusMessage(0);
			org_data.RedrawSelectArea();
		}else{
			//Clear selection
			tra=-256;
			org_data.PutMusic();//Redraw music
			RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
			SetEZCWindowMessage("");
			ClearDrag();
			ShowStatusMessage();
		}

		org_data.PutMusic();//Redraw music
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
	}else if(mouse_y >= WHeight+292-WHNM && mouse_y < WHeight+351+7-WHNM){//Pan arrangement
	}else if(mouse_y >= WHeight+365-WHNM && mouse_y < WHeight+428-WHNM){//Volume placement
	}else if(mouse_y >=WHeight-13 && mouse_y<=WHeight-2){ //Selection range
	}
	//Below is a test
//	char str[80];
//	HDC hdc;
//	hdc = GetDC(hWnd);
//	sprintf(str,"ypos:%4d",mouse_y);
//	TextOut(hdc,200,100,str,strlen(str));
//	ReleaseDC(hWnd,hdc);
}
