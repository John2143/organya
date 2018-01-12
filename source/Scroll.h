
#define DIRECTION_UP	0
#define DIRECTION_DOWN	1
#define DIRECTION_LEFT	2
#define DIRECTION_RIGHT	3

typedef struct ScrollData{
	private:
		long hpos;//Horizontal scroll value
		long vpos;//Vertical scroll value
		SCROLLINFO scr_info;
		long vScrollMax;
	public:
		void SetIniFile();
		BOOL InitScroll(void);//Initialization
		void VertScrollProc(WPARAM wParam);//Processing when scrolling
		void HorzScrollProc(WPARAM wParam);
		void WheelScrollProc(LPARAM lParam, WPARAM wParam); //Wheel...
		void GetScrollPosition(long *hp,long *vp);
		void SetHorzScroll(long x);
		void AttachScroll(void);	//Fixed scrolling according to window size change
		void KeyScroll(int iDirection); //For scrolling by key operation
		void PrintHorzPosition(void);
		void ChangeVerticalRange(int WindowHeight = -1); //Scroll bar according to window sizeRangechange
}SCROLLDATA;
extern SCROLLDATA scr_data;//Scroll data
