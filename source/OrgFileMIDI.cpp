#include <string.h>
#include "Setting.h"
#include "Gdi.h"
#include "DefOrg.h"
#include "OrgData.h"
#include <stdio.h>
#include <string.h>
#include "Sound.h"
#include "resource.h"
#include "Scroll.h"
#include "rxoFunction.h"

char strMIDI_TITLE[256];
char strMIDI_AUTHOR[256];
unsigned char ucMIDIProgramChangeValue[MAXTRACK];

//MIDIDelta Time(TLast - TFirst)And calculates the buffer*pStore in. The return value is the written byte length
int setDeltaTime(long TFirst, long TLast, unsigned char *p){
	unsigned long r, lDelta;
	unsigned long qbit[32], tbit[32], wbit[32];
	int i,j,k,flg;
	
	if(TLast > TFirst){
		lDelta = TLast - TFirst;
	}else{
		lDelta = TFirst - TLast;
	}

	if(lDelta <= 0x7F){ //1It fits in bytes, so it does not need subsequent troublesome calculations.
		*p = (unsigned char)(lDelta & 0x7F);
		return 1;
	}


	for(k=1,j=0,i=0; i<32; i++){
		if((i % 8) != 7){
			qbit[j] = k;
			j++;
		}
		tbit[i] = k;
		k = k * 2;
	}
	for(i=0;i<32;i++){
		wbit[i] = lDelta & tbit[i];
	}
	r = 0;
	for(i=0;i<32;i++){
		if(wbit[i] != 0){
			r = r | qbit[i];
		}
	}

	flg = 0;
	if((r & 0xFF000000) > 0){
		flg++;
		*p = (unsigned char)(0x80 | ((r / 0x1000000) & 0x7F)) ;
		p++;
	}
	if((r & 0xFF0000) > 0 || flg != 0){
		flg++;
		*p = (unsigned char)(0x80 | ((r / 0x10000) & 0x7F)) ;
		p++;
	}
	if((r & 0xFF00) > 0 || flg != 0){
		flg++;
		*p = (unsigned char)(0x80 | ((r / 0x100) & 0x7F)) ;
		p++;
	}
	flg++;
	*p = (unsigned char)(r & 0x7F);
	p++;

	return flg;
}


//standardMIDIOutput in format
BOOL OrgData::ExportMIDIData(char *strMidiFileName, int iRepeat){
	long iDeltaTime = 60; //iDeltaTime < 127It must be.
	unsigned char tmpuc;
	unsigned char *ucbuf,*p, lastVol = 100, *pBufLen;
	NOTELIST *np, *npStart;
	long CurrentX, LastX = 0, lenbuf;
	long  iUse,  iLeft;
	long  iUseTmp,  iLeftTmp;
	int rp, j;

	unsigned char flgNoteOff[MAXTRACK], flg89;
	//unsigned char NoteY[MAXTRACK] = {0,0,0,0, 0,0,0,0, 36, 38, 42, 46,  49, 50, 59, 66};
	unsigned char Convert_Wave_no_to_MIDI_no[] = { //Tam is easy to understand99.
        36,36,38,38,99,
        42,46,57,76,77,
        35,99,35,36,40,
        38,42,46,42,46,
        57,55,51,99,99,
        35,81,58,36,36,
        40,69,38,99,46,
        42,39,75,70,36,
        60,42,
        
        36,36,36,36,36,36,36,36, //←When adding a drum sound in future, please also modify here.
	};
	unsigned char Tom_DrumMap[] = { //99MadeTomSeasoned tone
		41, //0: Low Tom 2
		43, //1: Low Tom 1
		45, //2: Mid Tom 2
		47, //3: Mid Tom 1
		48, //4: High Tom 2
		50, //5: High Tom 1
	};
	unsigned char Tom_Threshold[] = { //TomSeasoning threshold value (When this value is exceeded, thatTombecome)
		0,  //0: Low Tom 2
		19, //1: Low Tom 1
		27, //2: Mid Tom 2
		34, //3: Mid Tom 1
		40, //4: High Tom 2
		47, //5: High Tom 1
	};
	unsigned char ucCurrentDrumKey[MAXTRACK];

	FILE *fp;
	if((fp=fopen(strMidiFileName,"wb"))==NULL){
		//MessageBox(hWnd,"File can not be accessed","Error (Export)",MB_OK);	// 2014.10.19 D
		msgbox(hWnd,IDS_WARNING_ACCESS_FILE,IDS_ERROR_EXPORT,MB_OK);	// 2014.10.19 A
		return(FALSE);
	}
	//                                                                   ↓Trnumber + ConductorTrack + Dummy + Drum
	unsigned char strMIDIHeader[] = {'M', 'T', 'h', 'd', 0,0,0,6,  0,1,  0,  9+1+1  ,0,0};
	//Resolution of quarter note
	strMIDIHeader[12] = (unsigned char)((info.dot * iDeltaTime / 0x100) & 0x7F);
	strMIDIHeader[13] = (unsigned char)((info.dot * iDeltaTime) & 0xFF);
	fwrite(strMIDIHeader, 14, 1, fp);

	unsigned char strConductorTrack[] = {'M','T','r','k',0,0,0,0x0B, 0x00, 0xFF, 0x51, 0x03,  0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, };
	//tempo
	//long ltempo, invtempo;
	long invtempo, LastPan;
	int i, iUseCode90, isdl;
	unsigned char *pLast;

	//ltempo = 60 * 1000 / (info.wait * info.dot * iDeltaTime);
	//invtempo = 60 * 1000000 / (60 * 1000 / (info.wait * info.dot ));
	
	p = &strConductorTrack[8]; lenbuf = 8;

	//Initial tempo=999
	*p++=0x00; lenbuf++;
	*p++=0xFF; lenbuf++;
	*p++=0x51; lenbuf++;
	*p++=0x03; lenbuf++;
	invtempo = 25 * info.wait * info.dot ;

	tmpuc = (unsigned char)((invtempo / 0x10000) % 0x0100); *p++ = tmpuc; lenbuf++;
	tmpuc = (unsigned char)((invtempo / 0x100) % 0x0100);   *p++ = tmpuc; lenbuf++;
	tmpuc = (unsigned char)((invtempo / 0x1) % 0x0100);     *p++ = tmpuc; lenbuf++;

	//Title display of song
	tmpuc = (unsigned char)strlen(strMIDI_TITLE);
	if(tmpuc>0){
		*p++=0x00; lenbuf++;
		*p++=0xFF; lenbuf++;
		*p++=0x03; lenbuf++;
		*p++=0x00; pBufLen = p-1; lenbuf++;
		strcpy((char *)p, strMIDI_TITLE);
		lenbuf += tmpuc;
		*pBufLen = tmpuc;
		p+=tmpuc;
	}

	//Copyright notice
	tmpuc = (unsigned char)strlen(strMIDI_AUTHOR);
	if(tmpuc>0){
		*p++=0x00; lenbuf++;
		*p++=0xFF; lenbuf++;
		*p++=0x02; lenbuf++;
		*p++=0x00; pBufLen = p-1; lenbuf++;
		//strcpy((char *)p, "(C)COMPOSER                "); lenbuf+=27; *pBufLen=27; p+=27;
		strcpy((char *)p, strMIDI_AUTHOR);
		lenbuf += tmpuc;
		*pBufLen = tmpuc;
		p+=tmpuc;
	}
	
	//Optional text
	*p++=0x00; lenbuf++;
	*p++=0xFF; lenbuf++;
	*p++=0x01; lenbuf++;
	*p++=0x00; pBufLen = p-1; lenbuf++;
	strcpy((char *)p, "                        "); //Clear with blank beforehand
	strcpy((char *)p, MessageString[IDS_STRING114]); //here "Exported by Organya Twei" The premise that it enters only.
	lenbuf+=24; *pBufLen=24; p+=24;	// 2014.10.19 D


	//Beat
	*p++=0x00; lenbuf++;
	*p++=0xFF; lenbuf++;
	*p++=0x58; lenbuf++;
	*p++=0x04; lenbuf++;
	*p++=info.line; lenbuf++;
	*p++=0x02; lenbuf++;
	*p++=0x18; lenbuf++;
	*p++=0x08; lenbuf++;

/*	
	//GM System ON
	*p++=0x00; lenbuf++;
	*p++=0xF0; lenbuf++;
	*p++=0x05; lenbuf++;
	*p++=0x7E; lenbuf++;
	*p++=0x7F; lenbuf++;
	*p++=0x09; lenbuf++;
	*p++=0x01; lenbuf++;
	*p++=0xF7; lenbuf++;
*/
	//XG System ON
	/*
	*p++=0x00; lenbuf++;
	*p++=0xF0; lenbuf++;
	*p++=0x08; lenbuf++;
	*p++=0x43; lenbuf++;
	*p++=0x10; lenbuf++;
	*p++=0x4C; lenbuf++;
	*p++=0x00; lenbuf++;
	*p++=0x00; lenbuf++;
	*p++=0x7E; lenbuf++;
	*p++=0x00; lenbuf++;
	*p++=0xF7; lenbuf++;
*/

	//tempo
	isdl = setDeltaTime((info.line - 1) * info.dot * iDeltaTime , 0, p); p+=isdl;  lenbuf+=isdl;
	*p++=0xFF; lenbuf++;
	*p++=0x51; lenbuf++;
	*p++=0x03; lenbuf++;
	//invtempo = 60 * 1000000 / (60 * 1000 / (info.wait * info.dot ));
	invtempo = 1000 * info.wait * info.dot ;
	tmpuc = (unsigned char)((invtempo / 0x10000) % 0x0100); *p++ = tmpuc; lenbuf++;
	tmpuc = (unsigned char)((invtempo / 0x100) % 0x0100);   *p++ = tmpuc; lenbuf++;
	tmpuc = (unsigned char)((invtempo / 0x1) % 0x0100);     *p++ = tmpuc; lenbuf++;

	//Termination
	*p++=0x00; lenbuf++;
	*p++=0xFF; lenbuf++;
	*p++=0x2F; lenbuf++;
	*p++=0x00; lenbuf++;


	strConductorTrack[7] = (unsigned char)(lenbuf-8);
	fwrite(strConductorTrack, lenbuf, 1, fp);

	unsigned char strDummyTrack[] = {'M','T','r','k', 0,0,0,0x14,  0x00, 0xff, 0x03, 0x0C, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x20, 0x53, 0x65, 0x74, 0x75, 0x70,  0x00,0xff,0x2f,0x00  };
	fwrite(strDummyTrack, 28, 1, fp);

	//unsigned char strDummyTrack[] = {0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x28, 0x00, 0xFF, 0x03, 0x0C, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x20, 0x53, 0x65, 0x74, 0x75, 0x70, 0x00, 0xF0, 0x05, 0x7E, 0x7F, 0x09, 0x01, 0xF7, 0x01, 0xF0, 0x08, 0x43, 0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00, 0xF7, 0x8D, 0x10, 0xFF, 0x2F, 0x00};

	/*
	//MTrk : System Setup
	//Length 172=0xA3
	//FF 21 01 pp : Output port specification (PresentSMFUndefined in) 0
	//146:GM System on,  141:GS Reset
	//Master VOL
	//Add drum
	unsigned char strDummyTrack[] ={
		0x4D, 0x54, 0x72, 0x6B, 
		0x00, 0x00, 0x00, 0xAC, 
		0x00, 0xFF, 0x03, 0x0C, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x20, 0x53, 0x65, 0x74, 0x75, 0x70, 
		0x00, 0xFF, 0x21, 0x01, 0x00,

		0x00, 0xF0, 0x05, 0x7E, 0x7F, 0x09, 0x01, 0xF7, 
		0x01, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7,
		0x01, 0xF0, 0x07, 0x7F, 0x7F, 0x04, 0x01, 0x00, 0x7F, 0xF7,

		0x00, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x11, 0x15, 0x00, 0x1A, 0xF7, 
		0x00, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x12, 0x15, 0x00, 0x19, 0xF7, 
		0x00, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x13, 0x15, 0x00, 0x18, 0xF7, 
		0x00, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x14, 0x15, 0x00, 0x17, 0xF7, 
		0x00, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x15, 0x15, 0x00, 0x16, 0xF7, 
		0x01, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x16, 0x15, 0x00, 0x15, 0xF7, 
		0x00, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x17, 0x15, 0x00, 0x14, 0xF7, 
		0x00, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x18, 0x15, 0x00, 0x13, 0xF7, 
		0x00, 0xF0, 0x0A, 0x41, 0x10, 0x42, 0x12, 0x40, 0x19, 0x15, 0x01, 0x11, 0xF7,
		0x01, 0xFF, 0x2F, 0x00,
	};
	fwrite(strDummyTrack, 8+0xAC, 1, fp);
	*/
	for(i=0;i<MAXMELODY ;i++){
		//if(NULL == info.tdata[i].note_list)continue;
		GetNoteUsed(&iUse, &iLeft, i);

		ucbuf = (unsigned char *)calloc(iUse * 2 * 16 * iRepeat + 1024, sizeof(char));
		
		//memset(ucbuf, 0, iUse * 2 * 16 * iRepeat + 1024); //Zero clear callocEven if it is okay.
		ucbuf[0]= 'M'; 
		ucbuf[1]= 'T'; 
		ucbuf[2]= 'r'; 
		ucbuf[3]= 'k';
		p = &ucbuf[8];
		lenbuf = 0;

		//Sequence name(Song title)· Track name 	
		*p++ = 0x00;
		*p++ = 0xFF;
		*p++ = 0x03; pBufLen = p;
		*p++ = 0x07; //Length = 7
		lenbuf += 4;
		if(i<MAXMELODY){
			strcpy((char *)p, "TRACK_"); p+=6;
			*p++ = '1' + i;
			lenbuf += (6 + 1);
			if(NULL == info.tdata[i].note_list){
				strcpy((char *)p, " empty"); p+=6;
				*pBufLen = (*pBufLen) + 6;
				lenbuf += 6;
			}
		}else{
			strcpy((char *)p, "----"); p+=4;
			*pBufLen = 4;
			lenbuf += 4;
			*p++ = 0x00;
			*p++ = 0xFF;
			*p++ = 0x01;
			*p++ = 21;
			lenbuf += 4;
			strcpy((char *)p, "---- EMPTY TRACK ----"); p+=21;
			lenbuf += 21;
		}


		//reset All Control
		*p++=0x00;
		*p++=0xB0 + i;
		*p++=0x79;
		*p++=0x00;
		lenbuf += 4;

		//volume(VOL)
		*p++=0x04;
		*p++=0xB0 + i;
		*p++=0x07;
		*p++=0x64;
		lenbuf += 4;

		//Reverb 0 
		*p++=0x00;
		*p++=0xB0 + i;
		*p++=0x5B;
		*p++=0x00;
		lenbuf += 4;

		//chorus 0 
		*p++=0x00;
		*p++=0xB0 + i;
		*p++=0x5D;
		*p++=0x00;
		lenbuf += 4;

		//volume(Exp)
		*p++=0x04;
		*p++=0xB0 + i;
		*p++=0x0B;
		*p++=0x7F;
		lenbuf += 4;

		//Sound(Brute force ^^; ...)
		*p++=0x04;
		*p++=0xC0 + i;
		//In the dialogucMIDIProgramChangeValue[i]It should be set appropriately.
		if(ucMIDIProgramChangeValue[i]<128){
			*p++ = ucMIDIProgramChangeValue[i];
		}else{
			*p++ = info.tdata[i].wave_no;
		}
		lenbuf += 3;

		//Pitch Bend En mm ll (3byte)
        //ChannelnTo the pitch bend valuellmmSend (Unceremoniouslylittle endianSo be careful). Pitch Bend-8192,0,8191Value corresponding tollmmAre 0x0000, 0x4000, 0x7F7F . 
		long lPvnd = (info.tdata[i].freq - 1000) + 0x4000;
		int iFlgOfOverRepeatPoint;
		*p++=0x04;
		*p++=0xE0 + i;
		*p++=(unsigned char)(lPvnd & 0xFF);
		*p++=(unsigned char)((lPvnd >> 8) & 0xFF);
		lenbuf += 4;

		LastX = -(info.line * info.dot * iDeltaTime - 4 - 4 - 4 - 4); lastVol = 100; iUseCode90 = 0;
		LastPan = -99999;
		
		npStart = info.tdata[i].note_list;
		rp = 0;
		iFlgOfOverRepeatPoint = 0;

		for(np = info.tdata[i].note_list; rp < iRepeat ; ){
			if(np == NULL || np->x >= info.end_x){
				if(rp < iRepeat - 1){
					rp++;
					np = npStart;
					LastX -=(info.end_x - info.repeat_x)* iDeltaTime;
					//np = npStart->to;
					continue;
				}else{
					break;
				}
			}
			
			if(np->x >= info.repeat_x && iFlgOfOverRepeatPoint == 0){ //First pass through the repeat starting pointX
				npStart = np;
				iFlgOfOverRepeatPoint = 1;
			}
			if(np->y != KEYDUMMY ){
				pLast = p;
				if(LastPan != (long)np->pan && np->pan != PANDUMMY){ //PANChange
					CurrentX = np->x * iDeltaTime; 
					isdl = setDeltaTime(CurrentX, LastX, p);
					p+=isdl;
					LastX = CurrentX;
					iUseCode90 = 0;
					*p++ = (unsigned char)(0xB0 + i);
					*p++ = 0x0A; // Pan
					long ipan = (np->pan - 6) * 63 ; ipan = ipan / 6; ipan += 64;
					*p++ = (unsigned char)(ipan);
					LastPan = np->pan;

				}
				//xx = np->x - LastX;
				CurrentX = np->x * iDeltaTime;
				isdl = setDeltaTime(CurrentX, LastX, p);
				LastX = CurrentX;

				p+=isdl;
				//*p++ = (unsigned char)xx;
				if(iUseCode90 == 0){
					*p++ = (unsigned char)(0x90 + i);
					iUseCode90 = 1;
				}
				*p++ = 12 + np->y;
				if(np->volume != VOLDUMMY){
					*p++ = (unsigned char)(np->volume / 2);
					lastVol = (np->volume / 2);
				}else{
					*p++ = lastVol;
				}
				//xx = np->length;
				//if(xx>=128){ //Sundry. Delta time is Mendoi.
					//*p++ = (unsigned char)(xx/128)|0x80;
					//*p++ = (unsigned char)xx%128;
				//}else{
				//	*p++ = (unsigned char)xx;
				//}
				long Amari;
				Amari = np->x + np->length - info.end_x;
				if(Amari < 0)Amari = 0;

				CurrentX = (np->x + np->length - Amari ) * iDeltaTime - (info.tdata[i].pipi == 0 ? 0 : iDeltaTime / 3 );
				isdl = setDeltaTime(CurrentX, LastX, p);
				p+=isdl;
				LastX = CurrentX;

				if(iUseCode90 == 0){
					*p++ = (unsigned char)(0x90 + i);
					iUseCode90 = 1;
				}
				*p++ = 12 + np->y;
				*p++ = 0; // To turn off the sound 
				lenbuf += (p - pLast);
			}
			np = np->to;
		}
		
		*p++=0x00;
		*p++=0xFF;
		*p++=0x2F;
		*p++=0x00;
		lenbuf += 4;

		//Block length
		tmpuc = (unsigned char)((lenbuf / 0x1000000) & 0xFF);
		ucbuf[4] = tmpuc;
		tmpuc = (unsigned char)((lenbuf / 0x10000) & 0xFF);
		ucbuf[5] = tmpuc;
		tmpuc = (unsigned char)((lenbuf / 0x100) & 0xFF);
		ucbuf[6] = tmpuc;
		tmpuc = (unsigned char)((lenbuf / 0x1) & 0xFF);
		ucbuf[7] = tmpuc;

		fwrite(ucbuf, lenbuf + 8, 1, fp);

		free(ucbuf);
	}

	unsigned char *bufv;
	long x, FinalNoteX;
	int bufsize = 256 * info.line * info.dot * 8 * 2, w, t;
	
	bufv = (unsigned char *)calloc(bufsize, sizeof(char));
	//memset(bufv, 0, bufsize);
	//FinalNoteX = 0;
	
	
	iUse = 0;
	for(t=0, i=MAXMELODY; i<MAXTRACK; t++, i++){
		GetNoteUsed(&iUseTmp, &iLeftTmp, i);
		if(iUseTmp > iUse)iUse = iUseTmp;
		for(np = info.tdata[i].note_list; np != NULL ; np = np->to){
			if(np->y != KEYDUMMY && np->volume>0 && np->volume<VOLDUMMY){
				w = t * 256 * info.line * info.dot * 2 + np->x * 2 ;
				*(bufv + w) = np->volume;
				*(bufv + w + 1) = np->y; //TomI want to know the height of
				//if(np->x > FinalNoteX) FinalNoteX = np->x;
			}
		}
	}
	FinalNoteX = info.end_x;

	ucbuf = (unsigned char *)calloc(iUse * 2 * 8 * 16 * iRepeat + 1024, sizeof(char));
	//memset(ucbuf, 0, iUse * 2 * 8 * 16 * iRepeat + 1024); //Zero clear callocEven if it is okay.
	ucbuf[0]= 'M'; ucbuf[1]= 'T'; ucbuf[2]= 'r'; ucbuf[3]= 'k';
	p = &ucbuf[8];
	lenbuf = 0;
	//Sequence name(Song title)· Track name 	
	*p++ = 0x00;
	*p++ = 0xFF;
	*p++ = 0x03; pBufLen = p;
	*p++ = 0x07; //Length = 7
	lenbuf += 4;
	strcpy((char *)p, "DRUMS"); p+=5;
	*pBufLen = 5;
	lenbuf += 5;

	//Output port specification
	*p++=0x00;
	*p++=0xFF;
	*p++=0x21;
	*p++=0x01;
	*p++=0x00;
	lenbuf += 5;

	//reset All Control
	*p++=0x04;
	*p++=0xB9;
	*p++=0x79;
	*p++=0x00;
	lenbuf += 4;

	//volume(VOL)
	*p++=0x04;
	*p++=0xB9; //B0 + 9so Aport 10channel = Drum Set
	*p++=0x07;
	*p++=0x64;
	lenbuf += 4;

	//Reverb 80 
	*p++=0x00;
	*p++=0xB9;
	*p++=0x5B;
	*p++=0x50;
	lenbuf += 4;

	//volume(Exp)
	*p++=0x04;
	*p++=0xB9;
	*p++=0x0B;
	*p++=0x7F;
	lenbuf += 4;
	LastX = -(info.line * info.dot * iDeltaTime - 4 - 4 - 4);
	
	for(i=0;i<MAXTRACK;i++){
		flgNoteOff[i] = 0;
		ucCurrentDrumKey[i] = 0;
	}
	rp = 0;
	for(x = 0 ; x <= FinalNoteX+1 ; x++){
		if(x == info.end_x){
			x = info.repeat_x;
			rp++;
			if(rp >= iRepeat)break;
			x--; //forIn processing x++Because it is done.
			LastX -=(info.end_x - info.repeat_x)* iDeltaTime;
			continue;
		}
		//break;
		pLast = p;
		flg89 = (unsigned char)(1 * iDeltaTime);
		for(t=0, i=MAXMELODY; i<MAXTRACK; t++, i++){
			if(flgNoteOff[i] > 0){
				*p++ = flg89;
				*p++ = 0x89;
				*p++ = ucCurrentDrumKey[i];
				*p++ = 0x00;
				flg89 = 0;
				flgNoteOff[i] = 0;
			}
		}
		if(flg89 == 0){
			LastX+= 1 * iDeltaTime;
		}

		for(t=0, i=MAXMELODY; i<MAXTRACK; t++, i++){
			w = t * 256 * info.line * info.dot * 2 + x * 2 ;
			if( *(bufv+w) > 0){
				CurrentX = x * iDeltaTime;
				isdl = setDeltaTime(CurrentX, LastX, p);
				p+=isdl;
				LastX = CurrentX;
				ucCurrentDrumKey[i] = Convert_Wave_no_to_MIDI_no[info.tdata[i].wave_no];
				if(ucCurrentDrumKey[i] == 99){ //TomWhen
					ucCurrentDrumKey[i] = Tom_DrumMap[0];
					for(j = 1; j< 6; j++){
						if( (*(bufv+w+1)) >= Tom_Threshold[j]) ucCurrentDrumKey[i] = Tom_DrumMap[j];
					}
				}else if(ucCurrentDrumKey[i] == 57){ //Crash Symbal 2
					if((*(bufv+w+1)) < 36)ucCurrentDrumKey[i] = 49; //Crash Symbal 1(To lower)
				}
				*p++ = 0x99;
				*p++ = ucCurrentDrumKey[i];
				*p++ = (unsigned char)((*(bufv+w)) / 2); //velo
				//*p++ = 0x64; //velo
				flgNoteOff[i] = 1;
			}
		}
		lenbuf += (p - pLast);
	}
	//Termination
	*p++=0x00;
	*p++=0xFF;
	*p++=0x2F;
	*p++=0x00;
	lenbuf += 4;


	//Block length
	tmpuc = (unsigned char)((lenbuf / 0x1000000) & 0xFF);
	ucbuf[4] = tmpuc;
	tmpuc = (unsigned char)((lenbuf / 0x10000) & 0xFF);
	ucbuf[5] = tmpuc;
	tmpuc = (unsigned char)((lenbuf / 0x100) & 0xFF);
	ucbuf[6] = tmpuc;
	tmpuc = (unsigned char)((lenbuf / 0x1) & 0xFF);
	ucbuf[7] = tmpuc;

	fwrite(ucbuf, lenbuf + 8, 1, fp);

	free(ucbuf);
	free(bufv);

  

	fclose(fp);
	return TRUE;
}
