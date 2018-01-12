#include <windows.h>
//Window size(I omit the frame)
#define WINDOWWIDTH		1180
#define WDWHEIGHTPLUS	0
#define WINDOWHEIGHT	(432+WDWHEIGHTPLUS)
#define WHNM			(432 + 16)

#define KEYWIDTH		64

//bitmapNO
#define BMPMUSIC		0
#define BMPNOTE			1
#define BMPNUMBER		2
#define BMPPAN			3


#define MSGCANCEL		1
#define MSGEXISFILE		2	//Existing file exists
#define MSGSAVEOK		3	//Save completed
#define MSGLOADOK		3	//Load completed

extern char music_file[];
extern HINSTANCE hInst;//Instance handle
extern HWND hWnd;//Main window handle
extern BOOL actApp;//Window active

extern int  WWidth, WHeight;	//Window size