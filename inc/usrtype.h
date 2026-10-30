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

/* ---------------------------------------- */
/* HISTORY & EVENT 저장용 메모리 영역       */
/* ---------------------------------------- */
#define SRAM_START_ADDRESS      0x60000000         /* this definition is related with dd and cpld */
#define SRAM_SIZE               0x200000           /* 2MK */

#define MRAM_START_ADDRESS     	0x80000000         /* this definition is related with dd and cpld */
#define MRAM_SIZE              	0x80000            /* 512K */

/* -------------------------------------------- */
/*  이중화 절체기 상태 정보                     */ 
/* -------------------------------------------- */
#define MANUAL_MODE         0
#define AUTO_MODE           1  

#define SYSTEM_A            0                
#define SYSTEM_B            1

#define CHANGE_S_TO_M       1       // Slave  에서 Master 로 절체
#define CHANGE_M_TO_S       2       // Master 에서 Slage  로 절체

#define CHANGE_ALL_DEVICE   1       // 전체 계전기 통신상태 
#define CHANGE_EACH_DEVICE  2       // 개별 계전기 통신상태


/* ---------------------------------------------------- */
/*  SIO 전면 Front & 내부 기능 IO                       */
/* ---------------------------------------------------- */  
#define ESIO_LED_OUTENB      0x01                // SIO 출력 Enable (Shutdown OFF)
#define ESIO_LED_SIOOUT      0x02                // SIO Install & Activity 
//#define	ESIO_LED_RUN		 0x10                // RUN-LED

#define	ESIO_LED_RUN		 0x01                // RUN-LED
 
/* ---------------------------------------- */
/*  MPU 상태정보                            */
/* ---------------------------------------- */  
#define MPU_SDP_A_BIT       0x04                // MPU 상태정보 : SDP-A, SDP-B 동작
#define MPU_INS_BIT         0x02                // CPU Status : 장착여부 [0 : 이상, 1: 장착]
#define MPU_ACT_BIT         0x01                // CPU Status : Master 동장모드 [0:Slave, 1: Master]


typedef struct
    {
        short   sec;
        short   min;
        short   hour;
        short   week;
        short   day;
        short   month;
        short   year;
        //long    rtctick;

    } __attribute__ ((packed)) RTC ;


typedef struct
    {
        int     esioID;                     // ESIO# 보드 - 장치 구분자, 0: SDP_NULL. 1:SDP_MPU, 2:SDP_SCADA, 3: SDP_DIAG, 4: SDP_EQ, 5:SDP_61850
        int     boardID;                    // ESIO# 보드 - VMEBUS 상의 Address
        //short   dipSwitch;                  // ESIO# 보드 - DIP-SWITCH, MPU-A/B 결정
        
        /* -------------------------------- */
        /*	CPU 이중화 설정 파라메터		*/
        /* -------------------------------- */
        short   scanRestart;                // SCAN : TASK 재기동 Flag
        word    scanWDT[MAX_SCAN_PORT];     // SCAN : 통신 Thread

        short   stsDumpPeriod;              // SCAN : 상태정보 Dump 주기
        short   anaDumpPeriod;              // SCAN : 아날로그 정보 Dump 주기
        short   soeDebounce;                // SCAN : MODBUS SOE Check 용
        short   sndSOECount;
        
        short   cpuMode;                    // MPU-A, B 상태
        short   runMode;                    // MASTER, SLAVE 동작상태
        short	vmeNetworkInit;				// VME Network 초기화
        //short   rtuinit;                    // RTU 초기화 상태
        
        short   wdtEnable;                  // SDP : H/W WDT Clear 운영모드
        short   romDBFail;
        
        byte    stscode;                    // ESIO 전면 상태 LED status
        
        short   max_sdpPoint;

        /* -------------------------------- */
        /*  EVENT 생성 및 저장 관련         */
        /* -------------------------------- */
        //short   nramAccess; 
        short   eventLogging;           // EVENT 생성중 
        
        /* SIMULATOR 용 이벤트 Queue */
        short   simStatus;                  // SIMULATOR : 상태정보
        short   rcvSimSeq;                  // SIMULATOR : 수신 Sequence No
        
        /* -------------------------------- */
        /*  RTU-Update & Time Sync 관련     */
        /* -------------------------------- */
        short   year;
        short   month;
        short   day;
        short   week;
        short   hour;
        short   min;
        short   sec;
        short   milisec;
        
        //short   getClockNTP;                // NTP Server - Time Sync 
        short   rtcUpdateFlag;              // TIME-Sync 요청 Flag
        
        /* -------------------------------- */
        /* CPU 이중화 관련 변수             */
        /* -------------------------------- */
		//short	dbDownFlag;     
		//short	dbCheckFlag;
		
        //short   pointDBDown;			    // POINT 구성정보 Down Flag
        //short   hostDBDown;				    // HOST 구성정보 Down Flag
        //short   scanDBDown;				    // SCAN 구성정보 Down Flag
        //short   devDBDown;                  // DEVICE 구성정보 Down Flag   
        //short	modbusDBDown;			    // MODBUS 구성정보 Down Flag
        
        /* ---------------------------- */
        /*  61850 계전기 ...            */
        /* ---------------------------- */
        short   db61850Change;              // 61850 계전기 DB 변경시... SET
        
        
        /* TEST 관련 변수       */
        short   testAnalogFLAG;
        short   testStatusFLAG;
        short   testOnlineFLAG;
        
        short   testDevid;
        short   testPoint;
        short   testState;
        float   testAIData;
        short   testFlag;
        
        short   rcvONtime;                  // 상위 수신한 계전기 제어시간
        short   rcvOFFtime;                 // 상위 수신한 계전기 제어시간
        
        short   cntrDevId[MAX_SCAN_PORT];
	    short   cntrFlag[MAX_SCAN_PORT];
	    short   cntrComTick[MAX_SCAN_PORT];     // 계전기 : 제어명령 송수신 카운트
	    
	    //short   hostCfgDown_OK;
        //short   esioCfgDown_OK;             // SIMULATOR : ESIO-Config Down OK
        //short   devCfgDown_OK;              // SIMULATOR : Device-Config Down OK
        //short   scanCfgDown_OK;             // SIMULATOR : SCAN-Config Down OK
        //short   pointCfgDown_OK;            // SIMULATOR : Point-Config Down OK

        //short   calPt_CfgDown_OK;
		short	scanInitial;
		short	scanRestartCount;
		
		//short	scanRestartTick;
		                         
		short	logFileWrite;			// LOG 화일 저장중 Flag.... 
				                         		
        /* -------------------------------- */
        /* 내부 운영 DEBUG 관련 변수        */ 
        /* -------------------------------- */       
        short   wdtDebug;
        short   mpuDebug;
        short   simDebug;   
        short   lineDebug;
        short   dnpDebug;           // DEVICE# 통신상태 Debug
        short   msgDebug;           // DEVICE# 메세지  Debug
        short   scanDebug;          // SCAN#  통신상태 Debug
        short   soeDebug;           // SOE Event Message Debug
        short   modDebug;           // MODBUS Message Debug 

#if 1	// 2016.07.25 ChoiBC Modify for 61850, ntp Debug(CLI)
        short   mmsDebug;
		short   c61850Debug;		// 61850 Task Debug
        short   ntpDebug;			// ntpClient Task Debug
		short	ntpKill;			// ntpClient Task Kill
									// 61850Client Task가 Set하면, ntpClient Task는 Reset하고 죽는다.
#endif
        
    } __attribute__ ((packed)) OPR_MSG;

