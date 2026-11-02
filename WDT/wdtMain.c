
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
#include    "61850Shm.h"
#include    "sdp_wdt.h"

int	            termExec;

extern  void    read48T35();
extern  void    readClock();
extern  int	    writeClock( int year, int month, int day, int hour, int min, int sec, int week);

extern  void    mpuParaConfig();

extern  int     readRTUdb();
extern  int     deviceParaConfig();
extern  int     pointParaConfig();
extern  int     esioParaConfig();

extern  int     readESIO_ID( ) ;            // VMEBUS-ID Read
extern  int     readESIO_SWITCH( );         // ESIO 내부 DIP SWITCH
extern  int     get_NTP_Info();             // NTP Time Sync-Client


extern  int led_toggle(void);
extern int initRunLad(void) ;

extern int init_esio_id(void);


extern int hub_pwer_on(void ) ;

/* ------------------------------------------------------------ */
/*  공유메모리 최대 Size = 33,554,432                           */
/* ------------------------------------------------------------ */
SHM_DESC	    shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};

SHM_MEMORY	    *shmPtr = NULL;

TASK_INFO	    *taskPtr= NULL;

RTC             *rtc    = NULL;                 // RTC 운영 Buffer
OPR_MSG         *opr    = NULL;                 // ESIO 운영 Buffer
CONSOLE_INFO	*console= NULL;                 // CONSOLE 운영 Buffer
SOE_QUEUE       *soeQueue = NULL;

MPU_CONFIG      *mpuCFG  = NULL;                 // MPU Config
ESIO_CONFIG     *esioCFG;                       // ESIO 장치 Config

//POINT_BUF       *pointCFG[MAX_DEV_POINT];       // SDP 포인트 Config 정보
SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config

SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
DEVICE_61850    *device61850[MAX_DEVICE];       // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)

TIME_SYNC_INFO	*timeSync = NULL;               // 61850 Time-Server 연동
	 
RTU_DATABASE    *rtudb = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트

HISTORY_QUE     localHQ, *framHque;

//MPC860IO_DESC    ledPort = {0, NULL, 0, "/dev/ledswitch", O_RDWR};
MPC860IO_DESC    ledPort = {0, NULL, 0, "/dev/esio_led", O_RDWR|O_NDELAY};
MPC860IO_DESC    idPort  = {0, NULL, 0, "/dev/i2c-0", O_RDWR|O_NDELAY};



#define WDT_DEVICE_PATH   "/dev/SDP_WDT"

int     no_wdt;

int     ledRunTick;

/* -------------------------------------------- */
/*  SDP-자장치 : 61850 - PROCESS 구조체 정의    */
/* -------------------------------------------- */
PROCESS_DESC	prcTable_61850[MAX_PROCESS] = 
            {
                {"MPUIF",       "", 0, 0},             /* MPU 정보연계 통신 Process */
                {"SIM",         "", 0, 0},             /* Simulator 정보연계 통신 Process */
                {"61850Client", "", 0, 0},             /* 61850Client : 61850 계전기 Process */
                {"ntpClient",   "", 0, 0},             /* ntpClient : NTP 통신 Process */
    	    };

/* -------------------------------------------- */
/*  SDP-자장치 : SCADA/DIAG/EQ - PROCESS 구조체 정의    */
/* -------------------------------------------- */
PROCESS_DESC	prcTable_scada[MAX_PROCESS] = 
            {
                {"MPUIF",   "", 0,0},             /* MPU 정보연계 통신 Process */
                {"SIM",         "", 0, 0},             /* Simulator 정보연계 통신 Process */
                {"SCAN",    "", 0, 0},             /* SCAN : DNP 계전기 정보연계 Process */
                {"",        "", 0, 0},             /* 예비 */
    	    };
    	    
void    *sram_ptr;
void    *mram_ptr;
int     sram_fd;                // SVME-860 Device Driver 

void    *nvram_ptr;
int     nvram_fd;                // SVME-860 Device Driver 

/* ---------------------------------------- */
/*  VMEBUS 관련 변수 초기화                 */
/* ---------------------------------------- */
void    *vmebus_ptr ;
int     vme_fd; 

VME_SIODCB      *vmeSioDCB;
VME_CHAN_DCB    *vmeChanDCB[SIO_CHANNEL_MAX];

VME_SIODCB      tempVmeSioDCB;
VME_CHAN_DCB    tempVmeChanDCB[SIO_CHANNEL_MAX];


int     wdtid;                      

byte	bitcode[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

//
// 모듈:	SigHandler()
//
void SigHandler(int sig)
{
	char    buffer[256];
	
    //if(opr->wdtDebug)
    Debug(console,"wdt> %4d/%2d/%2d-%02d:%02d:%02d ... signal generated (%2d)...!\n", rtc->year, rtc->month,rtc->day,rtc->hour, rtc->min, rtc->sec, sig);
    pause(100);
    
	switch(sig)
	{
    case SIGTERM:
        termExec = 0;
        //if(opr->wdtDebug)
        Debug(console,"wdt> ... signal [SIGTERM] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal [SIGTERM] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));      	
        pause(1000);    
        break;
	        
	case SIGBUS : 
	    termExec = 0;
	    //if(opr->wdtDebug)
        Debug(console,"wdt> ... signal [SIGBUS] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal [SIGBUS] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));      	
        pause(1000);  	    
	    break;
		    
	case SIGSEGV: 
	    termExec = 0;
	    //if(opr->wdtDebug)
        Debug(console,"wdt> ... signal [SIGSEGV] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal [SIGSEGV] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));      	
        pause(1000);  	    
	    break;
		    
	case SIGPIPE: 
	    //if(opr->wdtDebug)
        Debug(console,"wdt> ... signal [SIGPIPE] generated (%2d)...!\n", sig);
        /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal [SIGPIPE] generated (%2d)...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));      	
        pause(1000);   	    
		break;

	default : 
	    //if(opr->wdtDebug)
	    Debug(console,"wdt> ... signal[%2d] generated ...!\n", sig);
	    /* -------------------------------- */
        /* LOG File 저장                    */
        /* -------------------------------- */
        sprintf(buffer, "wdt> ... signal[%2d] generated ...!", sig);
      	LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));      	
        pause(1000);   
		break;
	}
	
}

/*
*
*/
int  init_fram(void)
{
    
    return (NOK);

}

/*
*   VMEBUS : SIO 관련 메모리 초기화...
*/
int  init_vmebus(int sioid)
{
    /* ------------------------------------ */
    /*  VMEBUS MEMORY DEVICE 초기화         */
    /* ------------------------------------ */
    if ((vme_fd = open("/dev/mem", O_RDWR) ) < 0)
    {
        printf("sio> *open VME_AREA : /dev/vme ... Error ! \n");
        return (NOK);
    }        

    /* -------------------------------------------- */
    /*  VME Backup Memory 초기화                    */
    /*  - SIO Module Size : 0x20000                 */
    /* -------------------------------------------- */
    if((vmebus_ptr = ( char  *) mmap(0, VMESIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vme_fd, (VMESIO_START_ADDRESS)) ) < 0)
    //if((vmebus_ptr = ( char  *) mmap(0, VMESIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vme_fd, (VMESIO_START_ADDRESS + (sioid * VMESIO_SIZE))) ) < 0)    
    {            
        printf("sio> *VME Memory Mapping ... Error ! \n");
        return (NOK);            
    }
    
    printf("ESIO%d> VMEBUS Init...OK !\n", sioid);
    
    return (OK);
}    

/*
*/
int	init_variable()
{
    word    chksum;
    int     size1;

    /* -------------------------------- */
    /* 1. 외부 메모리 포인트 초기화 ... */
    /* -------------------------------- */
    if(init_vmebus(opr->boardID) == OK)
    {
        /* SIO보드 초기화...내부 변수 초기화에 따른 변수 */             
	    vmeSioDCB  = (VME_SIODCB *)(  vmebus_ptr);   
	    size1 = sizeof(VME_SIODCB);
	    
	    printf("wdt> VMEBUS INIT-OK... VME SIO(%d)  Address : %p, size=%d\n", opr->boardID, vmeSioDCB, size1);  
	        
	    bzero(vmeSioDCB, sizeof(VME_SIODCB));
        
        vmeSioDCB->sioAccess = 0;
        vmeSioDCB->mpuAccess = 0;
        vmeSioDCB->initReq   = 0;   
        vmeSioDCB->initEnd   = 0;    
        vmeSioDCB->resetFlag = 0;          	    
    }
    else
    {
        printf("wdt> **** VMEBUS INIT-Fail... VME SIO(%d)\n", opr->boardID); 

        /* SIO보드 초기화...내부 변수 초기화에 따른 변수 */             
	    vmeSioDCB  = (VME_SIODCB *) &tempVmeSioDCB;   
	    size1 = sizeof(VME_SIODCB);
	    bzero(vmeSioDCB, sizeof(VME_SIODCB));

        vmeSioDCB->sioAccess = 0;
        vmeSioDCB->mpuAccess = 0;
        vmeSioDCB->initReq   = 0;   
        vmeSioDCB->initEnd   = 0;    
        vmeSioDCB->resetFlag = 0;          	    
    }
    
    /* -------------------------------- */
    /* 1. 외부 메모리 포인트 초기화 ... */
    /* -------------------------------- */
    if(init_fram() == OK)
    {
        framHque    = (HISTORY_QUE *)&localHQ; 
        size1 = sizeof(HISTORY_QUE);
        
	    printf("wdt> FRAM initial...OK... HQueue = %d\n", size1);  
	    
    }
    else
    {
    	framHque    = (HISTORY_QUE *)&localHQ; 
	    printf("wdt> *FRAM initial...Error \n"); 
    }

    /* ------------------------------------------------ */
    /*  2. 운영 History... initial database pointer     */
    /* ------------------------------------------------ */
    chksum = gensum((byte *) framHque, sizeof(HISTORY_QUE) - 2);
         
    /* database check */
    if(framHque->chksum != chksum)
    {
        printf("wdt> *FRAM History fail [rom = %x, cal = %x] ->cleared... \n", framHque->chksum, chksum);
        bzero(framHque, sizeof(HISTORY_QUE));
        
        chksum = gensum((byte *) framHque, sizeof(HISTORY_QUE) - 2);
        framHque->chksum = chksum;

        bzero(hque, sizeof(HISTORY_QUE));
        chksum = gensum((byte *) hque, sizeof(HISTORY_QUE) - 2);

        hque->chksum = chksum;        
    }
    
    
    
    esioCFG->modInit_OK = 0;
    
    return (0);
}


//
// 모듈: SetProcessInfo()
//
void SetProcessInfo(void)
{
	int		idx;
	TASK_INFO	    *taskPtr;
	PROCESS_DESC	*dscPtr;

	// Clear Process Information
	memset((char *)shmPtr->taskInfo, 0, sizeof(TASK_INFO)*MAX_PROCESS);

	// Initialize Process Information
	taskPtr = shmPtr->taskInfo;
	
	/* ------------------------------------------------ */
	/*  ESIO ID 에 따른 응용프로세스 선택 실행...       */
	/* ------------------------------------------------ */
	if(opr->esioID == SDP_ESIO_61850)   dscPtr = prcTable_61850;    
    else                                dscPtr = prcTable_scada; 

	for (idx = 0; idx < MAX_PROCESS; idx++, taskPtr++, dscPtr++)
	{
		if (!strlen(dscPtr->name))  continue;

		sprintf(taskPtr->name, "%s", dscPtr->name);
		taskPtr->define = 1;
		taskPtr->check = dscPtr->check;
		taskPtr->wdtCount = 0;
	}
	
}

//
// 모듈: TerminateProcess()
//
void TerminateProcess(void)
{
	int		idx;
	TASK_INFO	*taskPtr;

	if (shmPtr == NULL)
		return;
    
	// Terminate Process
	taskPtr = shmPtr->taskInfo;
	for (idx = 0; idx < MAX_PROCESS; idx++, taskPtr++)
	{
		//if (!prcPtr->st.defined || !prcPtr->st.active || prcPtr->pid <= 1)
		//	continue;
		KillProcess(taskPtr->pid, taskPtr->check);
	}
}

//
// 모듈: CheckProcess()
//
void CheckProcess(void)
{
	int		idx;
	int		retVal;
	
	TASK_INFO	*taskPtr;
	PROCESS_DESC	*dscPtr;
	char    buffer[256];

	taskPtr = (TASK_INFO *) shmPtr->taskInfo;
	
    /* ------------------------------------------------ */
	/*  ESIO ID 에 따른 응용프로세스 선택 실행...       */
	/* ------------------------------------------------ */
	if(opr->esioID == SDP_ESIO_61850)   dscPtr = prcTable_61850;    
    else                                dscPtr = prcTable_scada; 

	for (idx = 0; idx < MAX_PROCESS; idx++, taskPtr++, dscPtr++)
	{
		if (!taskPtr->define)   continue;

        /* -------------------------------- */
        /* PROCESS 초기 기동시 ...          */
        /* -------------------------------- */
		if (!taskPtr->active || IsProcessActive(taskPtr->pid) < 0)
		{
			if (dscPtr->waitCount > 0)  dscPtr->waitCount--;
			else
			{
			    if(opr->wdtDebug)   
			    Debug(console,"wdt>> [*] Process Start ...[%s] \n", taskPtr->name);
			        
				retVal = StartProcess(dscPtr->name, dscPtr->argument);
				
				printf("wdt> *** TASK Restart : [ %s ] Return status... %d\n", taskPtr->name, retVal);
				
				/* --------------------------------------------- */
				/*	2020.05.15  TASK 실행이상시...					 */
				/* --------------------------------------------- */
				if(retVal != 0)
				{
					printf("\n----------------------------------------\n");
					printf("wdt> *** [%s] TASK Restart... FAIL : REBOOT.. \n", taskPtr->name);
					printf("\n----------------------------------------\n");
					
					/* LOG File 저장 */
    				sprintf(buffer, "%s", "=====================================================");
    				LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
					sprintf(buffer, ">> wdt: *** [%s] TASK-Restart Fail...ESIO-REBOOT ! ", taskPtr->name);
					LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
					  
					sprintf(buffer, "%s", "=====================================================");
    				LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
			
					opr->wdtEnable = 0;
					return;
				}
					
				dscPtr->waitCount = WDT_WAIT_COUNT;
				dscPtr->waitCount = 20;
			}
		}
		else
		{
			dscPtr->waitCount = 0;
			if (taskPtr->wdtEnable)
			{
				taskPtr->wdtCount++;
				//Debug(console,"WDT> Task-%s, count=%d\n", taskPtr->name, taskPtr->wdtCount);
#if 1	// 2016.08.02 ChoiBC : increase wdtCoun ???????
				if (taskPtr->wdtCount > 60)
#else
				if (taskPtr->wdtCount > 10)
#endif
				{
				    printf("\n===================================\n");
                    printf("wdt> [*] Process KILL : Re-Start ... [%s] \n", taskPtr->name);
                    printf("===================================\n");
                    
					sprintf(buffer, ">> wdt: *** [%s] TASK WDT Fail...TASK-Restart ! ", taskPtr->name);
					LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
					  
					KillProcess(taskPtr->pid, taskPtr->check);
					pause(100);
					taskPtr->wdtCount = 0;
					//StartProcess(dscPtr->name, dscPtr->argument);
					//termExec = 0;
				}
			}
			else	
			    taskPtr->wdtCount = 0;	// Reset WDT Check Counter
		}
	}
}

//
// 모듈:	DisplayLogo()
//
void DisplayLogo(void)
{
	//ExecCommand("clear");
	pause(1000);
	printf("\n\n");
	printf("////////////////////////////////////////////////////////\n");  pause(100);
	
#if 0	// 2016.08.08 ChoiBC Modify : ?????
#ifdef	_ACECTRL	// 2018.07.23
	printf("//        [*] ACECTRL-SDP IEC61850 Client : V1.0.0.0  //\n");  pause(100);
	printf("//        [*] Designed by Ace Control Co., Ltd.       //\n");  pause(100);
#else
	printf("//        [*] SH-SDP IEC61850 Client : V1.0.0.0  //\n");  pause(100);
	printf("//        [*] Designed by SHInc.                 //\n");  pause(100);
#endif
#else
	printf("//        %s ESIO[#%d] Module            //\n", TARGET_NAME, opr->boardID ); pause(100);
	printf("//        [*] Designed by Ace Control Co.,Ltd.(THE-ROAD)       //\n");  pause(100);
	printf("//        [*] Version   %s                   //\n", VERSION_STRING);pause(100);
	printf("//        [*] %s     //\n", SIMULATOR_NAME);  pause(100);
#endif
	printf("//        [*] Update Date ...%s              //\n", DATE_STRING);pause(100);
	printf("////////////////////////////////////////////////////////\n\n");  pause(100);
	
	pause(1000);
	    
 }

/* ---------------------------------------------------------------- */
/*  ESIO program update (2026-10-30)                                */
/*   - MPU exit request (VME esioCmd/esioCmdSeq) -> ack -> stop     */
/*   - /mnt/bin/ESIO-VERSION.txt -> VME esioVersion                 */
/*  design : design/20261030-ESIO-SIDE-update.md                    */
/* ---------------------------------------------------------------- */
#define ESIO_VERSION_FILE   "/mnt/bin/ESIO-VERSION.txt"

static word lastCmdSeq = 0;         /* last handled request (compare with !=) */
static int  stopByMpu  = 0;         /* 1 : stop was requested by MPU          */

void init_esio_update(void)
{
    FILE    *fp;
    char    line[64], ver[ESIO_VERSION_LEN];
    char    buffer[160];
    int     i, n, c;

    /* requests already in VME at start are treated as handled */
    lastCmdSeq = vmeSioDCB->esioCmdSeq;

    /* ESIO-VERSION.txt : missing -> "not found", empty -> "invalid" */
    snprintf(ver, sizeof(ver), "not found");
    if ((fp = fopen(ESIO_VERSION_FILE, "r")) != NULL)
    {
        snprintf(ver, sizeof(ver), "invalid");
        if (fgets(line, sizeof(line), fp) != NULL)
        {
            n = 0;
            for (i = 0; line[i] != 0 && n < ESIO_VERSION_LEN - 1; i++)
            {
                c = (unsigned char) line[i];
                if (c == '\n' || c == '\r')     break;          /* first line only */
                if (c < 0x20 || c > 0x7e)       continue;       /* printable ASCII only */
                if (c == ' ' && n == 0)         continue;       /* trim leading space */
                ver[n++] = (char) c;
            }
            while (n > 0 && ver[n-1] == ' ')    n--;            /* trim trailing space */
            ver[n] = 0;
            if (n == 0)     snprintf(ver, sizeof(ver), "invalid");
        }
        fclose(fp);
    }

    memset((void *) vmeSioDCB->esioVersion, 0, ESIO_VERSION_LEN);
    memcpy((void *) vmeSioDCB->esioVersion, ver, strlen(ver));

    printf("wdt> ESIO version = [%s]  (sizeof(VME_SIODCB)=%d, offset(esioCmd)=%d)\n", ver,
           (int) sizeof(VME_SIODCB), (int) ((char *) &vmeSioDCB->esioCmd - (char *) vmeSioDCB));
    fflush(stdout);

    sprintf(buffer, "wdt> ESIO version = [%s]", ver);
    LogFile_MPU(shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
}

/* called every main-loop turn (100 ms) */
void check_MPU_command(void)
{
    word    seq, cmd;
    char    buffer[160];

    seq = vmeSioDCB->esioCmdSeq;
    if (seq == lastCmdSeq)      return;             /* no new request */
    lastCmdSeq = seq;
    if (seq == 0)               return;             /* 0 is not used by MPU */

    __sync_synchronize();                           /* MPU writes cmd first, seq last */
    cmd = vmeSioDCB->esioCmd;

    if (cmd == ESIO_CMD_RESET_EXIT)
    {
        vmeSioDCB->esioAckSeq = seq;                /* ack first */
        vmeSioDCB->esioState  = ESIO_ST_STOPPING;
        stopByMpu = 1;

        printf("wdt> [STOP] stop command received from MPU (seq=%d) - ack sent\n", seq);
        printf("wdt> [STOP] ESIO program will stop now ...\n");
        fflush(stdout);

        sprintf(buffer, "wdt> [STOP] stop command received from MPU (seq=%d)", seq);
        LogFile_MPU(shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));

        termExec = 0;                               /* same path as SIGTERM */
    }
    else
    {
        vmeSioDCB->esioErr    = ESIO_ERR_BAD_CMD;
        vmeSioDCB->esioAckSeq = seq;                /* ack so that MPU does not wait */

        printf("wdt> [CMD] unknown command 0x%04X from MPU (seq=%d) - ignored\n", cmd, seq);
        fflush(stdout);
    }
}

//
// 모듈:	ClearEnvironment()
//
void ClearEnv(void)
{
    int wdt_ctrl =0  ;
    // 수행중인 프로세스들을 종료시킨다
    printf("wdt> [STOP] stop in progress : terminating ESIO tasks%s ...\n", stopByMpu ? " (MPU request)" : "");
    fflush(stdout);
	TerminateProcess();
    if (vmeSioDCB != NULL)  vmeSioDCB->esioState = ESIO_ST_STOPPED;     /* all tasks terminated */
    printf("wdt> [STOP] ESIO tasks stopped - WDT exit\n");
    fflush(stdout);
	
	opr->stscode = 0;
		         	
	/* ---------------------------------------- */
    /* 프로그램 종료시 : WDT Disable...         */
    /* ---------------------------------------- */
    if(ioctl(wdtid, SDP_WDT_SET_CTRL, &wdt_ctrl) < 0 )
	{
	    Debug(console, "wdt> ioctl fail\n");
		opr->wdtEnable = 0;
    }		    
// hkkim    
//    close(ledPort.id);
//    close(idPort.id);

    // Shared memory lock file close
	d61850Shm_End ();
		    
	// 공유 메모리를 제거한다
	ShmDelete(&shmDesc);

}
					    

//
// 모듈:	InitEnvironment()
//
int InitEnv( void)
{
    int i;
    int	diff;
    byte	dipSwitch=0;
    
	// SIGNAL 처리 루틴 설정
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);

	/* -------------------------------------------- */
	/* 디바이스 관련 초기화 LED - IO Device Open ... */
	/* -------------------------------------------- */

    if (  initRunLad() )
    {
        printf("wdt> *Device Open Error ... LED \r\n");
        exit(1);
    }


    if ( init_esio_id())
    {
        printf("wdt> *Device Open Error ... VME-ID \r\n");
        exit(1);
    }

#if 0 //hkkim
    if((ledPort.id = open( ledPort.name, ledPort.attr)) <0)  
	{
        printf("wdt> *Device(%s) Open Error ... LED \n", ledPort.name);
        exit(1);
    }
    else
        printf("wdt> LED-Device(%s) Open ... OK \n", ledPort.name);

    if((idPort.id = open( idPort.name, idPort.attr)) <0)  
	{
        printf("wdt> *Device(%s) Open Error ... ID \n", idPort.name);
        exit(1);
    }
#endif 

#if 0
    printf("size of TASK_INFO       = %d\n", sizeof(TASK_INFO) *  (MAX_PROCESS+2));
    printf("size of RTC             = %d\n", sizeof(RTC));
    printf("size of OPR_MSG         = %d\n", sizeof(OPR_MSG));
    printf("size of CONSOLE_INFO    = %d\n", sizeof(CONSOLE_INFO));
    printf("size of MPU_CONFIG      = %d\n", sizeof(MPU_CONFIG));
    printf("size of ESIO_CONFIG     = %d\n", sizeof(ESIO_CONFIG));
    printf("size of POINT_BUF       = %d\n", sizeof(POINT_BUF) * MAX_DEV_POINT);
    printf("size of SCAN_CONFIG     = %d\n", sizeof(SCAN_CONFIG) * MAX_SCAN_PORT);
    
    printf("size of SCAN_CONFIG     = %d\n", sizeof(SCAN_CONFIG) * MAX_SCAN_PORT);
    printf("size of SDP_DEVICE      = %d\n", sizeof(SDP_DEVICE) * MAX_DEVICE);
    printf("size of ESIO_DATABASE   = %d\n", sizeof(RTU_DATABASE));
    
    printf("size of HISTORY_QUE     = %d\n", sizeof(HISTORY_QUE));
    printf("size of TIME_SYNC_INFO  = %d\n", sizeof(TIME_SYNC_INFO));
    printf("size of DEVICE_61850    = %d\n", sizeof(DEVICE_61850) * MAX_DEVICE);
    printf("size of SOE_QUEUE       = %d\n", sizeof(SOE_QUEUE));
#endif

    printf("\n----------------------------------------------\n");
    diff = 13719188 - sizeof(SHM_MEMORY);
    printf("wdt> Shared Memory.... size = %d byte (13,719,188 byte) => diff = %d\n", sizeof(SHM_MEMORY), diff);
    printf("----------------------------------------------\n");
        
    /* ------------------------------------ */
	/*  공유 메모리를 생성한다              */
	/* ------------------------------------ */
	if (ShmCreate(&shmDesc) < 0)
	{
		printf("wdt> *공유 메모리 Open 이상 \n");
		return(0);
	}
    else printf("wdt> ShmCreate OK\r\n");
    /* ------------------------------------ */
	/*  공유 메모리 상태 확인               */
	/* ------------------------------------ */
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("wdt> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
    else printf("wdt> ShmCheck OK\r\n");	
	/* ------------------------------------ */
	/* 공유 메모리상의 포인터 변수 초기화   */      
	/* ------------------------------------ */
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	taskPtr = (TASK_INFO *) &shmPtr->taskInfo[WDT_PROCESS];
	
	rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
	
	mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;                // MPU Network 구성정보
    esioCFG = (ESIO_CONFIG *) &shmPtr->esioConfig;
    
    soeQueue = (SOE_QUEUE *) &shmPtr->soeQueue;
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
    
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 

    /* 61850 Time-Server 연동 */
    timeSync = (TIME_SYNC_INFO *) &shmPtr->timeSync; 
    
    /* ESIO Buffer 초기화 */
    bzero( esioCFG, sizeof(ESIO_CONFIG));
    esioCFG->mpuTimeSyncOK = 0;
    
    /* ------------------------------------ */
    /* SCAN CONFIG : SCAN 구조체(MAX 16)    */
    /* ------------------------------------ */
    for(i=0; i< MAX_SCAN_PORT; i++)   
	{
	    scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];
	    bzero8248((byte *) scanCFG[i], sizeof(SCAN_CONFIG));
	}
	
	/* ------------------------------------ */
    /* 계전기 구조체 : 전체 계전기(MAX 64)  */
    /* ------------------------------------ */    
    for(i=0; i< MAX_DEVICE; i++)   
	{
	    deviceCFG[i]   = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	    device61850[i] = (DEVICE_61850 *) &shmPtr->device61850[i];
	    
	    bzero8248((byte *) deviceCFG[i], sizeof(SDP_DEVICE));
	    bzero8248((byte *) device61850[i], sizeof(DEVICE_61850));
	}

#if 0
    /* ------------------------------------ */
    /* 포인트 구조체 : 전체 포인트(4096)    */
    /* ------------------------------------ */   
	for(i=0; i< MAX_DEV_POINT; i++)   
	{
	    pointCFG[i] = (POINT_BUF *) &shmPtr->pointCFG[i];
	    bzero8248((byte *) pointCFG[i], sizeof(POINT_BUF));
	}	
#endif
	
	// Shared memory lock file open
	d61850Shm_Init (shmPtr);
	
	/* ------------------------------------ */
    /* 운영 Buffer Clear....                */
    /* ------------------------------------ */
    bzero( opr,     sizeof(OPR_MSG));
    bzero( rtc,     sizeof(RTC));
    bzero( rtudb,   sizeof(RTU_DATABASE));
    bzero( console, sizeof(CONSOLE_INFO));
    bzero( hque,    sizeof(HISTORY_QUE));
    
    
    opr->lineDebug  = 0xff;
    opr->dnpDebug   = 0xff;
    opr->msgDebug   = 0xff; 
    opr->modDebug   = 0xff; 
    opr->scanDebug  = 0xff;
    
    
    /* -------------------------------------------- */
    /*  해당 ESIO  ID & 운영모드 설정               */
    /*  - ID 정보 : 해당 ESIO ID 부여 (0,1,2,3,4)   */
    /*  - 운영모드 : MPU-A, MPU-B 장착여부 구분     */
    /* -------------------------------------------- */
    opr->boardID = readESIO_ID();                   // ESIO 보드 ID ...(0)-SIO,  (1)-ESIO1, (2)-ESIO2, (3)-ESIO3, (4)-ESIO4
    opr->esioID  = opr->boardID + 1;                // 0: SDP_NULL. 1:SDP_MPU, 2:SDP_SCADA, 3: SDP_DIAG, 4: SDP_EQ, 5:SDP_61850
    
    /* ESIO 장착 위치 결정... */
    dipSwitch = readESIO_SWITCH();             // ESIO 보드 DIP-Switch ...Bit0 : MPU-A/B

    if(dipSwitch & 0x01)	opr->cpuMode = MPU_A;
    else                    opr->cpuMode = MPU_B;            
    
    printf("\n========================================================================\n");
    if(opr->cpuMode == MPU_A)   printf(">> MPU-A... ESIO Config READ : Board-ID = %02d, ESIO-ID = %02d, Switch = %02x\n", opr->boardID,  opr->esioID, dipSwitch);
    else                        printf(">> MPU-B... ESIO Config READ : Board-ID = %02d, ESIO-ID = %02d, Switch = %02x\n", opr->boardID,  opr->esioID, dipSwitch);   
    printf("========================================================================\n\n");      
        
    
	// PROCESS 관련 정보를 초기화한다
	SetProcessInfo();
	nice(-10);

	// 프로세스 정보를 초기화한다
	taskPtr->initial = 1;
	UpdateProcessInfo(taskPtr, 1, getpid());





	return(1);
}


/*
*   VMECU 전면판 상태 LED
*/
void esio_LED_Update()
{
    /* ---------------------------------------- */    
    /*  ACE-VMESIO : 전면 Front LED Display     */
    /* ---------------------------------------- */  
    /*    7   6   5   4   3   2   1   0         */
    /*  ---------------------------------       */
    /*  | 0 | 0 | 0 |RUN| 0 | 0 |SIO|SHT|       */
    /*  ---------------------------------       */
    /*  - SHT Bit : Shutdown                    */
    /*  - SIO Bit : SIO-Out                     */
    /*  - RUN Bit : RUN-LED                     */
    /* ---------------------------------------- */
    
    if(opr->runMode == LOCAL_MASTER)    
    {    
             opr->stscode ^= ESIO_LED_RUN;              /* RUN LED */
            led_toggle();    
    }
    else
    {
        if(ledRunTick++ > 5)
        {
            ledRunTick = 0;
            opr->stscode ^= ESIO_LED_RUN;              /* RUN LED */
            // hkkim
            led_toggle();
        }        
    }            

#if 0 //hkkim
	//printf("stscode = %02x\n", opr->stscode);
	
    /* ---------------------------------------- */    
    /*  전면 LED 상태표출 ...                   */
    /* ---------------------------------------- */    
	if(write(ledPort.id, &opr->stscode, 1) != 1) 
	{                                       
		Debug(console, "wdt> *Led write error \n");           
		//exit(0);                            
	}        
#endif 	     
}


/*
*   NETWORK-Config 
*   2026-04-21 오후 4:17:41  값이 없거나 틀린겨웅..
*/
void esioNetwork_Initial()
{
    char    buffer[256];
    
    ESIO_NET_ENTRY   *esioNet1, *esioNet2;
// 2026-05-26 오후 5:28:13     
    ESIO_NET_ENTRY   *esioNet3, *esioNet4;
    
    /* -------------------------------------------- */
    /*  MPU 실장모드에 따라서 각 Network IP 설정    */
    /* -------------------------------------------- */
    if(opr->cpuMode == MPU_A)
    {
        printf("\n------------------------------------------\n");
        printf("esio> ***** MPU-A ESIO ...OK \n");
        printf("------------------------------------------\n");
 
        esioNet1 = (ESIO_NET_ENTRY *) &esioCFG->mstNetConfig[0];
        esioNet2 = (ESIO_NET_ENTRY *) &esioCFG->mstNetConfig[1];
        esioNet3 = (ESIO_NET_ENTRY *) &esioCFG->mstNetConfig[2];
        esioNet4 = (ESIO_NET_ENTRY *) &esioCFG->mstNetConfig[3];
    }        
    else
    {
        printf("\n------------------------------------------\n");
        printf("esio> ***** MPU-B ESIO ...OK \n");
        printf("------------------------------------------\n");
        
        esioNet1 = (ESIO_NET_ENTRY *) &esioCFG->slvNetConfig[0];
        esioNet2 = (ESIO_NET_ENTRY *) &esioCFG->slvNetConfig[1];
        esioNet3 = (ESIO_NET_ENTRY *) &esioCFG->slvNetConfig[2];
        esioNet4 = (ESIO_NET_ENTRY *) &esioCFG->slvNetConfig[3];
    }
        
   
    /* -------------------------------------------- */
    /*  ROM-VERSION 인경우... eth0 설정             */
    /* -------------------------------------------- */    
#ifdef  ROM_VERSION
    // hkkim  ip format check
    if ( validate_ip(esioNet1->ipAddr) == 0)
    {
    /* MASTER-CPU : eth0 Network...   */            
//    sprintf(buffer, "ifconfig eth0 %s netmask %s up\n", esioNet1->ipAddr, esioNet1->subMask);    
// hkkim 2026-05-07 오후 4:47:17
    sprintf(buffer, "ifconfig lan0 %s netmask %s up\n", esioNet1->ipAddr, esioNet1->subMask);
    printf("=> Network#1 : %s", buffer);
   	system(buffer);  
   	sleep_ms(100);  
    }
    else 
        printf("esioNet1 format error\r\n");
    
#endif

    //printf("=> Network#1 : ifconfig eth0 %s netmask %s up\n", esioNet1->ipAddr, esioNet1->subMask);
    
    /* MASTER-CPU : eth1 Network...   */ 
    if(esioNet2->useFlag == SET)
    {            
        if ( validate_ip(esioNet2->ipAddr) == 0)
        {
// hkkim 2026-05-07 오후 4:47:17
        sprintf(buffer, "ifconfig lan1 %s netmask %s up\n", esioNet2->ipAddr, esioNet2->subMask);
        printf("=> Network#2 : %s", buffer);
      	system(buffer);  
   	    sleep_ms(100);  
         }
    else 
        printf("esioNet2 format error\r\n");    
    }


    if(esioNet3->useFlag == SET)
    {            
        if ( validate_ip(esioNet3->ipAddr) == 0)
        {
// hkkim 2026-05-07 오후 4:47:17
        sprintf(buffer, "ifconfig lan2 %s netmask %s up\n", esioNet3->ipAddr, esioNet3->subMask);
        printf("=> Network#3 : %s", buffer);
      	system(buffer);  
   	    sleep_ms(100);  
         }
    else 
        printf("esioNet3 format error\r\n");    
    }
    
    if(esioNet4->useFlag == SET)
    {            
        if ( validate_ip(esioNet4->ipAddr) == 0)
        {
// hkkim 2026-05-07 오후 4:47:17
        sprintf(buffer, "ifconfig lan3 %s netmask %s up\n", esioNet4->ipAddr, esioNet4->subMask);
        printf("=> Network#4 : %s", buffer);
      	system(buffer);  
   	    sleep_ms(100);  
         }
    else 
        printf("esioNet4 format error\r\n");    
    }


    
}

/*----------------------------------------------------------------------------
* Function Name : setOffline()
* 수행내용: 정상적인 모듈이 Offline 되는 경우 해당모듈을 Offline 처리하는 함수
* ArgList :
*   1. ioid - Offline 되는 모듈번호
* Return  :
---------------------------------------------------------------------------- */
int setOffline_61850( int devid)
{
    SDP_DEVICE  *dev;
    SOEQ_ENTRY  event;


    /* DEVICE Offline 처리 */
    if((devid < 0) || (devid >= MAX_DEVICE))    return (0);
	
    dev = (SDP_DEVICE *) deviceCFG[devid];
    
    dev->online     = 0;
    dev->offlineTick= 0;
    dev->init_status= RESET;        // 상태덤프 초기화
    dev->init_analog= RESET;        // 상태덤프 초기화
    dev->sndLinkSts = RESET;
    dev->devStatus &= (~0x01);             // Online-Bit RESET
    
    /* ---------------------------------------- */
    /*  계전기 Offline 이벤트 생성              */
    /* ---------------------------------------- */
    event.eventCode = ENT_DEV_OFFLINE;                  // 계전기 Offline                      
    event.devNo     = devid + 1;                        // 계전기 번호 : 1 ~ 64
    event.pointNo   = 1;
    event.state     = 1;                                // Offline 정보

    gettimeofday(&event.updateTime, NULL);              // 이벤트 생성시각 추출...timeval  Form  

    //if(opr->soeDebug)
    Debug(console,">> *Module Offline ...  dev=%2d \n", devid+1);
    
    /* ---------------------------------------- */
    /* MPU 전송 - EVENT 저장                    */
    /* ---------------------------------------- */        
    storeESIO_event( shmPtr, &event);
    
    return (1);
}


/*----------------------------------------------------------------------------
* Function Name : setOnline()
* 수행내용: Offline된 모듈이 정상적으로 Online 되는 경우 해당모듈을 Online 처리하는 함수
* ArgList :
*   1. ioid - Online 되는 모듈번호
* Return  :
---------------------------------------------------------------------------- */
int setOnline_61850(int devid)
{
    SDP_DEVICE      *dev;
    SOEQ_ENTRY      event;
    
    /* DEVICE Online 처리 */
    if((devid < 0) || (devid >= MAX_DEVICE))    return (0);

    dev = (SDP_DEVICE *) deviceCFG[devid];
    dev->online     = SET;
    dev->init_status= RESET;        // 상태덤프 초기화
    dev->init_analog= RESET;        // 상태덤프 초기화
    dev->offlineTick = 0;

    dev->timeSyncReq = SET;
    dev->statusDump  = SET;
    
    dev->devStatus |= 0x01;             // Online-Bit SET
    
    /* ---------------------------------------- */
    /* MPU 전송 - SOE 이벤트 생성               */
    /* ---------------------------------------- */
    event.eventCode = ENT_DEV_ONLINE;                   // 계전기 Online
    event.devNo     = devid + 1;                        // 계전기 번호 : 1 ~ 64
    event.pointNo   = 1;                                // 포인트 번호 : 1 ~ 1024
    event.state     = 0;                                // Online 정보 = 0 

    /* SOE 시각정보-timeval 정보 생성 */    
    gettimeofday(&event.updateTime, NULL);
    
    //if(opr->soeDebug)
    Debug(console,">> Module Online ...  dev=%2d \n", devid+1);
                
    /* ---------------------------------------- */
    /* MPU 전송 - EVENT 저장                    */
    /* ---------------------------------------- */ 
    storeESIO_event(shmPtr, &event);
                
    return (1);
}


/*
*   61850 계전기 - 상태값 .... 실제 계전기 구조체로 Update
*/
int update_Online_61850()
{
    int     ioid, devid;
    int     point;
    int     index1, index2;
    byte    stsCount;
    byte    stsbuf[MAX_DI_DATA_NUM];
    
    DEVICE_61850    *dev61850;
    POINT_61850     *di61850;
    POINT_BUF       *diPoint;
    
    SDP_DEVICE      *dev;
    
    index1 = 0;
    index2 = 0;
    
    for(ioid=0; ioid < MAX_DEVICE; ioid++)
    {
        dev61850 = (DEVICE_61850 *) device61850[ioid];
        if(dev61850->config61850 == 0)  continue;

        /* ------------------------------------- */
        /* 실제 계전기 구조체로 정보 Update .... */
        /* ------------------------------------- */        
        devid = dev61850->src_ioid - 1;
        if(devid < 0) continue;
        
        dev = (SDP_DEVICE *) deviceCFG[devid];
        
        if(dev61850->online)
        {
            if(dev->online != 1)
            {
                printf("\n=================================\n");
                printf("esio%d> Device-61850(%d)  Online....\n", opr->boardID, devid+ 1);
                printf("=================================\n");
                
                setOnline_61850(devid);
                
            }
            dev->online = dev61850->online; 
            dev->init_analog = SET;
            dev->init_status = SET;
            
            //printf("61850-dev(%d) online\n", devid+1);
        }
        else
        {
            if(dev->online == 1)
            {
                printf("\n=================================\n");
                printf("esio%d> Device-61850(%d)  Offline.... \n", opr->boardID, devid+ 1);
                printf("=================================\n");
                
                setOffline_61850(devid);
            }   
            
            dev->online = dev61850->online; 
            dev->init_analog = RESET;  
            dev->init_status = RESET;
            
            //printf("61850-dev(%d) offline\n", devid+1);    
        }
        
        if(dev->online) dev->devStatus |= 0x01;
        else            dev->devStatus &= (~0x01);             // Online-Bit RESET                   
        
    }

    return (0);
}


/*
*   61850 계전기 - 상태값 .... 실제 계전기 구조체로 Update
*/
int update_Status_61850()
{
    int     ioid, devid;
    int     point;
    int     index1, index2;
    byte    stsCount;
    byte    stsbuf[MAX_DI_DATA_NUM];
    
    DEVICE_61850    *dev61850;
    POINT_61850     *di61850;
    POINT_BUF       *diPoint;
    
    SDP_DEVICE      *dev;
    
    index1 = 0;
    index2 = 0;
    
    for(ioid=0; ioid < MAX_DEVICE; ioid++)
    {
        dev61850 = (DEVICE_61850 *) device61850[ioid];
        if(dev61850->config61850 == 0)  continue;

        /* ------------------------------------- */
        /* 실제 계전기 구조체로 정보 Update .... */
        /* ------------------------------------- */        
        devid = dev61850->src_ioid - 1;
        if(devid < 0) continue;
        
        dev = (SDP_DEVICE *) deviceCFG[devid];
        
        bzero(stsbuf,MAX_DI_DATA_NUM); 

        /* 61850 계전기 상태정보 ... Update... */                
        for(point = 0; point < dev61850->devDiLastNo; point++)
        {
            di61850 = (POINT_61850 *) &dev61850->diPtBuf[point];
            
            /* 61850 계전기 포인트 정의여부 Check... */            
            if(di61850->config61850 == 0)	continue;
            	
            diPoint = (POINT_BUF *) &dev->diPtBuf[point];
            
			index1 = point / 8;
			index2 = point % 8;

            if(di61850->state)  stsbuf[index1] |= bitcode[index2];	
            diPoint->status =  di61850->state;   
        }
        
        //printf("61850-dev(%d) devDiLastNo= %d\n", devid+1, dev61850->devDiLastNo);    
        
        if(index2)  stsCount = index1 + 1;
        else        stsCount = index1;
        
        memcpy( dev->cursts, stsbuf, stsCount);   
        dev->stsByteCnt = stsCount;
        //dev->devDiPoint = dev61850->devDiLastNo;
        dev->init_status = SET;
        
        if(opr->msgDebug == devid)
        {     
            printf("Update. STSTUS.. 61850 : dev = %d (point=%d)  - %02x \n", devid + 1, point, stsbuf[0]);
        }
    }

    return (0);
}


/*
*   61850 계전기 - 계측값 .... 실제 계전기 구조체로 Update
*/
int update_Analog_61850()
{
    int     ioid, devid;
    int     point;
    
    POINT_61850     *ai61850;
    POINT_BUF       *aiPoint;
    
    DEVICE_61850    *dev61850;
    SDP_DEVICE      *dev;
    
    for(ioid=0; ioid < MAX_DEVICE; ioid++)
    {
        dev61850 = (DEVICE_61850 *) device61850[ioid];
        if(dev61850->config61850 == 0)  continue;

        /* ------------------------------------- */
        /* 실제 계전기 구조체로 정보 Update .... */
        /* ------------------------------------- */        
        devid = dev61850->src_ioid - 1;
        if(devid < 0) continue;
            
        dev = (SDP_DEVICE *) deviceCFG[devid];
        
        /* ------------------------------------- */
        /* 61850 계전기 상태정보 ... Update... */        
        /* ------------------------------------- */        
        for(point = 0; point < dev61850->devAiLastNo; point++)
        {
            ai61850 = (POINT_61850 *) &dev61850->aiPtBuf[point];

			/* 61850 계전기 포인트 정의여부 Check... */            
            if(ai61850->config61850 == 0)	continue;
           
           	aiPoint = (POINT_BUF *) &dev->aiPtBuf[point];
           
			/* 이벤트 시간-포인트 정의시:2000년 1월 1일 00:00:00 기준  */                
           	if(aiPoint->ptType == AI10VPT)
           	{
           		if(ai61850->floatData > 0)	aiPoint->floatData =  ai61850->floatData - 946652400;  	// 2000년 1월 1일 00:00:00 */ 	
           		else						aiPoint->floatData =  ai61850->floatData;   	
           	}
           	else
           	{		 	
            	aiPoint->floatData =  ai61850->floatData;   
            }
        }
        
        //dev->devAiPoint = dev61850->devAiLastNo;
        
        if(opr->msgDebug == devid)
        {     
            printf("Update. ANALOG.. 61850 : dev = %d (point=%d)  \n", devid + 1, point );
        }
    }

    return (0);
}


/*
*   사용자 데이터 입력 시험 : ANALOG
*/
void testAnalog_61850( )
{
    DEVICE_61850  *dev61850;
    POINT_61850   *aiPoint;

    Debug(console,"==> Test Analog Data Update....[%2d] !\n", rtc->sec);
    
    opr->testAnalogFLAG = RESET;
    
    Debug(console, ">> Test-ANALOG Create... dev=%2d, point=%2d, data = %4.2f\n", opr->testDevid + 1, opr->testPoint + 1, opr->testAIData);
    
    /* 계전기 포인트 정보 Update ... */    
    dev61850 = (DEVICE_61850 *) device61850[opr->testDevid];
    aiPoint  = (POINT_61850 *) &dev61850->aiPtBuf[opr->testPoint];

	/* for TEST */    
    dev61850->online = SET;
	dev61850->devAiLastNo = 16;
    
    aiPoint->floatData = opr->testAIData;
    gettimeofday(&aiPoint->updateTime, NULL);
    
}


/*
*   사용자 데이터 입력 시험 : STATUS
*/
void  testStatus_61850()
{
    SOEQ_ENTRY  event;
    
    DEVICE_61850  *dev61850;
    POINT_61850   *diPoint;

    opr->testStatusFLAG = RESET;
    
    Debug(console, ">> Test-SOE Create... dev=%2d, point=%2d, state=%d\n", opr->testDevid + 1, opr->testPoint + 1, opr->testState);
    
    /* 계전기 포인트 정보 Update ... */    
    dev61850 = (DEVICE_61850 *) device61850[opr->testDevid];
    
    /* for TEST */   
    dev61850->online = SET;
    dev61850->devDiLastNo = 16;
    
    diPoint  = (POINT_61850 *) &dev61850->diPtBuf[opr->testPoint];
    
    diPoint->state = opr->testState;
    gettimeofday(&diPoint->updateTime, NULL);
    
    /* 61850 SOE 포인트 정보 Update ... */    
    event.eventCode = ENT_SOE;
    event.devNo   = opr->testDevid + 1;
    event.pointNo = opr->testPoint + 1;
    event.state   = opr->testState;
    
    gettimeofday(&event.updateTime, NULL);
    
    d61850Shm_PushSoe (&event);
    
}            

static  int readyCount = 0;

/*
*   VME Config 상의 Network 정보...
*/
void    check_ESIO_Network()
{
    int     i;
    int     k,j;
    char    buffer[256];
    
#if 0    
    // hkkim
        word    useFlag[3];             // Network# 사용유무
        byte    ipAddr[3][16];          // Network# IP-Address
        byte    gwAddr[3][16];          // Network# GW-Address
        byte    subMask[3][16];         // Network# SUB-Mask
#endif     
     
    /* ------------------------------------------------ */
    /*  VMEBUS 상의 ESIO Network 정보 Update...         */
    /* ------------------------------------------------ */
    if(vmeSioDCB->netCfgChange == 0x1234)
    {
        vmeSioDCB->netCfgChange = 0;
        
        /* ------------------------------------ */
        /* ESIO Network Port : eth0, eth1 		*/
        /* ------------------------------------ */
        for(i=0; i < 4; i++)
        {

#if 0  // ARM-PPC 연결시
        	/* vme 에서 가져온 것을 endian 처리 해준다. */
                    for(k=0,j=0; k<16 ;j+=2,k+=2) // byte  단위로 쓴다.
                    {   // 
                       
                        ipAddr[i][k] = vmeSioDCB->ipAddr[i][j+1]    ;
                        ipAddr[i][k+1] = vmeSioDCB->ipAddr[i][j]  ;
                        
//                        +  (vmeSIO->ipAddr[j] << 8) ;
//                        printf("%04X-%02X:%02X - ", vmeSIO->ipAddr[i][k],  enetCfg->ipAddr[j ],enetCfg->ipAddr[j+1]  ) ;
//                        vmeSIO->gwAddr[i][k] = enetCfg->gwAddr[j+1] +  (enetCfg->gwAddr[j] << 8) ;
//                        vmeSIO->subMask[i][k]= enetCfg->subMask[j+1] +  (enetCfg->subMask[j] << 8) ;                                       
                    }
                    printf(" [%d] Network IP Addr  = %s\n", i+1, ( char *)ipAddr[i] );                    

#endif         	
        	
        	
        	
        	
        	Debug(console, "-------------------------------------\n");
            Debug(console, " [%d] Network USE Flag = %d\n", i+1, vmeSioDCB->useFlag[i]);
            Debug(console, " [%d] Network IP Addr  = %s\n", i+1, vmeSioDCB->ipAddr[i]);
            Debug(console, " [%d] Network Gate Way = %s\n", i+1, vmeSioDCB->gwAddr[i]);
            Debug(console, " [%d] Network Sub Mask = %s\n", i+1, vmeSioDCB->subMask[i]);

			/* VME Network 초기화 설정시... */
			// hkkim 2026-04-22 오후 12:45:49 
			//if(opr->vmeNetworkInit == SET)
	        {
	            if(vmeSioDCB->useFlag[i] == SET)
    	        {      
        	        sprintf(buffer, "ifconfig lan%d %s netmask %s up\n", i, (char *) vmeSioDCB->ipAddr[i], (char *) vmeSioDCB->subMask[i]);
            	    Debug(console, "=> Network: %s", buffer);

#ifdef  ROM_VERSION                
   	            	system(buffer);  
                	pause(100);  
#endif                
	            }
    	        else
        	    {
            	    sprintf(buffer, "ifconfig lan%d down\n", i);
                	Debug(console, "=> Network: %s", buffer);

#ifdef  ROM_VERSION                
	   	            system(buffer);  
    	            pause(100);  
#endif  
				}                              
            }      
        }
    }       
    else if(esioCFG->online != SET)
    {
    	/* ---------------------------------------------------- */
    	/*	ESIO와 MPU 가 Offline 인 경우... Network 초기화 요청 		*/
    	/* ---------------------------------------------------- */
        if(++readyCount > 30)
        {
            readyCount = 0;     
            vmeSioDCB->netChangeReq = 0x1234;
            
            Debug(console, "\n-------------------------------------\n");
            Debug(console, "wdt>> *** ESIO Netconfig REQ...%4x\n", vmeSioDCB->netChangeReq);
            Debug(console, "-------------------------------------\n");
        }
    }
    else // ????
    {
        vmeSioDCB->netChangeReq = 0;
        readyCount = 0;
    }        
    
}

static  int sioCount=0;
static  byte sioRunTick=0;
static  byte masterRunTick=0;
static  byte slaveRunTick=0;

/*
*	VMEBUS 상의 MPU와 ESIO간 동기 Check...
*/
void    check_ESIO_mode(int mpuCount)
{

    /* -------------------------------------------- */
    /* MPU와 SIO 간 Access 로 MASTER/SLAVE 구분     */
    /* -------------------------------------------- */
    // MPU에서 주기적으로 mpuCount를 증가한다. 
    if(mpuCount != sioCount)
    {
        sioRunTick = 0;
        if(vmeSioDCB->localMaster == SET)   
        {
        	if(++masterRunTick > 2)	// 약간 시간을 기다리는 것인데...
        	{
        		if(opr->runMode == LOCAL_SLAVE)	printf("===> SET MASTER-state....!\n");
        		opr->runMode = LOCAL_MASTER;
        		masterRunTick = 10;
        	}
        	slaveRunTick = 0;	
        }
        else
        {
        	if(++slaveRunTick > 1)	
        	{
        		if(opr->runMode == LOCAL_MASTER)	printf("===> SET SLAVE-state....!\n");
        		opr->runMode = LOCAL_SLAVE;
        		
        		slaveRunTick = 10;
        	}
        	masterRunTick = 0;
        }
    }
    else
    {
        if(++sioRunTick > 50) // VMEBUS 통신 또는 MPU가 이상한 경우겠지..
        {
        	if(opr->runMode == LOCAL_MASTER)	printf("===> SET VMEBUS Fail....SLAVE !\n");
            sioRunTick = 0;
            sioRunTick = 50; // 이 것 때문에 계속  slave 된다. 어  두번이네..
            opr->runMode = LOCAL_SLAVE;   
        }           
    }   
        
    sioCount = mpuCount;    // 이전값 .. Update

#if 0
	/* for TEST */
	if(rtc->sec > 30)	opr->runMode = LOCAL_MASTER;
    else                opr->runMode = LOCAL_SLAVE;  
#endif
   
}

/*
*	ESIO : DB 변경사항 Check... Update
*/
int	update_ESIO_DB()
{
 	char    buffer[256];
 	      
  	if(opr->esioID != SDP_ESIO_61850)
    {   
    	if(esioCFG->dbMpuUpdate == SET)  
	    {
	        Debug(console,">> rcv MPU-DB Updated...[%02d:%02d:%02d] !\n", rtc->hour, rtc->min, rtc->sec);
	        esioCFG->dbMpuUpdate = RESET;	    
	        
	        sprintf(buffer, ">> rcv MPU-DB Updated...! ");
			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  
	
            mpuParaConfig();          // ESIO 데이터베이스 : MPU DB 변경	
        } 
        else if(esioCFG->dbEsioUpdate == SET)  
       	{
	       Debug(console,">> rcv ESIO-DB Updated...[%02d:%02d:%02d] !\n", rtc->hour, rtc->min, rtc->sec);
	        esioCFG->dbEsioUpdate = RESET;	    
	        
	        sprintf(buffer, ">> rcv ESIO-DB Updated...! ");
			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  
			
            esioParaConfig();          // ESIO 데이터베이스 : ESIO DB 변경	
        } 
    	else if(esioCFG->dbModbusUpdate == SET)  
	    {
	        Debug(console,">> rcv MODBUS-DB Updated...[%02d:%02d:%02d] !\n", rtc->hour, rtc->min, rtc->sec);
	        esioCFG->dbModbusUpdate = RESET;	    
	        
	        sprintf(buffer, ">> rcv MODBUS-DB Updated...SCAN-RESTART ! ");
			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  
			
			deviceParaConfig();         // ESIO 데이터베이스 : MODBUS DB 변경	
            
            pause(1000);
            opr->scanRestart = SET;     // SCAN -TASK 재기동
        }
        else if(esioCFG->dbScanUpdate == SET)  
	    {
    	    Debug(console,">> rcv SCAN-DB Updated...[%02d:%02d:%02d] !\n", rtc->hour, rtc->min, rtc->sec);
	        esioCFG->dbScanUpdate = RESET;	    
	        
	        sprintf(buffer, ">> rcv SCAN-DB Updated...SCAN-RESTART ! ");
			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  
			
            deviceParaConfig();         // ESIO 데이터베이스 : SCAN DB 변경	
            
            pause(1000);
            opr->scanRestart = SET;     // SCAN -TASK 재기동
        }
        else if(esioCFG->dbDeviceUpdate == SET)  
	    {
    	    Debug(console,">> rcv DEVICE-DB Updated...[%02d:%02d:%02d] !\n", rtc->hour, rtc->min, rtc->sec);
	        esioCFG->dbDeviceUpdate = RESET;	   
	        
	        sprintf(buffer, ">> rcv DEVICE-DB Updated...SCAN-RESTART ! ");
			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  
			 
            deviceParaConfig();         // ESIO 데이터베이스 : 계전기 DB 변경	
            
            pause(1000);
            opr->scanRestart = SET;     // SCAN -TASK 재기동
        }
        else if(esioCFG->dbPointUpdate == SET)   
        {
            Debug(console,">> rcv POINT-DB Updated...[%02d:%02d:%02d] !\n", rtc->hour, rtc->min, rtc->sec);
            esioCFG->dbPointUpdate = RESET;
            
            sprintf(buffer, ">> rcv POINT-DB Updated...SCAN-RESTART ! ");
			LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  
			 
            pointParaConfig();          // ESIO 데이터베이스 : 포인트 DB 변경		
        }
    }
    
    return (1);
}	
	

/*
*   FUNCTION : scanDevInitial()
*   - 계전기 장치 구조체 ... 초기화
*/
int wdtDevInitial()
{
    int i;
    SDP_DEVICE  *dev;
    
    /* --------------------------- */
    /*    pointer init             */
    /* --------------------------- */
    for(i = 0; i < MAX_DEVICE; i++)
    {
        dev = (SDP_DEVICE *) deviceCFG[i];

        dev->offlineTick    = 0;
        
        /* 2019.07.17 Device 초기상태 Online */
        dev->online		    = 1;            // 초기상태 ... 미지정.
        dev->devStatus 		|= 0x01;        // Online-Bit SET
        
        dev->init_status    = RESET;        // 장치별 초기화 
        dev->init_analog    = RESET;        // 장치별 초기화 

        /* DNP initialize ... */
        dev->sndFlag        = RESET;
        dev->sndLinkSts     = RESET;
        dev->rcvLinkSts     = RESET;
        dev->timeOutTick    = 0;
        
        dev->selectReq      = RESET;
        dev->operateReq     = RESET;
        dev->anaVOutSelect  = RESET;
        dev->anaVOutOpr     = RESET;
        
        dev->timeSyncReq    = SET;
        
        dev->statusDump     = SET;
        dev->analogDump     = SET;

        dev->classReq       = RESET;
        dev->class1Req      = RESET;
        dev->class2Req      = RESET;
        dev->class3Req      = RESET;

    }
    
    Debug(console,"scan> => DEVICE Initial End...!\n");
    return (0);
}


#define SDP_WDT_CLR   do {  if(  ( write(wdtid, &no_wdt, 1)) <0) perror("wdt clear");  } while(0)

/* ======================================================== */
/*  SDP MAIN 프로그램 Start Routine ....                    */
/* ======================================================== */
int	main(int argc, char **argv)
{
    int     i;
	int		oldsec=0;
	int     size;
	word    cputick;
	char    buffer[256];
	
	int    wdt_ctrl  ;
    int     rc ;


    if ((rc= check_already_running("WDT")) < 0)
    {
        printf("WDT-ERR>> already running..\r\n");
        exit(0);
        
    }

#if 1
    if (  hub_pwer_on() < 0)
    {
        printf("WDT-ERR>> Can't turn on HUB-PWR\r\n");
        exit(0);
    }    
    printf("WDT> HUB POWR ON\r\n");
#endif 


	
    /* ------------------------------------------------ */
	/*  작업 환경을 초기화한다                          */
	/* ------------------------------------------------ */
	if ((termExec = InitEnv()) == 1)
	{
		DisplayLogo();
	}
    
    /* ------------------------------------------------ */
    /*  하드웨어 WDT 기능을 설정함...                   */
    /* ------------------------------------------------ */
    opr->wdtEnable = 1;




    /* --- Check device file exists (driver loaded?) ---------------------- */
    if (access(WDT_DEVICE_PATH, F_OK) != 0)
   {
        printf("[ERROR] %s not found ? is sdp_wdt driver loaded?\n"
                        "        Try: insmod sdp_wdt.ko\n", WDT_DEVICE_PATH);
        no_wdt=1;
		opr->wdtEnable = 0;
		
    }
    else 
    {
    	if ((wdtid=open(WDT_DEVICE_PATH,O_WRONLY|O_NDELAY)) < 0)  
    	{
    		printf("wdt> ***can't open %s\n",WDT_DEVICE_PATH);
    		printf("wdt> but can't reach here\r\n") ;
    		opr->wdtEnable = 0;
            no_wdt=1;    		
    	}        
        else // open success
        {       no_wdt = 0;
                if ( write(wdtid, &no_wdt, 1) < 0 )
                {
                    perror("wdt write");
                    exit(0);
                }
                sleep_ms(100);
                if ( write(wdtid, &no_wdt, 1) < 0 )
                {
                    perror("wdt write");
                    exit(0);
                }
                // enable 
                wdt_ctrl=1 ;
                if (ioctl(wdtid, SDP_WDT_SET_CTRL, &wdt_ctrl) < 0) 
                {
                      perror("ioctl SET_WDT_CTRL(1)");
                      close(wdtid);
                      return 1;
                 }
                sleep_ms(100);
                if ( write(wdtid, &no_wdt, 1) < 0 )
                {
                    perror("wdt write");
                    exit(0);
                }    
    
        }
    }    
#if 0  // hkkim
	if ((wdtid=open(WDT_DEVICE_PATH,O_WRONLY|O_NDELAY)) < 0)  
	{
		printf("wdt> ***can't open %s\n",WDT_DEVICE_PATH);
		opr->wdtEnable = 0;
	}
    else
    {
	    if(ioctl(wdtid,PMU_WDT_CTR_ON ,  NULL) < 0 )
	  	{  	
			printf("wdt> ioctl fail\n");
		}
    }
#endif     
	/* ------------------------------------------------ */
    /* 내부변수 초기화                                  */
    /* ------------------------------------------------ */
    init_variable();
    init_esio_update();                 /* ESIO program update : version + request init */

// hkkim    
//    ioctl(wdtid, PMU_WDT_CTR_CLR, NULL);   /* WDT Clear.... */
     SDP_WDT_CLR ;       
	/* ------------------------------------------------ */
    /* RTU 데이터베이스 화일 로드 & 초기화              */
    /* ------------------------------------------------ */
    size = sizeof(RTU_DATABASE);
    if(dbFileRead_ESIO(rtudb) < 0)
    {
        printf("\n=============================\n");
        printf("wdt> *** RTUDB Read Error...!\n");
        printf("=============================\n");
        opr->romDBFail = 1;
        bzero(rtudb, sizeof(RTU_DATABASE));
        
        dbFileWrite_ESIO(rtudb);
    }
    else
    {   
        printf("\n=============================\n");
        printf("wdt> *** RTUDB Read OK... size = %d !\n", size);
        printf("=============================\n");
        pause(100);
        
        readRTUdb();            // DB 초기화...
    }

    /* ------------------------------------------------ */
    /*  ESIO 모듈 타입에 따른 운영 프로세스 관리        */
    /* ------------------------------------------------ */
	opr->stscode = 0;

    printf(" %s WDT-MAIN PROCESS Activated ...ESIO-ID => %d !\n", TARGET_NAME, opr->boardID);

    /* ------------------------------------------------ */
    /*  ESIO 기동 Event...                               */
    /* ------------------------------------------------ */
    logEvent_MPU(shmPtr, ENT_NOT_DEFINED, 0, 0, 0, 0, NULL); 
    logEvent_MPU(shmPtr, ENT_CU_RESTART, 0, 0, 0, 0, NULL);     // MPU 재기동 Event...
    
    /* LOG File 저장 */
    sprintf(buffer, "%s", "=====================================================");
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));
    
    sprintf(buffer, "ESIO[%d] 재기동----------------", opr->boardID);
    LogFile_MPU (shmPtr, ENT_CU_RESTART, buffer, strlen(buffer));
    
    sprintf(buffer, "%s", "=====================================================");
    LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));

    /* ---------------------------------------------------- */
	/*  ESIO Network  환경정보 초기화 ...                   */
    /* ---------------------------------------------------- */
    esioNetwork_Initial();
    
    
    vmeSioDCB->netChangeReq = 0x1234;  // VME 상으로 MPU에게 Network 구성정보 요청...
    
    /* ------------------------------------------------ */
    /*  VMESIO 활성화 ...                               */
    /*   - OutEnable : SET, - SIOEnable : SET           */
    /* ------------------------------------------------ */
    opr->stscode = (ESIO_LED_OUTENB + ESIO_LED_SIOOUT);
    opr->runMode = LOCAL_SLAVE; 
    cputick = 0;
// hkkim    
//    ioctl(wdtid, PMU_WDT_CTR_CLR, NULL);   /* WDT Clear.... */
    SDP_WDT_CLR ;
    /* ------------------------------------ */
    /*  ESIO# : SCAN Device 정보            */
    /* ------------------------------------ */
    printf("=> ESIO(%d)-SCAN device Num = %02d ... ", opr->boardID, esioCFG->scanMaxNum);
    for(i=0; i< esioCFG->scanMaxNum; i++)  printf(" %2d", esioCFG->scanDevice[i]);
    printf("\n");
    
    /* 2019.07.17 WDT 기동시...DEVICE 상태 초기화 */
    wdtDevInitial();
    vmeSioDCB->esioState = ESIO_ST_RUN;
    
	while(termExec)
	{
        //SDP_WDT_CLR ;	    
	    readClock();
	    sleep_ms(100);
	    
	    /* ---------------------------------------- */
        /* SIO Active 상태 표시 => CPU 확인     			*/
        /* ---------------------------------------- */
        vmeSioDCB->sioValid = 0x1234;                       // Valid 정보
        vmeSioDCB->sioCount = vmeSioDCB->activeCount;       // activeCount => sioCount로 대치
        check_MPU_command();                                /* MPU exit request (every 100 ms) */
        cputick = vmeSioDCB->activeCount;
        
        check_ESIO_mode(cputick);       // ESIO 동작모드 구분 ...MASTER/SLAVE
        
	    /* ---------------------------------------- */
        /* VME-CPU와의 시각동기 ...            			*/
        /* ---------------------------------------- */
        if(opr->rtcUpdateFlag)
        {
            opr->rtcUpdateFlag = 0;
            writeClock( opr->year, opr->month, opr->day, opr->hour, opr->min, opr->sec, opr->week);
            readClock();
            
            if(esioCFG->mpuTimeSyncOK == RESET)
    		{
		    	sprintf(buffer, "=> MPU TIME-SYNC OK (from : %s) !", esioCFG->mpuIPAddr);
				LogFile_MPU (shmPtr, ENT_NOT_DEFINED, buffer, strlen(buffer));  
    		}
    	
    		esioCFG->mpuTimeSyncOK = SET;
    
		    logEvent_MPU(shmPtr, ENT_HOST_TIMESET, 0, 0, 0, 0, NULL);     // MASTER-MODE Event...
        }

	    esio_LED_Update();			// ESIO 모듈 : LED 구동부
	    
	    /* ---------------------------------------- */
       	/*  제어기 WDT & System-REBOOT Check        	*/
	    /* ---------------------------------------- */

        if(opr->wdtEnable == SET)  // wdtEnable 은 scanRestartcount가 일정 수 넘으면 clear되고..따라서 reset 된다.
	    {
     //       if(ioctl(wdtid,PMU_WDT_CTR_CLR,NULL) < 0 )  Debug(console, "wdt> *** ioctl fail\n");
#if 0                
                if ( write(wdtid, &no_wdt, 1) < 0 )
                {
                    perror("wdt write");
                    //exit(0);
                }    
#endif 
            SDP_WDT_CLR  ;              
                
        }      
         
        /* ============================================ */
        /*  초단위 처리루틴                 					*/
        /* ============================================ */
        if(oldsec == rtc->sec)   continue;
        oldsec = rtc->sec;

#if 0
        /* -------------------------------------------- */
        /* NTP Time GET...                          	*/
        /* -------------------------------------------- */
        if(opr->getClockNTP == SET)
	    {
	        opr->getClockNTP = 0;
	        get_NTP_Info();
	    }
#endif

	    /* -------------------------------------------- */
        /*  VMEBUS 상의 Network 정보 초기화             		*/
        /* -------------------------------------------- */      
        check_ESIO_Network();
        
        /* -------------------------------------------- */
        /*  61850 계전기 정보 초기화                    		*/
        /* -------------------------------------------- */
        if(opr->esioID == SDP_ESIO_61850)
        {
        	update_Status_61850();
            update_Analog_61850();
            
            /* 61850 계전기 통신상태 Check...	 */
            if((oldsec %5) == 0)	update_Online_61850();	
            	
            /* ---------------------------------------- */
	        /* 사용자 상태지정.. CONSOLE ......             	*/
    	    /* ---------------------------------------- */        
            if(opr->testAnalogFLAG == SET)  testAnalog_61850();
            if(opr->testStatusFLAG == SET)  testStatus_61850();
        }
        
        /* -------------------------------------------- */
        /*  PROCESS 동작상태 감시                           	*/
        /* -------------------------------------------- */                           
		CheckProcess();					// 프로세스 동작 상태 감시
		
		/* -------------------------------------------- */
        /*  데이터베이스 변동사항 감시                      		*/
        /* -------------------------------------------- */ 
        update_ESIO_DB();
        
	}

    printf("\n============================\n");
    printf("    *** WDT Disable...!       \n");
    printf("============================\n");
// hkkim    
//    if(ioctl(wdtid,PMU_WDT_CTR_OFF, NULL) < 0 )  printf("wdt> *** ioctl fail\n");
    
	/*  작업 환경을 정리한다    */
	ClearEnv();
	return (0);
	
}
