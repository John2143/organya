#include "Setting.h"
#include "DefOrg.h"
#include "OrgData.h"
#include "Sound.h"
#include "resource.h"
#include "Scroll.h"

long play_p;//Current playback position (canvas)
NOTELIST *np[MAXTRACK];//Currently ready to play notes
long now_leng[MAXMELODY] = {NULL};//Length of note during playback
extern HWND hDlgPlayer;
void OrgData::PlayData(void)
{
	char str[10];
	char oldstr[10];
	char end_cnt = MAXTRACK;
//	PlaySoundObject(1,1);
	//Melody playback
	for(int i = 0; i < MAXMELODY; i++){
//	int i = 6;
		if(np[i] != NULL &&play_p == np[i]->x ){//The sound came.
			if(mute[i] == 0){
				if(np[i]->y != KEYDUMMY){
					if( info.tdata[i].pipi )
						PlayOrganObject2(np[i]->y,-1,i,info.tdata[i].freq);
					else PlayOrganObject(np[i]->y,-1,i,info.tdata[i].freq);
					now_leng[i] = np[i]->length;
				}
			}
			if(np[i]->pan != PANDUMMY)ChangeOrganPan(np[i]->y,np[i]->pan,i);
			if(np[i]->volume != VOLDUMMY)ChangeOrganVolume(np[i]->y,np[i]->volume,i);
			np[i] = np[i]->to;//Point to the next note
		}
		if(now_leng[i] == 0 ){
			if(info.tdata[i].pipi == 0)
				PlayOrganObject(NULL,2,i,info.tdata[i].freq);
			else PlayOrganObject2(NULL,2,i,info.tdata[i].freq);
		}
		if(now_leng[i] > 0)now_leng[i]--;
	}
	//Playing the drum
	for(i = MAXMELODY; i < MAXTRACK; i++){
		if(np[i] != NULL &&play_p == np[i]->x ){//The sound came.
			if(np[i]->y != KEYDUMMY){//Normal slow
				if(mute[i] == 0)PlayDramObject(np[i]->y,1,i-MAXMELODY);
			}
			if(np[i]->pan != PANDUMMY)ChangeDramPan(np[i]->pan,i-MAXMELODY);
			if(np[i]->volume != VOLDUMMY)ChangeDramVolume(np[i]->volume,i-MAXMELODY);
			np[i] = np[i]->to;//Point to the next note
		}
	}
	//Show to player
	itoa(play_p/(info.dot*info.line),str,10);
	GetDlgItemText(hDlgPlayer,IDE_VIEWMEAS,oldstr,10);
	if(strcmp(str, oldstr) != 0) SetDlgItemText(hDlgPlayer,IDE_VIEWMEAS,str);

	itoa(play_p%(info.dot*info.line),str,10);
	GetDlgItemText(hDlgPlayer,IDE_VIEWXPOS,oldstr,10);
	if(strcmp(str, oldstr) != 0)SetDlgItemText(hDlgPlayer,IDE_VIEWXPOS,str);
	//Auto scroll
	if(actApp){//Only when active
		if(play_p%(info.dot*info.line) == 0 && play_p+1 != info.end_x )
			scr_data.SetHorzScroll(play_p/(info.dot*info.line));
	}
	play_p++;
	if(play_p >= info.end_x){
		play_p = info.repeat_x;//++Because it is
		SetPlayPointer(play_p);
	}

}
void OrgData::SetPlayPointer(long x)
{
	int i;
	for(i = 0; i < MAXTRACK; i++){
		np[i] = info.tdata[i].note_list;
		while(np[i] != NULL && np[i]->x < x)np[i] = np[i]->to;//Set notes to watch		
	}
	play_p = x;
}