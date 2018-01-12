
#define GET_MEAS1	1
#define GET_MEAS2	3
#define GET_BEAT1	2
#define GET_BEAT2	4

#define MESSAGE_STRING_MAX 1024


void TransportNote(int AddNotes , int Track );
void VolumeEdit(int AddNotes , int Track );
void VolumeWariaiEdit(int AddNotes , int Track ); //2014.04.30 A
void VolumeDecayEdit(int AddNotes , int Track , int Function ); //2014.05.01 A

void PanEdit(int AddNotes , int Track );
void MuteTrack(int Track);

extern char *TrackCode[]; //For track name and number substitution
extern int Menu_Recent[];
extern char RecentFileName[10][MAX_PATH];	//Recent file name

extern int ReverseTrackCode(char *strTrack); //Its inverse function
extern void ShowMemoryState();
extern void SortMusicNote(void); //Organizing notes


extern void ClearVirtualCB(void);
extern void AddIntegerToVirtualCB(int iNum);
extern void AddStartToVirtualCB(void);
extern bool ReadStartFromVirtualCB(void);
extern int ReadIntegerFromVirtualCB(void);
extern void SetClipBoardFromVCB(void);
extern void GetClipBoardToVCB(void);
extern void AddTrackSeparater(void);
extern void AddStartToVirtualCB(void);

extern void ReplaseUndo();
extern void SetUndo();
extern void ClearUndo();
extern void ReplaceRedo();
extern void ResetLastUndo(); //Take
int GetSelectMeasBeat(int GetToValue, int addValue=1);
int MinimumGrid(int x);
int MaximumGrid(int x);
int MinimumGridLine(int x);
int MaximumGridLine(int x);
extern void ChangeGridMode(int iValue=-1);
extern void ChangeSelAlwaysCurrent(int iValue=-1);
extern void ChangeDrawDouble(int iValue=-1);
void ChangeDragMode(int iValue=-1);
extern void ChangeEnablePlaying(int iValue=-1);
extern void ChangeFinish(int iValue=-1);
extern void ChangeSlideOverlapNoteMode(int iValue = -1);
extern void ChangePushStratchNOTE(int iValue = -1);

void PutRecentFile(char *FileName);
void LoadRecentFromIniFile();
void SaveRecentFilesToInifile();
void SetLoadRecentFile(int iNum);
void ClearRecentFile();

int msgbox(HWND hWnd , int MessageID, int TitleID, UINT uType);		// 2014.10.18 

//Release and initialize message table
void FreeMessageStringBuffer(void);	// 2014.10.19 
int AllocMessageStringBuffer(void);	// 2014.10.19 
//Message table
extern TCHAR *MessageString[MESSAGE_STRING_MAX];	// 2014.10.19 


extern int iChangeEnablePlaying; //2010.09.23 A
extern int iChangeFinish; //2010.09.23 A

extern void setRecentTrack(int iNewTrack); //Update the track number you used recently //2010.09.23 A
extern int getRecentTrack(int iOrder, int isDrumTrack); //Return the track you used recently //2010.09.23 A

extern int iActivatePAN; //2014.05.01 A
extern int iActivateVOL; //2014.05.01 A
extern int iSlideOverlapNotes; //2014.05.06 A

extern bool AutoLoadPVIFile();
extern bool AutoSavePVIFile();
extern int SaveWithInitVolFile;
extern void ChangeAutoLoadMode(int iValue = -1);
extern int tra;

extern int NoteWidth; //Musical note width 2014.05.26
extern int NoteEnlarge_Until_16px; //When displaying,NOTEMaximum head of16Stretch to the pixel.
extern void ChangeNoteEnlarge(int iValue = -1);
extern int iPushStratch;
extern int iLastEditNoteLength;
