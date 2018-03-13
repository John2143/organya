#define _WIN32_WINNT	0x0400
#define WINVER			0x0400
#define _WIN32_WINDOWS	0x0410
#define _WIN32_IE		0x0400
#define BUF_SIZE 256
//#define HENKOU_NO_SHIRUSHI	" [Change ali]"

#define MAIN_WINDOW "WINDOW"
#define COMMON_WINDOW "COMMONDIALOG"
#define TRACK_WINDOW "TRACK"
#define PLAY_WINDOW "PLAYWINDOW"
#define COPY_WINDOW "COPYWINDOW"
#define INIT_DATA	"INIT_PAN_VOL"
#define MIDI_EXPORT	"STANDARD_MIDI_EXPORT"

#define CDCD_INIT 0
#define CDCD_EXIT 1
#define CDCD_LOAD 2

#include <windows.h>
#include <winuser.h>
#include "Shellapi.h"
#include "Mmsystem.h"
#include "malloc.h"

#include "Setting.h"
#include "DefOrg.h"
#include "resource.h"
#include "Gdi.h"
#include "OrgData.h"
#include "Scroll.h"
#include "Mouse.h"
#include "Click.h"
#include <stdio.h>
#include <string.h>
#include "Filer.h"
#include "PtcopOrg.h"
#include "rxoFunction.h"
#include "util.h"

#include "Sound.h"
#include "Timer.h"

//Main procedure
LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK DialogSetting(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogDefault(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogDelete(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogCopy(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogCopy2(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogPan(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogTrans(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogVolume(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogWave(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogPlayer(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogTrack(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogNoteUsed(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogMemo(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogHelp(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogEZCopy(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogSwap(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);

void SetTitlebarText(char *name);
void SetTitlebarChange(void);
void ResetTitlebarChange(void);
void CheckLoupeMenu(void);

//Declare a global variable here
HINSTANCE hInst;//Instance handle
HWND hWnd;//Main window handle
HWND hDlgPlayer;
HWND hDlgTrack;
HWND hDlgEZCopy;
extern int EZCopyWindowState; //Easy copy status
HWND hDlgHelp = NULL;
BOOL actApp;


int WWidth = WINDOWWIDTH, WHeight = WINDOWHEIGHT;

char lpszName[MAX_PATH+15];// = "Orgagna 2 - ";//Name to register on Windows

ORGDATA org_data;//Main data
SCROLLDATA scr_data;//Scroll data
MOUSEDATA mouse_data;//Mouse data
extern char music_file[];//File name
extern int sGrid;	//Range selection is on a grid level
extern int sACrnt;	//Range selection is always the current track

extern void ChangeTrack(HWND hdwnd, int iTrack);
extern void ChangeTrackPlus(HWND hdwnd, int iValue);
extern char timer_sw; //During the performance?
extern int EZCopyWindowState; //Easy copy status
extern void ClearEZC_Message(); //EZErase messages and ranges
extern RECT CmnDialogWnd;
extern int SaveWithInitVolFile;	//Song data and…Do you want to save?
extern int Menu_Recent[];
extern int iDragMode;
extern int iDlgRepeat; //Repeat count obtained from dialog
extern char strMIDI_TITLE[256];
extern char strMIDI_AUTHOR[256];
extern unsigned char ucMIDIProgramChangeValue[MAXTRACK];

//For saving window size
RECT WinRect;
CHAR *buf;
CHAR app_path[BUF_SIZE];
CHAR num_buf[BUF_SIZE];
//So far


void SaveIniFile();

//Opening flash
//#define WAITFLASH	256
//BOOL    CALLBACK DialogFlash(HWND, UINT, WPARAM, LPARAM);
int gWidthWindow;
int gHeightWindow;

int gDrawDouble;	//Draw both track groups

int iChgTrackKey[16] = {
	ID_AC_1,	ID_AC_2,	ID_AC_3,	ID_AC_4,	ID_AC_5,	ID_AC_6,	ID_AC_7,	ID_AC_8,
	ID_AC_Q,	ID_AC_W,	ID_AC_E,	ID_AC_R,	ID_AC_T,	ID_AC_Y,	ID_AC_U,	ID_AC_I,
};

int iMuteKey[16]={
	ID_AC_S1, ID_AC_S2, ID_AC_S3, ID_AC_S4, ID_AC_S5, ID_AC_S6, ID_AC_S7, ID_AC_S8,
	ID_AC_SQ, ID_AC_SW, ID_AC_SE, ID_AC_SR, ID_AC_ST, ID_AC_SY, ID_AC_SU, ID_AC_SI, 
};

TCHAR strSize[128]; //for Debug	// 2010.08.14 A

int iKeyPhase[128];
int iCurrentPhase;
int iCast[256];
int iPushShift[2];

int iKeyPushDown[256]; // 2010.09.22 A

char *strMIDIFile;

int CancelDeleteCurrentData(int iMessagePattern = 1){
	if(iChangeFinish!=0){	// A 2010.09.22
		char cc[512],*chn;
		GetWindowText(hWnd,cc,512);
		//chn = strstr(cc, HENKOU_NO_SHIRUSHI);	// 2014.10.19 D
		chn = strstr(cc, MessageString[IDS_MODIFIED]);	// 2014.10.19 A
		if(chn!=NULL){
			//When there is a change, confirm the end. // A 2010.09.22
			if(iMessagePattern == 0){
				//if(MessageBox(hWnd,"Unsaved content will be discarded. Do you want to initialize it?", "Initialization confirmation",MB_OKCANCEL | MB_ICONASTERISK)==IDCANCEL)return 1;	// 2014.10.19 D
				if(msgbox(hWnd,IDS_NOTIFY_INITIALIZE, IDS_NOTIFY_TITLE_INITALIZE,MB_OKCANCEL | MB_ICONASTERISK)==IDCANCEL)return 1;	// 2014.10.19 A
			}else if(iMessagePattern == 1){
				//if(MessageBox(hWnd,"Unsaved content will be discarded. Do you want to end it?", "Confirm completion",MB_OKCANCEL | MB_ICONASTERISK)==IDCANCEL)return 1;	// 2014.10.19 D
				if(msgbox(hWnd,IDS_NOTIFY_EXIT, IDS_NOTIFY_TITLE_EXIT,MB_OKCANCEL | MB_ICONASTERISK)==IDCANCEL)return 1;	// 2014.10.19 A
			}else if(iMessagePattern == 2){
				//if(MessageBox(hWnd,"Unsaved content will be discarded. Do you want to load the file?", "Load confirmation",MB_OKCANCEL | MB_ICONASTERISK)==IDCANCEL)return 1;	// 2014.10.19 D
				if(msgbox(hWnd,IDS_NOTIFY_LOAD, IDS_NOTIFY_TITLE_LOAD,MB_OKCANCEL | MB_ICONASTERISK)==IDCANCEL)return 1;	// 2014.10.19 A
			}
		}
	} 
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR dropfile,int nCmdShow)
{//Main function
	MSG msg;//Parameters used by this application	
	WNDCLASSEX wc;
//	MessageBox(hWnd,dropfile,"Drap",MB_OK);
	InitMMTimer();  // 2010.09.21
	strSize[0]=0;	// 2010.08.14 A
	for(int jjj=0;jjj<128;jjj++)iKeyPhase[jjj]=-1;
	iCurrentPhase=0;
	iPushShift[0]=0;
	iPushShift[1]=0;
	int i, vvv;
	for(vvv=0;vvv<256;vvv++){
		iCast[vvv]=0xFFFF;
		iKeyPushDown[vvv]=0;
	}

	//Load message string
	AllocMessageStringBuffer();

	//Initial file name
    strcpy(music_file, MessageString[IDS_DEFAULT_ORG_FILENAME]);

	iCast['Z']= 33;
	iCast['S']= 34;
	iCast['X']= 35;
	iCast['C']= 36; //C … Csound of
	iCast['F']= 37;
	iCast['V']= 38; //     D
	iCast['G']= 39;
	iCast['B']= 40; //     E
	iCast['N']= 41; //     F
	iCast['J']= 42;
	iCast['M']= 43; //     G
	iCast['K']= 44;
	iCast[0xBC]=45; //,    A
	iCast['L']= 46;
	iCast[0xBE]=47; //.    B
	iCast[0xBF]=48; ///   C
	iCast[0xBA]=49; //:
	iCast[0xE2]=50; //￥
	iCast[0xDD]=51; //]
	strMIDIFile = (char *)malloc(MAX_PATH);

	HACCEL Ac; //For shortcut key

	LoadString(GetModuleHandle(NULL), IDS_TITLE, lpszName, sizeof(lpszName) / sizeof(lpszName[0]));

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;//CS_DBLCLKS| CS_OWNDC;//Application style
	wc.lpfnWndProc   = (WNDPROC)WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInst = hInstance;
	wc.hIcon         = LoadIcon(hInst,"ICO");//Large icon
	wc.hIconSm       = LoadIcon(hInst,"ICO");//Small icon
	wc.hCursor       = LoadCursor(hInst,"CURS");//cursor
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//Wind dash color
	wc.lpszMenuName  = "ORGANYAMENU";//Meni	Ue
	wc.lpszClassName = lpszName;

	int wnd_width;///hereWindowSpecify the width of the object.
	int wnd_height;
	gWidthWindow = wnd_width = GetSystemMetrics(SM_CXFRAME)*2+//Frame width
		GetSystemMetrics(SM_CXHSCROLL)+//Scroll bar width
		WWidth;
	gHeightWindow = wnd_height = GetSystemMetrics(SM_CYFRAME)*2 +//Frame Height
		GetSystemMetrics(SM_CYCAPTION)+//Caption Height
		GetSystemMetrics(SM_CYMENU)+//Menu bar height
		GetSystemMetrics(SM_CYVSCROLL)+//Scroll bar height
		WHeight;

	if(!RegisterClassEx(&wc)) return FALSE;
	
	GetModuleFileName(hInstance, app_path,BUF_SIZE - 1);
	buf=app_path + lstrlen(app_path) - 4;
	if(lstrcmpi(buf,".exe")==0){
		lstrcpy(buf,".ini");
	}else{
		lstrcat(app_path,".ini");
	}
	//NoteWidth = 16; //Width specification★
	NoteWidth =         GetPrivateProfileInt(MAIN_WINDOW,"NoteWidth",16,app_path);
	NoteWidth = (NoteWidth > 16) ? 16: ( (NoteWidth<4) ? 4: NoteWidth );

	WinRect.left=       GetPrivateProfileInt(MAIN_WINDOW,"left",0,app_path);
	WinRect.top=        GetPrivateProfileInt(MAIN_WINDOW,"top",0,app_path);
	WinRect.right=      GetPrivateProfileInt(MAIN_WINDOW,"right",560,app_path);
	WinRect.bottom=     GetPrivateProfileInt(MAIN_WINDOW,"bottom",560,app_path);
	CmnDialogWnd.left=	GetPrivateProfileInt(COMMON_WINDOW,"left",	20,app_path);
	CmnDialogWnd.top=	GetPrivateProfileInt(COMMON_WINDOW,"top",	20,app_path);
	CmnDialogWnd.right=	GetPrivateProfileInt(COMMON_WINDOW,"right",	550,app_path);
	CmnDialogWnd.bottom=GetPrivateProfileInt(COMMON_WINDOW,"bottom",560,app_path);
	iDlgRepeat =        GetPrivateProfileInt(MIDI_EXPORT,"Repeat",1,app_path);

	char strauthtmp[128];
	SYSTEMTIME stTime; GetLocalTime(&stTime); //stTime.wYear Year acquired	// 2014.10.18
	strcpy(strauthtmp, "(C) AUTHOR xxxxx,                 ");
	sprintf(strauthtmp + 18, "%d", stTime.wYear); //,Put the Christian calendar into the back of

	//GetPrivateProfileString(MIDI_EXPORT, "Author", "(C) AUTHOR xxxxx, 2014", strMIDI_AUTHOR, 255, app_path);	// 2045.01.18 D
	GetPrivateProfileString(MIDI_EXPORT, "Author", strauthtmp, strMIDI_AUTHOR, 255, app_path);	// 2045.01.18 A
	GetPrivateProfileString(MIDI_EXPORT, "Title", MessageString[IDS_DEFAULT_MIDI_TITLE], strMIDI_TITLE, 255, app_path);
	for(i=0;i<8;i++)ucMIDIProgramChangeValue[i]=255;

	//SetWindowPos(hWnd,HWND_TOP,WinRect.left,WinRect.top,WinRect.right,WinRect.bottom,SWP_HIDEWINDOW);

	unsigned long ul;
	ul = WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU|WS_THICKFRAME|WS_MAXIMIZEBOX;

	//Generate main window
	hWnd = CreateWindow(lpszName,
			"Orgagna",//Is displayed"name"
			ul,
			//WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU|WS_THICKFRAME|WS_MAXIMIZEBOX,
//            WS_CAPTION|WS_VISIBLE|WS_SYSMENU,//Window style
/*
            32,//WindowofX
			32,//WindowofY
            wnd_width,//width
            wnd_height,//height
			*/
			WinRect.left, WinRect.top, WinRect.right, WinRect.bottom,
            NULL, NULL, hInst, NULL);
	if(!hWnd) return FALSE;

//	DialogBox(hInst,"DLGFLASH",NULL,DialogFlash);

	Ac=LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1)); //access key

//Initialize image//////////
	StartGDI(hWnd);//GDIPreparation
	InitBitmap("MUSIC",BMPMUSIC);//Piano roll
	InitBitmap("NOTE",BMPNOTE);//note
	InitBitmap("NUMBER",BMPNUMBER);//A number
	InitBitmap("PAN",BMPPAN);//Bread and volume
//Initialize sound///////
	InitDirectSound(hWnd);
//	LoadWaveData100(); //File"Wave.dat"Load from
	InitWaveData100(); //Registry"WAVE100"Load from
	scr_data.InitScroll();
	
	hDlgPlayer = CreateDialog(hInst,"PLAYER",hWnd,DialogPlayer);
	hDlgTrack = CreateDialog(hInst,"TRACK",hWnd,DialogTrack);
	hDlgEZCopy = CreateDialog(hInst,"COPYBD",hWnd,DialogEZCopy);
	//hDlgShortCutList = CreateDialog(hInst,"DLGSHORTCUTINFO",hWnd,DialogShortCut);
	WinRect.left=GetPrivateProfileInt(TRACK_WINDOW,"left",200,app_path);
	WinRect.top=GetPrivateProfileInt(TRACK_WINDOW,"top",200,app_path);
	SetWindowPos(hDlgTrack,NULL,WinRect.left,WinRect.top,0,0,SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	WinRect.left=GetPrivateProfileInt(PLAY_WINDOW,"left",280,app_path);
	WinRect.top=GetPrivateProfileInt(PLAY_WINDOW,"top",280,app_path);
	SetWindowPos(hDlgPlayer,NULL,WinRect.left,WinRect.top,0,0,SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	WinRect.left=GetPrivateProfileInt(COPY_WINDOW,"left",180,app_path);
	WinRect.top=GetPrivateProfileInt(COPY_WINDOW,"top",380,app_path);
	SetWindowPos(hDlgEZCopy,NULL,WinRect.left,WinRect.top,0,0,SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	EZCopyWindowState = GetPrivateProfileInt(COPY_WINDOW,"show",1,app_path);
	if(EZCopyWindowState==0)ShowWindow(hDlgEZCopy, SW_HIDE);
	SaveWithInitVolFile = GetPrivateProfileInt(INIT_DATA,"autosave",0,app_path);
	ChangeAutoLoadMode(SaveWithInitVolFile);
	org_data.InitOrgData();

	
	CheckLoupeMenu();
	LoadRecentFromIniFile();
	ChangeGridMode(GetPrivateProfileInt(MAIN_WINDOW,"GridMode",0,app_path));
	ChangeSelAlwaysCurrent(GetPrivateProfileInt(MAIN_WINDOW,"AlwaysCurrent",0,app_path));
	ChangeDrawDouble(GetPrivateProfileInt(MAIN_WINDOW,"DrawDouble",1,app_path));
	ChangeDragMode(GetPrivateProfileInt(MAIN_WINDOW,"DragMode",1,app_path));
	ChangeEnablePlaying(GetPrivateProfileInt(MAIN_WINDOW,"EnablePlaying",1,app_path));
	ChangeFinish(GetPrivateProfileInt(MAIN_WINDOW,"QuitMessage",0,app_path));
	ChangeSlideOverlapNoteMode(GetPrivateProfileInt(MAIN_WINDOW,"SlideOverlapNoteMode",1,app_path));
	ChangePushStratchNOTE(GetPrivateProfileInt(MAIN_WINDOW,"EnablePressNoteStretch",1,app_path));
	ChangeNoteEnlarge(GetPrivateProfileInt(MAIN_WINDOW,"NoteEnlarge",1,app_path)); //2014.05.28 Make note heads stand out at reduced display(&O)
	
	org_data.PutMusic();//Show score

	if(GetPrivateProfileInt(MAIN_WINDOW,"WindowState",0,app_path)==1){
		ShowWindow(hWnd,SW_MAXIMIZE);
	}else{
		ShowWindow(hWnd,nCmdShow);//Main window display
	}
	UpdateWindow(hWnd);//Main window update

	DragAcceptFiles(hWnd,TRUE);//You can now drag
	//Player dialog(Modales)Generation

	//if(GetPrivateProfileInt(MAIN_WINDOW,"WindowState",0,app_path)==1){
	//	ShowWindow(hWnd,SW_MAXIMIZE);
	//}

	SetTitlebarText(music_file);//Title name set
	ClearVirtualCB(); //Virtual Clipboard initialization
	ClearUndo();

			//Show to player
	MUSICINFO mi;
			org_data.GetMusicInfo( &mi );
			SetDlgItemInt(hDlgTrack,IDE_VIEWWAIT,mi.wait,TRUE );
			//SetDlgItemInt(hDlgTrack,IDE_VIEWTRACK,0,TRUE );
			SetDlgItemText(hDlgTrack,IDE_VIEWTRACK,"1");
	FILE *fp;
	char kfn[MAX_PATH],gfn[MAX_PATH];
	if(dropfile[0]!=0){
		strcpy(kfn,dropfile);
		int ttt;
		if(dropfile[0]=='"'){	//Remove quotes
			ttt = 1;
		}else{
			ttt = 0;
		}
		strcpy(gfn,&kfn[ttt]);
		int ggg = strlen(gfn)-ttt;
		gfn[ggg]=0;
		//MessageBox(hWnd,gfn,"Error(Load)",MB_OK);
		fp=fopen(gfn,"rb");
		if(fp==NULL){
			//MessageBox(hWnd,"Failed to read","Error(Load)",MB_OK); //D 2010.09.28
		}else{
			char pass_chek[3];
			bool b_OrgFile = false;
			fread(&pass_chek[0], sizeof(char), 3, fp);
			b_OrgFile = (pass_chek[0]==0x4F) && (pass_chek[1]==0x72) && (pass_chek[2]==0x67); //'O', 'r', 'g'
			fclose(fp);
			//memcpy(music_file,dropfile,MAX_PATH);
			if(b_OrgFile){ //A 2010.09.25 SomehowOrgIf it was a format file
				strcpy(music_file, gfn);
				if(org_data.LoadMusicData()==TRUE){ //C 2010.09.25 Add judgment
					SetTitlebarText(music_file);//Title name set
					org_data.GetMusicInfo( &mi );
					SetDlgItemInt(hDlgTrack,IDE_VIEWWAIT,mi.wait,TRUE );
					SetDlgItemText(hDlgTrack,IDE_VIEWTRACK,"1");
					ClearEZC_Message();	SelectReset();	org_data.PutMusic();
				}else{
					//ORGBecause it was not a format file //A 2010.9.25
					//File name clear
					strcpy(music_file, MessageString[IDS_DEFAULT_ORG_FILENAME]);
				}
			}
		}
	}
	QuitMMTimer(); //A 2010.09.21
	//Generate message loop(Main loop)
	while(GetMessage(&msg,NULL,0,0)){
//		if(!TranslateAccelerator(hwnd,hAccel,&msg)){
		//If it is not a message for dialog
		if(!TranslateAccelerator(hWnd,Ac,&msg))
        {
			if(!IsDialogMessage(hDlgPlayer,&msg)){
				if(!IsDialogMessage(hDlgTrack,&msg)){
					if(!IsDialogMessage(hDlgEZCopy,&msg)){
						if(!IsDialogMessage(hDlgHelp,&msg)){
							TranslateMessage(&msg);//Keyboard available
							DispatchMessage(&msg);//ControlWindowsReturn to
						}
					}				
				}
			}
				//TranslateMessage(&msg);
				//DispatchMessage(&msg);
        }
	}
	//MessageBox(NULL, "I left the message loop", "OK", MB_OK);

	DestroyAcceleratorTable (Ac);
	return msg.wParam;//The application ends here
}
//Main procedure
LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
//	char str[64];
	int i, j;	// 2014.10.18 jAdd
	char res;
	bool AfterReSize=false;
	RECT rect = {0,0,WWidth,WHeight};//Area to update(Track change)
	MUSICINFO mi;
	MINMAXINFO *pmmi;

	


//	char name[MAX_PATH];	// 2010.12.01 MOVED
//	strcpy(name, MessageString[IDS_DEFAULT_ORG_FILENAME]);
	char str[128];
	
	switch(message){
	case WM_COMMAND:
		for(i=0;i<16;i++){
			if(LOWORD(wParam) == iChgTrackKey[i]){
				ChangeTrack(hDlgTrack,i);
				//return FALSE;
				//SendDlgItemMessage(hDlgTrack , IDC_TRACK0 , BM_CLICK , 0, 0);
			}
		}
		for(i=0;i<16;i++)if(LOWORD(wParam) == iMuteKey[i]){MuteTrack(i);
		//return FALSE;
		}
		if(LOWORD(wParam)==IDM_EZCOPYVISIBLE || LOWORD(wParam)==ID_AC_SHOWEZCOPY){
			if(EZCopyWindowState==0){
				EZCopyWindowState=1;
				ShowWindow(hDlgEZCopy, SW_SHOWNOACTIVATE);
			}else{
				EZCopyWindowState=0;
				ShowWindow(hDlgEZCopy, SW_HIDE);

			}
		}
		for(i=0;i<10;i++){	//Recent file
			if(LOWORD(wParam)==Menu_Recent[i]){
				if(CancelDeleteCurrentData(CDCD_LOAD))break;
				SetLoadRecentFile(i);	org_data.InitOrgData();	org_data.LoadMusicData();
				SetTitlebarText(music_file);//Title name set
				org_data.GetMusicInfo( &mi );
				SetDlgItemInt(hDlgTrack,IDE_VIEWWAIT,mi.wait,TRUE );
				SetDlgItemText(hDlgTrack,IDE_VIEWTRACK,"1");
				ClearEZC_Message();	SelectReset();	org_data.PutMusic();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				//for(i=0;i<8;i++)ucMIDIProgramChangeValue[i]=255;	// 2014.10.18 D
				for(j=0;j<8;j++)ucMIDIProgramChangeValue[j]=255;	// 2014.10.18 A
				break;	// 2014.10.18 A
			}
		}
		if(timer_sw==0){ //I do not like playing
			switch(LOWORD(wParam)){
			case ID_AC_LOAD_MOST_RECENT:
				SendMessage(hWnd, WM_COMMAND, Menu_Recent[0],0);
				break;
			case IDM_SORTMUSICNOTE: //To sort
				SetUndo();
				SortMusicNote();
				break;
			case IDM_DLGSETTING://Display setting dialog
			case ID_AC_SETTEMPO:
				DialogBox(hInst,"DLGSETTING",hwnd,DialogSetting);
				break;
			case IDM_DLGDEFAULT://Show default dialog
			case ID_AC_DEFAULT:
				DialogBox(hInst,"DLGDEFAULT",hwnd,DialogDefault);
				break;
			case IDM_DLGDELETE://
			case ID_AC_DELETE:
				DialogBox(hInst,"DLGDELETE",hwnd,DialogDelete);
				break;
			case IDM_DLGCOPY://
			case ID_AC_COPY:
				DialogBox(hInst,"DLGCOPY",hwnd,DialogCopy);
				break;
			case IDM_DLGCOPY2://
			case ID_AC_COPY2:
				DialogBox(hInst,"DLGCOPY2",hwnd,DialogCopy2);
				break;
			case IDM_DLGPAN://
			case ID_AC_DLG_PAN:
				DialogBox(hInst,"DLGPAN",hwnd,DialogPan);
				break;
			case IDM_DLGTRANS://
			case ID_AC_DLG_TRANS:
				DialogBox(hInst,"DLGTRANS",hwnd,DialogTrans);
				break;
			case ID_AC_STPLAY:
				SendMessage(hDlgPlayer , WM_COMMAND , IDC_PLAY , 0);
				break;
			case ID_AC_STBACK:
				SendMessage(hDlgPlayer , WM_COMMAND , IDC_START , 0);
				break;
			case IDM_DLGVOL://
			case ID_AC_DLG_VOL:
				DialogBox(hInst,"DLGVOLUME",hwnd,DialogVolume);
				break;
			case IDM_DLGUSED://
				DialogBox(hInst,"DLGUSED",hwnd,DialogNoteUsed);
				break;
			case IDM_DLGWAVE://Display setting dialog
			case ID_AC_WAVESELECT:
				DialogBox(hInst,"DLGWAVE",hwnd,DialogWave);
				break;
			case IDM_DLGSWAP:
			case ID_AC_DLG_SWAP:
				DialogBox(hInst,"DLGSWAP",hwnd,DialogSwap);
				break;
			case IDM_DLGMEMO://
				PlaySound( "CAT" , GetModuleHandle(NULL),SND_RESOURCE | SND_ASYNC); 
				DialogBox(hInst,"DLGMEMO",hwnd,DialogMemo);
				break;
			case IDM_DLGHELP://
			case ID_AC_HELP:
				//LoadFromResource(IDR_HELPHTML);
				if(!hDlgHelp){
					hDlgHelp = CreateDialog(hInst,"DLGHELP",hwnd,DialogHelp);
				}
				ShowWindow(hDlgHelp, SW_SHOWNOACTIVATE);
				//DialogBox(hInst,"DLGHELP",hwnd,DialogHelp);
				break;
			case IDM_SAVEOVER:
			case ID_AC_MENUOVERSAVE:
				org_data.SaveMusicData();
				ResetTitlebarChange();
				break;							  
			case IDM_SAVENEW://Save as a different name
			case ID_AC_MENUNEWSAVE:
				res = GetFileNameSave(hwnd,MessageString[IDS_STRING62]); //"Save as a different name"
				if(res == MSGCANCEL)break;
				if(res == MSGEXISFILE){
					//if(MessageBox(hwnd,"Do you want to overwrite?","There is a file with the same name",MB_YESNO | MB_ICONEXCLAMATION)	// 2014.10.19 D
					if(msgbox(hwnd,IDS_NOTIFY_OVERWRITE,IDS_INFO_SAME_FILE,MB_YESNO | MB_ICONEXCLAMATION)	// 2014.10.19 A
						==IDNO)break;
				}
				org_data.SaveMusicData();
				SetTitlebarText(music_file);//Title name set
				ResetTitlebarChange();
				break;
			case IDM_IMPORT_PTCOP:
			case ID_AC_PTCOP:
				if(CancelDeleteCurrentData(CDCD_LOAD))break;

				if(GetFileNameLoadPtcop(hWnd,MessageString[IDS_STRING119]) != MSGLOADOK)break;//"Reading song data"
				
				ClearUndo();
				//this function gives printf insane values for some reason, but only half way through printing all the values
				
				org_data.InitOrgData();
				LoadPtcopData();
				
				SetTitlebarText(music_file);//Title name set
				//Show to player
				org_data.GetMusicInfo( &mi );
				SetDlgItemInt(hDlgTrack,IDE_VIEWWAIT,mi.wait,TRUE );
				//SetDlgItemInt(hDlgTrack,IDE_VIEWTRACK,0,TRUE );
				SetDlgItemText(hDlgTrack,IDE_VIEWTRACK,"1");

				ClearEZC_Message();
				SelectReset();
				org_data.PutMusic();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				for(i=0;i<8;i++)ucMIDIProgramChangeValue[i]=255;
				break;
			case IDM_EXPORT_MIDI: //Export 2014.05.11
			case ID_AC_MIDI:
				
				res = GetFileNameMIDI(hwnd,MessageString[IDS_STRING63], strMIDIFile );//"standardMIDIExport in format"
				if(res == MSGCANCEL)break;
				if(res == MSGEXISFILE){
					//if(MessageBox(hwnd,"Do you want to overwrite?","There is a file with the same name",MB_YESNO | MB_ICONEXCLAMATION)	// 2014.10.19 D
					if(msgbox(hwnd,IDS_NOTIFY_OVERWRITE,IDS_INFO_SAME_FILE,MB_YESNO | MB_ICONEXCLAMATION)	// 2014.10.19 A
						==IDNO)break;
				}
				org_data.ExportMIDIData(strMIDIFile, iDlgRepeat);
				//SetTitlebarText(music_file);//Title name set
				//ResetTitlebarChange();
				break;
			case IDM_DUMMY_TATE_SEPARATOR: //do nothing
				break;
			case IDM_LOAD:
			case ID_AC_MENUOPEN:
			case IDM_LOAD2:
			case ID_AC_LOAD2:
				if(CancelDeleteCurrentData(CDCD_LOAD))break;
				i = 0;
				if(LOWORD(wParam)==IDM_LOAD2 || LOWORD(wParam)==ID_AC_LOAD2)i=1; 
				if(GetFileNameLoad(hWnd,MessageString[IDS_STRING61],i) != MSGLOADOK)break;//"Reading song data"
				
				ClearUndo();
				org_data.InitOrgData();
				org_data.LoadMusicData();
				SetTitlebarText(music_file);//Title name set
				//Show to player
				org_data.GetMusicInfo( &mi );
				SetDlgItemInt(hDlgTrack,IDE_VIEWWAIT,mi.wait,TRUE );
				//SetDlgItemInt(hDlgTrack,IDE_VIEWTRACK,0,TRUE );
				SetDlgItemText(hDlgTrack,IDE_VIEWTRACK,"1");

				ClearEZC_Message();
				SelectReset();
				org_data.PutMusic();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				for(i=0;i<8;i++)ucMIDIProgramChangeValue[i]=255;
				break;
			case IDM_EXIT:
				//if(iChangeFinish!=0){	// A 2010.09.22
				//	char cc[512],*chn;
				//	GetWindowText(hWnd,cc,512);
				//	chn = strstr(cc, HENKOU_NO_SHIRUSHI);
				//	if(chn!=NULL){
				//		//When there is a change, confirm the end. // A 2010.09.22
				//		if(MessageBox(hwnd,"Unsaved content will be discarded. Do you want to end it?","Confirm completion",MB_OKCANCEL | MB_ICONASTERISK)==IDCANCEL)break;
				//	}
				//}
				if(CancelDeleteCurrentData(CDCD_EXIT))break;
				SaveIniFile();
				QuitMMTimer();
				PostQuitMessage(0);
				EndDirectSound();
				org_data.ReleaseNote();
				EndGDI();
				if(!hDlgPlayer)DestroyWindow(hDlgPlayer);
				if(!hDlgTrack)DestroyWindow(hDlgTrack);
				if(!hDlgEZCopy)DestroyWindow(hDlgEZCopy);
				if(!hDlgHelp)DestroyWindow(hDlgHelp);
				
				if(!hwnd)DestroyWindow(hwnd);
				PostQuitMessage(0);
				break;
			case IDM_2BAI:
				SetUndo();
				org_data.EnlargeAllNotes(2);
				scr_data.SetHorzScroll(0);org_data.SetPlayPointer(0);SetFocus(hWnd);//Cueing
				org_data.PutMusic();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				org_data.GetMusicInfo( &mi );
				itoa(mi.wait,str,10);
				SetDlgItemText(hDlgTrack,IDE_VIEWWAIT,str);
				break;
			case IDM_3BAI:
				SetUndo();
				org_data.EnlargeAllNotes(3);
				scr_data.SetHorzScroll(0);org_data.SetPlayPointer(0);SetFocus(hWnd);//Cueing
				org_data.PutMusic();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				org_data.GetMusicInfo( &mi );
				itoa(mi.wait,str,10);
				SetDlgItemText(hDlgTrack,IDE_VIEWWAIT,str);
				break;
			case IDM_2BUNNO1:
				SetUndo();
				org_data.ShortenAllNotes(2);
				scr_data.SetHorzScroll(0);org_data.SetPlayPointer(0);SetFocus(hWnd);//Cueing				
				org_data.PutMusic();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				org_data.GetMusicInfo( &mi );
				itoa(mi.wait,str,10);
				SetDlgItemText(hDlgTrack,IDE_VIEWWAIT,str);
				break;
			case IDM_3BUNNO1:
				SetUndo();
				org_data.ShortenAllNotes(3);
				scr_data.SetHorzScroll(0);org_data.SetPlayPointer(0);SetFocus(hWnd);//Cueing				
				org_data.PutMusic();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				org_data.GetMusicInfo( &mi );
				itoa(mi.wait,str,10);
				SetDlgItemText(hDlgTrack,IDE_VIEWWAIT,str);
				break;
			case IDM_CT_L1: //Linear weakness            IDM_CT_L1~9Be a sequential number!
			case IDM_CT_L2: //Convex weakness on the top
			case IDM_CT_L3: //Convex weakness downward
			case IDM_CT_L4: //Normalize
			case IDM_CT_L5: //Gradation
			case IDM_CT_L6: //vibrato
			case IDM_CT_L7: //
			case IDM_CT_L8: //
			case IDM_CT_L9: //
			case IDM_CT_L10:
			case IDM_CT_L11:
			case IDM_CT_L12:
			case IDM_CT_L13:
			case IDM_CT_L14:
			case IDM_CT_L15:
			case IDM_CT_L16:
			case IDM_CT_L17:
			case IDM_CT_L18:
			case IDM_CT_L19:
				SetUndo(); VolumeDecayEdit(1, -4, LOWORD(wParam) - IDM_CT_L1 + 1); //The first argument is0In the skyreturn Because it is
				break;
			case IDM_CT_S0: 
			case IDM_CT_S1: 
			case IDM_CT_S2: 
			case IDM_CT_S3: 
			case IDM_CT_S4: //Filling up
			case IDM_CT_S5: 
			case IDM_CT_S6: 
			case IDM_CT_S7: 
			case IDM_CT_S8: 
			case IDM_CT_S9: 
			case IDM_CT_S10: 
			case IDM_CT_S11: 
			case IDM_CT_S12: 
			case IDM_CT_S13: 
			case IDM_CT_S14: 
			case IDM_CT_S15: 
			case IDM_CT_S16: 
			case IDM_CT_S17: 
			case IDM_CT_S18: 
			case IDM_CT_S19: 
			case IDM_CT_S20: //Clear
				SetUndo(); VolumeDecayEdit(1, -4, LOWORD(wParam) - IDM_CT_S1 + 1 + 20);
				break;
			case ID_AC_C0://Clear
				SetUndo(); VolumeDecayEdit(1, -4, IDM_CT_S20     - IDM_CT_S1 + 1 + 20); 
				break;
			case ID_AC_C1: //Ctrl+1
			case ID_AC_C2:
			case ID_AC_C3:
			case ID_AC_C4:
			case ID_AC_C5:
			case ID_AC_C6:
			case ID_AC_C7:
			case ID_AC_C8:
			case ID_AC_C9:
				SetUndo(); VolumeDecayEdit(1, -4, LOWORD(wParam) - ID_AC_C1 + 1 + 20);
				break;
			case ID_AC_CA0:
			case ID_AC_CA1:
			case ID_AC_CA2:
			case ID_AC_CA3:
				SetUndo(); VolumeDecayEdit(1, -4, LOWORD(wParam) - ID_AC_CA0 + 1 + 20 + 9);
				break;
			case ID_AC_PRESS_LEFT: //Instead of left click
				ClickProcL(wParam, lParam);
				break;
			case ID_AC_PRESS_RIGHT: //Instead of right click
				ClickProcR(wParam, lParam);
				break;
			case ID_AC_CT_KEY_OCT_DOWN: //Lower octave
			case IDM_CT_OCT_DOWN:
				SetUndo();
				TransportNote(-12 , -4 );
				break;
			case ID_AC_CT_KEY_OCT_UP:
			case IDM_CT_OCT_UP:
				SetUndo();
				TransportNote(12 , -4 );
				break;
			case ID_AC_CT_PAN_R:
			case IDM_CT_PAN_R:
				SetUndo();
				PanEdit( 1,-4 );
				break;
			case ID_AC_CT_PAN_L:
			case IDM_CT_PAN_L:
				SetUndo();
				PanEdit( -1,-4 );
				break;
			case IDM_CT_PAN_REVERSE: //PANReversal
			case ID_AC_PAN_REVERSE:
				SetUndo(); PanEdit( 254, -4 );
				break;
			case ID_AC_CT_KEY_UP:
			case IDM_CT_TRANS_UP:
				SetUndo();
				TransportNote(1 , -4 );
				break;
			case ID_AC_CT_KEY_DOWN:
			case IDM_CT_TRANS_DOWN:
				SetUndo();
				TransportNote(-1 , -4 );
				break;
			case IDM_CT_VOL_PLUS:
			case ID_AC_CT_VOL_UP:
				SetUndo();
				VolumeEdit(5,-4);
				break;
			case IDM_CT_VOL_MINUS:
			case ID_AC_CT_VOL_DOWN:
				SetUndo();
				VolumeEdit(-5,-4);
				break;
			case IDM_CT_VOLWARIAI_UP:
			case ID_AC_VOLWARIAI_UP:
				SetUndo();
				VolumeWariaiEdit(144,-4);
				break;
			case IDM_CT_VOLWARIAI_DOWN:
			case ID_AC_VOLWARIAI_DOWN:
				SetUndo();
				VolumeWariaiEdit(112,-4);
				break;
			case IDM_ML_PAN_R:
				SetUndo();
				PanEdit( 1,-1 );
				break;
			case IDM_ML_PAN_L:
				SetUndo();
				PanEdit( -1,-1 );
				break;
			case IDM_ML_TRANS_UP:
			case ID_AC_ML_KEY_UP:
				SetUndo();
				TransportNote(1 , -1 );
				break;
			case IDM_ML_TRANS_DOWN:
			case ID_AC_ML_KEY_DOWN:
				SetUndo();
				TransportNote(-1 , -1 );
				break;
			case IDM_ML_VOL_PLUS:
			case ID_AC_ML_VOL_UP:
				SetUndo();
				VolumeEdit(5,-1);
				break;
			case IDM_ML_VOL_MINUS:
			case ID_AC_ML_VOL_DOWN:
				SetUndo();
				VolumeEdit(-5,-1);
				break;
			case IDM_DR_VOL_PLUS:
			case ID_AC_DR_VOL_UP:
				SetUndo();
				VolumeEdit(5,-2);
				break;
			case IDM_DR_VOL_MINUS:
			case ID_AC_DR_VOL_DOWN:
				SetUndo();
				VolumeEdit(-5,-2);
				break;
			case IDM_UNDO:
			case ID_AC_UNDO:
				ReplaseUndo();
				org_data.PutMusic();//Show score
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				org_data.GetMusicInfo( &mi );
				itoa(mi.wait,str,10);
				SetDlgItemText(hDlgTrack,IDE_VIEWWAIT,str);
				break;
			case IDM_REDO:
			case ID_AC_REDO:
				ReplaceRedo();
				org_data.PutMusic();//Show score
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				org_data.GetMusicInfo( &mi );
				itoa(mi.wait,str,10);
				SetDlgItemText(hDlgTrack,IDE_VIEWWAIT,str);
				break;
			case IDM_SELECT_ALL:
			case ID_AC_SELECT_ALL:
				SelectAll(0);
				break;
			case IDM_SELECT_RESET:
			case ID_AC_SELECT_RESET:
				SelectReset();
				break;
			case IDM_GRIDMODE:
			case ID_AC_GRIDMODE:
				ChangeGridMode();
				break;
			case IDM_AUTOLOADPVI:
				ChangeAutoLoadMode();
				break;
			case IDM_ALWAYS_CURRENT:
			case ID_AC_ALWAYS_CURRENT:
				ChangeSelAlwaysCurrent();
				org_data.PutMusic();//Show score
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				break;
			case ID_AC_DRAWDOUBLE:
			case IDM_DRAWDOUBLE:
				ChangeDrawDouble();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				break;
			case IDM_NOTE_ENLARGE: //    Make note heads stand out at reduced display(&O) 2014.05.28
				ChangeNoteEnlarge();
				org_data.PutMusic();//Show score
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				break;
			case IDM_ENABLEPLAYING: //You can hit the keyboard while playing 2010.09.23 A
				ChangeEnablePlaying();
				break;
			case IDM_CHANGEFINISH: //Confirm at the end 2010.09.23 A
				ChangeFinish();
				break;
			case ID_AC_HOMEBACK: //home
				SendMessage(hDlgPlayer , WM_COMMAND , IDC_START , 0);
				break;
			case IDM_RECENT_CLEAR:
				ClearRecentFile();
				break;
			case IDM_STOPNOWALL:
				Rxo_StopAllSoundNow();
				break;
			case ID_AC_DRAGMODE:
			case IDM_DRAGMODE:
				ChangeDragMode();
				break;
			case IDM_SLIDEOVERLAPNOTES: //2014.05.06 A
			case ID_AC_SLIDEOVERLAPNOTES:
				ChangeSlideOverlapNoteMode();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				break;
			case IDM_PRESSNOTE:
			case ID_AC_PRESSNOTE:
				ChangePushStratchNOTE();
				break;
			case IDM_INIT:
			case ID_AC_INIT:
				//if(MessageBox(hwnd,"Unsaved content will be discarded. Do you want to initialize it?","Initialization confirmation",MB_OKCANCEL)==IDCANCEL)break; //2010.09.25 A
				if(CancelDeleteCurrentData(CDCD_INIT))break;
				ClearUndo();
				memset(music_file, 0 , MAX_PATH);
				strcpy(music_file, MessageString[IDS_DEFAULT_ORG_FILENAME]);
				//for(i = 0; i < 12; i++){
				//	music_file[i] = name[i];
				//}
				org_data.InitOrgData();
				org_data.SetPlayPointer(0);
				scr_data.SetHorzScroll(0);
				//Reflect on player
				SetDlgItemText(hDlgPlayer,IDE_VIEWWAIT,"128");
				SetDlgItemText(hDlgPlayer,IDE_VIEWMEAS,"0");
				SetDlgItemText(hDlgPlayer,IDE_VIEWXPOS,"0");
				SetTitlebarText(music_file);
				//MessageBox(hwnd,"Initialized","Message",MB_OK);
				ClearEZC_Message(); //EZErase messages and ranges
				SelectReset();
				org_data.PutMusic();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				for(i=0;i<8;i++)ucMIDIProgramChangeValue[i]=255;

				break;
				//↓	// 2010.12.01 A
			case ID_AC_SELECT_CLEAR: //Cancel selection
			case ID_AC_SELECT_CLEAR2: //Cancel selection //2014.04.13
				ClearEZC_Message();
				org_data.PutMusic();
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				break;
			case ID_AC_SELECT_BACKDEL: //2014.04.13
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_DELETEBUTTON_2  , 0);
				break;
			case ID_AC_SELECT_INSERT: //2014.04.13
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_INSERTBUTTON  , 0);
				break;
			//Range selection operation by ten key operation
			case ID_AC_NUM1:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB1 , 0);
				break;
			case ID_AC_NUM2:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB2 , 0);
				break;
			case ID_AC_NUM3:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB3 , 0);
				break;
			case ID_AC_NUM4:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB4 , 0);
				break;
			case ID_AC_NUM5:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB5 , 0);
				break;
			case ID_AC_NUM6:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB6 , 0);
				break;
			case ID_AC_C_NUM1:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB7 , 0);
				break;
			case ID_AC_C_NUM2:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB8 , 0);
				break;
			case ID_AC_C_NUM3:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB9 , 0);
				break;
			case ID_AC_C_NUM4:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB10 , 0);
				break;
			case ID_AC_C_NUM5:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB11 , 0);
				break;
			case ID_AC_C_NUM6:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_CTB12 , 0);
				break;
			case ID_AC_NUM7:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_PST1 , 0);
				break;
			case ID_AC_NUM8:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_PST2 , 0);
				break;
			case ID_AC_NUM9:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_PST3 , 0);
				break;
			case ID_AC_NUMPLUS:
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_PST4 , 0);
				break;
			case ID_AC_DELETEKEY: //Add 2014/04/12
				SendMessage(hDlgEZCopy , WM_COMMAND , IDC_DELETEBUTTON , 0);
				break;
			}
		}else{
			//Only during performance
			//None
		}
		//Allowed during playing
		switch(LOWORD(wParam)){
		case IDM_LOUPE_MINUS:
		case ID_AC_LOUPE_MINUS:
			NoteWidth -= 2; if(NoteWidth<4)NoteWidth=4;
			org_data.PutBackGround();
			org_data.PutMusic();//Show score
			RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
			switch(NoteWidth){
			case 4:		SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[25.%]"); break;
			case 6:		SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[37.5%]"); break;
			case 8:		SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[50%]"); break;
			case 10:	SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[62.5%]"); break;
			case 12:	SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[75%]"); break;
			case 14:	SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[87.5%]"); break;
			case 16:	SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[100%]"); break;
			}
			CheckLoupeMenu();
			break;
		case IDM_LOUPE_PLUS:
		case ID_AC_LOUPE_PLUS:
			NoteWidth += 2; if(NoteWidth>16)NoteWidth=16;
			org_data.PutBackGround();
			org_data.PutMusic();//Show score
			RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
			switch(NoteWidth){
			case 4:		SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[25%]"); break;
			case 6:		SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[37.5%]"); break;
			case 8:		SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[50%]"); break;
			case 10:	SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[62.5%]"); break;
			case 12:	SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[75%]"); break;
			case 14:	SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[87.5%]"); break;
			case 16:	SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, "[100%]"); break;
			}
			CheckLoupeMenu();
			break;
		}

		break;
	case WM_ACTIVATEAPP:
		actApp = wParam;
		break;
	case WM_MENUSELECT:

		switch(LOWORD(wParam)){
		default: SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, ""); break;
		case ID_MENUITEM40265:      SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING78]); break;
		case IDM_EXPORT_MIDI:       SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING79]); break;
		case IDM_IMPORT_PTCOP:      SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING79]); break;
		case IDM_LOAD2:             SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING80]); break;
		case IDM_SAVEOVER:          SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, music_file); break; 
		case IDM_SAVENEW:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING81]); break; 
		case IDM_RECENT1:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[0]); break;
		case IDM_RECENT2:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[1]); break;
		case IDM_RECENT3:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[2]); break;
		case IDM_RECENT4:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[3]); break;
		case IDM_RECENT5:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[4]); break;
		case IDM_RECENT6:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[5]); break;
		case IDM_RECENT7:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[6]); break;
		case IDM_RECENT8:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[7]); break;
		case IDM_RECENT9:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[8]); break;
		case IDM_RECENT0:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, RecentFileName[9]); break;
		case IDM_EXIT:              SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING82]); break;
		case IDM_DLGWAVE:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING83]); break;
		case IDM_DLGSETTING:        SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING84]); break;
		case IDM_DLGDEFAULT:        SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING85]); break;
		case IDM_CT_L0:             SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING86]); break;
		case IDM_CT_S0:             SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING87]); break;
		case IDM_DLGUSED:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING88]); break;
		case IDM_DRAWDOUBLE:        SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING89]); break;
		case IDM_SLIDEOVERLAPNOTES: SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING90]); break;
		case IDM_LOUPE_PLUS:        SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING91]); break;
		case IDM_LOUPE_MINUS:       SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING92]); break;
		case IDM_PRESSNOTE:         SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING93]); break;
		case IDM_GRIDMODE:          SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING94]); break;
		case IDM_ALWAYS_CURRENT:    SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING95]); break;
		case IDM_DRAGMODE:          SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING96]); break;
		case IDM_ENABLEPLAYING:     SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING97]); break;
		case IDM_SORTMUSICNOTE:     SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING98]); break;
		case IDM_STOPNOWALL:        SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING99]); break;
		case IDM_RECENT_CLEAR:      SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING100]); break;
		case IDM_AUTOLOADPVI:       SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING101]); break;
		case IDM_DLGHELP:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING102]); break;
		case IDM_DLGMEMO:           SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING103]); break;		}
		break;
	case WM_DROPFILES://File Drop
		//SetWindowPos(hWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );
		//SendMessage(hWnd, WM_PAINT, 0, 0);
		//SetFocus(hWnd);
		SetForegroundWindow(hwnd);
		if(CancelDeleteCurrentData(CDCD_LOAD))break;
		//DragQueryFile((HDROP)wParam,0,music_file,MAX_PATH);	// 2014.05.22 D
		DragQueryFile((HDROP)wParam,0,strMIDIFile,MAX_PATH);	// 2014.05.22 A
		if(org_data.FileCheckBeforeLoad(strMIDIFile)){
			SetDlgItemText(hDlgEZCopy, IDC_MESSAGE, MessageString[IDS_STRING64]); //<!>The file can not be opened or the format is illegal.
			break;
		}
		strcpy(music_file, strMIDIFile);

		ClearUndo();
//		MessageBox(hWnd,music_file,"",MB_OK);
		org_data.InitOrgData();
		org_data.LoadMusicData();
		org_data.PutMusic();//Show score
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
			//Show to player
			org_data.GetMusicInfo( &mi );
			SetDlgItemInt(hDlgTrack,IDE_VIEWWAIT,mi.wait,TRUE );
			//SetDlgItemInt(hDlgTrack,IDE_VIEWTRACK,0,TRUE );
			SetDlgItemText(hDlgTrack,IDE_VIEWTRACK,"1");
		SetTitlebarText(music_file);//Title name set
		break;
	case WM_PAINT://Display message
		HDC hdc;
		PAINTSTRUCT ps;

		hdc = BeginPaint(hWnd,&ps);
		RefleshScreen(hdc);
		//static TCHAR strSize[128];
		//wsprintf(strSize , "width = %d : height = %d" , WWidth , WHeight);
		TextOut(hdc , 10 , 10 , strSize , lstrlen(strSize));
		EndPaint(hWnd,&ps);


		break;
	case WM_CLOSE:
		//if(iChangeFinish!=0){	// A 2010.09.22
		//	char cc[512],*chn;
		//	GetWindowText(hWnd,cc,512);
		//	chn = strstr(cc, HENKOU_NO_SHIRUSHI);
		//	if(chn!=NULL){
		//		//When there is a change, confirm the end. // A 2010.09.22
		//		if(MessageBox(hwnd,"Unsaved content will be discarded. Do you want to end it?","Confirm completion",MB_OKCANCEL| MB_ICONASTERISK)==IDCANCEL)break;
		//	}
		//}
		if(CancelDeleteCurrentData(CDCD_EXIT))break;

		SaveIniFile();
		QuitMMTimer();
		PostQuitMessage(0);
		break;
	case WM_QUIT:
		break;
	case WM_DESTROY://For post processing
		EndDirectSound();
		org_data.ReleaseNote();
		DeleteWaveData100(); //add to20140401 Normal,WM_CLOSE → WM_DESTROY → WM_QUIT It seems to be called in order of.
		EndGDI();
		if(!hDlgPlayer)DestroyWindow(hDlgPlayer);
		if(!hDlgTrack)DestroyWindow(hDlgTrack);
		if(!hDlgEZCopy)DestroyWindow(hDlgEZCopy);
		if(!hwnd)DestroyWindow(hwnd);
		PostQuitMessage(0);
		free(strMIDIFile); //2014.05.11
		FreeMessageStringBuffer();	// 2014.10.19 
		break;
	case WM_KEYDOWN://Keyboard pressed
		switch(wParam){
		case VK_UP:
			scr_data.KeyScroll(DIRECTION_UP);
			break;
		case VK_DOWN:
			scr_data.KeyScroll(DIRECTION_DOWN);
			break;
		case VK_LEFT:
			scr_data.KeyScroll(DIRECTION_LEFT);
			break;
		case VK_RIGHT:
			scr_data.KeyScroll(DIRECTION_RIGHT);
			break;
		case VK_F5:
		case VK_NUMPAD0:
			if(timer_sw == 0)SendMessage(hDlgPlayer , WM_COMMAND , IDC_PLAY ,0 );
			else SendMessage(hDlgPlayer , WM_COMMAND , IDC_STOP , 0);
			break;
		//case VK_HOME:
		//	SendMessage(hDlgPlayer , WM_COMMAND , IDC_START , NULL);
		//	break;
		case 'Z':
		case 'S':
		case 'X':
		case 'C':
		case 'F':
		case 'V':
		case 'G':
		case 'B':
		case 'N':
		case 'J':
		case 'M':
		case 'K':
		case 0xBC:
		case 'L':
		case 0xBE:
		case 0xBF:
		case 0xBA:
		case 0xE2:
		case 0xDD:

			if((lParam & 0x40000000) ==0 && (timer_sw==0 || iChangeEnablePlaying!=0)){
				iKeyPhase[iCast[wParam]]=iCurrentPhase;
				iCurrentPhase=-iCurrentPhase + 1;
				Rxo_PlayKey(iCast[wParam] + iPushShift[0]*12 -iPushShift[1]*12 , org_data.track, 1000, iKeyPhase[iCast[wParam]]);
				iKeyPushDown[iCast[wParam]+ iPushShift[0]*12 -iPushShift[1]*12] = 1;
				org_data.PutMusic();//Redraw music
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);

			}

			break;
		case VK_TAB:
			if((lParam & 0x40000000) ==0 && (timer_sw==0 || iChangeEnablePlaying!=0)) iPushShift[0]=1;
			Rxo_StopAllSoundNow();
			break;
		case VK_SHIFT:
			if((lParam & 0x40000000) ==0 && (timer_sw==0 || iChangeEnablePlaying!=0)) iPushShift[1]=1;
			Rxo_StopAllSoundNow();
			break;
		}
		break;
//				wsprintf(strSize , "U lParam = %x" , lParam);
//				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
	case WM_KEYUP: //Keyboard released
		switch(wParam){
		case 'Z':
		case 'S':
		case 'X':
		case 'C':
		case 'F':
		case 'V':
		case 'G':
		case 'B':
		case 'N':
		case 'J':
		case 'M':
		case 'K':
		case 0xBC:
		case 'L':
		case 0xBE:
		case 0xBF:
		case 0xBA:
		case 0xE2:
		case 0xDD:
			if((timer_sw==0 || iChangeEnablePlaying!=0)){
				Rxo_StopKey(iCast[wParam]+ iPushShift[0]*12 -iPushShift[1]*12, org_data.track, iKeyPhase[iCast[wParam]]);
				iKeyPhase[iCast[wParam]] = -1;
				iKeyPushDown[iCast[wParam]+ iPushShift[0]*12 -iPushShift[1]*12] = 0;
				org_data.PutMusic();//Redraw music
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
			}
			break;
		case VK_TAB:
			if((timer_sw==0 || iChangeEnablePlaying!=0)){
				iPushShift[0]=0;
				for(i=0;i<256;i++)iKeyPushDown[i]=0;
				org_data.PutMusic();//Redraw music
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				Rxo_StopAllSoundNow();
			}
			break;
		case VK_SHIFT:
			if((timer_sw==0 || iChangeEnablePlaying!=0)){
				iPushShift[1]=0;
				for(i=0;i<256;i++)iKeyPushDown[i]=0;
				org_data.PutMusic();//Redraw music
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
				Rxo_StopAllSoundNow();
			}
			break;
		}
		break;

    case WM_LBUTTONDOWN://mouse(left)Was pressed
		ClickProcL(wParam, lParam);
		break;
    case WM_RBUTTONDOWN://mouse(right)Was pressed
		ClickProcR(wParam, lParam);
		break;
    case WM_MBUTTONDOWN://mouse(Center)Was pressed
		ClickProcM(wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		MouseDrag(wParam, lParam);
		break;
    case WM_LBUTTONUP://mouse(left)Was released
		LButtonUP(wParam, lParam);
		break;
    case WM_RBUTTONUP://mouse(right)Was released
		RButtonUP(wParam, lParam);
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		break;
	case WM_HSCROLL:
		scr_data.HorzScrollProc(wParam);
		break;
	case WM_VSCROLL:
		scr_data.VertScrollProc(wParam);
		break;
	case WM_MOUSEWHEEL:
		scr_data.WheelScrollProc(lParam, wParam);		
		break;
	case WM_GETMINMAXINFO:
			pmmi = (MINMAXINFO *)lParam;
			if ( pmmi )
			{
				pmmi->ptMinTrackSize.x = 420;  // Minimum width
				pmmi->ptMinTrackSize.y = 480;  // Minimum height
				//pmmi->ptMaxTrackSize.x = 1024; // Maximum width
				//pmmi->ptMaxTrackSize.y = 768;  // Maximum height
			}
		
		break;
	case 0x020C: //Odd button press
		//MessageBox(hWnd,"Failed to read","Error(Load)",MB_OK);
		switch HIWORD(wParam){
		case 0x0001: //▼
			ChangeTrackPlus(hDlgTrack , 1);
			break;
		case 0x0002: //▲
			ChangeTrackPlus(hDlgTrack , -1);
			break;
		}
		break;
	case WM_SIZE:
		WWidth = LOWORD(lParam);	//Size of client area
		WHeight = HIWORD(lParam);
		rect.right = WWidth;		//A 2008/05/14
		rect.bottom = WHeight;		//A 2008/05/14
		if(!org_data.PutBackGround())break;
		org_data.PutMusic();
		
//				wsprintf(strSize , "Height = %d" , (WHeight - 158)/12);
//				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		scr_data.ChangeVerticalRange(WHeight);
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);

		break;
	case WM_SIZING: //Changing size
		//org_data.PutBackGround();
		//org_data.PutMusic();
		
		//RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		
		//AfterReSize = true;
		break;
	//Below is a test
/*	case WM_MOUSEMOVE:
		char str[80];
		long mouse_x,mouse_y;
//		HDC hdc;
		hdc = GetDC(hWnd);
		mouse_data.GetMousePosition(&mouse_x,&mouse_y);
		sprintf(str,"Mouse X:%4d Mouse Y:%4d  ",mouse_x,mouse_y);
		TextOut(hdc,500,16,str,strlen(str));
		ReleaseDC(hWnd,hdc);
		break;
*/
	default:return DefWindowProc(hwnd, message, wParam, lParam);//Other than
	}

	return FALSE;
}
//Display file name in title bar
void SetTitlebarText(char *name)
{
	int i,j;
	char set_name[MAX_PATH+15];//Display space in title
	char file_name[MAX_PATH];//Processing names (excluding directories)

	i = 0;
	while(name[i] != '\0')i++;//First to the hips
	while(i != 0 && name[i-1] != '\\')i--; //Last circle mark
	
	//Create a file name
	j = 0;
	while(name[i] != '\0'){
		file_name[j] = name[i];
		i++;
		j++;
	}
	file_name[j] = '\0';
	//Pour the application title
	for(i = 0; i < 15; i++){
		set_name[i] = lpszName[i];
	}
	//Pour the file name
	for(j = 0; j < MAX_PATH; j++){
		set_name[i] = file_name[j];
		if(set_name[i] == '\0')break;
		i++;
	}
	SetWindowText(hWnd,&set_name[0]);
}


//Change sign of Ali.
void SetTitlebarChange(void)
{
	char cc[512],*chn;
	GetWindowText(hWnd,cc,512);
	//chn = strstr(cc, HENKOU_NO_SHIRUSHI);	// 2014.10.19 D
	chn = strstr(cc, MessageString[IDS_MODIFIED]);	// 2014.10.19 A
	if(chn==NULL){
		//strcat(cc, HENKOU_NO_SHIRUSHI);	// 2014.10.19 D
		strcat(cc, MessageString[IDS_MODIFIED]);	// 2014.10.19 A
		SetWindowText(hWnd,cc);
	}

}
void ResetTitlebarChange(void)
{
	char cc[512],*chn;
	GetWindowText(hWnd,cc,512);
	//chn = strstr(cc, HENKOU_NO_SHIRUSHI);	// 2014.10.19 D
	chn = strstr(cc, MessageString[IDS_MODIFIED]);	// 2014.10.19 A
	if(chn!=NULL){
		*chn = '0';//To erase
		*(chn+1) = '0';//To erase
		SetWindowText(hWnd,cc);
	}
}

void SaveIniFile()
{
	int i;
	WINDOWPLACEMENT WPM;
	WPM.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd, &WPM);
	if(WPM.showCmd == SW_SHOWMAXIMIZED)i=1;else i=0;
	ShowWindow( hWnd, SW_RESTORE );	//Original sedani

	GetWindowRect(hWnd,(LPRECT)&WinRect);
	wsprintf(num_buf,"%d",WinRect.left);
	WritePrivateProfileString(MAIN_WINDOW,"left",num_buf,app_path);
	wsprintf(num_buf,"%d",WinRect.top);
	WritePrivateProfileString(MAIN_WINDOW,"top",num_buf,app_path);
	wsprintf(num_buf,"%d",WinRect.right - WinRect.left);
	WritePrivateProfileString(MAIN_WINDOW,"right",num_buf,app_path);
	wsprintf(num_buf,"%d",WinRect.bottom - WinRect.top);
	WritePrivateProfileString(MAIN_WINDOW,"bottom",num_buf,app_path);
	wsprintf(num_buf,"%d",i);
	WritePrivateProfileString(MAIN_WINDOW,"WindowState",num_buf,app_path);
	wsprintf(num_buf,"%d",sGrid);
	WritePrivateProfileString(MAIN_WINDOW,"GridMode",num_buf,app_path);
	wsprintf(num_buf,"%d",sACrnt);
	WritePrivateProfileString(MAIN_WINDOW,"AlwaysCurrent",num_buf,app_path);
	wsprintf(num_buf,"%d",gDrawDouble);
	WritePrivateProfileString(MAIN_WINDOW,"DrawDouble",num_buf,app_path);
	wsprintf(num_buf,"%d",iDragMode);
	WritePrivateProfileString(MAIN_WINDOW,"DragMode",num_buf,app_path);
	wsprintf(num_buf,"%d",iChangeEnablePlaying);
	WritePrivateProfileString(MAIN_WINDOW,"EnablePressNoteStretch",num_buf,app_path);
	wsprintf(num_buf,"%d",iPushStratch);
	WritePrivateProfileString(MAIN_WINDOW,"EnablePlaying",num_buf,app_path);
	wsprintf(num_buf,"%d",iChangeFinish);
	WritePrivateProfileString(MAIN_WINDOW,"QuitMessage",num_buf,app_path);
	wsprintf(num_buf,"%d",iSlideOverlapNotes);
	WritePrivateProfileString(MAIN_WINDOW,"SlideOverlapNoteMode",num_buf,app_path);
	wsprintf(num_buf,"%d",NoteWidth);
	WritePrivateProfileString(MAIN_WINDOW,"NoteWidth",num_buf,app_path);
	wsprintf(num_buf,"%d",NoteEnlarge_Until_16px);
	WritePrivateProfileString(MAIN_WINDOW,"NoteEnlarge",num_buf,app_path);


	GetWindowRect(hDlgTrack,(LPRECT)&WinRect);
	wsprintf(num_buf,"%d",WinRect.left);
	WritePrivateProfileString(TRACK_WINDOW,"left",num_buf,app_path);
	wsprintf(num_buf,"%d",WinRect.top);
	WritePrivateProfileString(TRACK_WINDOW,"top",num_buf,app_path);

	GetWindowRect(hDlgPlayer,(LPRECT)&WinRect);
	wsprintf(num_buf,"%d",WinRect.left);
	WritePrivateProfileString(PLAY_WINDOW,"left",num_buf,app_path);
	wsprintf(num_buf,"%d",WinRect.top);
	WritePrivateProfileString(PLAY_WINDOW,"top",num_buf,app_path);
	GetWindowRect(hDlgEZCopy,(LPRECT)&WinRect);
	wsprintf(num_buf,"%d",WinRect.left);
	WritePrivateProfileString(COPY_WINDOW,"left",num_buf,app_path);
	wsprintf(num_buf,"%d",WinRect.top);
	WritePrivateProfileString(COPY_WINDOW,"top",num_buf,app_path);
	wsprintf(num_buf,"%d",EZCopyWindowState);
	WritePrivateProfileString(COPY_WINDOW,"show",num_buf,app_path);

	wsprintf(num_buf,"%d",CmnDialogWnd.left);
	WritePrivateProfileString(COMMON_WINDOW,"left",num_buf,app_path);
	wsprintf(num_buf,"%d",CmnDialogWnd.top);
	WritePrivateProfileString(COMMON_WINDOW,"top",num_buf,app_path);
	wsprintf(num_buf,"%d",CmnDialogWnd.right );
	WritePrivateProfileString(COMMON_WINDOW,"right",num_buf,app_path);
	wsprintf(num_buf,"%d",CmnDialogWnd.bottom );
	WritePrivateProfileString(COMMON_WINDOW,"bottom",num_buf,app_path);
	wsprintf(num_buf,"%d",iDlgRepeat );
	WritePrivateProfileString(MIDI_EXPORT,"Repeat",num_buf,app_path);

	WritePrivateProfileString(MIDI_EXPORT,"Author", strMIDI_AUTHOR, app_path);
	WritePrivateProfileString(MIDI_EXPORT,"Title", strMIDI_TITLE, app_path);

	wsprintf(num_buf,"%d",SaveWithInitVolFile );
	WritePrivateProfileString(INIT_DATA,"autosave",num_buf,app_path);

	SaveRecentFilesToInifile();
	scr_data.SetIniFile();

}

/*
//Opening flash
BOOL CALLBACK DialogFlash(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
	case WM_INITDIALOG:
		int i;
		long screen_w;
		long screen_h;
		long my_w;
		long my_h;
		long x,y,w,h;

		//Screen size
		screen_w = GetSystemMetrics(SM_CXSCREEN);
		screen_h = GetSystemMetrics(SM_CYSCREEN);
		//Window size
		my_w = gWidthWindow;
		my_h = gHeightWindow;

		MoveWindow( hDlg, screen_w/2, screen_h/2, 1, 1, TRUE );
		ShowWindow( hDlg, SW_SHOW );
		for( i = 1; i <= WAITFLASH; i++ ){
			x = screen_w/2 - ( my_w/2 * i/WAITFLASH );
			y = screen_h/2 - ( my_h/2 * i/WAITFLASH );
			w = my_w * i / WAITFLASH;
			h = my_h * i / WAITFLASH;
			MoveWindow( hDlg, x, y, w, h, TRUE );
		}
		ShowWindow(hWnd,SW_SHOW);
		EndDialog(hDlg,0);
		return 1;
	default:return FALSE;
	}
	return TRUE;
}
*/

void CheckLoupeMenu(void)
{
	if(NoteWidth == 4){
		EnableMenuItem(GetMenu(hWnd), IDM_LOUPE_MINUS, MF_GRAYED);
	}else{
		EnableMenuItem(GetMenu(hWnd), IDM_LOUPE_MINUS, MF_ENABLED);
	}
	if(NoteWidth == 16){
		EnableMenuItem(GetMenu(hWnd), IDM_LOUPE_PLUS, MF_GRAYED);
	}else{
		EnableMenuItem(GetMenu(hWnd), IDM_LOUPE_PLUS, MF_ENABLED);
	}

}
