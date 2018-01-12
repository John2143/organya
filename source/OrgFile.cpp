#include <string.h>
#include "Setting.h"
#include "Gdi.h"
#include "DefOrg.h"
#include "OrgData.h"
#include <stdio.h>
#include "Sound.h"
#include "resource.h"
#include "Scroll.h"
#include "rxoFunction.h"
#include "util.h"

//Of the songPATHI will leave it here
char music_file[MAX_PATH]; //NewData.org
char pass[7] = "Org-01";
char pass2[7] = "Org-02";//Pipi
char pass3[7] = "Org-03";//Various timbres added without permission.

////Below are organular song data structures (items included in Phil)
typedef struct{
	long x;//position
	unsigned char y;//Sound height
	unsigned char length;//Sound length
	unsigned char volume;//Volume
	unsigned char pan;//Bread
}ORGANYANOTE;
typedef struct{
	unsigned short freq;//+αfrequency(1000ButDefault)
	unsigned char wave_no;//WaveformNo
	unsigned char pipi;//☆
	unsigned short note_num;//Number of notes
}ORGANYATRACK;
typedef struct{
	unsigned short wait;
	unsigned char line;
	unsigned char dot;
	long repeat_x;//repeat
	long end_x;//End of song(Return to repeat)
	ORGANYATRACK tdata[MAXTRACK];
}ORGANYADATA;


//Detects the number of notes used for that track
unsigned short OrgData::GetNoteNumber(char track,NOTECOPY *nc)
{
	NOTELIST *np;
	unsigned short num = 0;
	np = info.tdata[track].note_list;
	if(nc == NULL){
		while(np != NULL){
			num++;
			np = np->to;
		}
		return num;
	}else{
		//Take the range
		while(np != NULL && np->x < nc->x1_1){
			np = np->to;
		}
		if(np == NULL)return 0;
		//If it exceeds the range(maximumXAlso detects the same thing)
		while(np != NULL && np->x <= nc->x1_2){
			num++;
			np = np->to;
		}
		return num;
	}
}
//Save song data
BOOL OrgData::SaveMusicData(void)
{
	ORGANYADATA org_data;
	NOTELIST *np;
	int i,j;
	//Generation of song information for recording
	org_data.wait = info.wait;
	org_data.line = info.line;
	org_data.dot = info.dot;
	org_data.repeat_x = info.repeat_x;
	org_data.end_x = info.end_x;

	for(i = 0; i < MAXTRACK; i++){
		org_data.tdata[i].freq = info.tdata[i].freq;
		org_data.tdata[i].wave_no = info.tdata[i].wave_no;
		org_data.tdata[i].pipi = info.tdata[i].pipi;
		org_data.tdata[i].note_num = GetNoteNumber(i,NULL);
	}
	//&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;save
	FILE *fp;
	if((fp=fopen(music_file,"wb"))==NULL){
		//MessageBox(hWnd,"File can not be accessed","Error (Save)",MB_OK);	// 2014.10.19 D
		msgbox(hWnd,IDS_WARNING_ACCESS_FILE,IDS_ERROR_SAVE,MB_OK);	// 2014.10.19 A
		return(FALSE);
	}
	//password
	j=2;
	for(i=8;i<15;i++){
		if(org_data.tdata[i].wave_no>=12)j=3;	//If you are using the drum&#39;s new soundVer.3
	}
	if(j==2)fwrite(&pass2[0], sizeof(char), 6, fp);
	else fwrite(&pass3[0], sizeof(char), 6, fp);
	//Writing song information
	fwrite(&org_data, sizeof(ORGANYADATA), 1, fp);
	//Saving notes
	for(j = 0; j < MAXTRACK; j++){
		if(info.tdata[j].note_list == NULL)continue;
		np = info.tdata[j].note_list;//X coordinate
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fwrite(&np->x,      sizeof(long), 1, fp);
			np = np->to;
		}
		np = info.tdata[j].note_list;//Y coordinate
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fwrite(&np->y,      sizeof(unsigned char), 1, fp);
			np = np->to;
		}
		np = info.tdata[j].note_list;//length
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fwrite(&np->length, sizeof(unsigned char), 1, fp);
			np = np->to;
		}
		np = info.tdata[j].note_list;//volume
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fwrite(&np->volume, sizeof(unsigned char), 1, fp);
			np = np->to;
		}
		np = info.tdata[j].note_list;//Bread
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fwrite(&np->pan,    sizeof(unsigned char), 1, fp);
			np = np->to;
		}
	}	
	fclose(fp);
	PutRecentFile(music_file);
//	MessageBox(hWnd,"Saved","Message (Save)",MB_OK);
	//↓2014.05.06 A
	if(SaveWithInitVolFile != 0){
		AutoSavePVIFile();
	}

	return TRUE;
}
//Read song data
extern char *dram_name[];
extern HWND hDlgPlayer;

//If the file is loadable0If it is abnormal1return it. Data loading is not done. 2014.05.22
int OrgData::FileCheckBeforeLoad(char *checkfile)
{
	FILE *fp;
	char pass_check[6];
	char ver = 0;
	if((fp=fopen(checkfile,"rb"))==NULL){
		//MessageBox(hWnd,"File can not be accessed","Error (Load)",MB_OK);
		return 1;
	}

	fread(&pass_check[0], sizeof(char), 6, fp);
	if( !memcmp( pass_check, pass, 6 ) )ver = 1;
	if( !memcmp( pass_check, pass2, 6 ) )ver = 2;
	if( !memcmp( pass_check, pass3, 6 ) )ver = 2;
	if( !ver ){
		fclose(fp);
		//MessageBox(hWnd,"This file can not be used","Error (Load)",MB_OK);
		return 1;
	}
	fclose(fp);

	return 0;
}
BOOL OrgData::LoadMusicData(void)
{
	ORGANYADATA org_data;
	NOTELIST *np;
	int i,j;
	char pass_check[6];
	char ver = 0;

	//&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;&quot;Load
	FILE *fp;
	if((fp=fopen(music_file,"rb"))==NULL){
		//MessageBox(hWnd,"File can not be accessed","Error (Load)",MB_OK);	// 2014.10.19 D
		msgbox(hWnd,IDS_WARNING_ACCESS_FILE,IDS_ERROR_LOAD,MB_OK);	// 2014.10.19 A
		return(FALSE);
	}
	//Password check
	fread(&pass_check[0], sizeof(char), 6, fp);
	if( !memcmp( pass_check, pass, 6 ) )ver = 1;
	if( !memcmp( pass_check, pass2, 6 ) )ver = 2;
	if( !memcmp( pass_check, pass3, 6 ) )ver = 2;
	if( !ver ){
		fclose(fp);
		//MessageBox(hWnd,"This file can not be used","Error (Load)",MB_OK);	// 2014.10.19 D
		msgbox(hWnd,IDS_ERROR_FILE,IDS_ERROR_LOAD,MB_OK);	// 2014.10.19 A
		return FALSE;
	}
//	for(i = 0; i < 4; i++){
//		if(pass[i] != pass_check[i]){
//			MessageBox(hWnd,"This file can not be used","Error (Load)",MB_OK);
//			fclose(fp);
//			return FALSE;
//		}
//	}
	//Reading song information
	fread(&org_data, sizeof(ORGANYADATA), 1, fp);

	//Set song information
	info.wait = org_data.wait;
	info.line = org_data.line;
	info.dot = org_data.dot;
	info.repeat_x = org_data.repeat_x;
	info.end_x = org_data.end_x;
	for(i = 0; i < MAXTRACK; i++){
		info.tdata[i].freq = org_data.tdata[i].freq;
		if( ver == 1 )info.tdata[i].pipi = 0;
		else info.tdata[i].pipi = org_data.tdata[i].pipi;
		info.tdata[i].wave_no = org_data.tdata[i].wave_no;
	}

	//Load notes
	for(j = 0; j < MAXTRACK; j++){
		//The first notefromButNULLBecome
		if(org_data.tdata[j].note_num == 0){
			info.tdata[j].note_list = NULL;
			continue;
		}
		//Make a list
		np = info.tdata[j].note_p;
		info.tdata[j].note_list = info.tdata[j].note_p;
		np->from = NULL;
		np->to = (np + 1);
		np++;
		for(i = 1; i < org_data.tdata[j].note_num; i++){
			np->from = (np - 1);
			np->to = (np + 1);
			np++;
		}
		//Of the last notetoIsNULL
		np--;
		np->to = NULL;

		//Substitute content
		np = info.tdata[j].note_p;//X coordinate
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fread(&np->x,      sizeof(long), 1, fp);
			np++;
		}
		np = info.tdata[j].note_p;//Y coordinate
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fread(&np->y,      sizeof(unsigned char), 1, fp);
			np++;
		}
		np = info.tdata[j].note_p;//length
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fread(&np->length, sizeof(unsigned char), 1, fp);
			np++;
		}
		np = info.tdata[j].note_p;//volume
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fread(&np->volume, sizeof(unsigned char), 1, fp);
			np++;
		}
		np = info.tdata[j].note_p;//Bread
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			fread(&np->pan,    sizeof(unsigned char), 1, fp);
			np++;
		}
	}
	fclose(fp);
	//Enable data
	for(j = 0; j < MAXMELODY; j++)
		MakeOrganyaWave(j,info.tdata[j].wave_no, info.tdata[j].pipi);
	for(j = MAXMELODY; j < MAXTRACK; j++){
		i = info.tdata[j].wave_no;
		InitDramObject(dram_name[i],j-MAXMELODY);
	}
	//Show to player
	char str[32];
	SetPlayPointer(0);//Cueing
	scr_data.SetHorzScroll(0);
	itoa(info.wait,str,10);
	SetDlgItemText(hDlgPlayer,IDE_VIEWWAIT,str);
	SetDlgItemText(hDlgPlayer,IDE_VIEWMEAS,"0");
	SetDlgItemText(hDlgPlayer,IDE_VIEWXPOS,"0");

	MakeMusicParts(info.line,info.dot);//Generate parts
	MakePanParts(info.line,info.dot);
	PutRecentFile(music_file);
	//↓2014.05.06 A
	if(SaveWithInitVolFile != 0){
		AutoLoadPVIFile();
	}
	return TRUE;
}

void OrgData::SortNotes()
{
	NOTELIST *pntl,*pNtls,*np;
	ORGANYADATA org_data;
	int i,j;

	org_data.wait = info.wait;
	org_data.line = info.line;
	org_data.dot = info.dot;
	org_data.repeat_x = info.repeat_x;
	org_data.end_x = info.end_x;

	for(i = 0; i < MAXTRACK; i++){
		org_data.tdata[i].freq = info.tdata[i].freq;
		org_data.tdata[i].wave_no = info.tdata[i].wave_no;
		org_data.tdata[i].pipi = info.tdata[i].pipi;
		org_data.tdata[i].note_num = GetNoteNumber(i,NULL);
	}

	pNtls = new NOTELIST[4096]; //For evacuation
	
	for(j = 0; j < MAXTRACK; j++){
		if(info.tdata[j].note_list == NULL)continue;
		pntl = pNtls;
		np = info.tdata[j].note_list;//Beginning of note
		for(i = 0; i < org_data.tdata[j].note_num; i++){
			pNtls[i].x = np->x;
			pNtls[i].y = np->y;
			pNtls[i].pan = np->pan;
			pNtls[i].volume = np->volume;
			pNtls[i].length = np->length;
			np = np->to;
		}
		//Make a list
		if(true){
			np = info.tdata[j].note_p; //Start of area
			info.tdata[j].note_list = info.tdata[j].note_p;
			np->from = NULL;
			np->to = (np + 1);
			np++;
			for(i = 1; i < org_data.tdata[j].note_num; i++){
				np->from = (np - 1);
				np->to = (np + 1);
				np++;
			}
			//Of the last notetoIsNULL
			np--;
			np->to = NULL;
			np = info.tdata[j].note_p;//X coordinate
			for(i = 0; i < org_data.tdata[j].note_num; i++){
				np->x = pNtls[i].x;
				np->y = pNtls[i].y;
				np->length = pNtls[i].length;
				np->pan = pNtls[i].pan;
				np->volume = pNtls[i].volume;
				np++;
			}
		}else{ //Experimentally make it Sakashi
			np = info.tdata[j].note_p+4095; //Start of area
			info.tdata[j].note_list = info.tdata[j].note_p;
			np->from = NULL;
			np->to = (np - 1);
			np--;
			for(i = 1; i < org_data.tdata[j].note_num; i++){
				np->from = (np + 1);
				np->to = (np - 1);
				np--;
			}
			//Of the last notetoIsNULL
			np++;
			np->to = NULL;
			np = info.tdata[j].note_p;//X coordinate
			for(i = 0; i < org_data.tdata[j].note_num; i++){
				np->x = pNtls[i].x;
				np->y = pNtls[i].y;
				np->length = pNtls[i].length;
				np->pan = pNtls[i].pan;
				np->volume = pNtls[i].volume;
				np++;
			}

		}
	}	
	delete [] pNtls;	// 2014.10.18 Added release.
	//Show to player
	char str[32];
	SetPlayPointer(0);//Cueing
	scr_data.SetHorzScroll(0);
	itoa(info.wait,str,10);
	SetDlgItemText(hDlgPlayer,IDE_VIEWWAIT,str);
	SetDlgItemText(hDlgPlayer,IDE_VIEWMEAS,"0");
	SetDlgItemText(hDlgPlayer,IDE_VIEWXPOS,"0");

	MakeMusicParts(info.line,info.dot);//Generate parts
	MakePanParts(info.line,info.dot);

}
