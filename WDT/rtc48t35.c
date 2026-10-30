/*
 * ==============================================================
 * System TARGET : ACE Control SDP-2000 Ver 1.0   
 * Target CPU    : MPC8248, VME6U
 * Main Factors  : 스마트급전용 SDP ESIO  
 *     - MPU 이중화 구성
 *     - ESOP/SIO 연계 : 전력감시, 원격진단, 전력품질, 고장점 등
 *     - 통합 시뮬레이터 : 2013/07/08 V8.2 적용 
 * --------------------------------------------------------------
 * System DESIGN : SANE-SYSTEM   .... by  Lee Ho-Sang
 * Initial-DATA  : 2016,03,25
 * Last Updated  : 2016,03,25
 * ==============================================================
 */
 
#include	"localLib.h"
#include    "external.h"

extern  MPC860IO_DESC    ledPort;
extern  MPC860IO_DESC    rtcPort;

int rtc_get_time(struct tm *tmval);
int rtc_set_time(struct tm *tmval, int century);
//int display_tm(struct tm *tmval);

static unsigned long epoch = 1900;      /* year corresponding to 0x00   */


unsigned char bcd_to_char(unsigned char bcd)
{

	if ( bcd <= 0x09) return bcd ;
	else if (  (bcd >=0x10) & (bcd <=0x19)) return (bcd - 6) ;
	else if (  (bcd >=0x20) & (bcd <=0x29)) return (bcd - 12) ;
	else if (  (bcd >=0x30) & (bcd <=0x39)) return (bcd - 18) ;
	else if (  (bcd >=0x40) & (bcd <=0x49)) return (bcd - 24) ;
	else if (  (bcd >=0x50) & (bcd <=0x59)) return (bcd - 30) ;
	
	return  100;
}


unsigned char char_to_bcd (unsigned char data)
{
	if( data <= 9)  return data;
	else if( (data >=10) & (data <=19) )  return (data+6); 
	else if( (data >=20) & (data <=29) )  return (data+0x0c); 
	else if( (data >=30) & (data <=39) )  return (data+0x12); 
	else if( (data >=40) & (data <=49) )  return (data+0x18); 
	else if( (data >=50) & (data <=59) )  return (data+0x1E); 
	else if( (data >=60) & (data <=69) )  return (data+0x24); 
	else if( (data >=70) & (data <=79) )  return (data+0x2A); 
	else if( (data >=80) & (data <=89) )  return (data+0x30); 
	
	return 100;
}


/*
*/
void display_tm(struct tm *tmval)
{
    //if(opr->msgDebug)
	Debug(console, "disp : %04d/%02d/%02d/ week=%d %02d:%02d %02d\n",
		tmval->tm_year+epoch,tmval->tm_mon+1, tmval->tm_mday,tmval->tm_wday,
   		tmval->tm_hour, tmval->tm_min, tmval->tm_sec ) ;
}

/*
*/
void	readClock()
{
    time_t  tm;
    struct  tm  *localtm;

#if 1
    time(&tm);
    localtm = localtime(&tm);

	rtc->year 	= localtm->tm_year + 1900;
	rtc->month 	= localtm->tm_mon + 1;
	rtc->day 	= localtm->tm_mday;
	rtc->week 	= localtm->tm_wday;
	rtc->hour 	= localtm->tm_hour;
	rtc->min 	= localtm->tm_min;
	rtc->sec 	= localtm->tm_sec;
	
	//printf("host Time Read : %4d/%2d/%2d-%d-%02d:%02d:%02d\n",
    //    rtc->year+2000, rtc->month, rtc->day, rtc->week, rtc->hour, rtc->min, rtc->sec);
                
	//rtc.week 	= 1;
#endif

}

/*
*/
int	writeClock( int year, int month, int day, int hour, int min, int sec, int week)
{
    int     ioid;
    time_t	systime;
	struct	tm tmval ;  
    struct	timeval tv;  
	struct	timezone tz; 

	/* ------------------------------------ */
	/* [1]. SYSTEM Clock Setting ....		*/
	/* ------------------------------------ */
	year = year % 100;
	tmval.tm_sec  = sec;
	tmval.tm_min  = min;
	tmval.tm_hour = hour;
	tmval.tm_mday = day;
	tmval.tm_mon  = month - 1;
	tmval.tm_year = year + 100;
	tmval.tm_wday = week;
	tmval.tm_yday = 0;
	tmval.tm_isdst= 0;

    if(opr->wdtDebug)
	Debug(console,"[RTC-SET] %02d:%02d:%02d-%02d-%02d:%02d:%02d\n",
		tmval.tm_year+1900,tmval.tm_mon+1,tmval.tm_mday,tmval.tm_wday,
		tmval.tm_hour,tmval.tm_min,tmval.tm_sec);
		

	//rtc_set_time((struct tm *)&tmval, century);
	
	systime = mktime(&tmval);
    Debug(console,"WDT> RTC time %s\n",ctime( &systime)) ;	

    
	tv.tv_sec = systime;
	tv.tv_usec = 0;
	 
	tz.tz_minuteswest = 0;
	tz.tz_dsttime = 0;

#if 0  // 2026-08-27 오후 5:24:49 
	if (settimeofday (&tv, &tz) != 0) 
#else 
	if (settimeofday (&tv,NULL) != 0) 
#endif 	
	{
//	    if(opr->wdtDebug) 2026-08-27 오후 5:25:34 
		Debug(console, "clk> unable to set time -- probably you are not root\n");
		return (0);
	}

    readClock();
    
    /* ---------------------------------------- */
    /* 계전기 Time Sync ...                     */
    /* ---------------------------------------- */
    for(ioid =0; ioid < MAX_DEVICE; ioid++)
    {
        deviceCFG[ioid]->timeSyncReq = SET;
        deviceCFG[ioid]->statusDump  = SET;
    }
    
    return (1);
}




