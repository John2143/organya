#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Setting.h"
#include "Gdi.h"
#include "DefOrg.h"
#include "OrgData.h"
#include "rxoFunction.h"
#define PI 3.14159265358979323846
/*
//Structure for editing///////////////
typedef struct{//●Copy structure
	char track1;//Of this track
	long x1_1;//from here
	long x1_2;//So far
	char track2;//Of this track
	long x2;//here
	long num;//How many times
}NOTECOPY;

typedef struct{//●Transport and pan pot
	char track;//Of this track
	long x1;//from here
	long x2;//So far
	unsigned char a;//only this 
	unsigned char mode;//Takesu
}PARCHANGE;
*/
BOOL OrgData::DelateNoteData(PARCHANGE *pc)
{
	NOTELIST *np;
	NOTELIST *np_from;//With this
	NOTELIST *np_to;//Join this guy
	NOTELIST *np_clear;//For tail removal
	long note_x1,note_x2;//Save range of songs
	np = info.tdata[pc->track].note_list;
	if(np == NULL)return FALSE;//If there is no note, it ends

	np = info.tdata[pc->track].note_list;
	note_x1 = np->x;//Record first x
	while(np->to != NULL)np = np->to;//Search for the End
	note_x2 = np->x;//Save the End
	if(note_x1 > pc->x2 || pc->x1 > note_x2){//The range is far from the score
		return FALSE;
	}else if(note_x1 < pc->x1 && pc->x2 < note_x2){//The range is enclosed in the score.
		np = info.tdata[pc->track].note_list;
		while(np->x < pc->x1)np = np->to;//Search head of range
		np_from = np->from;//Save left end(For connection)
		while(np->x <= pc->x2){
			np->length = 0;
			np->y=KEYDUMMY; np->pan=PANDUMMY; np->volume=VOLDUMMY;
			np = np->to;//Search range&#39;s butt
		}
		np_to = np;//Save right end(For connection)
		//Connect both ends
		np_from->to = np_to;
		np_to->from = np_from;
		return TRUE;

	}else if(pc->x1 <= note_x1 && pc->x2 < note_x2){//There is no sound at the head of the range to be erased.
		np = info.tdata[pc->track].note_list;
		while(np->x <= pc->x2){
			np->length = 0;
			np->y=KEYDUMMY; np->pan=PANDUMMY; np->volume=VOLDUMMY;
			np = np->to;//Search range&#39;s butt
		}
		np_to = np;//Save right end(For connection)
		np->from = NULL;//Take this on your head
		info.tdata[pc->track].note_list = np;//Register on the list
		return TRUE;
	}else if(note_x1 < pc->x1 && note_x2 <= pc->x2 ){//There is no sound in the butt of the range to be erased.
		np = info.tdata[pc->track].note_list;
		while(np->to->x < pc->x1)np = np->to;//Search head of range
		np_clear = np;//Address of the last note
		np = np->to;
		while(np != NULL){
			np->length = 0;
			np->y=KEYDUMMY; np->pan=PANDUMMY; np->volume=VOLDUMMY;
			np = np->to;
		}
		np_clear->to = NULL;
		return TRUE;
	}else if(note_x1 >= pc->x1 && note_x2 <= pc->x2 ){//Complete erase
		np = info.tdata[pc->track].note_list;
		while(np != NULL){
			np->length = 0;
			np->y=KEYDUMMY; np->pan=PANDUMMY; np->volume=VOLDUMMY;
			np = np->to;
		}
		info.tdata[pc->track].note_list = NULL;
		return TRUE;
	}
	return TRUE;
}

BOOL OrgData::CopyNoteData(NOTECOPY *nc)
{
	int i,j;
	PARCHANGE pc;//For paste area clearing
	NOTELIST *np;
//	NOTELIST *p_in1,*p_in2;//Place to insert
	NOTELIST *p_list1,*p_list2;//List to insert
	NOTELIST *work;//Work area
	NOTELIST *wp;//Work pointer
	long ind_x;//Index value (x)
	long copy_num;//Number of notes to be copied
	if(nc->num == 0)return FALSE;//Number of copies 0
	copy_num = GetNoteNumber(nc->track1,nc);
	if(copy_num == 0)return FALSE;
	wp = work = (NOTELIST *)malloc(sizeof(NOTELIST)*copy_num);//Secure space for work
	ind_x = nc->x1_1;
	
	//Copy to work area
	np = info.tdata[nc->track1].note_list;
	while(np != NULL && np->x < nc->x1_1)np = np->to;
	if(np == NULL){
		free( work );
		return TRUE;//No note in the copy source(While head searchingOUT)
	}
	for(i = 0; i < copy_num; i++){//1 Paste
		wp->length = np->length;
		wp->pan    = np->pan;
		wp->volume = np->volume;
		wp->y      = np->y;
		//Deduct the index value
		wp->x      = np->x - ind_x;
		//next
		np = np->to;
		wp++;
	}

	//Clear paste destination
	pc.track = nc->track2;//Of this track
	pc.x1 = nc->x2;//from here
	pc.x2 = nc->x2 + (nc->x1_2+1 - nc->x1_1)*nc->num-1;//So farx1_1+1(x1_2Is−1 value)
	DelateNoteData(&pc);//To erase

	//Generate a paste list
	np = p_list1 = p_list2= SearchNote(info.tdata[nc->track2].note_p);
	np->length = 1;//Provisional generation
	if(np == NULL){//Lack of unused score
		free( work );
		return FALSE;
	}
	for(i = 1; i < copy_num*nc->num; i++){
		np = SearchNote(info.tdata[nc->track2].note_p);
		if(np == NULL){//Lack of unused score
			free( work );
			return FALSE;
		}
		np->length = 1;//Provisional generation
		p_list2->to = np;
		np->from = p_list2;
		p_list2 = np;
	}

	long index;
	//Paste in paste list
	np = p_list1;
	for(j = 0; j < nc->num;j++){//Number of paste
		wp = work;
		index = nc->x2+ (nc->x1_2 - nc->x1_1 +1)*j;
		for(i = 0; i < copy_num; i++){//1 Paste
			np->length    = wp->length;
			np->pan       = wp->pan;
			np->volume    = wp->volume; 
			np->y         = wp->y; 
			//Add index value
			np->x         = wp->x + index;

			//next
			np = np->to;
			wp++;
		}
	}



	np = info.tdata[nc->track2].note_list;
	if(np == NULL){//Completely new
		info.tdata[nc->track2].note_list = p_list1;
		p_list1->from = NULL;
		p_list2->to = NULL;
	}else{
		//Detect place to insert
//		np = info.tdata[nc->track2].note_list;
		if(np->x > p_list2->x){//Add to head
			np->from = p_list2;
			p_list2->to = np;
			p_list1->from = NULL;
			info.tdata[nc->track2].note_list = p_list1;
			free( work );
			return TRUE;
		}
		while(np->to != NULL && np->to->x < nc->x2)np = np->to;
		if(np->to == NULL){//Add to the buttocks
			np->to = p_list1;
			p_list1->from = np;
			p_list2->to = NULL;
			free( work );
			return TRUE;
		}else{//Insert
			np->to->from = p_list2;
			p_list2->to = np->to;
			np->to = p_list1;
			p_list1->from = np;
			delete work;
			return TRUE;
		}

	}
	free( work );
	return TRUE;
}
//Pan is minimum 0 and maximum is 12
BOOL OrgData::ChangePanData(PARCHANGE *pc)
{
	int i;
	long num = 0;
	NOTECOPY nc;
	NOTELIST *np;
	//Detect number of notes
	nc.x1_1 = pc->x1;
	nc.x1_2 = pc->x2;
	num = GetNoteNumber(pc->track,&nc);

	np = info.tdata[pc->track].note_list;
	if(np == NULL || num == 0)return FALSE;
	while(np != NULL && np->x < pc->x1)np = np->to;
	if(pc->a == 254){ //2014.05.03 Reversal
		for(i = 0; i < num; i++){
			if(np->pan == PANDUMMY);
			else np->pan = 12 - np->pan;
			np = np->to;
		}
		
	}else{
		if(pc->mode == MODEPARADD){//Addition mode
			for(i = 0; i < num; i++){
				if(np->pan == PANDUMMY);
				else if(np->pan + pc->a > 12)np->pan = 12;
				else np->pan += pc->a;
				np = np->to;
			}
		}else{//Subtraction mode
			for(i = 0; i < num; i++){
				if(np->pan == PANDUMMY);
				else if(np->pan - pc->a < 0)np->pan = 0;
				else np->pan -= pc->a;
				np = np->to;
			}
		}
	}
	return TRUE;
}

//The key is95ButMAX
BOOL OrgData::ChangeTransData(PARCHANGE *pc)
{
	int i;
	long num = 0;
	NOTECOPY nc;
	NOTELIST *np;
	//Detect number of notes
	nc.x1_1 = pc->x1;
	nc.x1_2 = pc->x2;
	num = GetNoteNumber(pc->track,&nc);

	np = info.tdata[pc->track].note_list;
	if(np == NULL || num == 0)return FALSE;
	while(np != NULL && np->x < pc->x1)np = np->to;
	if(pc->mode == MODEPARADD){//Addition mode
		for(i = 0; i < num; i++){
			if(np->y == KEYDUMMY);
			else if(np->y + pc->a > 95)np->y = 95;
			else np->y += pc->a;
			np = np->to;
		}
	}else{//Subtraction mode
		for(i = 0; i < num; i++){
			if(np->y == KEYDUMMY);
			else if(np->y - pc->a < 0)np->y = 0;
			else np->y -= pc->a;
			np = np->to;
		}
	}
	return TRUE;
}
//Volume is254ButMAX
BOOL OrgData::ChangeVolumeData(PARCHANGE *pc)
{
	int i,j;
	long num = 0, lFirstx, lLastx;
	NOTECOPY nc;
	NOTELIST *np;
	unsigned char uc, ucMax = 0, ucMin = 254, ucFirstFlg = 255, ucFirst, ucLast;
	double d, dnum, di, dnorm = 0, dnormnum = 0, dt;
	//Detect number of notes
	nc.x1_1 = pc->x1;
	nc.x1_2 = pc->x2;
	num = GetNoteNumber(pc->track,&nc);
	dnum = num - 1.0f;

	np = info.tdata[pc->track].note_list;
	if(np == NULL || num == 0)return FALSE;
	
	//Headache
	while(np != NULL && np->x < pc->x1)np = np->to;
	//ucMax, ucMinTo investigate
	for(i = 0; i < num; i++){
		if(np->volume != VOLDUMMY){
			if(ucFirstFlg == 255){ucFirst = np->volume; lFirstx = np->x; ucFirstFlg = 0;}
			ucLast = np->volume;
			lLastx = np->x;
			if(np->volume > ucMax)ucMax = np->volume; else if(np->volume < ucMin)ucMin = np->volume;
			dnorm += (double)np->volume;
			dnormnum += 1.0f;
		}
		np = np->to;
	}
	if(dnormnum > 0)dnorm = dnorm / dnormnum; //Average value

	//Rewind When Headache
	np = info.tdata[pc->track].note_list;
	while(np != NULL && np->x < pc->x1)np = np->to;


	if(pc->mode == MODEPARADD){//Addition mode
		for(i = 0; i < num; i++){
			if(np->volume == VOLDUMMY);
			else if(np->volume + pc->a > 254)np->volume = 254;
			else np->volume += pc->a;
			np = np->to;
		}
	}else if(pc->mode == MODEPARSUB){//Subtraction mode
		for(i = 0; i < num; i++){
			if(np->volume == VOLDUMMY);
			else if(np->volume - pc->a < 0)np->volume = 0;
			else np->volume -= pc->a;
			np = np->to;
		}
	}else if(pc->mode == MODEMULTIPLY){//Multiplication mode
		// ExistingvolumeTo a / 128Double To
		for(i = 0; i < num; i++){
			d = (double)np->volume * (double) pc->a / (double)128.0;
			if(d>254.0)d=254.0; else if(d<0)d=0;

			uc = (unsigned char)d;

			if(np->volume == VOLDUMMY);
			else np->volume = uc;
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 3)){ //Linear attenuation 2014.05.01
		for(i = 0; i < num; i++){
			if(np->volume != VOLDUMMY){
				d = ((double)ucMax - (double)0) / (double)(num - 0) * (double)(num - 0 - i); if(d>254.0)d=254.0; else if(d<0)d=0;
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 12)){ //Raised to the second power 2014.05.17
		for(i = 0; i < num; i++){
			if(np->volume != VOLDUMMY){
				if(num > 1){
					d = (double)(ucMax) / (double)((num - 1)*(num - 1))  * (double)(i*i); if(d>254.0)d=254.0; else if(d<0)d=0;
				}else{
					d = ucMax;
				}
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 13)){ //Linear rise 2014.05.17
		for(i = 0; i < num; i++){
			if(np->volume != VOLDUMMY){
				if(num > 1){
					d = (double)ucMax / (double)(num - 1) * (double)i; if(d>254.0)d=254.0; else if(d<0)d=0;
				}else{
					d = ucMax;
				}
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 2)){ //Upward convex damping 2014.05.01
		if(num < 4)dnum = (double)num;
		else if(num == 4)dnum = (double)num-0.5f;
		for(i = 0; i < num; i++){
			di = (double)i;
			if(np->volume != VOLDUMMY){
				if(dnum > 0){
					d = 1.0f - ((exp( (di/dnum)*(di/dnum) ) - 1.0f) / 1.718281828459045);
				}else d = 1.0f;
				d = d * (double)ucMax; if(d>254.0)d=254.0; else if(d<0)d=0;
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 1)){ //Upward convex damping 2014.05.01
		if(num < 4)dnum = (double)num-0.4f;
		else if(num == 4)dnum = (double)num-0.5f;
		else dnum = num - 0.8f;
		for(i = 0; i < num; i++){
			di = (double)i;
			if(np->volume != VOLDUMMY){
				if(dnum > 0){
					d = 1.0f - ((exp( (di/dnum)*(di/dnum)*(di/dnum)*(di/dnum) ) - 1.0f) / 1.718281828459045);
				}else d = 1.0f;
				d = d * (double)ucMax; if(d>254.0)d=254.0; else if(d<0)d=0;
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 4)){ //Convex damping downward 2014.05.01
		for(i = 0; i < num; i++){
			di = (double)i; dnum = (double)num;
			if(np->volume != VOLDUMMY){
				if(dnum > 0){
					d = (exp( pow(((dnum-di)/dnum), 2)) - 1.0f) / 1.718281828459045;
				}else d = 1.0f;
				d = d * (double)ucMax; if(d>254.0)d=254.0; else if(d<0)d=0;
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 5)){ //Convex damping downward 2014.05.01
		for(i = 0; i < num; i++){
			di = (double)i; dnum = (double)num;
			if(np->volume != VOLDUMMY){
				if(dnum > 0){
					d = (exp( pow(((dnum-di)/dnum), 4)) - 1.0f) / 1.718281828459045;
				}else d = 1.0f;
				d = d * (double)ucMax; if(d>254.0)d=254.0; else if(d<0)d=0;
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 6)){ //Normalize 2014.05.01
		for(i = 0; i < num; i++){
			//di = (double)i; dnum = (double)num;
			if(np->volume != VOLDUMMY){
				//np->volume = (unsigned char)dnorm;
				np->volume = org_data.def_volume[pc->track];//Initialization
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 11)){ //Gradation 2014.05.01
		for(i = 0; i < num; i++){
			di = (double)i; dnum = (double)num;
			if(np->volume != VOLDUMMY && num > 1){
				d = ((double)ucFirst - (double)ucLast) / (double)(num - 1) * (double)(num - 1 - i) + (double)ucLast ;
				if(d>254.0)d=254.0; else if(d<0)d=0;
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 8)){ //At the end and decay
		dt = (double)np->volume;
		j = lLastx - lFirstx; 
		for(i = 0; i < num; i++){
			di = (double)i; dnum = (double)num;
			if(np->volume != VOLDUMMY){
				//d = dt;
				switch(j){
				case 0:
					d = dt;
					break;
				case 1:
					if     (lFirstx + 0 == np->x)d = dt;
					else if(lFirstx + 1 == np->x)d = dt * 0.92;
					break;
				case 2:
					if     (lFirstx + 0 == np->x)d = dt;
					else if(lFirstx + 1 == np->x)d = dt * 0.95;
					else if(lFirstx + 2 == np->x)d = dt * 0.87;
					break;
				case 3:
					if     (lFirstx + 0 == np->x)d = dt;
					else if(lFirstx + 1 == np->x)d = dt * 0.95;
					else if(lFirstx + 2 == np->x)d = dt * 0.90;
					else if(lFirstx + 3 == np->x)d = dt * 0.75;
					break;
				default:
					if     (lLastx - 2 == np->x)d = dt * 0.92;
					else if(lLastx - 1 == np->x)d = dt * 0.87;
					else if(lLastx - 0 == np->x)d = dt * 0.55;
					else d = dt;
					break;

				}

				if(d>254.0)d=254.0; else if(d<0)d=0;

				np->volume = (unsigned char)d;
				if(d == dt) dt = (double)np->volume;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 9)){ //vibrato(Base convex damping pattern on top) 2014.05.01
		dt = 1.02;
		for(i = 0; i < num; i++){
			di = (double)i; dnum = (double)num - 0.8f;
			dt += 0.0075; if(dt>1.3)dt=1.3;
			if(np->volume != VOLDUMMY && num > 1){
				if(dnum > 0){
					d = 1.0f - ((exp( (di/dnum)*(di/dnum) ) - 1.0f) / 1.718281828459045);
				}else d = 1.0f;
				d = d * (double)ucMax; 
				if(i%2==0)d=d*dt;else d=d/dt;
				
				if(d>254.0)d=254.0; else if(d<0)d=0;
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 10)){ //random 2014.05.15
		for(i = 0; i < num; i++){
			double x, y;
			double s, t;
			double r1, r2;
			di = (double)i; dnum = (double)num;
			if(np->volume != VOLDUMMY){
				// Pseudorandom number following uniform distribution x, y make
				do { x = (double)rand()/(double)RAND_MAX; } while (x == 0.0); // x But 0 Avoid becoming
				y = (double)rand()/(double)RAND_MAX;
    
				// s, t Calculation
				s = sqrt(-2.0 * log(x)); // C, Java Then. log Is natural logarithm
				t = 2.0 * PI * y;
    
				// Pseudorandom number following standard normal distribution r1, r2
				r1 = s * cos(t);  r2 = s * sin(t);
				double dval = (double)np->volume + (i%2==1 ? r1 : r2) * 12.0;
				if(dval < 0)dval = 0; else if(dval > 250)dval = 250.0f;

				np->volume = (unsigned char)dval;
		
			}
			np = np->to;
		}
	}else if(pc->mode == (MODEDECAY + 7)){ //χS curve like 2014.05.15
		//if(num < 4)dnum = (double)num;
		//else if(num == 4)dnum = (double)num-0.5f;
		dnum = (double)num / 2.0;
		long halfnum = (num+1) / 2; if (halfnum <= 0) halfnum = 1;

		for(i = 0; i < halfnum; i++){ //Convex upward
			di = (double)i;
			if(np->volume != VOLDUMMY){
				if(dnum > 0){
					d = 1.0f - ((exp( (di/dnum)*(di/dnum) ) - 1.0f) / 1.718281828459045);
				}else d = 1.0f;
				d = d * ((double)ucMax / 2.0) + ((double)ucMax / 2.0); if( d > 254.0) d = 254.0; else if(d < 0)d = 0;
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}

		for(i = halfnum; i < num; i++){ //Convex downward
			di = (double)(i - halfnum); dnum = (double)num / 2.0;
			if(np->volume != VOLDUMMY){
				if(dnum > 0){
					d = (exp( pow(((dnum-di)/dnum), 2)) - 1.0f) / 1.718281828459045;
				}else d = 1.0f;
				d = d * (double)ucMax / 2.0; if(d>254.0)d=254.0; else if(d<0)d=0;
				np->volume = (unsigned char)d;
			}
			np = np->to;
		}
	}
	return TRUE;
}

char *test[16]={
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"10",
	"11",
	"12",
	"13",
	"14",
	"15",
};
//Total check of musical score tail
BOOL OrgData::CheckNoteTail(char tr)
{
	NOTELIST *np;
	NOTELIST *tail,*head;
	np = info.tdata[tr].note_list;
//	MessageBox(hWnd,test[track],test[track],MB_OK);
	while(np != NULL){
		if(np->y != KEYDUMMY){//If you have a lengthy note
			tail = np;
			head = np->to;
			while(head != NULL && head->y == KEYDUMMY){
	//			MessageBox(hWnd,test[track],test[track],MB_OK);
				head = head->to;
			}
			if(head == NULL)return TRUE;
			else{
				if(tail->x + tail->length > head->x){
					tail->length = (unsigned char)(head->x - tail->x);
				}
//				np = head->from;// =to Anticipating
			}
		}
		np = np->to;
	}
	return TRUE;	
}

//[new]Musical notePowerStretch to double
BOOL OrgData::EnlargeAllNotes(int Power)
{
	if(Power<=0)return FALSE;
	int i,j;
	NOTELIST *np;
	for(i=0;i<16;i++){
		np = info.tdata[i].note_list; //The first note
		if(np != NULL){
			while(np != NULL){
				j = np->x * Power;
				np->x = j; //Doubled Mace
				j = np->length * Power;
				if(j>255)j=255;
				if(info.tdata[i].pipi != 0)j = np->length;
				if(np->y != KEYDUMMY && i < MAXMELODY) np->length = (unsigned char)j; //2014.05.02 Modify
				np = np->to;
			}
		}
		
	}
	for(i=0;i<16;i++){ //Want to see the tail just in case?
		CheckNoteTail(i);
	}
	info.wait = info.wait / Power; if(info.wait <=0) info.wait = 1;
	info.repeat_x = info.repeat_x * Power;
	info.end_x = info.end_x * Power;
	j = info.dot * Power;
	if(j<=255)info.dot = (unsigned char)j;
	MakeMusicParts(info.line,info.dot);//Generate parts
	MakePanParts(info.line,info.dot);

	return TRUE;
}
//[new]Musical note1/PowerShrink to double
BOOL OrgData::ShortenAllNotes(int Power)
{
	if(Power<=1)return FALSE;
	int i,j,k;
	NOTELIST *np, *np2;
	for(i=0;i<16;i++){
		np = info.tdata[i].note_list; //The first note
		if(np != NULL){
			while(np != NULL){
				np2 = np->to;
				k = np->x % Power; //This is0When it is not, I will erase it unconditionally.
				if(k==0){
					j = np->x / Power;			
					np->x = j; //Doubled Mace
					j = np->length / Power;
					//I will be sorry for the note of length 1.
					if(np->length != 1) np->length = (unsigned char)j;
				}else{
					//I will erase the notes.
					PARCHANGE p;
					p.track = i;
					p.x1 = np->x;
					p.x2 = p.x1 ;
					DelateNoteData(&p);
				}
				np = np2;
			}
		}
		
	}
	for(i=0;i<16;i++){ //Want to see the tail just in case?
		CheckNoteTail(i);
	}

	info.wait = info.wait * Power;
	 
	info.repeat_x = info.repeat_x / Power;
	info.end_x = info.end_x / Power;

	j = info.dot / Power;
	if(j>=1)info.dot = (unsigned char)j;
	MakeMusicParts(info.line,info.dot);//Generate parts
	MakePanParts(info.line,info.dot);

	return TRUE;
	
}

//[new]Fill in the blank 2014.05.01
BOOL OrgData::EnsureEmptyArea(PARCHANGE *pc, int Function)
{
	int j, iLength, iFlg;
	int iBufMax;
	long num = 0, tmpx;
	NOTECOPY nc;
	NOTELIST *np, *p, *npnext;
	PARCHANGE tmpc;

	nc.x1_1 = pc->x1; nc.x1_2 = pc->x2; num = GetNoteNumber(pc->track,&nc);
	np = info.tdata[pc->track].note_list;
	if(np == NULL || num == 0)return FALSE;
	
	iBufMax =  (pc->x2 - pc->x1 + 4096);

	while(np != NULL && np->x < pc->x1)np = np->to;

	for(;np != NULL && np->x <= pc->x2; np = npnext){
		iLength = np->length;
		npnext = np->to;

		for(j = 2; j <= iLength; j++){
			iFlg = 1;
			tmpx = np->x + j - 1;
			if(tmpx >= pc->x1  &&  tmpx <= pc->x2){
				for(p = np; p!= NULL && p->x <= np->x+iLength-1; p = p->to){
					if(p->x == tmpx){
						iFlg = 0;
						break;
					}
				}
				if(Function == 8 && iFlg == 1 && tmpx < np->x + iLength - 3)iFlg = 0; //I do not have to bother adding it unless it&#39;s the last one.
				if(Function < 20){
					if(iFlg == 1){
						if(FALSE == SetVolume(tmpx, np->volume)){
							return FALSE;
						}
					}
				}else if(Function == 20){
					if(iFlg == 0){
						if(FALSE == CutVolume(tmpx, 0)){ //I think the second argument is meaningless.
							return FALSE;
						}
					}

				}
			}
		}
		if(Function >= 1 && Function < 20){
			tmpc.track = pc->track;  tmpc.x1 = np->x;  tmpc.x2 = np->x + np->length - 1;  tmpc.a = 1;  
			tmpc.mode = (unsigned char)(MODEDECAY + Function);
			if(np->y!=KEYDUMMY)ChangeVolumeData(&tmpc);
		}else if(Function == 20){ //Clear
			//do nothing
		}
	}
	return TRUE;
}

//iFullTrack==1WheniTrackIgnore.
bool OrgData::CopyNoteDataToCB(NOTECOPY *nc, int iTrack, int iFullTrack )
{
	/*
	int i,j,t,n;
	PARCHANGE pc;//For paste area clearing
	NOTELIST *np;
//	NOTELIST *p_in1,*p_in2;//Place to insert
	NOTELIST *p_list1,*p_list2;//List to insert
	NOTELIST *work;//Work area
	NOTELIST *wp;//Work pointer
	long ind_x;//Index value (x)
	long copy_num;//Number of notes to be copied

	ClearVirtualCB();
	AddStartToVirtualCB(); //Start assigning to clipboard
	AddIntegerToVirtualCB(iFullTrack);
	AddIntegerToVirtualCB(iTrack);
	AddIntegerToVirtualCB(nc->x1_1); //from here
	AddIntegerToVirtualCB(nc->x1_2); //So far

	//Anyway, transfer contents to clipboard

	ind_x = nc->x1_1;

	for(t=0;t<MAXTRACK;t++){
		if(iFullTrack==1 || iTrack==t){
			copy_num = GetNoteNumber(t,nc);//Detects the number of notes used in that range
			AddIntegerToVirtualCB(copy_num); //How many notes?

			if(copy_num == 0){
				AddTrackSeparater(); //Insert @
				continue;
			}

			//wp = work = (NOTELIST *)malloc(sizeof(NOTELIST)*copy_num);//Secure space for work
			//Copy to work area
			np = info.tdata[nc->track1].note_list;
			while(np != NULL && np->x < nc->x1_1)np = np->to;
			if(np == NULL){
				//free( work );
				AddTrackSeparater(); //Insert @
				continue;//No note in the copy source(While head searchingOUT)...Is there such a situation?
			}
			for(i = 0; i < copy_num; i++){//1 Paste
				//wp->length = np->length;
				//wp->pan    = np->pan;
				//wp->volume = np->volume;
				//wp->y      = np->y;
				//Deduct the index value
				//wp->x      = np->x - ind_x;
				//next
				AddIntegerToVirtualCB((int)np->length);
				AddIntegerToVirtualCB((int)np->pan);
				AddIntegerToVirtualCB((int)np->volume);
				AddIntegerToVirtualCB((int)np->y);
				n = np->x - ind_x;
				AddIntegerToVirtualCB(n);
				np = np->to;
				//wp++;
			}
			//free( work );
			AddTrackSeparater(); //Insert @
		}
	}
	SetClipBoardFromVCB(); //On a true clipboard
*/
	return true;
}


BOOL OrgData::SwapTrack(NOTECOPY *pc)
{
	TRACKDATA tmp;
	
	memcpy(&tmp, &info.tdata[ pc->track1 ], sizeof(TRACKDATA));
	memcpy(&info.tdata[ pc->track1 ], &info.tdata[ pc->track2 ], sizeof(TRACKDATA));
	memcpy(&info.tdata[ pc->track2 ], &tmp, sizeof(TRACKDATA));

	unsigned char uctmp;
	uctmp = def_pan[pc->track1];
	def_pan[pc->track1] = def_pan[pc->track2];
	def_pan[pc->track2] = uctmp;

	uctmp = def_volume[pc->track1];
	def_volume[pc->track1] = def_volume[pc->track2];
	def_volume[pc->track2] = uctmp;

	//char ctmp;
	//ctmp = mute[pc->track1];
	//mute[pc->track1] = mute[pc->track2];
	//mute[pc->track2] = ctmp;


	return TRUE;
}