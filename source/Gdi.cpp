#include "Setting.h"
#include "DefOrg.h"
#include "OrgData.h"

#define MAXBITMAP		64

HBITMAP hBmp[MAXBITMAP];
HBITMAP hbWork;//The so-called back buffer
HBITMAP hbMparts;//Sheet music work
HBITMAP hbPan;//Work for pan volume

extern int gDrawDouble;	//Draw both track groups

extern RECT WinRect; //For saving window size A 2010.09.22
extern int NoteWidth;
extern int NoteEnlarge_Until_16px;
//GDIInitialize
BOOL StartGDI(HWND hwnd)
{
	HDC hdc;//Device context
	BOOL status = FALSE;//Return value of this function
    int nDesktopWidth = GetSystemMetrics( SM_CXFULLSCREEN );
    int nDesktopHeight = GetSystemMetrics( SM_CYFULLSCREEN );	//Task bar consideration
    int nScreenWidth = GetSystemMetrics( SM_CXSCREEN );
    int nScreenHeight = GetSystemMetrics( SM_CYSCREEN );


	int nVirtualWidth = WinRect.right - WinRect.left;	//A 2010.09.22
	int nVirtualHeight =WinRect.bottom - WinRect.top;	//A 2010.09.22

	if(nVirtualWidth > nScreenWidth)nScreenWidth = nVirtualWidth;	//A 2010.09.22
	if(nVirtualHeight > nScreenHeight)nScreenHeight = nVirtualHeight;	//A 2010.09.22

	hdc = GetDC(hwnd);//DCGet
	//To create a back surface
	if((hbWork = CreateCompatibleBitmap(hdc,nScreenWidth,nScreenHeight)) == NULL){
		status = FALSE;
	}
	if((hbMparts = CreateCompatibleBitmap(hdc,nScreenWidth,144)) == NULL){
		status = FALSE;
	}
	if((hbPan = CreateCompatibleBitmap(hdc,nScreenWidth,144+16)) == NULL){
		status = FALSE;
	}
	ReleaseDC(hwnd,hdc);
	return(status);
}
//When resized (failed function)
BOOL ResizeGDI(HWND hwnd)
{
	if(hbWork != NULL)DeleteObject(hbWork);
	HDC hdc;//Device context
	BOOL status = FALSE;//Return value of this function

	hdc = GetDC(hwnd);//DCGet
	if((hbWork = CreateCompatibleBitmap(hdc,WWidth,WHeight)) == NULL){
		status = FALSE;
	}
	ReleaseDC(hwnd,hdc);
	return(status);

}
//GDIOpening
void EndGDI(void)
{
	int i;
	for(i = 0; i < MAXBITMAP; i++){
		if(hBmp[i] != NULL)DeleteObject(hBmp[i]);
	}
	if(hbWork != NULL)DeleteObject(hbWork);
	if(hbMparts != NULL)DeleteObject(hbMparts);
	if(hbPan != NULL)DeleteObject(hbPan);
}
//Load images(From resources)
HBITMAP InitBitmap(char *name,int no)
{
	hBmp[no] = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		name,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
	return hBmp[no];
}

//The so-called flip
void RefleshScreen(HDC hdc)
{

	HDC hdcwork;//Back of the surfaceDC
	HBITMAP hbold;//Save past handles
	
	hdcwork = CreateCompatibleDC(hdc);//DCGeneration
	hbold = (HBITMAP)SelectObject(hdcwork,hbWork);//Select back surface
	//display(Flip)
	BitBlt(hdc, 0, 0, WWidth, WHeight,hdcwork,0,0,SRCCOPY);
	SelectObject(hdcwork, hbold);//Restore selected objects
	DeleteDC(hdcwork);//Delete device context

}

void PutBitmap(long x,long y, RECT *rect, int bmp_no)
{
	HDC hdc,toDC,fromDC;
	HBITMAP toold,fromold;

	hdc = GetDC(hWnd);
	toDC   = CreateCompatibleDC(hdc);
	fromDC = CreateCompatibleDC(hdc);
	toold   = (HBITMAP)SelectObject(toDC,hbWork);
	fromold = (HBITMAP)SelectObject(fromDC,hBmp[bmp_no]);

	BitBlt(toDC,x,y,rect->right - rect->left,
		rect->bottom - rect->top,fromDC,rect->left,rect->top,SRCCOPY);//display

	SelectObject(toDC,toold);
	SelectObject(fromDC,fromold);
	DeleteDC(toDC);
	DeleteDC(fromDC);
	ReleaseDC(hWnd,hdc);
}

void PutBitmapCenter16(long x,long y, RECT *rect, int bmp_no) //Draw in the center 2014.05.26
{
	if(rect->right - rect->left != 16 || NoteWidth == 16){
		PutBitmap(x, y, rect, bmp_no);
		return;
	}
	HDC hdc,toDC,fromDC;
	HBITMAP toold,fromold;

	hdc = GetDC(hWnd);
	toDC   = CreateCompatibleDC(hdc);
	fromDC = CreateCompatibleDC(hdc);
	toold   = (HBITMAP)SelectObject(toDC,hbWork);
	fromold = (HBITMAP)SelectObject(fromDC,hBmp[bmp_no]);

	int ww = NoteWidth - 4;

	BitBlt(toDC,x     ,y,2   , rect->bottom - rect->top,  fromDC,  rect->left   ,rect->top,SRCCOPY);//display
	BitBlt(toDC,x+2   ,y,ww  , rect->bottom - rect->top,  fromDC,  rect->left+2 ,rect->top,SRCCOPY);//display
	BitBlt(toDC,x+2+ww,y,2   , rect->bottom - rect->top,  fromDC,  rect->left+14,rect->top,SRCCOPY);//display

	SelectObject(toDC,toold);
	SelectObject(fromDC,fromold);
	DeleteDC(toDC);
	DeleteDC(fromDC);
	ReleaseDC(hWnd,hdc);
}
///////////////////////////////////////////////
////After that, if it is unique function////////////////////////
///////////////////////////////////////////////
//Parts generation of score
bool MakeMusicParts(unsigned char line,unsigned char dot)
{
	if(line*dot==0)return false;
//	RECT m_rect[] = {
//		{  0,  0, 64,144},//keyboard
//		{ 64,  0, 80,144},//Bar line
//		{ 80,  0, 96,144},//One beat line
//		{ 96,  0,112,144},//1/16line
//	};
	HDC hdc,toDC,fromDC;
	HBITMAP toold,fromold;

	hdc = GetDC(hWnd);
	toDC   = CreateCompatibleDC(hdc);
	fromDC = CreateCompatibleDC(hdc);
	toold   = (HBITMAP)SelectObject(toDC,hbMparts);
	fromold = (HBITMAP)SelectObject(fromDC,hBmp[BMPMUSIC]);

	int x;
	if(org_data.track>=8)x=0;
	else x=0;

	for(int i = 0; i < (WWidth/NoteWidth)+15; i++){
		if(i%(line*dot) == 0)//line
//			BitBlt(toDC,i*16,0,16,192+WDWHEIGHTPLUS,fromDC,x+64,0,SRCCOPY);//display
			BitBlt(toDC,i*NoteWidth,0,NoteWidth,WHeight+192-WHNM,fromDC,x+64,0,SRCCOPY);//display
		else if(i%dot == 0)//Dashed line
			BitBlt(toDC,i*NoteWidth,0,NoteWidth,WHeight+192-WHNM,fromDC,x+64+16,0,SRCCOPY);//display
		else{
			if(NoteWidth>=8){
				BitBlt(toDC,i*NoteWidth,0,NoteWidth,WHeight+192-WHNM,fromDC,x+64+32,0,SRCCOPY);//display
			}else{
				BitBlt(toDC,i*NoteWidth,0,NoteWidth,WHeight+192-WHNM,fromDC,x+64+32+1,0,SRCCOPY);//display
			}
		}
	}

	SelectObject(toDC,toold);
	SelectObject(fromDC,fromold);
	DeleteDC(toDC);
	DeleteDC(fromDC);
	ReleaseDC(hWnd,hdc);
	return true;
}

void PutMusicParts(long x,long y)
{
	HDC hdc,toDC,fromDC;
	HBITMAP toold,fromold;

	hdc = GetDC(hWnd);
	toDC   = CreateCompatibleDC(hdc);
	fromDC = CreateCompatibleDC(hdc);
	toold   = (HBITMAP)SelectObject(toDC,hbWork);
	fromold = (HBITMAP)SelectObject(fromDC,hbMparts);

	BitBlt(toDC,x,y,WWidth,WHeight+192-WHNM,fromDC,0,0,SRCCOPY);//display

	SelectObject(toDC,toold);
	SelectObject(fromDC,fromold);
	DeleteDC(toDC);
	DeleteDC(fromDC);
	ReleaseDC(hWnd,hdc);
}
//Pan / volume line display
void PutPanParts(void)
{
	HDC hdc,toDC,fromDC;
	HBITMAP toold,fromold;

	hdc = GetDC(hWnd);
	toDC   = CreateCompatibleDC(hdc);
	fromDC = CreateCompatibleDC(hdc);
	toold   = (HBITMAP)SelectObject(toDC,hbWork);
	fromold = (HBITMAP)SelectObject(fromDC,hbPan);

	BitBlt(toDC,64,WHeight+288-WHNM,WWidth,WHeight+192-WHNM,fromDC,0,0,SRCCOPY);//display

	SelectObject(toDC,toold);
	SelectObject(fromDC,fromold);
	DeleteDC(toDC);
	DeleteDC(fromDC);
	ReleaseDC(hWnd,hdc);
}

void MakePanParts(unsigned char line,unsigned char dot)
{
//	RECT m_rect[] = {
//		{  0,  0, 64,144},//keyboard
//		{ 64,  0, 80,144},//Bar line
//		{ 80,  0, 96,144},//One beat line
//		{ 96,  0,112,144},//1/16line
//	};
	HDC hdc,toDC,fromDC;
	HBITMAP toold,fromold;

	hdc = GetDC(hWnd);
	toDC   = CreateCompatibleDC(hdc);
	fromDC = CreateCompatibleDC(hdc);
	toold   = (HBITMAP)SelectObject(toDC,hbPan);
	fromold = (HBITMAP)SelectObject(fromDC,hBmp[BMPPAN]);


//	for(int i = 0; i < 40; i++){
	for(int i = 0; i < (WWidth/NoteWidth)+15; i++){
		if(i%(line*dot) == 0)//line
			BitBlt(toDC,i*NoteWidth,0,NoteWidth,WHeight+192-WHNM+16,fromDC,64,0,SRCCOPY);//display
		else if(i%dot == 0)//Dashed line
			BitBlt(toDC,i*NoteWidth,0,NoteWidth,WHeight+192-WHNM+16,fromDC,64+16,0,SRCCOPY);//display
		else {
			if(NoteWidth>=8){
				BitBlt(toDC,i*NoteWidth,0,NoteWidth,WHeight+192-WHNM+16,fromDC,64+32,0,SRCCOPY);//display
			}else{
				BitBlt(toDC,i*NoteWidth,0,NoteWidth,WHeight+192-WHNM+16,fromDC,64+32+1,0,SRCCOPY);//display
			}
		}
	}

	SelectObject(toDC,toold);
	SelectObject(fromDC,fromold);
	DeleteDC(toDC);
	DeleteDC(fromDC);
	ReleaseDC(hWnd,hdc);
}

void PutSelectParts(void)
{
	HDC hdc,toDC,fromDC;
	HBITMAP toold,fromold;

	hdc = GetDC(hWnd);
	toDC   = CreateCompatibleDC(hdc);
	fromDC = CreateCompatibleDC(hdc);
	toold   = (HBITMAP)SelectObject(toDC,hbWork);
	fromold = (HBITMAP)SelectObject(fromDC,hbPan);

	BitBlt(toDC,64,WHeight-16,WWidth,WHeight,fromDC,0,144,SRCCOPY);//display

	SelectObject(toDC,toold);
	SelectObject(fromDC,fromold);
	DeleteDC(toDC);
	DeleteDC(fromDC);
	ReleaseDC(hWnd,hdc);

}

//The following is Tito special. It is used only when drawing notes.
HDC		Dw_hdc, Dw_toDC, Dw_fromDC;
HBITMAP Dw_toold, Dw_fromold;

void Dw_BeginToDraw(void)
{
	Dw_hdc = GetDC(hWnd);
	Dw_toDC   = CreateCompatibleDC(Dw_hdc);
	Dw_fromDC = CreateCompatibleDC(Dw_hdc);
	Dw_toold   = (HBITMAP)SelectObject(Dw_toDC,hbWork);
	Dw_fromold = (HBITMAP)SelectObject(Dw_fromDC,hBmp[BMPNOTE]);
	
}

void Dw_FinishToDraw(void)
{
	SelectObject(Dw_toDC,Dw_toold);
	SelectObject(Dw_fromDC,Dw_fromold);
	DeleteDC(Dw_toDC);
	DeleteDC(Dw_fromDC);
	ReleaseDC(hWnd,Dw_hdc);

}

void Dw_PutBitmap(long x,long y, RECT *rect, int bmp_no) //The last argument is no longer meaningless.
{
	if(NoteWidth == 16){
		BitBlt(Dw_toDC,x,y,rect->right - rect->left,
			rect->bottom - rect->top,Dw_fromDC,rect->left,rect->top,SRCCOPY);//display
	}else if(NoteWidth >= 4){ //In case of shortening
		int ww = NoteWidth - 4;
		BitBlt(Dw_toDC,x,y,2, rect->bottom - rect->top,
			Dw_fromDC,rect->left,rect->top,SRCCOPY);//display
		
		if(ww>0){
			BitBlt(Dw_toDC,x+2,y,ww, rect->bottom - rect->top,
				Dw_fromDC,rect->left + 2,rect->top,SRCCOPY);//display
		}
		BitBlt(Dw_toDC,x+2+ww,y,2, rect->bottom - rect->top,
			Dw_fromDC,rect->left + 14,rect->top,SRCCOPY);//display

	}
}

int Dw_PutBitmap_Head(long x,long y, RECT *rect, int bmp_no, int iNoteLength) //bmp_noIt is no longer meaningless. iLengthBe sure to1that&#39;s all. np->lengthSubstitute as it is.
{
	int iTotalLength = NoteWidth * iNoteLength;
	int bitWidth = iTotalLength; if(bitWidth > 16)bitWidth = 16;
	if(NoteEnlarge_Until_16px == 0){bitWidth = NoteWidth; iTotalLength = bitWidth;}

	if(NoteWidth == 16 || iTotalLength >= 16){
		BitBlt(Dw_toDC,x,y,rect->right - rect->left,
			rect->bottom - rect->top,Dw_fromDC,rect->left,rect->top,SRCCOPY);//display
		return 16;
	}else if(NoteWidth >= 4){ //In case of shortening
		int ww = bitWidth - 4;

		BitBlt(Dw_toDC,x,y,2, rect->bottom - rect->top,
			Dw_fromDC,rect->left,rect->top,SRCCOPY);//display
		
		if(ww>0){
			BitBlt(Dw_toDC,x+2,y,ww, rect->bottom - rect->top,
				Dw_fromDC,rect->left + 2,rect->top,SRCCOPY);//display
		}
		BitBlt(Dw_toDC,x+2+ww,y,2, rect->bottom - rect->top,
			Dw_fromDC,rect->left + 14,rect->top,SRCCOPY);//display

	}
	return bitWidth;
}

//PAN, VOLI am specializing in
void Dw_PutBitmap_Center(long x,long y, RECT *rect, int bmp_no) //The last argument is no longer meaningless.
{
	if(NoteWidth == 16){
		BitBlt(Dw_toDC,x,y,rect->right - rect->left,
			rect->bottom - rect->top,Dw_fromDC,rect->left,rect->top,SRCCOPY);//display
	}else if(NoteWidth >= 4){ //In case of shortening
//		int ww = (16 - NoteWidth) / 2;
//		BitBlt(Dw_toDC,x ,y,rect->right - rect->left - 2 * ww,
//			rect->bottom - rect->top,Dw_fromDC,rect->left + ww,rect->top,SRCCOPY);//display
		int ww = NoteWidth / 2;
		BitBlt(Dw_toDC,x ,y,
			ww,
			rect->bottom - rect->top,
			Dw_fromDC,rect->left ,rect->top,SRCCOPY);//display

		//BitBlt(Dw_toDC,x+ww-1 ,y-3,
		//	2,
		//	rect->bottom - rect->top,
		//	Dw_fromDC,rect->left+7 ,rect->top,SRCCOPY);//display

		BitBlt(Dw_toDC,x+ww ,y,
			ww,
			rect->bottom - rect->top,
			Dw_fromDC,rect->left+16-ww ,rect->top,SRCCOPY);//display

		BitBlt(Dw_toDC,x+ww-1 ,y,
			2,
			rect->bottom - rect->top,
			Dw_fromDC,rect->left+7 ,rect->top,SRCCOPY);//display

	}
}


