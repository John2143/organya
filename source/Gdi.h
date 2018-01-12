#include <windows.h>
#define MAXBITMAP		64

//void PutMusic(void);
//GDIInitialize
BOOL StartGDI(HWND hwnd);

BOOL ResizeGDI(HWND hwnd);

//GDIOpening
void EndGDI(void);
//Load images(From resources)
HBITMAP InitBitmap(char *name,int bmp_no);
//The so-called flip
void RefleshScreen(HDC hdc);
void PutBitmap(long x,long y, RECT *rect, int bmp_no);
void PutBitmapCenter16(long x,long y, RECT *rect, int bmp_no);
////////////////////////////////
///The following is a unique function//////////
bool MakeMusicParts(unsigned char line,unsigned char dot);
void PutMusicParts(long x,long y);
void PutPanParts(void);
void MakePanParts(unsigned char line,unsigned char dot);

void PutSelectParts(void);

//The following is Tito special. It is used only when drawing notes.
void Dw_BeginToDraw(void);
void Dw_FinishToDraw(void);
void Dw_PutBitmap(long x,long y, RECT *rect, int bmp_no);
int  Dw_PutBitmap_Head(long x,long y, RECT *rect, int bmp_no, int iNoteLength);
void Dw_PutBitmap_Center(long x,long y, RECT *rect, int bmp_no); //2014.05.26 A

