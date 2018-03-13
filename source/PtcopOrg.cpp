#include "Setting.h"
#include "Gdi.h"
#include <stdio.h>
#include "resource.h"
#include "DefOrg.h"
#include "OrgData.h"
#include "Sound.h"
#include "Scroll.h"
#include "rxoFunction.h"
//#include "util.h"
#include "math.h"
#include <algorithm>
#include "Commdlg.h"

#define NUMUNIT 16

#pragma pack(1) //one byte imprecision is too much...

typedef struct __attribute__((packed))
{
	short beatclock;      //number of ticks per beat
	char beat;            //number of beats per measure
	float beattempo;      //bpm
	int repeat;           
	int last;
	
}MASTERV5BLOCK;

#pragma pack()

typedef struct
{
	int position;
	char event_id;
	int value;
	int relativePos;
	
}PxEvent;

typedef struct
{
	PxEvent Events[8000];
	int next;
	bool unused;

}PxUnit;

extern char *dram_name[];

extern HWND hDlgPlayer;

float resolution = 4; //notes above 255 length are cut off
int drumThreshold = 4;
int pitchOffset = 0;
bool ignorePitchBend = false;
bool clampPanValues = false;//pxtone's pan is less extreme, but org's pan really goes from one ear to the next
bool allowOverlapNotes = false;
bool shiftOverlap = false;
bool suppressOutput = false;
int defaultWaveNumbers[16] = {NULL};

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

int decodePxInt(FILE * fp)
{
	int v = 0x00;
	fread(&v, 1, 1, fp);
	if (v > 0x7f)
		return v + 0x80*(decodePxInt(fp) - 1);
	else
		return v;
}

BOOL CALLBACK RelocateControls(HWND hwnd, LPARAM DlgWidth)
{
	//cursed function!!! relocates controls ON OTHER PROCESSES?!?!?!?!? INCLUDING EXPLORER.EXE
	LPRECT lpRect;
	GetWindowRect(hwnd, lpRect);
	int width = ((DlgWidth - 555)/4);
	if(width <= 0) width = 1;
	SetWindowPos(hwnd, HWND_TOP, width, lpRect->top, 1, 1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	return true;
}

UINT CALLBACK OFNHookProcPTCOP(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
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
			
			sprintf(ctmp, "%.3g", resolution);
			SetDlgItemText(hdlg,IDC_PTCOP_RES, ctmp);
			itoa(drumThreshold, ctmp, 10);
			SetDlgItemText(hdlg,IDC_PTCOP_DT, ctmp);
			itoa(pitchOffset, ctmp, 10);
			SetDlgItemText(hdlg,IDC_PTCOP_POFF, ctmp);
			
			if(ignorePitchBend) CheckDlgButton(hdlg, IDC_PTCOP_IGNOREPBEND, BST_CHECKED);
			if(allowOverlapNotes) CheckDlgButton(hdlg, IDC_PTCOP_OVERLAP, BST_CHECKED);
			
			//Edit_SetCueBannerText(haDlg, "4");
			//Edit_SetCueBannerText(hCtrl, IDC_PTCOP_DT, 4);
			//Edit_SetCueBannerText(hCtrl, IDC_PTCOP_POFF, 0);
 			/*for(j=0;j<8;j++){
				SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_RESETCONTENT,0,0);//Initialization
				for(i=0;i<128;i++){
					SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_ADDSTRING,0,(LPARAM)MIDIPC[i]);
				}
				if(ucMIDIProgramChangeValue[j] == 255){
					ucMIDIProgramChangeValue[j] = mi.tdata[j].wave_no; //Initialized in this place
				}
				SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_SETCURSEL,ucMIDIProgramChangeValue[j],0);
			}*/
           return TRUE;

		case WM_SIZE:
			return TRUE;
			DlgWidth  = LOWORD(lParam);	//Size of client area
			DlgHeight = HIWORD(lParam);
			
			//EnumChildWindows(hdlg, RelocateControls, DlgWidth);
			return TRUE;

		case WM_PAINT:
			hdc = BeginPaint(hdlg, &ps);
			//DrawGr(hdlg, hdc);
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
				GetDlgItemText(hdlg,IDC_PTCOP_RES,ctmp,12);
				//move to dialog...
				resolution = atof(ctmp); //don't see why i shouldn't allow float values
				GetDlgItemText(hdlg,IDC_PTCOP_DT,ctmp,12);
				drumThreshold = atol(ctmp);
				GetDlgItemText(hdlg,IDC_PTCOP_POFF,ctmp,12);
				pitchOffset = atol(ctmp);
				
				ignorePitchBend = IsDlgButtonChecked(hdlg, IDC_PTCOP_IGNOREPBEND);
				allowOverlapNotes = IsDlgButtonChecked(hdlg, IDC_PTCOP_OVERLAP);
				
				//vector math

				/*for(j=0;j<8;j++){
					ucMIDIProgramChangeValue[j] = (unsigned char)SendDlgItemMessage(hdlg,IDC_MIDIPC1+j,CB_GETCURSEL,0,0);
				}*/

				//MessageBox(NULL,ctmp,"Message",MB_OK);
			}
			//MessageBox(NULL,"Initialized","Message",MB_OK);
			return TRUE;
    }
    return FALSE;
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
	ofn.Flags       = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_ENABLESIZING;
	
	ofn.lpfnHook = OFNHookProcPTCOP;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_PTCOP);

	ofn.lpstrDefExt = "ptcop";
	
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

BOOL ConvertPtcopData(PxUnit * Units, MASTERV5BLOCK song_data)
{
	MUSICINFO& info = org_data.info;
	NOTELIST* np;
	
	//1,  2  3, 4, 5, 6, 7,  8, 9,  10, 11,  12
	//120,60,40,30,24,20,17*,15,13*,12, 10.9,10
	
	bool repeatAtEnd = false;
	int highPos = 0;
	
	int overlappingNotes[NUMUNIT];
	int lowestNoteLengths[NUMUNIT];
	int lowestNotePos[NUMUNIT];
	int inaccurateNotePos[NUMUNIT];
	int truncatedNoteLengths[NUMUNIT];
	
	int numOrgEventsTotal = 0;
	
	if(resolution >= 32 || drumThreshold <= -8)
	{/*
		TASKDIALOGCONFIG pTaskConfig; 
		memset(&pTaskConfig, 0, sizeof(TASKDIALOGCONFIG));
		
		pTaskConfig.cbSize = sizeof(TASKDIALOGCONFIG);
		pTaskConfig.hwndParent = NULL;
		pTaskConfig.hInstance = NULL;
		pTaskConfig.dwFlags = TDF_USE_COMMAND_LINKS;
		pTaskConfig.dwCommonButtons = TDCBF_YES_BUTTON;
		pTaskConfig.pszWindowTitle = L"info";
		pTaskConfig.hMainIcon = NULL;
		pTaskConfig.pszMainIcon = TD_INFORMATION_ICON;
		pTaskConfig.pszMainInstruction = L"thats TOO MANY";
		pTaskConfig.cButtons = 1;
		TASKDIALOG_BUTTON buttons[] = { 
			{0,L"try agaig\nno you bloated doushe"}
		};
		pTaskConfig.pButtons = buttons;
		pTaskConfig.nDefaultButton = 0,
		
		TaskDialogIndirect(&pTaskConfig, NULL, NULL, NULL);*/
		MessageBox(hWnd,"thats TOO MANY\ntry agaig","no you bloated doushe",MB_OK);
		return FALSE;
	}
	
	if(resolution <= 0) {}
	
	info.dot = song_data.beat * resolution;
	if(info.dot <= 0) info.dot = 1;
	info.line = song_data.beat;
	
	float wait = (60000 / song_data.beattempo) / info.line;
	info.wait = round(wait / resolution);
	if(info.wait <= 0) info.wait = 1;
	
	info.repeat_x = (song_data.repeat / song_data.beatclock) * info.dot;
	info.end_x = (song_data.last / song_data.beatclock) * info.dot;
	
	if(info.end_x == 0) repeatAtEnd = true;
	
	for (int i=0; i < NUMUNIT; i++) //iterate through UNIT
	{
		PxUnit& unit = Units[i];
		PxEvent* events = unit.Events;
		NOTELIST* lastOn = NULL;
		
		bool isDrum = false;
		int drumTrack = 0;
		int lastPos = 0;
		
		bool on = false;
		int length = 1;
		int newLength = 1;
		int trueLength = 0;

		bool newPan = true;
		bool keyChange = false;
		bool newVolume = true;
		bool volPanEve = false;
		
		int pitch = 24576; //default pitch for samples, A 4?
		const double panDivisor = 10.666666666;
		int levelPan = 64;
		int velocity = 104;
		int levelVolume = 104;
		int portament = 0;
		
		int numOrgEvents = 0;
		
		overlappingNotes[i] = 0;
		lowestNoteLengths[i] = 99999999;
		lowestNotePos[i] = 99999999;
		inaccurateNotePos[i] = 0;
		truncatedNoteLengths[i] = 0;
		
		if(unit.unused) continue;
	
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
		if(i > 15 || drumTrack > 15)
		{
			MessageBox(hWnd,"Attempted to initialize more than 16 tracks! \
			\nPlease increase the drum threshold.","Errrrrrrrrror (Load)",MB_OK);
			//msgbox(hWnd,IDS_WARNING_PTCOP,IDS_ERROR_LOAD,MB_OK);
			return FALSE;
		}
		np->from = NULL;
		np->to = (np + 1);
		//np++; //first event being completely invalid causes the rest of the song to not play
			  //if you don't start from the next measure
		
		for(int j=0; j <= unit.next; j++)
		{
			PxEvent e = events[j];
			//printf("Unit: %i ", i);
			//printf("Event: %i Position: %i Event Id; %i Value: %i \n", j, e.position, e.event_id, e.value);
			switch(e.event_id) //unimplemented events can't trigger note placement
			{
				case EVENTKIND_NULL:
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
			if(e.relativePos <= lowestNotePos[i] && e.relativePos > 0) lowestNotePos[i] = e.relativePos;
			
			if (e.position != lastPos || e.position == -1)
			{
				//---------commit note
				
				//handle case where there's already a note in that position
				//push note forward where there's room, or ignore
				// and count how many times this happens
				
				//pitch bends do not create a on event, so the length is implicitly carried from the remaining time of the initial note
				
				if(numOrgEvents > 0) 
				{
					//np->from = (np - 1);//not setting the from pointer on all events seems to be fine...
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
					//lastOn = np; // don't know
				}
				
				if(e.position == -1)// this is the last note
				{
					np->to = NULL;
				}
				else
				{
					np->to = (np + 1);
				}
				
				int xScaled = lastPos / (song_data.beatclock / info.line / resolution);
				//printf ("event pos: %i np->x %i \n", lastPos, xScaled); 
				if ((np-1) && xScaled <= (np-1)->x && numOrgEvents > 0)         ///////////////////////placeholder until np->from works again
				{
					overlappingNotes[i]++;
					if (shiftOverlap)
					{
						xScaled = (np-1)->x + 1;
					}
					else if(!allowOverlapNotes)
					{
						if(!on)
						{
							lastPos = e.position; 
							continue;
						}
						np--; //allows important notes to replace the previous one, for usually better sounding results at low res
						//lastOn = NULL;
					}
				}
				np->x = xScaled;
				if(xScaled > highPos) highPos = xScaled;

				int inaccuracy = np->x * (song_data.beatclock / info.line / resolution);
				if(inaccuracy != lastPos)
				{
					inaccurateNotePos[i]++;
				}
				
				//-------------LENGTH HANDLING
				if(isDrum && on)
				{
					np->length = 1;
					lastOn = np;
				}
				else if(keyChange && !on)
				{
					if(lastOn && np->x > (lastOn->x + lastOn->length))
					{
						lastPos = e.position;
						continue; //this is not a valid note...
					}
					else
					{
						if(ignorePitchBend && portament != 0)
						{
							np->y = KEYDUMMY;
							np->length = 1;
							newPan = true;
							newVolume = true;
							volPanEve = true;
						}
						else
						{
							//changing key
							if(lastOn)
							{
									
								int difference = (np->x - lastOn->x);
								//printf("difference %i skipping \n", difference);
								if(trueLength >= 1)
								{
									newLength = trueLength - difference;
									if(newLength >= 255) newLength = 255;
								}
								else
								{
									newLength = lastOn->length - difference;
								}
								trueLength -= newLength;
								//printf("newLength %i \n", newLength);
								lastOn->length = difference;
								np->length = newLength;
								lastOn = np;
							}
							else
							{
								printf("WHAT????//?????????????//?????????????//?????????????//?????????????//?????????????");
								np->length = 1; //?????????????
							}
							newPan = true;
							newVolume = true;
						}
					}
				}
				else if(on)
				{
					int lengthScaled = length / (song_data.beatclock / info.line / resolution);
					if (lengthScaled >= 255) 
					{
						trueLength = lengthScaled;
						lengthScaled = 255; 
						truncatedNoteLengths[i] += 1;
					}
					np->length = lengthScaled;
					newPan = true;
					newVolume = true;
					//this is a band aid fix for length issues that should not be happening
					//if (lastOn && lastOn->x + lastOn->length >= np->x) lastOn->length -= (lastOn->x + lastOn->length) - np->x;
					
					lastOn = np;
				}
				else // only a volume/pan event
				{
					np->y = KEYDUMMY;
					np->length = 1;
					volPanEve = true;
				}
				if(!np->length) np->length = 1;
				
				if(!volPanEve)
				{
					np->y = ((pitch - 0x3F00) / 0x100) + pitchOffset;
				}
				
				if(newVolume)
				{
					/*double velocityDiv = (velocity / 1.20);
					double volumeDiv = (levelVolume / 1.28);
					double volumePct = (levelVolume / 100.00);
					printf("vd %f vd %f vp %f \n", velocityDiv, volumeDiv, volumePct);*/
					double work = 1;
					work = (work * velocity) / 128;
					work = (work * levelVolume) / 128;
					work = log(work*12.8) * 99.6295206; //approximate calculation, mostly a guess but it sounds right
					
					//work = (work * 254);
					//work = work + (work * 0.4); //apply a nice 40% bonus
					
					np->volume = std::min(std::max((int)(work), 1), 254); //most songs don't go over 104 velocity/volume
					//(velocity + levelVolume)
					//velocityDiv + volumeDiv) * (volumePct)
				}
				else
				{
					np->volume = VOLDUMMY;
				}
				
				if(newPan)
				{
					np->pan = round((double)levelPan / panDivisor);
					//printf("levelpan %i/ divide %f / pan %i \n", levelPan, (double)levelPan / panDivisor, np->pan);
				}
				else
				{
					np->pan = PANDUMMY;
				}
				newPan = false;
				keyChange = false;
				newVolume = false;
				on = false;
				volPanEve = false;
				
				////printf("np from %i np to %i", np->from, np->to);
				
				np++;
				numOrgEvents++;
				
				length = 1;
				//if(!isDrum) pitch = KEYDUMMY; //pitch events are implicitly carried over
				
				lastPos = e.position;
			}

			switch(e.event_id)
			{
				case EVENTKIND_ON:			
					length = e.value;
					if(length <= lowestNoteLengths[i] && length > 0) lowestNoteLengths[i] = length; 
					on = true;
					break;
				case EVENTKIND_KEY:			pitch = e.value; keyChange = true; break; //DISABLE OPTION
				case EVENTKIND_PORTAMENT:	portament = e.value; break;
				case EVENTKIND_PAN_VOLUME:	levelPan = e.value; newPan = true; break;
				case EVENTKIND_VELOCITY:	velocity = e.value; newVolume = true; break;
				case EVENTKIND_VOLUME:		levelVolume = e.value; newVolume = true; break;
				default: break;
			}
	
		}
		
		//np->to = NULL;
		numOrgEventsTotal += numOrgEvents;
		
	}
	
	for(int i=0;i<NUMUNIT;i++)
	{
		if(!Units[i].unused)
		{
			printf("track %i: lowestlength: %i lowestpos: %i overlap: %i inaccuracy: %i truncation: %i \n", \
					i, lowestNoteLengths[i], lowestNotePos[i], overlappingNotes[i], inaccurateNotePos[i], truncatedNoteLengths[i]);
		}
	}
	printf("numOrgEventsTotal: %i", numOrgEventsTotal);
	
	if(repeatAtEnd)
	{
		info.end_x = ceil((double)highPos / (info.line * info.dot)) * (info.line * info.dot);
	}
	
	//sort note data, since the "going back" link of a linked list is completely gone
	//this is no longer true, but undefined behavior still causes crashes so keep this in
	org_data.SortNotes();
	
	//pasted from loadmusicdata
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
	sprintf(str, "%d", info.wait);
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
	
	PxUnit Units[NUMUNIT];
	memset(&Units, 0, sizeof(Units));

	int numEvents = 0;
	int blockSize = 0;
	
	int position = 0;
	int absolutePosition = 0;
	
	int unit_id = 0;
	int event_id = 0;
	int event_value = 0;
	
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
	printf("beat: %i tempo: %f repeat: %i last: %i \n", song_data.beat, song_data.beattempo, song_data.repeat, song_data.last);
	
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
		if(unit_id > 15)
		{
			fclose(fp);
			MessageBox(hWnd,"Attempted to load more than 16 tracks! \
			\nPlease reduce the number of tracks in the PTCOP.","Errrrrrrrrror (Load)",MB_OK);
			return FALSE;
		}
		fread(&event_id, 1, 1, fp);
		event_value = decodePxInt(fp);
		
		absolutePosition += position;
		
		//printf("Event: %i Position: %i Unit Id: %i Event Id; %i Value: %i \n", i, absolutePosition, unit_id, event_id, event_value);
		
		int& next = Units[unit_id].next;
		
		if(next >= 8000)
		{
			fclose(fp);
			MessageBox(hWnd,"Attempted to load more than 8000 events in 1 unit! \
			\nPlease reduce the size of the ptcop.","Errrrrrrrrror (Load)",MB_OK);
			//msgbox(hWnd,IDS_WARNING_PTCOP,IDS_ERROR_LOAD,MB_OK);
			return FALSE;
		}
		
		Units[unit_id].Events[next].position = absolutePosition;
		Units[unit_id].Events[next].relativePos = position;
		Units[unit_id].Events[next].event_id = event_id;
		Units[unit_id].Events[next].value = event_value;
		next++;
	}
	
	for (int i=0; i < NUMUNIT; i++) //iterate through UNIT
	{
		int& next = Units[i].next;
		Units[i].unused = (next <= 1);
		if(Units[i].unused) continue;
		
		Units[i].Events[next].position = -1; //end of events marker...
		Units[i].Events[next].event_id = 1;
		
	}
	//here we go
	if(!ConvertPtcopData(Units, song_data))
	{
		fclose(fp);
		return FALSE;
	}
	//append a .org so you don't compeltely obliterate your ptcop by trying to save
	strcat(music_file, ".org");
	
	return TRUE;
	
}
