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

//Note list structure
typedef struct NOTELIST{
	NOTELIST *from;//Previous address
	NOTELIST *to;//Next address
	long x;//position
	unsigned char length;//Sound length
	unsigned char y;//Sound height
	unsigned char volume;//Volume
	unsigned char pan;//Bread
}NOTELIST;
//Track data * 8
typedef struct{
	unsigned short freq;//+αfrequency(1000ButDefault)
//	unsigned short note_num;//Number of notes
	unsigned char wave_no;//WaveformNo
	char pipi;
	NOTELIST *note_p;//NoteDataStart address of area
	NOTELIST *note_list;//Start address of the list1
}TRACKDATA;
//Unique information held in song units
typedef struct MUSICINFO{
	unsigned short wait;
	unsigned char line;//Number of lines in one measure
	unsigned char dot;//Number of dots per night
	unsigned short alloc_note;//Number of secured notes
	long repeat_x;//repeat
	long end_x;//End of song(Return to repeat)
	TRACKDATA tdata[16];
}MUSICINFO;
//Main class. The heart of this application. (I use class for the first time)
typedef struct OrgData{
	OrgData();//constructor
	~OrgData();//Destructor
	public:
		MUSICINFO info;
		//unsigned char *noteon;
		BOOL SetNote_afterSetLength(long x);
		BOOL SetNote_onlyLength(long x, long Length);

		void RedrawSelectArea(void);
		//For undo
		int ReplaceFromRedoData();
		int ReplaceFromUndoData();
		void ClearUndoData();
		int SetUndoData();
		int ResetLastUndo(void); //To the end that there was no undo! (Such as an empty click)
		MUSICINFO ud_tdata[32]; //Tentatively, Undo32Times
		NOTELIST  ud_note[32][16][4096];
		bool UndoEnable;
		bool RedoEnable;
		int CurrentUndoCursor; //Write to this number
		int MinimumUndoCursor; //It will go back to this number
		int MaximumUndoCursor; //You can redo until this number (redo possible)

		char track;
		char mute[MAXTRACK];
		unsigned char def_pan[MAXTRACK];
		unsigned char def_volume[MAXTRACK];
		void InitOrgData(void);
		void GetMusicInfo(MUSICINFO *mi);//Acquire song information
		//Set song information.flagSpecify setting item
		BOOL SetMusicInfo(MUSICINFO *mi,unsigned long flag);
		BOOL NoteAlloc(unsigned short note_num);//As many as specifiedNoteDataSecure an area of
		void ReleaseNote(void);//NoteDataOpen
		NOTELIST *SearchNote(NOTELIST *np);//unusedNOTESearch
		void TouchKeyboard(unsigned char y);//Touch the keyboard
		BOOL SetNote(long x,unsigned char y, int DragMode = 0);//Place notes
		BOOL CutNote(long x,unsigned char y);//Cut the note
		BOOL SetPan(long x,unsigned char y);//Place pan notes
		BOOL SetPan2(long x,unsigned char y);//Place pan notes
		BOOL CutPan(long x,unsigned char y);//Cut of pan notes
		BOOL SetVolume(long x,unsigned char y);//Place volume notes
		BOOL SetVolume2(long x,unsigned char y,long fade); //Fade out	// 2010.08.17 A
		BOOL CutVolume(long x,unsigned char y);//Cut volume notes
		BOOL EnlargeAllNotes(int Power);//[new]Musical notePowerStretch to double
		BOOL ShortenAllNotes(int Power);//[new]Musical note1/PowerShrink to double

		void SortNotes(void);	//Organizing notes
		int SearchNote(long x,unsigned char y, int YuusenTrack);//Which note is on a track? Search
		int SearchNoteB(long x,unsigned char y, long *lStartx, long *lLastx, int YuusenTrack);//Which note is on a track? Search 2014.05.02
		int SearchNoteC(long x,unsigned char y, long xWidth, long xMod);//Search in reduced display.
		

		void GetNoteUsed(long *use,long *left,char track);//Musical score remaining
		//Displayed below
		void PutNotes(int TPCY=0);
		void PutNotes2(int TPCY=0);
		void PutMusic(void);
		void PutNumber(void);
		void PutRepeat(void);
		bool PutBackGround(void);	//Redraw background
		void PutSelectArea(void);
		//The following
		void PlayData(void);
		void SetPlayPointer(long x);//Set playback pointer to specified position
		//Below is the file relation
		unsigned short GetNoteNumber(char track,NOTECOPY *nc);
		BOOL SaveMusicData(void);
		BOOL LoadMusicData(void);
		int FileCheckBeforeLoad(char *checkfile); //If the file is loadable0If it is abnormal1return it. Data loading is not done. 2014.05.22
		//The following are editorial relations
		BOOL DelateNoteData(PARCHANGE *pc);
		BOOL CopyNoteData(NOTECOPY *nc);
		BOOL ChangePanData(PARCHANGE *pc);
		BOOL ChangeTransData(PARCHANGE *pc);
		BOOL ChangeVolumeData(PARCHANGE *pc);
		BOOL CheckNoteTail(char track);

		BOOL EnsureEmptyArea(PARCHANGE *pc, int Function); //[new]Fill in the blank 2014.05.01
		BOOL SwapTrack(NOTECOPY *pc); //[new]Swap tracks 2014.05.10
		BOOL ExportMIDIData(char *strMidiFileName, int iRepeat); //MIDIExport,Repeat count 2014.05.11



	bool CopyNoteDataToCB(NOTECOPY *nc, int iTrack, int iFullTrack);
}ORGDATA;
extern ORGDATA org_data;
