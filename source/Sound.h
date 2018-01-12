/*****							  *****/
/*****   DirectSoundSupport function   *****/
/*****							  *****/

//#include <dsound.h>

// DirectSoundThe start of the
BOOL InitDirectSound(HWND hwnd);
BOOL LoadSoundObject( char *file_name, int no);
// DirectSoundTermination
void EndDirectSound(void);
void ReleaseSoundObject(int no);//Release of buffer
BOOL InitSoundObject(LPCSTR resname, int no);
// Play sound
void PlaySoundObject(int no, int mode);
void ChangeSoundVolume(int no, long volume);//9999ButMAX2195?Is normal
void ChangeSoundFrequency(int no, DWORD rate);//300ButMAX300Is normal
void ChangeSoundPan(int no, long pan);

BOOL MakeSoundObject8(char *wavep,char track, char pipi);

void ChangeOrganFrequency(unsigned char key,char track,DWORD a);//9999ButMAXso2195?Is normal
void ChangeOrganVolume(int no, long volume,char track);//300ButMAXso300Is normal
void ChangeOrganPan(unsigned char key, unsigned char pan,char track);//0But←so6Is normal11But→
void PlayOrganObject(unsigned char key, int mode,char track,DWORD freq);
void PlayOrganObject2(unsigned char key, int mode,char track,DWORD freq);
void PlayOrganKey(unsigned char key,char track,DWORD freq, int Nagasa = 80);
BOOL LoadWaveData100(void);
BOOL InitWaveData100(void);
BOOL DeleteWaveData100(void);

BOOL MakeOrganyaWave(char track,char wave_no, char pipi);

//BOOL LoadDramObject(char *file_name, int no);
BOOL InitDramObject(LPCSTR resname, int no);
void ReleaseDramObject(char track);
void ChangeDramFrequency(unsigned char key,char track);
void ChangeDramPan(unsigned char pan,char track);//512ButMAXso256Is normal
void ChangeDramVolume(long volume,char track);//300ButMAXso300Is normal
void PlayDramObject(unsigned char key, int mode,char track);


void Rxo_ShowDirectSoundObject(HWND hwnd);
void Rxo_StopAllSoundNow(void);

void Rxo_PlayKey(unsigned char key,char track,DWORD freq, int Phase);
void Rxo_StopKey(unsigned char key,char track, int Phase);