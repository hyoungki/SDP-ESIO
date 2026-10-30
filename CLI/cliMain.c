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

#include    "cli.h"

int	        termExec;

SHM_DESC	shmDesc = {-1, SHM_KEY, sizeof(SHM_MEMORY), NULL};
SHM_MEMORY	*shmPtr = NULL;
TASK_INFO	*taskPtr = NULL;

RTC             *rtc    = NULL;
OPR_MSG         *opr    = NULL;
CONSOLE_INFO	*console= NULL;

MPU_CONFIG      *mpuCFG  = NULL;                 // MPU Config
ESIO_CONFIG     *esioCFG;                       // ESIO 장치 Config

//POINT_BUF       *pointCFG[MAX_DEV_POINT];       // SDP 포인트 Config 정보
SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];        // 하위계전기 SCAN Config

SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
DEVICE_61850    *device61850[MAX_DEVICE];       // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
	 
RTU_DATABASE    *rtudb  = NULL;                 // SDP 데이터베이스
HISTORY_QUE     *hque   = NULL;                 // CONSOLE 용 이벤트

int     passTimeCLI;

/* THREAD 관련 변수 정의 */
void            *debug_thread(int  *arg);
THREAD_ENTRY    debugTHREAD;

extern  void InputProcessing(void);

//
// 모듈:	SigHandler()
//
void SigHandler(int sig)
{
    if(opr->wdtDebug)	printf("cli> ... signal generated (%2d)...!\n", sig);
    pause(100);
    
	switch(sig)
	{
    case SIGTERM:
        termExec = 0;
        if(opr->wdtDebug)   printf("cli> ... signal [SIGTERM] generated (%2d)...!\n", sig);
        pause(1000);     
        break;
	        
	case SIGBUS : 
	    termExec = 0;
        if(opr->wdtDebug)   printf("cli> ... signal [SIGBUS] generated (%2d)...!\n", sig);
        pause(1000);     	    
	    break;
		    
	case SIGSEGV: 
	    termExec = 0;
        if(opr->wdtDebug)   printf("cli> ... signal [SIGSEGV] generated (%2d)...!\n", sig);
        pause(1000);     	    
	    break;
		    
	case SIGPIPE: 
        if(opr->wdtDebug)   printf("cli> ... signal [SIGPIPE] generated (%2d)...!\n", sig);
        pause(1000);     	    
		break;

	default : 
	    if(opr->wdtDebug)	printf("cli> ... signal[%2d] generated ...!\n", sig);
		break;
	}
}


//
// 모듈:	ClearEnvironment()
//
void ClearEnv(void)
{
	// 공유 메모리 설정 해제
	if (shmPtr != NULL)
		ShmDetach(&shmDesc);

    //PortClose(&consolePort);
	printf("[*] Close Device ....4!\n");
}
					    

//
// 모듈:	InitEnvironment()
//
int InitEnv( void)
{
    int     i;
    
	// SIGNAL 처리 루틴 설정
	signal( SIGBUS,  SigHandler);
	signal( SIGSEGV, SigHandler);
	signal( SIGTERM, SigHandler);

	// 공유 메모리 상태 확인
	if (ShmCheck(&shmDesc) < 0)
	{
		printf("cli> *ERR_공유 메모리 상태 이상\n");
		return(0);
	}
	
	// 포인터 변수 초기화
	shmPtr = (SHM_MEMORY *) shmDesc.address;
	
	taskPtr = &shmPtr->taskInfo[CLI_PROCESS];

    rtc     = (RTC *)       &shmPtr->rtc;
	opr     = (OPR_MSG *)   &shmPtr->opr_msg;
	
	mpuCFG  = (MPU_CONFIG *) &shmPtr->mpuConfig;                // MPU Network 구성정보
    esioCFG = (ESIO_CONFIG *) &shmPtr->esioConfig;
    
	rtudb   = (RTU_DATABASE *) &shmPtr->rtuDatabase;
    console = (CONSOLE_INFO *) &shmPtr->console; 
    
	hque    = (HISTORY_QUE *)  &shmPtr->localHistoryQ; 

    /* ------------------------------------ */
    /* ESIO CFG : ESIO 구조체 (MMAX 5)      */
    /* ------------------------------------ */ 
    for(i=0; i< MAX_SCAN_PORT; i++) scanCFG[i] = (SCAN_CONFIG *) &shmPtr->scanCFG[i];

    /* ------------------------------------ */
    /* 계전기 구조체 : 전체 계전기(MAX 64)  */
    /* ------------------------------------ */    
    for(i=0; i< MAX_DEVICE; i++)   
	{
	    deviceCFG[i]   = (SDP_DEVICE *) &shmPtr->deviceCFG[i];
	    device61850[i] = (DEVICE_61850 *) &shmPtr->device61850[i];
	}

	//for(i=0; i< MAX_DEV_POINT; i++) pointCFG[i] = (POINT_BUF *) &shmPtr->pointCFG[i];
    
	return(1);
}

#define  ESIO_COMPILED_DATE_STRING "2026-09-02 오전 9:54:04"

//
// 모듈:	DisplayLogo()
//
void DisplayLogo(void)
{
	ExecCommand("clear");
	printf("\t\t////////////////////////////////////////////////////////////\n");
	printf("\t\t//        %s ESIO[#%d] USER Debugger         //\n", TARGET_NAME, opr->boardID );
//	printf("\t\t//        [*] Designed by Ace Control Co.,Ltd.(THE-ROAD)       //\n");  pause(100);
	printf("\t\t//        [*] Version   %s                       //\n", VERSION_STRING);
	printf("\t\t//        [*] %s         //\n", SIMULATOR_NAME);  
	printf("\t\t//        [*] Update Date ...%s                   //\n", ESIO_COMPILED_DATE_STRING); // 날짜 변경
	printf("\t\t////////////////////////////////////////////////////////////\n\n");
	
 }

/*
*   NETWORK Debug MAIN() 
*/
void debug_thread_call()
{
    int     i;     
    int	    state;

    i = 0;
    
    debugTHREAD.tid        = i;
    sprintf(debugTHREAD.name, "%s%d","DEBUG", i);
    debugTHREAD.priority   = 0;
    debugTHREAD.options    = 0;
    
    debugTHREAD.entryPt    = debug_thread;  // MODBUS-THREAD
    debugTHREAD.arg        = 0;
    
    state = pthread_create( &debugTHREAD.tid, NULL, debugTHREAD.entryPt, &i);
   	if(state != 0)
    {
	    printf("CLI> *Thread# [%s] Create Error ...!\n", debugTHREAD.name);
	    exit(1);
    }
	
	pause(100);
}
void dispDB_SIZE(void)
{
#if 0    
typedef struct
        {
        //      DB_MPU_CONFIG2  기존 ESIO 와 호환을 위한 것..       
        DB_ESIO_MPU_CONFIG		esio_mpuConfig;					        // SDP : MPU Network 구성정보 & Parameter
        DB_ESIO_CONFIG      esioConfig[MAX_DB_ESIO];            // SDP : ESIO Network 구성정보 & Parameter, MAX 6
		DB_HOST_CONFIG 		hostCfg[MAX_DB_HOST];		        // SDP : HOST 구성정보 & 운영파라메터, MAX 8

        DB_ICCP_CONFIG      iccpConfig;                         // SDP : ICCP HOST 구성정보 & Parameter
        
        byte    portdb[32];                			            // SDP : HARRIS-HOST 구성정보
        byte    chassisNum;                 			        // SDP : LANDIS Chassis Number
        byte    chassis[16][16];            			        // SDP : LANDIS RTU 구성정보
        
        DB_MODBUS_PROFILE	modbusProfile[MAX_DB_MODBUS_PROFILE];  // SDP : MODBUS 프로파일 구성정보, MAX 16
        
        DB_SCAN_CONFIG	scanConfig[MAX_DB_SCAN_PORT];	            // SDP : 계전기 통신포트 구성, MAX 32
        DB_SDP_DEVICE   deviceConfig[MAX_DB_DEVICE];	            // SDP : 계전기별 운영 구성, MAX 112
        DB_POINT_BUF    pointBuf[MAX_DB_POINT]; 	            	// SDP : 운영 포인트 구성, MAX 4096
        
        DB_CAL_POINT        calPointBuf[MAX_DB_CAL_POINT];         	// SDP : 연산포인트 구성, MAX 32
		
		byte    chksum[2];
        
	    } __attribute__ ((packed)) ESIO_RTU_DATABASE;
#endif 	    
	    printf("SIZEOF(RTU_DATABASE)  %d\r\n",sizeof(RTU_DATABASE ))  ;
	    printf("SIZEOF(DB_MPU_CONFIG) %d \r\n",sizeof(DB_MPU_CONFIG ))  ;
        printf("MAX_DB_ESIO %d\r\n",MAX_DB_ESIO);
	    printf("SIZEOF(DB_ESIO_CONFIG) %d \r\n",sizeof(DB_ESIO_CONFIG ))  ;
	    printf("SIZEOF(DB_HOST_CONFIG) %d \r\n",sizeof(DB_HOST_CONFIG ))  ;
        printf("MAX_DB_HOST %d\r\n",MAX_DB_HOST);

	    printf("SIZEOF(DB_ICCP_CONFIG) %d \r\n",sizeof(DB_ICCP_CONFIG ))  ;
	    printf("SIZEOF(DB_MODBUS_PROFILE) %d \r\n",sizeof(DB_MODBUS_PROFILE ))  ;	    	    	    

	    printf("SIZEOF(DB_SCAN_CONFIG)  %d\r\n",sizeof(DB_SCAN_CONFIG ))  ;
	    printf("SIZEOF(DB_SDP_DEVICE)  %d\r\n",sizeof(DB_SDP_DEVICE ))  ;
	    printf("SIZEOF(DB_POINT_BUF)  %d\r\n",sizeof(DB_POINT_BUF ))  ;
	    printf("SIZEOF(DB_CAL_POINT)  %d\r\n",sizeof(DB_CAL_POINT ))  ;
  	       
    
}
 
	
 
	
/* ------------------------------------------------------- */
/*  신호제어기 MAIN 프로그램 Start Routine ....             */
/* ------------------------------------------------------- */
int	main(int argc, char **argv)
{
	// 작업 환경을 초기화한다
	if ((termExec = InitEnv()) == 1)
	{
		DisplayLogo();
	}
		
    printf("...CLI start...!\n");

// dispDB_SIZE();

#ifdef  NETWORK_CONSOLE

	debug_thread_call();

#endif
	
	console->front = 0;
	console->rear  = 0;
	console->active = SET;
	
    passTimeCLI = 0;
    
	while(termExec)
	{
		InputProcessing();
		usleep(100000);		/* 100 msec delay */
		
	}

    console->front = 0;
	console->rear  = 0;
	console->active = RESET;
	
	// 작업 환경을 정리한다
	ClearEnv();
	
	return (0);
}
