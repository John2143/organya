#include "Setting.h"
#include "Gdi.h"
#include <stdio.h>
#include "resource.h"
#include "DefOrg.h"
#include "OrgData.h"
#include "Sound.h"
#include "Scroll.h"
#include "rxoFunction.h"
#include "util.h"
#include "math.h"

RECT CmnDialogWnd;
int count_of_INIT_DONE;
int iDlgRepeat; //Repeat count obtained from dialog
extern char strMIDI_TITLE[256];
extern char strMIDI_AUTHOR[256];
extern char *dram_name[];
LPCTSTR  MIDIPC[]={
	"000 Acoustic Grand Piano","001 Bright Acoustic Piano","002 Electric Grand Piano","003 Honky-tonk Piano","004 Electric Piano 1","005 Electric Piano 2","006 Harpsichord","007 Clavi",
	"008 Celesta","009 Glockenspiel","010 Music Box","011 Vibraphone","012 Marimba","013 Xylophone","014 Tubular Bells","015 Dulcimer",
	"016 Drawbar Organ","017 Percussive Organ","018 Rock Organ","019 Church Organ","020 Reed Organ","021 Accordion","022 Harmonica","023 Tango Accordion",
	"024 Acoustic Guitar (nylon)","025 Acoustic Guitar (steel)","026 Electric Guitar (jazz)","027 Electric Guitar (clean)","028 Electric Guitar (muted)","029 Overdriven Guitar","030 Distortion Guitar","031 Guitar Harmonics",
	"032 Acoustic Bass","033 Electric Bass (finger)","034 Electric Bass (pick)","035 Fretless Bass","036 Slap Bass 1","037 Slap Bass 2","038 Synth Bass 1","039 Synth Bass 2",
	"040 Violin","041 Viola","042 Cello","043 Contrabass","044 Tremolo Strings","045 Pizzicato Strings","046 Orchestral Harp","047 Timpani",
	"048 String Ensemble 1","049 String Ensemble 2","050 Synth Strings 1","051 Synth Strings 2","052 Choir Aahs","053 Voice Oohs","054 Synth Voice","055 Orchestra Hit",
	"056 Trumpet","057 Trombone","058 Tuba","059 Muted Trumpet","060 French Horn","061 Brass Section","062 Synth Brass 1","063 Synth Brass 2",
	"064 Soprano Sax","065 Alto Sax","066 Tenor Sax","067 Baritone Sax","068 Oboe","069 English Horn","070 Bassoon","071 Clarinet",
	"072 Piccolo","073 Flute","074 Recorder","075 Pan Flute","076 Blown Bottle","077 Shakuhachi","078 Whistle","079 Ocarina",
	"080 Lead 1 (square)","081 Lead 2 (sawtooth)","082 Lead 3 (calliope)","083 Lead 4 (chiff)","084 Lead 5 (charang)","085 Lead 6 (voice)","086 Lead 7 (fifths)","087 Lead 8 (bass+lead)",
	"088 Pad 1 (new age)","089 Pad 2 (warm)","090 Pad 3 (polysynth)","091 Pad 4 (choir)","092 Pad 5 (bowed)","093 Pad 6 (metallic)","094 Pad 7 (halo)","095 Pad 8 (sweep)",
	"096 FX 1 (ice rain)","097 FX 2 (soundtrack)","098 FX 3 (crystal)","099 FX 4 (atmosphere)","100 FX 5 (brightness)","101 FX 6 (goblins)","102 FX 7 (echoes)","103 FX 8 (sci-fi)",
	"104 Sitar","105 Banjo","106 Shamisen","107 Koto","108 Kalimba","109 Bag Pipe","110 Fiddle","111 Shanai",
	"112 Tinkle Bell","113 Agogo","114 Steel Drums","115 Woodblock","116 Taiko Drum","117 Melodic Tom","118 Synth Drum","119 Reverse Cymbal",
	"120 Guitar Fret Noise","121 Breath Noise","122 Seashore","123 Bird Tweet","124 Telephone Ring","125 Helicopter","126 Applause","127 Gunshot"
};

extern unsigned char ucMIDIProgramChangeValue[MAXTRACK];

 extern HWND hDlgPlayer;

char GetFileNameSave(HWND hwnd,char *title)
{//Get file name(save)
	OPENFILENAME ofn;
	FILE *fp;
//	char res;//Result of file open

	memset(&ofn,0,sizeof(OPENFILENAME));
//	strcpy(GetName,"*.pmd");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner   = hwnd;
	ofn.hInstance   = hInst;
	//ofn.lpstrFilter = "OrganyaData[*.org]¥0*.org¥0All formats [*.*]¥0*.*¥0¥0";	// 2014.10.19 D
	ofn.lpstrFilter = MessageString[IDS_STRING109];	// 2014.10.19 A
	ofn.lpstrFile   = music_file;
	ofn.nMaxFile    = MAX_PATH;
	ofn.lpstrTitle  = title;
	ofn.Flags       = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_CREATEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "org";

	//Attempt to acquire the file name.
	if(GetSaveFileName(&ofn));//InvalidateRect(hwnd,NULL,TRUE);
	else return MSGCANCEL;//Cancel0Will return
	fp = fopen(music_file,"rb");
	//Existing file exists? OFN_OVERWRITEPROMPT It made unnecessary by designation.
	//if(fp != NULL)
	//{
	//	fclose(fp);
	//	return MSGEXISFILE;//Existing file
	//}
	return MSGSAVEOK;
}
// Graphics drawing 
int DrawGr(HWND hWnd, HDC hdc) {
	return 0; //I will not do anything anymore.
	HPEN hPen, hOldPen;
	RECT rt;
	GetClientRect(hWnd, &rt);
	hPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	hOldPen = (HPEN )SelectObject(hdc, hPen);
	MoveToEx(hdc, 0, 0, NULL);
	LineTo(hdc, rt.right, rt.bottom);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
	hPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 255));
	hOldPen = (HPEN )SelectObject(hdc, hPen);
	MoveToEx(hdc, rt.right, 0, NULL);
	LineTo(hdc, 0, rt.bottom);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
	return 0;
} 
//Hook Procedure
UINT CALLBACK OFNHookProcMID(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//RECT  rcWindow;
	LPOFNOTIFY lpOfn = (LPOFNOTIFY)lParam;
	char ctmp[12];
	int DlgWidth, DlgHeight, i,j;
	HWND haDlg;
	HDC hdc;
	PAINTSTRUCT ps;
	static MUSICINFO mi;
	switch(msg){
        case WM_INITDIALOG:
			org_data.GetMusicInfo(&mi);
			//iDlgRepeat = 1;
			itoa(iDlgRepeat ,ctmp, 10);
			SetDlgItemText(hdlg, IDC_REPEAT, ctmp);
			SetDlgItemText(hdlg, IDC_AUTHOR, strMIDI_AUTHOR);
			SetDlgItemText(hdlg, IDC_TITLE,  strMIDI_TITLE);
 			for(j=0;j<8;j++){
				SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_RESETCONTENT,0,0);//Initialization
				for(i=0;i<128;i++){
					SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_ADDSTRING,0,(LPARAM)MIDIPC[i]);
				}
				if(ucMIDIProgramChangeValue[j] == 255){
					ucMIDIProgramChangeValue[j] = mi.tdata[j].wave_no; //Initialized in this place
				}
				SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_SETCURSEL,ucMIDIProgramChangeValue[j],0);
			}
           return TRUE;

		case WM_SIZE:
			DlgWidth  = LOWORD(lParam);	//Size of client area
			DlgHeight = HIWORD(lParam);
			haDlg = GetDlgItem(hdlg, IDC_TITLE);
			SetWindowPos(haDlg, HWND_TOP, 1, 1, DlgWidth-150, 18, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
			haDlg = GetDlgItem(hdlg, IDC_AUTHOR);
			SetWindowPos(haDlg, HWND_TOP, 1, 1, DlgWidth-150, 18, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
			return TRUE;

		case WM_PAINT:
			hdc = BeginPaint(hdlg, &ps);
			DrawGr(hdlg, hdc);
			EndPaint(hdlg, &ps);
			return TRUE; 		
		case WM_COMMAND:
			switch(LOWORD(wParam)){
			case IDC_ALL_SQUAREWAVE1:
				i = SendDlgItemMessage(hdlg,IDC_MIDIPC1,CB_GETCURSEL,0,0);
				for(j=1;j<8;j++) SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_SETCURSEL,i,0);
				break;
			case IDC_ALL_SQUAREWAVE2:
				for(j=0;j<8;j++) SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_SETCURSEL,16,0);
				break;
			case IDC_ALL_SQUAREWAVE3:
				for(j=0;j<8;j++) SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_SETCURSEL,80,0);
				break;
			case IDC_ALL_SQUAREWAVE4:
				for(j=0;j<8;j++) SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_SETCURSEL,(rand() & 0x7F),0);
				break;
			}

			return TRUE;
		case WM_NOTIFY:
			if(lpOfn->hdr.code == CDN_INITDONE ){
			}
			if(lpOfn->hdr.code == CDN_FILEOK ){
				GetDlgItemText(hdlg,IDC_REPEAT,ctmp,12); iDlgRepeat = atol(ctmp);
				iDlgRepeat = (iDlgRepeat>0xFFFF) ? 0xFFFF : ((iDlgRepeat<=0) ? 1: iDlgRepeat);
				memset(strMIDI_AUTHOR, 0, 255);
				GetDlgItemText(hdlg, IDC_AUTHOR, strMIDI_AUTHOR, 250);
				memset(strMIDI_TITLE, 0, 255);
				GetDlgItemText(hdlg, IDC_TITLE,  strMIDI_TITLE,  250);
				for(j=0;j<8;j++){
					ucMIDIProgramChangeValue[j] = (unsigned char)SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_GETCURSEL,0,0);
				}

				//MessageBox(NULL,ctmp,"Message",MB_OK);
			}
			//MessageBox(NULL,"Initialized","Message",MB_OK);
			return TRUE;
    }
    return FALSE;
}
char GetFileNameMIDI(HWND hwnd,char *title, char *filename)
{//Get file name(MIDI)
	OPENFILENAME ofn;
	FILE *fp;
//	char res;//Result of file open

	memset(&ofn,0,sizeof(OPENFILENAME));
	strcpy(filename, music_file);
	char *p;
	if( (p = strstr(filename, ".org")) != NULL ){
		strcpy(p, ".mid");
	}

//	strcpy(GetName,"*.pmd");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner   = hwnd;
	ofn.hInstance   = hInst;
	//ofn.lpstrFilter = "standardMIDIFile[*.mid]¥0*.mid¥0All formats [*.*]¥0*.*¥0¥0";	// 2014.10.19 D
	ofn.lpstrFilter = MessageString[IDS_STRING110];	// 2014.10.19 A
	ofn.lpstrFile   = filename;
	ofn.nMaxFile    = MAX_PATH;
	ofn.lpstrTitle  = title;
    ofn.Flags       = OFN_EXPLORER |
                      OFN_OVERWRITEPROMPT |
                      OFN_CREATEPROMPT |
                      OFN_HIDEREADONLY |
                      OFN_ENABLESIZING |
                      OFN_ENABLEHOOK |     //Use hook procedure
                      OFN_ENABLETEMPLATE;
	ofn.lpfnHook = OFNHookProcMID;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_MIDI);
		

	ofn.lpstrDefExt = "mid";

	//Attempt to acquire the file name.
	if(GetSaveFileName(&ofn));//InvalidateRect(hwnd,NULL,TRUE);
	else return MSGCANCEL;//Cancel0Will return
	fp = fopen(filename,"rb");

	//Existing file exists?  OFN_OVERWRITEPROMPT It made unnecessary by designation.
	//if(fp != NULL)
	//{
	//	fclose(fp);
	//	return MSGEXISFILE;//Existing file
	//}
	return MSGSAVEOK;
}

//Hook Procedure
UINT CALLBACK OFNHookProc(HWND hdlg, UINT msg, WPARAM wParam,LPARAM lParam)
{
	RECT  rcWindow;
	LPOFNOTIFY lpOfn = (LPOFNOTIFY)lParam;

    switch(msg){
        case WM_INITDIALOG:
            return TRUE;
			break;
		case WM_NOTIFY:
			if(lpOfn->hdr.code == CDN_INITDONE ){
			}
			if(lpOfn->hdr.code == CDN_FILEOK ){
			}
				if(count_of_INIT_DONE>=0)count_of_INIT_DONE++;
				if(count_of_INIT_DONE>=3){
					ShowWindow(GetParent(hdlg),SW_SHOW);
					GetWindowRect( GetParent(hdlg), &rcWindow );
					SetWindowPos( GetParent(hdlg), HWND_TOP, CmnDialogWnd.left, CmnDialogWnd.top,
						CmnDialogWnd.right,CmnDialogWnd.bottom, SWP_NOZORDER ); 
					count_of_INIT_DONE = -9999999;
				}
			//MessageBox(NULL,"Initialized","Message",MB_OK);
			return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case IDC_BUTTON1:
					GetWindowRect( GetParent(hdlg), &rcWindow );
					CmnDialogWnd.top = rcWindow.top;
					CmnDialogWnd.left = rcWindow.left;
					CmnDialogWnd.right = rcWindow.right;
					CmnDialogWnd.bottom = rcWindow.bottom;
                  return TRUE;
            }
            break;

    }
    return FALSE;
}

char GetFileNameLoad(HWND hwnd,char *title, int OpenType)
{//Get file name(Load)
	OPENFILENAME ofn;
	FILE *fp;
//	char res;//Result of file open

	memset(&ofn,0,sizeof(OPENFILENAME));
//	strcpy(GetName,"*.pmd");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner   = hwnd;
	ofn.hInstance   = hInst;
	//ofn.lpstrFilter = "OrganyaData[*.org]¥0*.org¥0All formats [*.*]¥0*.*¥0¥0";	// 2014.10.19 D
	ofn.lpstrFilter = MessageString[IDS_STRING111];	// 2014.10.19 A
	ofn.lpstrFile   = music_file;
	ofn.nMaxFile    = MAX_PATH;
	ofn.lpstrTitle  = title;
	ofn.Flags       = OFN_CREATEPROMPT | OFN_HIDEREADONLY ;
	if(OpenType==1){
		ofn.Flags       = OFN_CREATEPROMPT | OFN_HIDEREADONLY |OFN_EXPLORER | 
						OFN_ENABLESIZING | OFN_ENABLEHOOK |     //Use hook procedure
							OFN_ENABLETEMPLATE; 
		ofn.lpfnHook = OFNHookProc;
		ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOGOPEN);

	}
	ofn.lpstrDefExt = "org";
	
	count_of_INIT_DONE = 0;
	//Attempt to acquire the file name.
	if(GetOpenFileName(&ofn));//InvalidateRect(hwnd,NULL,TRUE);
	else return MSGCANCEL;//Cancel0Will return
	fp = fopen(music_file,"rb");
	if(fp == NULL){
		//MessageBox(hwnd,"File can not be accessed","",MB_OK);	// 2014.10.19 D
		msgbox(hwnd,IDS_WARNING_ACCESS_FILE,IDS_ERROR,MB_OK);	// 2014.10.19 A
		return MSGCANCEL;//Specified file does not exist
	}
	fclose(fp);

	return MSGLOADOK;
}

char GetFileNameLoadPtcop(HWND hwnd,char *title)
{//Get file name(Load)
	OPENFILENAME ofn;
	FILE *fp;
//	char res;//Result of file open

	memset(&ofn,0,sizeof(OPENFILENAME));
//	strcpy(GetName,"*.pmd");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner   = hwnd;
	ofn.hInstance   = hInst;
	//ofn.lpstrFilter = "OrganyaData[*.org]¥0*.org¥0All formats [*.*]¥0*.*¥0¥0";	// 2014.10.19 D
	ofn.lpstrFilter = MessageString[IDS_STRING120];	// 2014.10.19 A
	ofn.lpstrFile   = music_file;
	ofn.nMaxFile    = MAX_PATH;
	ofn.lpstrTitle  = title;
	ofn.Flags       = OFN_HIDEREADONLY ;

	ofn.lpstrDefExt = "ptcop";
	
	count_of_INIT_DONE = 0;
	//Attempt to acquire the file name.
	if(GetOpenFileName(&ofn));//InvalidateRect(hwnd,NULL,TRUE);
	else return MSGCANCEL;//Cancel0Will return
	fp = fopen(music_file,"rb");
	if(fp == NULL){
		//MessageBox(hwnd,"File can not be accessed","",MB_OK);	// 2014.10.19 D
		msgbox(hwnd,IDS_WARNING_ACCESS_FILE,IDS_ERROR,MB_OK);	// 2014.10.19 A
		return MSGCANCEL;//Specified file does not exist
	}
	fclose(fp);

	return MSGLOADOK;
}

int decodePxInt(FILE * fp)
{
	int v = 0x00;
	fread(&v, 1, 1, fp);
	//printf("%i\n", v);
	if (v > 0x7f)
		return v + 0x80*(decodePxInt(fp) - 1);
	else
		return v;
}

#pragma pack(1) //one byte imprecision is too much...

typedef struct __attribute__((packed))
{
	short unknown1 = 0x1E0;
	byte beat;
	short unknown2 = 0;
	short tempo;
	int repeat;
	int last;
	
}MASTERV5BLOCK;

#pragma pack()

typedef struct
{
	int position;
	byte event_id;
	int value;
	
}PxEvent;

typedef struct
{
	PxEvent Events[16383];
	int next;
	bool unused;

}PxUnit;

BOOL ConvertPtcopData(PxUnit * Units, MASTERV5BLOCK song_data);

enum
{
	EVENTKIND_NULL  = 0 ,//  0

	EVENTKIND_ON        ,//  1
	EVENTKIND_KEY       ,//  2
	EVENTKIND_PAN_VOLUME,//  3
	EVENTKIND_VELOCITY  ,//  4
	EVENTKIND_VOLUME    ,//  5
	EVENTKIND_PORTAMENT ,//  6
	EVENTKIND_BEATCLOCK ,//  7
	EVENTKIND_BEATTEMPO ,//  8
	EVENTKIND_BEATNUM   ,//  9
	EVENTKIND_REPEAT    ,// 10
	EVENTKIND_LAST      ,// 11
	EVENTKIND_VOICENO   ,// 12
	EVENTKIND_GROUPNO   ,// 13
	EVENTKIND_TUNING    ,// 14
	EVENTKIND_PAN_TIME  ,// 15

	EVENTKIND_NUM       ,// 16
};

BOOL ConvertPtcopData(PxUnit * Units, MASTERV5BLOCK song_data)
{
	MUSICINFO& info = org_data.info;
	NOTELIST* np;
	
	int resolution = 10; //notes above 255 length are cut off
	
	//simple maths up ahead
	double exponent = ((song_data.tempo - 16243.57) / 185.45);
	int wait = (60000 / pow(2.71828, exponent)) / 4;
	info.wait = (int)(wait / resolution);//todo, convert based on number of steps
	if(!info.wait) info.wait = 1;
	
	info.line = song_data.beat;
	info.dot = 4 * resolution;
	info.repeat_x = (song_data.repeat / song_data.beat / 0x1E0);
	info.end_x = 555555;//(song_data.last / song_data.beat / 0x1E0);
	
	//move this **** to rainfunction.cpp
	
	for (int i=0; i < 8; i++) //iterate through UNIT
	{
		PxUnit& unit = Units[i];
		PxEvent* events = unit.Events;
		
		bool isDrum;
		int lowLength = 9999999;
		int drumThreshold = 4;
		int drumTrack;
		int lastPos = 0;
		
		bool on = true;
		int length = 1;
		int newLength = 1;
	
		
		bool newPan = true;
		bool keyChange = true;
		bool newVolume = true;
		
		int pitch = 0x1000;
		int levelPan = 6 * 11;
		int velocity = 10;
		int levelVolume = 64;
		
		if(unit.unused) continue; //is this really needed?
	
		if(i >= drumThreshold)
		{
			isDrum = true;
			drumTrack = i+8-drumThreshold;
			np = info.tdata[drumTrack].note_p;
			info.tdata[drumTrack].note_list = info.tdata[drumTrack].note_p;
		}
		else
		{
			isDrum = false;
			np = info.tdata[i].note_p;
			info.tdata[i].note_list = info.tdata[i].note_p;
		}
		np->from = NULL;
		np->to = (np + 1);
		np++; //first event being completely invalid causes the rest of the song to not play
			  //if you don't start from the next measure
		
		for(int j=0; j <= unit.next; j++)
		{
			PxEvent e = events[j];
			printf("Unit: %i ", i);
			printf("Event: %i Position: %i Event Id; %i Value: %i \n", j, e.position, e.event_id, e.value);
			switch(e.event_id) //unimplemented events can't trigger note placement
			{
				case EVENTKIND_NULL:
				case EVENTKIND_PORTAMENT:
				case EVENTKIND_BEATCLOCK:
				case EVENTKIND_BEATTEMPO:
				case EVENTKIND_BEATNUM:  
				case EVENTKIND_REPEAT:   
				case EVENTKIND_LAST:     
				case EVENTKIND_VOICENO:  
				case EVENTKIND_GROUPNO:  
				case EVENTKIND_TUNING:   
				case EVENTKIND_PAN_TIME: 
				continue;
				break;
			}
			if (e.position != lastPos || e.position == -1)
			{
			
				//commit note
				//option to ignore pitch bends...
				
				//handle case where there's already a note in that position
				//push note forward where there's room, or ignore
				// and count how many times this happens
				
				//pitch bends do not create a on event, so the length is implicitly carried from the remaining time of the initial note
				
				if(j > 0) 
				{
					np->from = (np - 1); //not setting the from pointer on all events seems to be fine...
										 //but you just can't access the previous linked note from that pointer
										 //it seems to crash half the time in PutNotes while checking if the ptr is null
										 //if the froms/to's are (properly?) set
										 //but when it doesn't crash the notes seem to play fine,
										 //just that nothing renders
										 //other times only one sound plays ever
				}
				else
				{
					np->from = NULL;
				}
				if(e.position == -1)// this is the last note
				{
					np->to = NULL;
				}
				else
				{
					np->to = (np + 1);
				}
				int overlap = lastPos / (120 / resolution);
				printf ("event pos: %i np->x %i \n", lastPos, overlap); /////////////////why do (parts of) tracks disappear randomly?
				if (np->from && overlap <= np->from->x && np->from->x != 0)         //why don't drums work?
				                                                        //implement overlap properly
				{
					lastPos = e.position; continue;
				}
				np->x = overlap;
				
				/////////LENGTH HANDLING
				if(isDrum)
				{
					np->length = 1;
				}
				else if(keyChange && !on)
				{
					/*pitch = KEYDUMMY; // ignore pitch bends
					np->length = 1;
					newPan = true;
					newVolume = true;*/
					//changing key
					
					//also affects pan and volume events too, fix
					NOTELIST* lastOn = np->from;
					if(np->from)
					{
						//DIRTY hack: we know that pitch bends only exist if the last on's length is above 1... maybe
						while (lastOn->length <= 1)
						{
							printf("laston length %i skipping \n", lastOn->length);
							lastOn--;
						}
							
						int difference = (np->x - lastOn->x);
						printf("difference %i skipping \n", difference);
						int newLength = lastOn->length - difference;
						printf("newLength %i \n", newLength);
						lastOn->length -= newLength;
						np->length = newLength;
					}
					else
					{
						printf("WHAT????//?????????????//?????????????//?????????????//?????????????//?????????????");
						np->length = length; //?????????????
					}

					newPan = true;
					newVolume = true;
				}
				else if(on) //given that i set the length (and all the other values) to 1 after every note placement,
							//are any of these bools neccesary?
				{
					np->length = length / (120 / resolution);
					newPan = true;
					newVolume = true;
				}
				else // only a volume/pan event
				{
					pitch = KEYDUMMY;
					np->length = 1;
				}
				if(!np->length) np->length = 1;
				
				
				if(pitch == KEYDUMMY)
				{
					np->y = pitch;
				}
				else
				{
					np->y = (pitch - 0x3F00) / 0x100;
				}
				if(newVolume)
				{
					np->volume = (velocity); //calc
				}
				else
				{
					np->volume = VOLDUMMY;
				}
				
				if(newPan)
				{
					np->pan = levelPan / 0xB;
				}
				else
				{
					np->pan = PANDUMMY;
				}
				newPan = false;
				keyChange = false;
				newVolume = false;
				on = false;
				
				//printf("np from %i np to %i", np->from, np->to);
				
				np++;
				
				length = 1;
				pitch = KEYDUMMY;
				
				lastPos = e.position;
			}

			switch(e.event_id)
			{
				case EVENTKIND_ON:			
					length = e.value;
					if(length <= lowLength) lowLength = length; 
					on = true;
					break;
				case EVENTKIND_KEY:			pitch = e.value; keyChange = true; break; //DISABLE OPTION
				case EVENTKIND_PAN_VOLUME:	levelPan = e.value; newPan = true; break;
				case EVENTKIND_VELOCITY:	velocity = e.value; newVolume = true; break;
				case EVENTKIND_VOLUME:		levelVolume = e.value; newVolume = true; break;
				default: break;
			}
	
		}
		
		//np->to = NULL;
		
		printf("lowest length %i \n", lowLength);
		
		
	}
	
	//pasted from loadmusicdata
	//can't seam to load another ptcop after already loading one,
	//even though i don't see anything the other load functions do differently
	int i;
	for(int j = 0; j < MAXMELODY; j++)
		MakeOrganyaWave(j,info.tdata[j].wave_no, info.tdata[j].pipi);
	for(int j = MAXMELODY; j < MAXTRACK; j++){
		i = info.tdata[j].wave_no;
		InitDramObject(dram_name[i],j-MAXMELODY);
	}
	//Show to player
	char str[32];
	org_data.SetPlayPointer(0);//Cueing
	scr_data.SetHorzScroll(0);
	itoa(info.wait,str,10);
	SetDlgItemText(hDlgPlayer,IDE_VIEWWAIT,str);
	SetDlgItemText(hDlgPlayer,IDE_VIEWMEAS,"0");
	SetDlgItemText(hDlgPlayer,IDE_VIEWXPOS,"0");

	MakeMusicParts(info.line,info.dot);//Generate parts
	MakePanParts(info.line,info.dot);
	//PutRecentFile(music_file);
	//↓2014.05.06 A
	if(SaveWithInitVolFile != 0){
		AutoLoadPVIFile();
	}
	return TRUE;
}

BOOL LoadPtcopData(void)
{
	static const char* _code_proj_v5      = "PTCOLLAGE-071119";
	static const char* _code_MasterV5     = "MasterV5";
	static const char* _code_Event_V5     = "Event V5";
	
	//ORGANYADATA org_data;
	
	PxUnit Units[8];
	
	int numEvents;
	int blockSize;
	
	int position;
	int absolutePosition = 0;
	
	int unit_id;
	int event_id;
	int event_value;
	
	char pass_check[16];
	char tag_check[8];
	
	MASTERV5BLOCK song_data;

	FILE *fp;
	if((fp=fopen(music_file,"rb"))==NULL)
	{
		msgbox(hWnd,IDS_WARNING_ACCESS_FILE,IDS_ERROR_LOAD,MB_OK);
		return(FALSE);
	}
	//verify ptcop header
	fread(&pass_check[0], sizeof(char), 16, fp);
	if( memcmp(pass_check, _code_proj_v5, 16) )
	{
		fclose(fp);
		msgbox(hWnd,IDS_WARNING_PTCOP,IDS_ERROR_LOAD,MB_OK);
		return FALSE;
	}

	//seek to masterv5, skipping some int
	fseek(fp, 4, SEEK_CUR);
	fread(&tag_check[0], sizeof(char), 8, fp);
	if( memcmp(tag_check, _code_MasterV5, 8) )
	{
		fclose(fp);
		MessageBox(hWnd,"masterv5 missing","Errrrrrrrrror (Load)",MB_OK);
		//msgbox(hWnd,IDS_WARNING_PTCOP,IDS_ERROR_LOAD,MB_OK);
		return FALSE;
	}
	
	fread(&blockSize, 4, 1, fp);
	printf("MasterV5 size: %i \n", blockSize);
	
	fread(&song_data, sizeof(MASTERV5BLOCK), 1, fp);
	printf("beat: %i tempo: %i repeat: %i last: %i \n", song_data.beat, song_data.tempo, song_data.repeat, song_data.last);
	
    fread(&tag_check[0], sizeof(char), 8, fp);
	if( memcmp(tag_check, _code_Event_V5, 8) )
	{
		printf("HELP ME %s", tag_check);
		fclose(fp);
		MessageBox(hWnd,"eventv5 missing","Errrrrrrrrror (Load)",MB_OK);
		//msgbox(hWnd,IDS_WARNING_PTCOP,IDS_ERROR_LOAD,MB_OK);
		return FALSE;
	}
	
	fread(&blockSize, 4, 1, fp);
	printf("EventV5 size: %i \n", blockSize);
	
	fread(&numEvents, sizeof(int), 1, fp);
	printf("NumEvents %i \n", numEvents);
	
	for( int i = 0; i < numEvents; i++ )
	{
		position = decodePxInt(fp);
		fread(&unit_id, 1, 1, fp);
		fread(&event_id, 1, 1, fp);
		event_value = decodePxInt(fp);
		
		absolutePosition += position;
		
		//printf("Event: %i Position: %i Unit Id: %i Event Id; %i Value: %i \n", i, absolutePosition, unit_id, event_id, event_value);
		
		int& next = Units[unit_id].next;
		Units[unit_id].Events[next].position = absolutePosition;
		Units[unit_id].Events[next].event_id = event_id;
		Units[unit_id].Events[next].value = event_value;
		next++;
	}
	
	for (int i=0; i < 8; i++) //iterate through UNIT
	{
		int& next = Units[i].next;
		Units[i].Events[next].position = -1; //end of events marker...
		Units[i].Events[next].event_id = 1;
		
		if(next < 2)
		{
			Units[i].unused = true;
		
		}
		else
		{
			Units[i].unused = false;
		}
	}
	//here we go
	ConvertPtcopData(Units, song_data);
	
	fclose(fp);
	
	return TRUE;
	
}
