#include <windows.h>
#include <stdio.h>
#include <dplay.h>
#include <dsound.h>
#include "Setting.h"
#include "DefOrg.h"
#define SE_MAX	512

// Symbol definition.
#define	SMPFRQ			48000				//!< Sampling frequency.
#define	BUFSIZE			((SMPFRQ * 4) / 10)	//!< Data buffer size (100msEquivalent).


// DirectSoundStructure 
LPDIRECTSOUND       lpDS = NULL;            // DirectSoundobject
LPDIRECTSOUNDBUFFER lpPRIMARYBUFFER = NULL; // Temporary buffer
LPDIRECTSOUNDBUFFER lpSECONDARYBUFFER[SE_MAX] = {NULL};
LPDIRECTSOUNDBUFFER lpORGANBUFFER[8][8][2] = {NULL};
LPDIRECTSOUNDBUFFER lpDRAMBUFFER[8] = {NULL};

//For recording
//HANDLE						CapEvent[2];			//!< Input event object.
//DWORD						CapBufSize;				//!< Capture buffer size.
//DWORD						GetPos;					//!< Capture buffer read start position.
//DWORD						PutPos;					//!< Writing start position of capture buffer.
//BYTE*						DataBuff;				//!< Data buffer.
//LPDIRECTSOUNDCAPTURE 		CapDev;					//!< IDirectSoundCaptureInterface Pointer.
//LPDIRECTSOUNDCAPTUREBUFFER	CapBuf;					//!< IDirectSoundBufferInterface Pointer.

DWORD						OutBufSize;				//!< Stream buffer size.


// DirectSoundThe start of the 
BOOL InitDirectSound(HWND hwnd)
{
//    int i;
    DSBUFFERDESC dsbd;

    // DirectDrawInitialize
    if(DirectSoundCreate(NULL, &lpDS, NULL) != DS_OK){
		lpDS = NULL;
		return(FALSE);
	}
    lpDS->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE);

    // Initialization of primary buffer
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER; // | DSBCAPS_CTRLPOSITIONNOTIFY;
    lpDS->CreateSoundBuffer(&dsbd, &lpPRIMARYBUFFER, NULL);

//    for(i = 0; i < SE_MAX; i++) lpSECONDARYBUFFER[i] = NULL;
	
	//Create capture buffer First argumentNULLDefault with. How about this?
//	if( DirectSoundCaptureCreate( NULL, &CapDev, NULL ) != S_OK ){
//		return FALSE;
//	}
//	dsbd.dwFlags = 0; //Secondary buffer
//	CapDev->CreateCaptureBuffer(&dsbd, &CapBuf, NULL);


    return(TRUE);
}

// DirectSoundTermination 
void EndDirectSound(void)
{
    int i;

    for(i = 0; i < 8; i++){
        if(lpSECONDARYBUFFER[i] != NULL){
			lpSECONDARYBUFFER[i]->Release();
			lpSECONDARYBUFFER[i] = NULL;
		}
    }
    if(lpPRIMARYBUFFER != NULL){
		lpPRIMARYBUFFER->Release();
		lpPRIMARYBUFFER = NULL;
	}
    if(lpDS != NULL){
		lpDS->Release();
		lpDS = NULL;
	}
//	if( CapBuf ){
//		CapBuf->Stop();
//	}
//    if(CapDev != NULL){
//		CapDev->Release();
//		CapDev = NULL;
//	}
}
void ReleaseSoundObject(int no){
	if(lpSECONDARYBUFFER[no] != NULL){
		lpSECONDARYBUFFER[no]->Release();
		lpSECONDARYBUFFER[no] = NULL;
	}
}


// Sound settings 
BOOL InitSoundObject( LPCSTR resname, int no)
{
    HRSRC hrscr;
    DSBUFFERDESC dsbd;
    DWORD *lpdword;//Resource address
    // Search resources
    if((hrscr = FindResource(NULL, resname, "WAVE")) == NULL)
                                                    return(FALSE);
    // Get resource address
    lpdword = (DWORD*)LockResource(LoadResource(NULL, hrscr));
	// Generation of secondary buffer
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = 
		DSBCAPS_STATIC|
		DSBCAPS_STICKYFOCUS
		|DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
	dsbd.dwBufferBytes = *(DWORD*)((BYTE*)lpdword+0x36);//WAVEData size
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)(lpdword+5); 
	if(lpDS->CreateSoundBuffer(&dsbd, &lpSECONDARYBUFFER[no],
								NULL) != DS_OK) return(FALSE);
    LPVOID lpbuf1, lpbuf2;
    DWORD dwbuf1, dwbuf2;
    // Secondary buffer lock
    lpSECONDARYBUFFER[no]->Lock(0, *(DWORD*)((BYTE*)lpdword+0x36),
                        &lpbuf1, &dwbuf1, &lpbuf2, &dwbuf2, 0); 
	// Setting of sound source data
	CopyMemory(lpbuf1, (BYTE*)lpdword+0x3a, dwbuf1);
    if(dwbuf2 != 0) CopyMemory(lpbuf2, (BYTE*)lpdword+0x3a+dwbuf1, dwbuf2);
	// Unlock secondary buffer
	lpSECONDARYBUFFER[no]->Unlock(lpbuf1, dwbuf1, lpbuf2, dwbuf2); 

    return(TRUE);
}

//extern LPDIRECTDRAW            lpDD;	// DirectDrawobject
BOOL LoadSoundObject(char *file_name, int no)
{
	DWORD i;
	DWORD file_size = 0;
	char check_box[58];
	FILE *fp;
	if((fp=fopen(file_name,"rb"))==NULL){
//		char msg_str[64];				//For numerical confirmation
//		lpDD->FlipToGDISurface(); //Flip toward the message display
//		sprintf(msg_str,"%sI can not find it",file_name);
//		MessageBox(hWND,msg_str,"title",MB_OK);
//		SetCursor(FALSE); // Clear cursor
		return(FALSE);
	}
	for(i = 0; i < 58; i++){
		fread(&check_box[i],sizeof(char),1,fp);
	}
	if(check_box[0] != 'R')return(FALSE);
	if(check_box[1] != 'I')return(FALSE);
	if(check_box[2] != 'F')return(FALSE);
	if(check_box[3] != 'F')return(FALSE);
	file_size = *((DWORD *)&check_box[4]);

	DWORD *wp;
	wp = (DWORD*)malloc(file_size);//Make a file workspace
	fseek(fp,0,SEEK_SET);
	for(i = 0; i < file_size; i++){
		fread((BYTE*)wp+i,sizeof(BYTE),1,fp);
	}
	fclose(fp);
	//Generate secondary buffer
	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_STATIC|DSBCAPS_STICKYFOCUS|DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
	dsbd.dwBufferBytes = *(DWORD*)((BYTE*)wp+0x36);//WAVEData size
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)(wp+5); 
	if(lpDS->CreateSoundBuffer(&dsbd, &lpSECONDARYBUFFER[no],
								NULL) != DS_OK){
		free(wp);
		return(FALSE);	
	}
    LPVOID lpbuf1, lpbuf2;
    DWORD dwbuf1, dwbuf2;
	HRESULT hr;
	hr = lpSECONDARYBUFFER[no]->Lock(0, file_size-58,
							&lpbuf1, &dwbuf1, &lpbuf2, &dwbuf2, 0); 
	if(hr != DS_OK){
		free(wp);
		return (FALSE);
	}
	CopyMemory(lpbuf1, (BYTE*)wp+0x3a, dwbuf1);//+3aThe head of data
	if(dwbuf2 != 0)	CopyMemory(lpbuf2, (BYTE*)wp+0x3a+dwbuf1, dwbuf2);
	lpSECONDARYBUFFER[no]->Unlock(lpbuf1, dwbuf1, lpbuf2, dwbuf2); 
	
	free(wp);
	return(TRUE);
}

// Play sound 
void PlaySoundObject(int no, int mode)
{
    if(lpSECONDARYBUFFER[no] != NULL){
		switch(mode){
		case 0: // Stop
			lpSECONDARYBUFFER[no]->Stop();
			break;
		case 1: // Playback
			lpSECONDARYBUFFER[no]->Stop();
			lpSECONDARYBUFFER[no]->SetCurrentPosition(0);
            lpSECONDARYBUFFER[no]->Play(0, 0, 0);
            break;
		case -1: // Loop play
			lpSECONDARYBUFFER[no]->Play(0, 0, DSBPLAY_LOOPING);
			break;
		}
    }
}

void ChangeSoundFrequency(int no, DWORD rate)//100ButMIN9999ButMAXso2195?Is normal
{
	if(lpSECONDARYBUFFER[no] != NULL)
		lpSECONDARYBUFFER[no]->SetFrequency( rate );
}
void ChangeSoundVolume(int no, long volume)//300ButMAXso300Is normal
{
	if(lpSECONDARYBUFFER[no] != NULL)
		lpSECONDARYBUFFER[no]->SetVolume((volume-300)*8);
}
void ChangeSoundPan(int no, long pan)//512ButMAXso256Is normal
{
	if(lpSECONDARYBUFFER[no] != NULL)
		lpSECONDARYBUFFER[no]->SetPan((pan-256)*10);
}

/////////////////////////////////////////////
//■Orgagna■■■■■■■■■■■■///////
/////////////////////


typedef struct{
	short wave_size;//Waveform size
	short oct_par;//Multiplying factor to achieve octave(/8)
	short oct_size;//Multiplying factor to achieve octave(/8)
}OCTWAVE;

OCTWAVE oct_wave[8] = {
	{256,  1, 4},//0 Oct
	{256,  2, 8},//1 Oct
	{128,  4, 12},//2 Oct
	{128,  8, 16},//3 Oct
	{ 64, 16, 20},//4 Oct
	{ 32, 32, 24},
	{ 16, 64, 28},
	{  8,128, 32},
};
BYTE format_tbl2[] = {0x01,0x00,0x01,0x00,0x22,0x56,0x00,//22050HzofFormat
0x00,0x22,0x56,0x00,0x00,0x01,0x00,0x08,0x00,0x00,0x00};
//BYTE format_tbl3[] = {0x01,0x00,0x01,0x00,0x44,0xac,0x00,//441000HzofFormat
//0x00,0x44,0xac,0x00,0x00,0x08,0x00,0x00,0x00,0x66,0x61};
BOOL MakeSoundObject8(char *wavep,char track, char pipi )
{
	DWORD i,j,k;
	unsigned long wav_tp;//WAVPointer pointing to table
	DWORD wave_size;//256;
	DWORD data_size;
	BYTE *wp;
	BYTE *wp_sub;
	int work;
	//Generate secondary buffer
	DSBUFFERDESC dsbd;

	for(j = 0; j < 8; j++){
		for(k = 0; k < 2; k++){
			wave_size = oct_wave[j].wave_size;
			if( pipi )data_size = wave_size * oct_wave[j].oct_size;
			else data_size = wave_size;
			ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
			dsbd.dwSize = sizeof(DSBUFFERDESC);
			dsbd.dwFlags = DSBCAPS_STATIC|
					DSBCAPS_STICKYFOCUS|
					DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;//DSBCAPS_STATIC|DSBCAPS_CTRLDEFAULT;
			dsbd.dwBufferBytes = data_size;//file_size;
			dsbd.lpwfxFormat = (LPWAVEFORMATEX)(&format_tbl2[0]);
				if(lpDS->CreateSoundBuffer(&dsbd, &lpORGANBUFFER[track][j][k],//j = se_no
										NULL) != DS_OK) return(FALSE);
			wp = (BYTE*)malloc(data_size);//Make a file workspace
			wp_sub = wp;
			wav_tp = 0;
			for(i = 0; i < data_size; i++){
				work = *(wavep+wav_tp);
				work+=0x80;
				*wp_sub=(BYTE)work;
				wav_tp += 256/wave_size;
				if( wav_tp > 255 ) wav_tp -= 256;
				wp_sub++;
			}
			//Data transfer
			LPVOID lpbuf1, lpbuf2;
			DWORD dwbuf1, dwbuf2=0;
			HRESULT hr;
			hr = lpORGANBUFFER[track][j][k]->Lock(0, data_size,//-58,
								&lpbuf1, &dwbuf1, &lpbuf2, &dwbuf2, 0);		
			if(hr != DS_OK){
				free( wp );
				return (FALSE);
			}
			CopyMemory(lpbuf1, (BYTE*)wp,dwbuf1);//+3aThe head of data
			if(dwbuf2 != 0)	CopyMemory(lpbuf2, (BYTE*)wp+dwbuf1, dwbuf2);
			lpORGANBUFFER[track][j][k]->Unlock(lpbuf1, dwbuf1, lpbuf2, dwbuf2); 
			lpORGANBUFFER[track][j][k]->SetCurrentPosition(0);
			free( wp );
		}
	}
	return(TRUE);
}
//2.1.0so I changed from integer type to decimal type.20140401
//short freq_tbl[12] = {261,278,294,311,329,349,371,391,414,440,466,494};
double freq_tbl[12] = {261.62556530060, 277.18263097687, 293.66476791741, 311.12698372208, 329.62755691287, 349.22823143300, 369.99442271163, 391.99543598175, 415.30469757995, 440.00000000000, 466.16376151809, 493.88330125612};
void ChangeOrganFrequency(unsigned char key,char track, DWORD a)
{
	double tmpDouble;
	for(int j = 0; j < 8; j++)
		for(int i = 0; i < 2; i++){
			tmpDouble = (((double)oct_wave[j].wave_size * freq_tbl[key])*(double)oct_wave[j].oct_par)/8.00f + ((double)a - 1000.0f);
			
			
			lpORGANBUFFER[track][j][i]->SetFrequency(//1000To+αDefault value of
				(DWORD)tmpDouble
//				((oct_wave[j].wave_size*freq_tbl[key])*oct_wave[j].oct_par)/8 + (a-1000)
			);
		}
}
short pan_tbl[13] = {0,43,86,129,172,215,256,297,340,383,426,469,512}; 
unsigned char old_key[MAXTRACK] = {255,255,255,255,255,255,255,255};//Sound during playback
unsigned char key_on[MAXTRACK] = {0};//Key switch
unsigned char key_twin[MAXTRACK] = {0};//The key you are using now(Two are prepared for noise prevention at the time of continuous)
void ChangeOrganPan(unsigned char key, unsigned char pan,char track)//512ButMAXso256Is normal
{
	if(old_key[track] != 255)
		lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->SetPan((pan_tbl[pan]-256)*10);
}
void ChangeOrganVolume(int no, long volume,char track)//300ButMAXso300Is normal
{
	if(old_key[track] != 255)
		lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->SetVolume((volume-255)*8);
}
// Play sound 
void PlayOrganObject(unsigned char key, int mode,char track,DWORD freq)
{
	
    if(lpORGANBUFFER[track][key/12][key_twin[track]] != NULL){
		switch(mode){
		case 0: // Stop
			lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->Stop();
			lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->SetCurrentPosition(0);
			break;
		case 1: // Playback
//			if(key_on == 1 && no == old_key/12)//
//				lpORGANBUFFER[old_key/12]->Stop();
//				ChangeOrganFrequency(key%12);//Set the frequency
//				lpORGANBUFFER[no]->Play(0, 0, 0);
//			if(key_on == 1 && no == old_key/12){//It&#39;s ringingWAVSameWAVNOIf
//				old_key = key;
//				ChangeOrganFrequency(key%12);//Just change the frequency
//			}
			break;
		case 2: // Stop walking
			if(old_key[track] != 255){
				lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->Play(0, 0, 0);
				old_key[track] = 255;
			}
            break;
		case -1:
			if(old_key[track] == 255){//To ring a new sound
				ChangeOrganFrequency(key%12,track,freq);//Set the frequency
				lpORGANBUFFER[track][key/12][key_twin[track]]->Play(0, 0, DSBPLAY_LOOPING);
				old_key[track] = key;
				key_on[track] = 1;
			}else if(key_on[track] == 1 && old_key[track] == key){//Same sound
				//Stop walking now and stop
				lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->Play(0, 0, 0);
				key_twin[track]++;
				if(key_twin[track] == 2)key_twin[track] = 0; 
				lpORGANBUFFER[track][key/12][key_twin[track]]->Play(0, 0, DSBPLAY_LOOPING);
			}else{//If you play a different sound
				lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->Play(0, 0, 0);//Stop walking now and stop
				key_twin[track]++;
				if(key_twin[track] == 2)key_twin[track] = 0; 
				ChangeOrganFrequency(key%12,track,freq);//Set the frequency
				lpORGANBUFFER[track][key/12][key_twin[track]]->Play(0, 0, DSBPLAY_LOOPING);
				old_key[track] = key;
			}
			break;
		}
    }
}
//Crack
void PlayOrganObject2(unsigned char key, int mode,char track,DWORD freq)
{
	
    if(lpORGANBUFFER[track][key/12][key_twin[track]] != NULL){
		switch(mode){
		case 0: // Stop
			lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->Stop();
			lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->SetCurrentPosition(0);
			break;
		case 1: // Playback
//			if(key_on == 1 && no == old_key/12)//
//				lpORGANBUFFER[old_key/12]->Stop();
//				ChangeOrganFrequency(key%12);//Set the frequency
//				lpORGANBUFFER[no]->Play(0, 0, 0);
//			if(key_on == 1 && no == old_key/12){//It&#39;s ringingWAVSameWAVNOIf
//				old_key = key;
//				ChangeOrganFrequency(key%12);//Just change the frequency
//			}
			break;
		case 2: // Stop walking
			if(old_key[track] != 255){
//				lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->Play(0, 0, 0);
				old_key[track] = 255;
			}
            break;
		case -1:
			if(old_key[track] == 255){//To ring a new sound
				ChangeOrganFrequency(key%12,track,freq);//Set the frequency
				lpORGANBUFFER[track][key/12][key_twin[track]]->Play(0, 0, 0);//DSBPLAY_LOOPING);
				old_key[track] = key;
				key_on[track] = 1;
			}else if(key_on[track] == 1 && old_key[track] == key){//Same sound
				//Stop walking now and stop
//				lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->Play(0, 0, 0);
				key_twin[track]++;
				if(key_twin[track] == 2)key_twin[track] = 0; 
				lpORGANBUFFER[track][key/12][key_twin[track]]->Play(0, 0, 0);//DSBPLAY_LOOPING);
			}else{//If you play a different sound
//				lpORGANBUFFER[track][old_key[track]/12][key_twin[track]]->Play(0, 0, 0);//Stop walking now and stop
				key_twin[track]++;
				if(key_twin[track] == 2)key_twin[track] = 0; 
				ChangeOrganFrequency(key%12,track,freq);//Set the frequency
				lpORGANBUFFER[track][key/12][key_twin[track]]->Play(0, 0, 0);//DSBPLAY_LOOPING);
				old_key[track] = key;
			}
			break;
		}
    }
}
//Opening Orgagna Object
void ReleaseOrganyaObject(char track){
	for(int i = 0; i < 8; i++){
		if(lpORGANBUFFER[track][i][0] != NULL){
			lpORGANBUFFER[track][i][0]->Release();
			lpORGANBUFFER[track][i][0] = NULL;
		}
		if(lpORGANBUFFER[track][i][1] != NULL){
			lpORGANBUFFER[track][i][1]->Release();
			lpORGANBUFFER[track][i][1] = NULL;
		}
	}
}
//Load waveform data
//char wave_data[100*256];
char *wave_data = NULL;
BOOL InitWaveData100(void)
{
	if(wave_data == NULL)wave_data = (char *)malloc(sizeof(char) * 256 * 256);
    HRSRC hrscr;
    DWORD *lpdword;//Resource address
    // Search resources
    if((hrscr = FindResource(NULL, "WAVE100", "WAVE")) == NULL)
                                                    return(FALSE);
    // Get resource address
    lpdword = (DWORD*)LockResource(LoadResource(NULL, hrscr));
	memcpy(wave_data,lpdword,100*256);
	return TRUE;
}
BOOL LoadWaveData100(void)
{
	if(wave_data == NULL)wave_data = (char *)malloc(sizeof(char) * 256 * 256);
	FILE *fp;
	if((fp=fopen("Wave.dat","rb"))==NULL){
		return FALSE;
	}
//	wave_data = new char[100*256];
	fread(wave_data, sizeof(char), 256*100, fp);
	fclose(fp);
	return TRUE;
}
BOOL DeleteWaveData100(void)
{
//	delete wave_data;
	free(wave_data);
	return TRUE;
}
//Select and create a waveform from 100
BOOL MakeOrganyaWave(char track,char wave_no, char pipi)
{
	if(wave_no > 99)return FALSE;
	ReleaseOrganyaObject(track);
	MakeSoundObject8(&wave_data[0] + wave_no*256,track, pipi);	
	return TRUE;
}
/////////////////////////////////////////////
//■Organa drums■■■■■■■■///////
/////////////////////
//Opening Orgagna Object
void ReleaseDramObject(char track){
	for(int i = 0; i < 8; i++){
		if(lpDRAMBUFFER[track] != NULL){
			lpDRAMBUFFER[track]->Release();
			lpDRAMBUFFER[track] = NULL;
		}
	}
}
// Sound settings 
BOOL InitDramObject( LPCSTR resname, int no)
{
    HRSRC hrscr;
    DSBUFFERDESC dsbd;
    DWORD *lpdword;//Resource address
    // Search resources
	ReleaseDramObject(no); //I tried it here.

    if((hrscr = FindResource(NULL, resname, "WAVE")) == NULL)
                                                    return(FALSE);
    // Get resource address
    lpdword = (DWORD*)LockResource(LoadResource(NULL, hrscr));
	// Generation of secondary buffer
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = 
		DSBCAPS_STATIC|
		DSBCAPS_STICKYFOCUS
		|DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
	dsbd.dwBufferBytes = *(DWORD*)((BYTE*)lpdword+0x36);//WAVEData size
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)(lpdword+5); 
	if(lpDS->CreateSoundBuffer(&dsbd, &lpDRAMBUFFER[no],NULL) != DS_OK) return(FALSE);
    LPVOID lpbuf1, lpbuf2;
    DWORD dwbuf1, dwbuf2;
    // Secondary buffer lock
    lpDRAMBUFFER[no]->Lock(0, *(DWORD*)((BYTE*)lpdword+0x36),
                        &lpbuf1, &dwbuf1, &lpbuf2, &dwbuf2, 0); 
	// Setting of sound source data
	CopyMemory(lpbuf1, (BYTE*)lpdword+0x3a, dwbuf1);
    if(dwbuf2 != 0){
		CopyMemory(lpbuf2, (BYTE*)lpdword+0x3a+dwbuf1, dwbuf2);
		
		
	}

	// Unlock secondary buffer
	lpDRAMBUFFER[no]->Unlock(lpbuf1, dwbuf1, lpbuf2, dwbuf2); 
	lpDRAMBUFFER[no]->SetCurrentPosition(0);

    return(TRUE);
}

/*
BOOL LoadDramObject(char *file_name, int no)
{
	DWORD i;
	DWORD file_size = 0;
	char check_box[58];
	FILE *fp;
	ReleaseDramObject(no);
	if((fp=fopen(file_name,"rb"))==NULL){
//		char msg_str[64];				//For numerical confirmation
//		lpDD->FlipToGDISurface(); //Flip toward the message display
//		sprintf(msg_str,"%sI can not find it",file_name);
//		MessageBox(hWND,msg_str,"title",MB_OK);
//		SetCursor(FALSE); // Clear cursor
		return(FALSE);
	}
	for(i = 0; i < 58; i++){
		fread(&check_box[i],sizeof(char),1,fp);
	}
	if(check_box[0] != 'R')return(FALSE);
	if(check_box[1] != 'I')return(FALSE);
	if(check_box[2] != 'F')return(FALSE);
	if(check_box[3] != 'F')return(FALSE);
	file_size = *((DWORD *)&check_box[4]);

	DWORD *wp;
	wp = (DWORD*)malloc(file_size);//Make a file workspace
	fseek(fp,0,SEEK_SET);
	for(i = 0; i < file_size; i++){
		fread((BYTE*)wp+i,sizeof(BYTE),1,fp);
	}
	fclose(fp);
	//Generate secondary buffer
	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_STATIC|
		DSBCAPS_STICKYFOCUS
		|DSBCAPS_CTRLDEFAULT;
	dsbd.dwBufferBytes = *(DWORD*)((BYTE*)wp+0x36);//WAVEData size
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)(wp+5); 
	if(lpDS->CreateSoundBuffer(&dsbd, &lpDRAMBUFFER[no],
								NULL) != DS_OK) return(FALSE);	
    LPVOID lpbuf1, lpbuf2;
    DWORD dwbuf1, dwbuf2;
	HRESULT hr;
	hr = lpDRAMBUFFER[no]->Lock(0, file_size-58,
							&lpbuf1, &dwbuf1, &lpbuf2, &dwbuf2, 0); 
	if(hr != DS_OK)return (FALSE);
	CopyMemory(lpbuf1, (BYTE*)wp+0x3a, dwbuf1);//+3aThe head of data
	if(dwbuf2 != 0)	CopyMemory(lpbuf2, (BYTE*)wp+0x3a+dwbuf1, dwbuf2);
	lpDRAMBUFFER[no]->Unlock(lpbuf1, dwbuf1, lpbuf2, dwbuf2); 
	
	free(wp);
	return(TRUE);
}*/
void ChangeDramFrequency(unsigned char key,char track)
{
	lpDRAMBUFFER[track]->SetFrequency(key*800+100);
}
void ChangeDramPan(unsigned char pan,char track)//512ButMAXso256Is normal
{
	lpDRAMBUFFER[track]->SetPan((pan_tbl[pan]-256)*10);
}
void ChangeDramVolume(long volume,char track)//
{
	lpDRAMBUFFER[track]->SetVolume((volume-255)*8);
}
// Play sound 
void PlayDramObject(unsigned char key, int mode,char track)
{
	
    if(lpDRAMBUFFER[track] != NULL){
		switch(mode){
		case 0: // Stop
			lpDRAMBUFFER[track]->Stop();
			lpDRAMBUFFER[track]->SetCurrentPosition(0);
			break;
		case 1: // Playback
			lpDRAMBUFFER[track]->Stop();
			lpDRAMBUFFER[track]->SetCurrentPosition(0);
			ChangeDramFrequency(key,track);//Set the frequency
			lpDRAMBUFFER[track]->Play(0, 0, 0);
			break;
		case 2: // Stop walking
            break;
		case -1:
			break;
		}
    }
}
void PlayOrganKey(unsigned char key,char track,DWORD freq,int Nagasa)
{
	if(key>96)return;
	if(track < MAXMELODY){
		DWORD wait = timeGetTime();
		ChangeOrganFrequency(key%12,track,freq);//Set the frequency
		lpORGANBUFFER[track][key/12][0]->SetVolume((160-255)*8);
		lpORGANBUFFER[track][key/12][0]->Play(0, 0, DSBPLAY_LOOPING);
		do{
		}while(timeGetTime() < wait + (DWORD)Nagasa);
//		lpORGANBUFFER[track][key/12][0]->Play(0, 0, 0); //C 2010.09.23 Immediately stop.
		lpORGANBUFFER[track][key/12][0]->Stop();
	}else{
		lpDRAMBUFFER[track - MAXMELODY]->Stop();
		lpDRAMBUFFER[track - MAXMELODY]->SetCurrentPosition(0);
		ChangeDramFrequency(key,track - MAXMELODY);//Set the frequency
		lpDRAMBUFFER[track - MAXMELODY]->SetVolume((160-255)*8);
		lpDRAMBUFFER[track - MAXMELODY]->Play(0, 0, 0);
	}
}


//2010.08.14 A
void Rxo_PlayKey(unsigned char key,char track,DWORD freq, int Phase)
{
	if(key>96)return;
	if(track < MAXMELODY){
		ChangeOrganFrequency(key%12,track,freq);
		lpORGANBUFFER[track][key/12][Phase]->SetVolume((160-255)*8);
		lpORGANBUFFER[track][key/12][Phase]->Play(0, 0, DSBPLAY_LOOPING);
	}else{
		lpDRAMBUFFER[track - MAXMELODY]->Stop();
		lpDRAMBUFFER[track - MAXMELODY]->SetCurrentPosition(0);
		ChangeDramFrequency(key,track - MAXMELODY);//Set the frequency
		lpDRAMBUFFER[track - MAXMELODY]->SetVolume((160-255)*8);
		lpDRAMBUFFER[track - MAXMELODY]->Play(0, 0, 0);
	}
}
//2010.08.14 A
void Rxo_StopKey(unsigned char key,char track, int Phase)
{
	if(track < MAXMELODY){
		//lpORGANBUFFER[track][key/12][Phase]->Play(0, 0, 0);	// 2010.08.14 D
		lpORGANBUFFER[track][key/12][Phase]->Stop();	// 2010.08.14 A
	}else{
		lpDRAMBUFFER[track - MAXMELODY]->Stop();
		lpDRAMBUFFER[track - MAXMELODY]->SetCurrentPosition(0);
	}	
}

//For debugging purposes. Output various states.
void Rxo_ShowDirectSoundObject(HWND hwnd)
{
	
}

//Stop all sound immediately
void Rxo_StopAllSoundNow(void)
{
	int i,j,k;
	for(i=0;i<SE_MAX;i++) if(lpSECONDARYBUFFER[i]!=NULL)lpSECONDARYBUFFER[i]->Stop();
	
	for(i=0;i<8;i++){
		for(j=0;j<8;j++)for(k=0;k<2;k++)lpORGANBUFFER[i][j][k]->Stop();
		lpDRAMBUFFER[i]->Stop();
	}
	for(i=0;i<MAXTRACK;i++)old_key[i]=255; //2014.05.02 A This will make it a strange sound.
}
