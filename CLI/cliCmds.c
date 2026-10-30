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
 
//
// 헤더 파일
//
#include    "cli.h"
#include    "external.h"
//#include    "eventmsg.h"

//
// External Variables / Functions
//
extern  int     CheckEOT(void);
extern  int     GetNumber(int low, int high, int *value, char *message);
extern  int     GetToken(void);
extern  void    PrnLine(char mode);

extern  int     dispDevStatus(int devid, int point);
extern  int     dispDevice(int devid);
extern  int     dispDevAnalog(int devid, int point);



extern int	termExec;
extern char	token[TOKEN_LENGTH];
extern char	command[CMD_BUF_LEN];
extern char	*cmdPtr;
extern SHM_MEMORY	*shmPtr;

//
// Function Type Definition
//
void	CmdClear();
void	CmdHelp(), CmdProcess(), CmdQuit(), CmdExit();
void    CmdSETRTC();

void    CmdOffDebug(void);
void    CmdDispVERSION(void);

void    CmdEvent(void);
void    CmdMPUDebug(void);
#if 1	// 2016.07.25 ChoiBC Modify for 61850, ntp Debug(CLI)
void    Cmd61850Debug(void);
void    CmdNtpDebug(void);
void    CmdMMSDebug(void);
#endif
void    CmdDispRelayCfg(void);
void    CmdDispMPUCfg(void);
void    CmdDispESIOCfg(void);

void    CmdDevDebug(void);
void    CmdMessageDebug(void);
void    CmdModbusDebug(void);
void    CmdSCANDebug(void);

void    CmdDispMODBUSCfg(void);
void    CmdDispDevice(void);
void    CmdSOEDebug(void);
void    CmdDispPOINTConfig(void);
void    CmdDispPOINTConfig_61850(void);
void    CmdWDTDebug(void);

void    CmdSETAnalog(void);
void    CmdSETStatus(void);

void    CmdSIMDebug(void);
void    CmdTRIPControl(void);
void    CmdCLOSEControl(void);

void    CmdONLINETest(void);


void    CmdDispTask(void);

CMD_ENTRY   cmdTable[MAX_COMMAND] = {
    {"    ",        CmdHelp,	        "   ",	"----------------------------"},
	{"help",        CmdHelp,	        "help  <ENTER>",	    "Command-List Display "},
	{"process",	    CmdProcess,	        "process <ENTER>",	    "PROCESSOR Status Display"},
	{"clear",	    CmdClear,	        "clear <ENTER>",	    "Clear EVENT-Queue."},
	{"setrtc",	    CmdSETRTC,          "setrtc <ENTER>",       "SET CU-Time Sync"},	
	{"event",	    CmdEvent,	        "event <ENTER>",        "CU History-Queue Display"},	
	{"off",	        CmdOffDebug,	    "OFF <ENTER>",          "Monitoring Debug Flag - OFF."},	
// 2026-08-25 오후 4:22:31 temparary
/*	{"version",	    CmdDispVERSION,	    "version <ENTER>",      "DISPLAY CU-Version"},	*/
	{"quit",	    CmdQuit,	        "quit <ENTER>",	        "CONSOLE-TASK quit"},
	{"exit",	    CmdExit,	        "exit <ENTER>",         "***> VME-CU Exit<***"},
    {"    ",        CmdHelp,	        "   ",	"----------------------------"},
    {"mpu",	        CmdMPUDebug,        "mpu <ENTER>",          "MPU Comm Debug"},
    {"sim",	        CmdSIMDebug,        "sim <ENTER>",          "SIM Comm Debug"},
#if 1	// 2016.07.25 ChoiBC Modify for 61850, ntp Debug(CLI)
    {"mms",	        CmdMMSDebug,	    "mms  <ENTER>",	        "ICCP-MMS Communication Debug ON"}, 
    {"61850",	    Cmd61850Debug,      "61850 <ENTER>",        "61850 Client Debug"},
    {"ntp",	        CmdNtpDebug,        "ntp <ENTER>",          "NTP Client Debug"},
#endif
    {"dev",	        CmdDevDebug,	    "dev # <ENTER>",	    "Device Communication Debug ON"},  
	{"msg",	        CmdMessageDebug,	"msg <ENTER>",	        "Device Message Debug ON"},  
	{"mod",	        CmdModbusDebug,	    "mod <ENTER>",	        "MODBUS Check Point Debug ON"},  
	{"soe",	        CmdSOEDebug,	    "soe <ENTER>",	        "[SOE] Monitoring Debug ON"}, 
	{"wdt",	        CmdWDTDebug,        "wdt <ENTER>",          "WDT Message Debug"},
	{"scan",	    CmdSCANDebug,	    "scan # <ENTER>",	    "SCAN Communication Debug ON"},  
	{"device",	    CmdDispDevice,	    "device # <ENTER>",     "DEVICE Report-Info Display"},	  
    {"    ",        CmdHelp,	        "   ",	"----------------------------"},
    {"mpucfg",	    CmdDispMPUCfg,      "mpucfg <ENTER>",	    "MPU Configuration Display"}, 
    {"esiocfg",	    CmdDispESIOCfg,     "esiocfg <ENTER>",	    "ESIO Configuration Display"}, 
    {"relay",	    CmdDispRelayCfg,    "relay <ENTER>",	    "DEVICE Configuration Display"},
    {"modcfg",	    CmdDispMODBUSCfg,   "modbus <ENTER>",	    "DEVICE MODBUS-Config Display"}, 
    {"pointcfg",    CmdDispPOINTConfig,	"pointcfg <ENTER>",     "POINT Configuration Display"},
    {"point61850",  CmdDispPOINTConfig_61850,	"point61850 <ENTER>",     "POINT Configuration Display"},
    {"task",        CmdDispTask,        "task <ENTER>",  "Display Task info" } ,
    {"    ",        CmdHelp,	        "   ",	"----------------------------"},
    {"analog",	    CmdSETAnalog,	    "analog <ENTER>",       "[ANALOG] Test Data Input"},	
    {"status",	    CmdSETStatus,	    "status <ENTER>",       "[STATUS] Test Data Input"},
    {"trip",	    CmdTRIPControl,	    "trip  <ENTER>",        "[TRIP]   Control TEST."},	
    {"close",	    CmdCLOSEControl,	"close <ENTER>",        "[CLOSE]  Control TEST."},	
    {"testwdt",	    CmdONLINETest,	    "testwdt <ENTER>",        "[CLOSE]  Control TEST."},	
    {"    ",        CmdHelp,	        "   ",	"----------------------------"},
    		
};


/*
 * get decimal word
 */
int getDecWord()
{
    int ch;
    int ret_val = 0;

    while((ch = getchar()) != 0x0a)
    {
        //printf("%c", ch);
        if(isdigit(ch))
            ret_val = ret_val * 10 + (ch - '0');
    }
    //printf("\n\r");
    return(ret_val);
}

void CmdSETRTC(void)
{

    int  year, month, day, week, hour, min, sec;

    printf("[*] Clock Setting\n");
    printf("year [2000-2099] = "); year  = getDecWord();
    printf("month[1-12] 	 = "); month = getDecWord();
    printf("day  [1-31] 	 = "); day   = getDecWord();
    printf("hour [0-23] 	 = "); hour  = getDecWord();
    printf("min  [0-59] 	 = "); min   = getDecWord();
    printf("sec  [0-59] 	 = "); sec   = getDecWord();
    printf("week [0 -6] 	 = "); week  = getDecWord();
    printf("correct or not (y/n) ? ");
    if(getchar() == 'y')
    {
        opr->year   = year;
        opr->month  = month;
        opr->day    = day;
        opr->week   = week;
        opr->hour   = hour;
        opr->min    = min;
        opr->sec    = sec;
        
        opr->rtcUpdateFlag = SET;
        
        printf(">> CU Time SET : %04d/%02d/%02d-%d-%02d:%02d:%02d\n", year, month, day, week, hour, min, sec);	        
        printf("*clock modified\n");
        
        /* -------------------------------- */
		/* LOG File 저장                    */
    	/* -------------------------------- */
    	logEvent_MPU(shmPtr, ENT_LOCAL_TIME, 0, 0, 0, 0, NULL);     // MASTER-MODE Event...
        
    }
    else
    {
        printf("*clock cancel\n");
    }

}
// hkkim 2026-04-24 오후 2:35:16
void CmdDispTask(void)
{
	int		idx;
	int		retVal;
	
	TASK_INFO	*taskPtr;
	//PROCESS_DESC	*dscPtr;
	char    buffer[256];

	taskPtr = (TASK_INFO *) shmPtr->taskInfo;
	printf("No.  NAME    PID   WDTCNT\r\n");
	for (idx = 0; idx < MAX_PROCESS; idx++, taskPtr++)    
	{
	    printf("%02d  %-10s  %04d  %d\r\n",idx,  taskPtr->name, taskPtr->pid,  idx, taskPtr->wdtCount) ; 	    
	}
    
    printf("ESIO scanWDT Values\r\n");
    for ( idx=0 ; idx < MAX_SCAN_PORT; idx++)
    {
        printf("%03d ", opr->scanWDT[idx]);
        if ( idx  ==  8)   printf("---");
    }
    printf("\r\n");
    printf("\r\n");
}

/* 
*   TRIP-제어시험
*/
void CmdONLINETest(void)
{
    int     devid, online;
    
	if (CheckEOT() < 0) return;

	opr->scanRestart = SET;
	
	printf("[*] Test SCAN-RESTART Test.... !\n");
	
#if 0
    printf("[*] Test ONLINE/OFFLINE Test.... !\n");
    printf("1. Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Online/Offline [0:0ffline, 1: online] ... : ");
    online = getDecWord();    
    
    if((devid < 1) || (devid > MAX_DEVICE))
    {
        printf("*** Invalid Control... :  %2d \n", devid);
        return;
    }

    opr->testDevid = devid - 1;
    opr->testPoint = online;

    opr->testOnlineFLAG = SET;

#endif
    
}

/* 
*   TRIP-제어시험
*/
void CmdTRIPControl(void)
{
    int     i, j;
    int     devid, point;
    char    buffer[256];        
    //int     index;
    //word    chksum;
    
    DEVICE_61850    *dev61850;
    SDP_DEVICE      *dev;
    SCAN_CONFIG     *scan;
    POINT_BUF       *ptBuf;
    
	if (CheckEOT() < 0) return;

    printf("[*] Test TRIP Control.... !\n");
    printf("1. Control Device# [1..%d] : ", MAX_DEVICE);
    devid = (getDecWord() - 1 )& 0x3f;
    printf("2. Control Point # [1..%d] : ", MAX_DEV_DO_POINT);
    point = getDecWord() - 1;    
    
    if((devid < 0) || (devid >= MAX_DEVICE) || (point < 0) || (point >= MAX_DEV_DO_POINT))
    {
        printf("*** Invalid Control... :  %2d %2d\n", devid + 1, point+1);
        return;
    }
    
    printf("==> Control Device-%02d, Point-%02d ... TRIP Control...[%4d/%2d/%2d %02d:%02d:%02d]\n",  devid+1, point+1,
       	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
       	
    /* ------------------------------------------------ */
    /*  61850 계전기의 경우...                          */
    /* ------------------------------------------------ */
    if(opr->esioID == SDP_ESIO_61850)
    {
        dev61850 = (DEVICE_61850 *) device61850[devid];
        
        dev61850->cntPoint   = point + 1;       // Control Point, 1,2
        dev61850->cntTcf     = 0x01;            // TRIP Control
        dev61850->controlReq = SET;             // Control Point, 1,2

        logEvent_MPU(shmPtr, ENT_USER_CNTR, devid+1, point+1, TRIP_CONTROL, 0, NULL);
                
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "<-- USER> 61850 Control INFO : Dev=%d, PT=%d, TCF[TRIP=1, CLOSE=2]= %2x... ", devid, point, TRIP_INFO);
        LogFile_MPU (shmPtr, ENT_USER_CNTR, buffer, strlen(buffer));    
                        
        printf("==> 61850 Control Device-%02d, Point-%02d... TRIP(%02x) Control...%d [%4d/%2d/%2d %02d:%02d:%02d]\n",  devid + 1, 
            dev61850->cntPoint, dev61850->cntTcf, dev61850->controlReq, ((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
        return;
    }

    /* ---------------------------------------- */
    /* 일반계전기 SCAN# 제어정보 Setting ...    */
    /* ---------------------------------------- */
    for(i=0; i< MAX_SCAN_PORT; i++)
    {
        scan = (SCAN_CONFIG *) scanCFG[i];
        
        if(scan->useFlag == 0)  continue;
        if(scan->targetID != opr->esioID)  continue;  // SDP SCAN :  0:사용않함, 1: ESIO1, 2:ESIO2, 3:ESIO3, 4:ESIO4, 5:SIO
        if(scan->devNum <= 0)   continue;
        
        for(j=0; j< scan->devNum; j++)
        {
            /* 제어대상 모듈을 search... */
            if(scan->device[j] == devid)
            {
                dev = (SDP_DEVICE *) deviceCFG[devid];
                ptBuf = (POINT_BUF *) &dev->doPtBuf[point];
                
                dev->cntPoint = point;
                dev->cntrType = ptBuf->ptConfig;
                
                dev->cntTCF   = TRIP_CONTROL;               /* TRIP */        
                dev->cntrType = ptBuf->ptConfig;            // 제어속성 : TC(0), Pulse(1), Latch(2) 제어
                dev->selectReq  = SET;
                
                if(ptBuf->dbmax <= 0)   opr->rcvONtime = 500;   
                else                    opr->rcvONtime = ptBuf->dbmax;
                    
                opr->cntrDevId[i]   = devid;
                opr->cntrComTick[i] = 0;
                opr->cntrFlag[i]    = SET;

                logEvent_MPU(shmPtr, ENT_USER_CNTR, devid+1, point+1, TRIP_CONTROL, 0, NULL);
                
                /* -------------------------------- */
                /* LOG File 저장                    */
                /* -------------------------------- */
                sprintf(buffer, "<-- USER> Control INFO : Dev=%d, PT=%d, TCF[TRIP=1, CLOSE=2]= %2x...%s ", devid, point, TRIP_INFO, ptBuf->ptNameStr);
                LogFile_MPU (shmPtr, ENT_USER_CNTR, buffer, strlen(buffer));    
    
                printf(">> Control-OPR SET : dev=%d, point=%d, TCF=%2x...TRIP Control! ... %s \n", devid+1, dev->cntPoint+1, dev->cntTCF, ptBuf->ptNameStr);
                break;
            }                            
        }
    }
        
}

/* 
*   CLOSE-제어시험
*/
void CmdCLOSEControl(void)
{
    int     i, j;
    int     devid, point;
    char    buffer[256];
    //int     index;
    //word    chksum;
    
    DEVICE_61850    *dev61850;
    SDP_DEVICE      *dev;
    SCAN_CONFIG     *scan;
    POINT_BUF       *ptBuf;
    
	if (CheckEOT() < 0) return;

    printf("[*] Test CLOSE Control.... !\n");
    printf("1. Control Device# [1..%d] : ", MAX_DEVICE);
    devid = (getDecWord() - 1 )& 0x3f;
    printf("2. Control Point # [1..%d] : ", MAX_DEV_DO_POINT);
    point = getDecWord() - 1;    
    
    if((devid < 0) || (devid >= MAX_DEVICE) || (point < 0) || (point >= MAX_DEV_DO_POINT))
    {
        printf("*** Invalid Control... :  %2d %2d\n", devid + 1, point+1);
        return;
    }
    
    printf("==> Control Device-%02d, Point-%02d ... CLOSE Control...[%4d/%2d/%2d %02d:%02d:%02d]\n",  devid+1, point+1,
       	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);

    /* ------------------------------------------------ */
    /*  61850 계전기의 경우...                          */
    /* ------------------------------------------------ */
    if(opr->esioID == SDP_ESIO_61850)
    {
        dev61850 = (DEVICE_61850 *) device61850[devid];
        
        dev61850->cntPoint   = point + 1;       // Control Point, 1,2
        dev61850->cntTcf     = 0x02;            // CLOSE Control
        dev61850->controlReq = SET;             // Control Point, 1,2
        
        logEvent_MPU(shmPtr, ENT_USER_CNTR, devid+1, point+1, CLOSE_CONTROL, 0, NULL);
                
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "<-- USER> 61850 Control INFO : Dev=%d, PT=%d, TCF[TRIP=1, CLOSE=2]= %2x... ", devid, point, CLOSE_INFO);
        LogFile_MPU (shmPtr, ENT_USER_CNTR, buffer, strlen(buffer));    
        
        printf("==> 61850 Control Device-%02d, Point-%02d... CLOSE(%02x) Control...%d [%4d/%2d/%2d %02d:%02d:%02d]\n",  devid, 
            dev61850->cntPoint, dev61850->cntTcf, dev61850->controlReq, ((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
       	
        return;
    }
        
    /* ---------------------------------------- */
    /* 일반계전기 SCAN# 제어정보 Setting ...    */
    /* ---------------------------------------- */
    for(i=0; i< MAX_SCAN_PORT; i++)
    {
        scan = (SCAN_CONFIG *) scanCFG[i];
        
        if(scan->useFlag == 0)  continue;
        if(scan->targetID != opr->esioID)  continue;  // SDP SCAN :  0:사용않함, 1: ESIO1, 2:ESIO2, 3:ESIO3, 4:ESIO4, 5:SIO
        if(scan->devNum <= 0)   continue;
        
        for(j=0; j< scan->devNum; j++)
        {
            /* 제어대상 모듈을 search... */
            if(scan->device[j] == devid)
            {
                dev = (SDP_DEVICE *) deviceCFG[devid];
                ptBuf = (POINT_BUF *) &dev->doPtBuf[point];
                
                dev->cntPoint = point;
                dev->cntrType = ptBuf->ptConfig;
                
                dev->cntTCF   = CLOSE_CONTROL;              // CLOSE
                dev->cntrType = ptBuf->ptConfig;            // 제어속성 : TC(0), Pulse(1), Latch(2) 제어
                dev->selectReq  = SET;
                
                if(ptBuf->dbmax <= 0)   opr->rcvONtime = 500;   
                else                    opr->rcvONtime = ptBuf->dbmax;
                    
                opr->cntrDevId[i]   = devid;
                opr->cntrComTick[i] = 0;
                opr->cntrFlag[i]    = SET;

                logEvent_MPU(shmPtr, ENT_USER_CNTR, devid+1, point+1, CLOSE_CONTROL, 0, NULL);
                
                /* -------------------------------- */
                /* LOG File 저장                    */
                /* -------------------------------- */
                sprintf(buffer, "<-- USER> Control INFO : Dev=%d, PT=%d, TCF[TRIP=1, CLOSE=2]= %2x...%s ", devid, point, CLOSE_INFO, ptBuf->ptNameStr);
                LogFile_MPU (shmPtr, ENT_USER_CNTR, buffer, strlen(buffer));    
    
                printf(">> Control-OPR SET : dev=%d, point=%d, TCF=%2x...CLOSE Control! ... %s \n", devid+1, dev->cntPoint+1, dev->cntTCF, ptBuf->ptNameStr);
                break;
            }                            
        }
    }
        
}


void CmdSETAnalog(void)
{
    int     devid, point;
    float   adata;
    
	if (CheckEOT() < 0) return;

    printf("[*] Test ANALOG Point Update.... !\n");
    printf("1. Input Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Input Point # [1..80] : ");
    point = getDecWord();    
    printf("3. Input Analog# [0...2048] : ");
    adata = getDecWord() * 1.0;
    
    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DI_POINT))
    {
        //printf("*** Invalid Status Information ... :  %2d %2d\n", devid, point);
        return;
    }

    printf("\n----------------------------------------------------->>\n");
    printf("==> Analog Info Device-%02d, Point-%02d ... Data = %4.2f !\n", devid, point, adata);
    printf("----------------------------------------------------->>\n");
    
    opr->testDevid = devid - 1;
    opr->testPoint = point - 1;
    opr->testAIData= adata;

    opr->testAnalogFLAG = SET;
    
}

void CmdSETStatus(void)
{
    //int     i,j;
    int     devid, point, state;
    
	if (CheckEOT() < 0) return;

    printf("[*] Test STATUS Point Update.... !\n");
    printf("1. Input Device# [1..%d] : ", MAX_DEVICE);
    devid = getDecWord() & 0x3f;
    printf("2. Input Point # [1..80] : ");
    point = getDecWord();    
    printf("3. Input Status# [0...1] : ");
    state = getDecWord() & 0x01;    
    
    if((devid < 1) || (devid > MAX_DEVICE) || (point < 1) || (point > MAX_DEV_DI_POINT))
    {
        //printf("*** Invalid Status Information ... :  %2d %2d\n", devid, point);
        return;
    }

    printf("\n----------------------------------------------------->>\n");
    printf("==> Status Info Device-%02d, Point-%02d ... state = %d !\n", devid, point, state);
    printf("----------------------------------------------------->>\n");
    
    opr->testDevid = devid - 1;
    opr->testPoint = point - 1;
    opr->testState = state;

    opr->testStatusFLAG = SET;
                        
}


//
// 모듈:	CmdClear()
//
void CmdClear(void)
{
    //word    chksum;
	if (CheckEOT() < 0) return;

#if 0	
    /* -------------------------------------------- */
    /* 내부 History & Event Queue 내용 Clear...     */
    /* -------------------------------------------- */
    printf("[*] EVENT Queue Cleared.... !\n");
    bzero8248(sque, sizeof(SYSTEM_QUE));
    chksum = gensum((byte *) sque, sizeof(SYSTEM_QUE) - 2);

    sque->chksum = chksum;
    sque->clear  = 255;  
    
    pause(100);
    
    printf("[*] HISTORY Queue Cleared.... !\n");
    bzero8248(hque, sizeof(HISTORY_QUE));
    chksum = gensum((byte *) hque, sizeof(HISTORY_QUE) - 2);

    hque->chksum = chksum;
    hque->clear  = 255;  
#endif
    
}


//
// 모듈:	CmdWDTDebug()
//
void CmdWDTDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] WDT Message Debug ON ...!\n");
    opr->wdtDebug = 1;
    
}


/*
*
*/
void CmdSOEDebug(void)
{
	if (CheckEOT() < 0) return;

    opr->soeDebug = SET;
    printf("[*] SOE Queue ... Message Debug ON...!\n");    
}

void CmdSCANDebug(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(0, MAX_SCAN_PORT, &vmsid, "SCAN Port Comm Debug...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    opr->scanDebug = vmsid - 1;
    
    printf("[*] SCAN Port (%2d) Comm Debug... ...!\n", vmsid);
    
}

//
// 모듈:	CmdDevDebug()
//
void CmdDevDebug(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(0, MAX_DEVICE, &vmsid, "DNP Device Comm Debug...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    if(vmsid == 0)  opr->dnpDebug = MAX_DEVICE;
    else            opr->dnpDebug = vmsid - 1;
    
    printf("[*] DNP Device (%2d) Comm Debug... ...!\n", vmsid);
    
}

//
// 모듈:	CmdMessageDebug()
//
void CmdMessageDebug(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_DEVICE, &vmsid, "DNP Device Message Debug...?  ") < 0)	return;
	if (CheckEOT() < 0) return;
	
    opr->msgDebug = vmsid - 1;
    printf("[*] DNP Device (%2d) Message Debug... ...!\n", vmsid);
    
}

//
// 모듈:	CmdMessageDebug()
//
void CmdModbusDebug(void)
{
	int		vmsid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_DEVICE, &vmsid, "MODBUS Check Point Debug ON..?  ") < 0)	return;
	if (CheckEOT() < 0) return;
	
    opr->modDebug = vmsid - 1;
    printf("[*] MODBUS Device (%2d) Check Point Debug... ...!\n", vmsid);
    
}

//
// 모듈:	CmdSimDebug()
//
void CmdMPUDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] MPU Comm Debug ON ...!\n");
    opr->mpuDebug = 1;
}

//
// 모듈:	CmdSimDebug()
//
void CmdSIMDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] SIM Comm Debug ON ...!\n");
    opr->simDebug = 1;
}


#if 1	// 2016.07.25 ChoiBC Modify for 61850, ntp Debug(CLI)
//
// 모듈:	Cmd61850Debug()
//
void Cmd61850Debug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] 61850 Client Debug ON ...!\n");
    opr->c61850Debug = 1;
}

//
// 모듈:	CmdICCPDebug()
//
void CmdMMSDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] 61850-MMS Comm Debug ON ...!\n");
    opr->mmsDebug = 1;
    
}

//
// 모듈:	CmdNtpDebug()
//
void CmdNtpDebug(void)
{
	if (CheckEOT() < 0) return;
    printf("[*] NTP Client Debug ON ...!\n");
    opr->ntpDebug = 1;
}
#endif

//
// 모듈:	CmdQuit()
//
void CmdOffDebug(void)
{
	if (CheckEOT() < 0) return;

    printf("[*] Debug OFF ...!\n");
    
    /* SOE 생성시험 */
    opr->testStatusFLAG = RESET;
    
    opr->wdtDebug   = 0;
#if 1	// 2016.07.25 ChoiBC Modify for 61850, ntp Debug(CLI)
    opr->c61850Debug = 0;
    opr->ntpDebug    = 0;
#endif

    opr->mpuDebug   = 0; 
    opr->simDebug   = 0;
    opr->soeDebug   = 0; 
    opr->mmsDebug = 0;
    
    opr->lineDebug  = 0xff;
    opr->dnpDebug   = 0xff;
    opr->msgDebug   = 0xff; 
    opr->modDebug   = 0xff; 
    opr->scanDebug  = 0xff;
    
}

//
// 모듈:	CmdQuit()
//
void CmdQuit(void)
{
	if (CheckEOT() < 0)
		return;

	termExec = 0;
	CmdOffDebug();
	
}

//
// 모듈:	CmdExit()
//
void CmdExit(void)
{
    char    buffer[256];
    TASK_INFO	*wdtPtr = NULL;
    
	if (CheckEOT() < 0)
		return;
		
    printf(">> Program Terminate.... Are You Sure...? ");
    if(getchar() == 'y')
    {
        /* 운영자 종료명령 */
        logEvent_MPU(shmPtr, ENT_RESET_USER, 30, 0, 0, 0, NULL);
        
        sprintf(buffer, "%s", "----------------------------------------------------");
        LogFile_MPU(shmPtr, ENT_RESET_USER, buffer, strlen(buffer));
        
        pause(1000);
    
        wdtPtr = (TASK_INFO *) &shmPtr->taskInfo[WDT_PROCESS];
        printf("[*] WDT Process ... pid= %d\n", wdtPtr->pid);
        ExecCommand("kill %d \n", wdtPtr->pid);
    
	    termExec = 0;
    }
    	    
}

//
// 모듈:	CmdHelp()
//
void CmdHelp(void)
{
	int		index, fIndex, length;
	CMD_ENTRY	*cp;

	if (GetToken() > 0)
	{
		cp = cmdTable;
		for (index = 0, fIndex = 0; index < MAX_COMMAND; index++, cp++)
		{
			if (strlen(cp->name) == 0)
				continue;
			length = strlen(cp->name) < strlen(token) ? strlen(cp->name) :
				strlen(token);
			if (strncmp(cp->name, token, length) != 0)
				continue;
			printf("Command %s:\n", cp->name);
			printf("\tDescription: %s\n", cp->help);
			printf("\tUsage: %s\n", cp->usage);
			fIndex++;
		}
		if (fIndex == 0)
			printf("%%%%WRN_Invalid Command %s\n", token);
		return;
    }

	for (index = 0, cp = cmdTable; index < MAX_COMMAND; index++, cp++)
	{
		if (strlen(cp->name) == 0)
		{
			break;
        }			
		printf("%-10s %s\n", cp->name, cp->help);
    }
}


//
// 모듈:	CmdProcess()
//
void CmdProcess_LHS(void)
{
	int		idx;
	TASK_INFO	*ptr;

	// 전체 프로세스의 상태를 표시한다.
	PrnLine('=');
	printf(" Process DEF ACT DBG CHK %-11s RST %-8s Last %-6s %-6s\n",
		"WDT-Count", "PID", "MaxET", "PrevET");
	PrnLine('-');
	for (idx = 0, ptr = shmPtr->taskInfo; idx < MAX_PROCESS; idx++, ptr++)
	{
		if (strlen(ptr->name) == 0)
			continue;
		printf(" %-7s %-3d %-3d %-3d %-3d %4d/%-6d %-3d %-8d %-4d %-6d %-6d\n",
			ptr->name, ptr->define, ptr->active, ptr->debug,
			ptr->check, ptr->wdtCount, ptr->wdtEnable, ptr->restart,
			ptr->pid, ptr->lastWdt, ptr->maxEtime / 1000,
			ptr->prevEtime / 1000);
	}
	PrnLine('=');
                    
}

//
// 모듈:	CmdProcess()
//
void CmdProcess(void)
{
    int     i, ch;
	int		idx;
	TASK_INFO	*ptr;
	SCAN_CONFIG *scan;

    while(1)
    {   
	    // 전체 프로세스의 상태를 표시한다.
    	PrnLine('=');
	    printf(" Process DEF ACT DBG CHK %-11s RST %-8s Last %-6s %-6s\n", 	"WDT-Count", "PID", "MaxET", "PrevET");
    	PrnLine('-');
	    for (idx = 0, ptr = shmPtr->taskInfo; idx < MAX_PROCESS; idx++, ptr++)
    	{
	    	if (strlen(ptr->name) == 0)
		    	continue;
    		printf(" %-7s %-3d %-3d %-3d %-3d %4d/%-6d %-3d %-8d %-4d %-6d %-6d\n",
	    		ptr->name, ptr->define, ptr->active, ptr->debug,
			    ptr->check, ptr->wdtCount, ptr->wdtEnable, ptr->restart,
			    ptr->pid, ptr->lastWdt, ptr->maxEtime / 1000,
			    ptr->prevEtime / 1000);
	    }
	    PrnLine('=');
	
        /* -------------------------------------------- */
	    /* THREAD 감시 : Internal WDT Flag......        */
	    /* -------------------------------------------- */
	    printf("SCAN-WDT: ");
	    for(i=0; i< MAX_SCAN_PORT; i++)
	    {
	        scan = (SCAN_CONFIG *) scanCFG[i];
	        if(scan->useFlag == RESET)	continue;
            printf("(%d)-%d ", i+1, opr->scanWDT[i]);
        }
        printf("\n");
        	
	    ch = getchar();
	    if((ch == 'q') || (ch == 'x'))  break;
	    
    }
                  
}



/*
*   GIPAM2000 - DEVICE 
*/
void CmdDispDevice(void)
{
    char    type;
    int     point;
	int		vmsid;
	int devid;

	// 명령 형식을 점검한다.
	if (GetNumber(1, MAX_DEVICE, &vmsid, "DNP Device Data Display...?  ") < 0)	return;
	if (CheckEOT() < 0) return;

    devid = vmsid - 1;
    
    printf("=>Select DEVICE = %d\n", devid +1);
    printf("=>Select TYPE  [0: ALL, 1: STATUS, 2: ANALOG] = ");
    type   = getDecWord();
    
    printf("=>Select Device (%2d)/Point Number (Max 1024) = ", devid + 1); 
    point = getDecWord() - 1;
    
    if(point < 0)   type = 0;
        
    /* -------------------------------------------- */
    /*  DISPLAY Type에 따른 계전기 데이터 출력 ...  */
    /* -------------------------------------------- */
    if(type == 1)       dispDevStatus(devid, point);
    else if(type == 2)  dispDevAnalog(devid, point);     
    else                dispDevice(devid);       
            
}


//
// 모듈:	CmdEvent()
// SDP 내부 운영 History 정보 참조
//
void CmdEvent(void)
{
    int  j;
    int ch, mode;
    int id, ioid, point, state, hostid;
    word    front;
    SYSLOG_FORM   *log;
    struct tm cTime, sTime;
    int milisec1, milisec2;
    SDP_DEVICE  *dev;
    POINT_BUF   *ptBuf;
        
        
	// 명령 형식을 점검한다.
	if (CheckEOT() < 0)	return;

    printf("\n*** CU-HISTORY Data Display ***\n");
    printf(">> SDP Controller : %s \n", TARGET_NAME);
    printf(">> Manufacture    : SANE System Co.,Ltd\n");
    printf(">> Model Name     : %s\n", VERSION_STRING);
    printf(">> Update Date    : %s\n", DATE_STRING);
    printf(">> Event Number   : %3d ... over =%d\n", hque->front, hque->overlab );
    printf("\n");

    printf("\n--------------------------------------------------------------\n");
    printf("[SELECT Info :  [1] SOE, [2] CONTROL, [3] COMM, [4] SYSTEM ... ");

    /* Select EVENT-CODE */            
    mode = getDecWord();
    if((mode != 1) && (mode != 2) && (mode != 3) && (mode != 4))   mode = 0;
    printf("--------------------------------------------------------------\n");
    
    front = (hque->front - 1) & HISTORY_QUE_MASK;
    front = hque->front & HISTORY_QUE_MASK;
    for(j = 0; ;)
    {
        /* end 지정 */
        front = (front - 1) & HISTORY_QUE_MASK;
        
        if((hque->overlab == 0) && (front == HISTORY_QUE_MASK))    break;
        else if(front == hque->front) break; 
                  
        log = (SYSLOG_FORM *) &hque->queue[front];
        
        id      = log->logid;       // 이벤트 코드 구분 : SOE/Control/System ...
        ioid    = log->ioid;        // 대상 계전기 번호 [1...112]
        point   = log->point;       // 대상 포인트 번호 [1...1024]
        state   = log->state;       // 이벤트/제어 상태
        hostid  = log->hostid;      // 발생주체, 0: MPU, 1 ~ 8 : HOST

        localtime_r (&log->logTime.tv_sec, &cTime);
        milisec1 = log->logTime.tv_usec / 1000;
        
        /* ------------------------------------------------ */
        /*  이벤트 코드에 따른 분류...                      */
        /* ------------------------------------------------ */
        if(mode == 1)
        {
            if(id != ENT_SOE)   continue;    
        }   
        else if(mode == 2)
        {
            if((id < ENT_ICCP_CNTR) || (id > ENT_LINK_CNTR))    continue;    
        }   
        else if(mode == 3)
        {
            if((id < ENT_ICCP_ONLINE) || (id > ENT_VME_OFFLINE))    continue;    
        }   
        else if(mode == 4)
        {
            if(id > ENT_DEVICE_SOE) continue; 
            if((id > ENT_WDT_HOST) && (id < ENT_DEVICE_SOE))    continue;    
        }   
        
        /* ------------------------------------------------ */
        /*  이벤트 코드별 출력형식 분류...                  */
        /* ------------------------------------------------ */  
        if(id == ENT_SOE)
        {   
            /* SOE 생성시각 표시... */
            localtime_r (&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;
            
            dev = (SDP_DEVICE *) deviceCFG[ioid -1];
            ptBuf = (POINT_BUF *) &dev->diPtBuf[point -1];
               
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] SOE:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid,
                sTime.tm_year + 1900, sTime.tm_mon +1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, ptBuf->ptNameStr);
        }
        else if((id == ENT_HOST_CNTR) ||(id == ENT_USER_CNTR))
        {   
            dev = (SDP_DEVICE *) deviceCFG[ioid -1];
            ptBuf = (POINT_BUF *) &dev->doPtBuf[point -1];
               
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid,
                ptBuf->ptNameStr);
        }
        else if((id == ENT_DEV_ONLINE) || (id == ENT_DEV_OFFLINE))
        {
            /* SOE 생성시각 표시... */
            localtime_r (&log->soeTime.tv_sec, &sTime);
            milisec2 = log->soeTime.tv_usec / 1000;
            
            dev = (SDP_DEVICE *) deviceCFG[ioid -1];

            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] EVT:%04d/%02d/%02d-%02d:%02d:%02d-%03d %s \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid,
                sTime.tm_year + 1900, sTime.tm_mon +1, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, milisec2, dev->devNameStr);
        }       
        else
        {
            printf("%3d. %04d/%02d/%02d-%02d:%02d:%02d-%03d [%s : %2d/%2d/%02x/%2d] \n", front + 1,
                cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec, milisec1, errmsg[id], ioid, point, state, hostid);
        }
                      
        if(((++j)%32) == 0) 
        {
            ch = getchar();
            if(ch == 'q')   break;     
        }            
        
        /* end 지정 */
        //front = (front - 1) & HISTORY_QUE_MASK;
        
    }
    
    printf("\n [ EVENT-Q ] end ---------------------------------------------\n");
}
