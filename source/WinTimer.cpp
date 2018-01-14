//Module   : WinTimer.cpp
//include  : mmsystem.h
//Import   : WinMM.lib
//
//Contents : &quot;How to use the multimedia timer&quot;
//

#include <windows.h>  //Win32n API's
#include <windowsx.h> //mmsystem.h
#include "DefOrg.h"
#include "OrgData.h"

//Error check macro
//#define MMInspect(ret)  if((ret) != TIMERR_NOERROR) return FALSE;

/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
//prototype declaration
/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/

BOOL InitMMTimer();
BOOL StartTimer(DWORD dwTimer);
VOID CALLBACK TimerProc(UINT uTID,UINT uMsg,DWORD dwUser,DWORD dwParam1,DWORD dwParam2);
BOOL QuitMMTimer();

/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
//Global variables
/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
static UINT ExactTime   = 13;//Minimum precision
static UINT TimerID     = 0;

/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
//Set the timer accuracy.
//This function is called once at application initialization.
/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
BOOL InitMMTimer()
{
	TIMECAPS tc;
	MMRESULT ret;

	//Acquire timer accuracy information
	ret = timeGetDevCaps(&tc,sizeof(TIMECAPS));
	if(ret != TIMERR_NOERROR) return FALSE;
	if(ExactTime < tc.wPeriodMin)ExactTime = tc.wPeriodMin;
	//Initialize with this accuracy
	ret = timeBeginPeriod(ExactTime);
	if(ret != TIMERR_NOERROR) return FALSE;
	return TRUE;
}

/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
//Start the timer.
//dwTimer   Timer interval to set
/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
BOOL StartTimer(DWORD dwTimer)
{
	MMRESULT ret = 0;
	ExactTime = dwTimer;
	//Generate a timer
	TimerID = timeSetEvent
	(
		dwTimer,       //Timer time
		10,             //Acceptable timer accuracy
		(LPTIMECALLBACK)TimerProc, //Callback procedure
        0,          //The user callback functiondwUserInformation value to send to
		TIME_PERIODIC //Generate an event at every timer time
	);
	if(ret != TIMERR_NOERROR) return FALSE;
	return TRUE;
}
/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
//Timer&#39;s callback function
/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
VOID CALLBACK TimerProc(UINT uTID,UINT uMsg,DWORD dwUser,DWORD dwParam1,DWORD dwParam2)
{
	DWORD dwNowTime;
	dwNowTime = timeGetTime();
	//===================================================================================
	//Write a user-defined source here.
	//Basically, you only need to call the function and you should do the other function.
	//===================================================================================
	org_data.PlayData();
}

/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
//Release the timer resource.
//Call it once at the end of the application.
/*■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■*/
BOOL QuitMMTimer()
{
	MMRESULT ret;

	if(TimerID != TIMERR_NOERROR)
	{
		//Terminate the timer if it is in use
		ret = timeKillEvent(TimerID);
		if((ret) != TIMERR_NOERROR) return FALSE;
	}
	//Free timer resources
	ret = timeEndPeriod(ExactTime);
	if((ret) != TIMERR_NOERROR) return FALSE;
	return TRUE;
}
