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

#ifndef	RTUBUF_HEADER_INCLUDED
#define	RTUBUF_HEADER_INCLUDED

//
// Include Files
//
#include "localLib.h"

/* ESIO# ID 별 보드 기능 할당 */
#define SDP_NULL            0           // ESIO 장치 TYPE : 미정의
#define SDP_MPU             1           // ESIO 장치 TYPE : 주제어부
#define SDP_ESIO_SCADA      2           // ESIO 장치 TYPE : 전철제어
#define SDP_ESIO_DIAG       3           // ESIO 장치 TYPE : 원격진단
#define SDP_ESIO_EQ         4           // ESIO 장치 TYPE : 전력품질
#define SDP_ESIO_61850      5           // ESIO 장치 TYPE : 61850

#define SDP_SIO_ASYNC       6           // SIO 장치 TYPE : 전용선 SIO


/*
* ----------------------------------------------------------
*   MPU_CONFIG : MPU 운영 Buffer
* ----------------------------------------------------------
*/
typedef struct
    {
        byte    useFlag;            // Network# 사용유무
        byte    reserved;           // 예약
        
        char    ipAddr[16];         // Network# IP-Address
        char    gwAddr[16];         // Network# GW-Address
        char    subMask[16];        // Network# SUB-Mask

    } __attribute__ ((packed)) MPU_NET_ENTRY;


typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */  
        byte    sdpNameStr[20];         // SDP 장치이름
        MPU_NET_ENTRY   master_netCfg[3];
        MPU_NET_ENTRY   slave_netCfg[3];
        
        byte    dualMpu;                // MPU Parameter : CPU 이중화 운영모드
        byte	dualModule;             // MPU Parameter : Module 이중화 운영모드 
        byte    scuUseFlag;        	    // MPU Parameter : SCU 모듈 사용유무
        byte	mmiUseFlag;             // MPU Parameter : MMI 모듈 사용유무
        
        byte    statusDump;        	    // MPU Parameter : STATUS Dump 주기
       	byte    analogDump;        	    // MPU Parameter : ANALOG Dump 주기
        byte    debounce;               // MPU Parameter : Function 코드#1
        byte    dbCheck;                // MPU Parameter : Function 코드#2
        byte    hostWDT;                // MPU Parameter : Function 코드#3
        byte    iccpFlag;               // MPU Parameter : Function 코드#4
        
        byte    func1;
        byte    func2;
        byte    func3;
        byte    func4;
        byte    func5;
        byte    func6;

        /* ------------------------ */
        /*  MPU 운영 Parameter      */
        /* ------------------------ */  
        byte    linkStatus;             // MPU LINK 상태정보
        
        byte    mpuStatus;              // MPU 상태정보 status, MASTER/SLAVE, SDP-A/SDP-B     
        byte    mpuRackSts;             // MPU RACK 실장정보
        byte    rcvMpuSts;              // 수신 MPU 동작상태
        byte    rcvRackSts;             // 수신 MPU RACK 실장상태
        
    } __attribute__ ((packed)) MPU_CONFIG;
    
/* ---------------------------------------------------- */
/* STURECTURE 정의 : DB_ESIO_CONFIG                     */
/* - ESIO Network 구성정보 및 운영 Parameter DB         */
/* ---------------------------------------------------- */       
typedef struct
    {
        byte    useFlag;            // Network# 사용유무
        char    ipAddr[16];         // Network# IP-Address
        char    gwAddr[16];         // Network# GW-Address
        char    subMask[16];        // Network# SUB-Mask

    } __attribute__ ((packed)) ESIO_NET_ENTRY;

typedef struct
    {
        byte    useFlag;            // PORT# 사용유무
        byte    function;           // PORT# Function
        byte    comMode;            // PORT# 통신모드, 0:RS232, 1:MODEM, 2:RS485, 3:TCPIP
        byte    comSpeed;           // PORT# 통신속도, [0] 1200 ~ [7] 115200
        
        byte    portNameStr[20];    // PORT# 포트이름

    } __attribute__ ((packed)) ESIO_PORT_ENTRY;


typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;                // ESIO Parameter : ESIO 사용유무
        byte	targetID;               // ESIO Parameter : ESIO Target Module-ID, 0:사용않함, 1: ESIO1, 2:ESIO2, 3:ESIO3, 4:ESIO4, 5:SIO
        byte    linkMth;                // ESIO Parameter : MPU 연계방식
        byte    comDelay;               // ESIO Parameter : MPU 통신 지연 (10ms)
        
        byte    esioNameStr[20];        // ESIO 장치이름
        
        // 2026-05-26 오후 5:35:44  3 (?) --> 4
        DB_ENET_ENTRY   mstNetConfig[4];
        DB_ENET_ENTRY   slvNetConfig[4];
        
        ESIO_PORT_ENTRY portConfig[MAX_ESIO_PORT];  // ESIO Parameter : PORT Config
        
        /* ------------------------ */
        /*  APP-프로그램 참조       */
        /* ------------------------ */
        short	mpuTimeSyncOK;				// MPU 시각동기 여부
        
        short   modInit_OK;                 // 61850 : ESIO 할당 계전기 DB 초기화 완료 ... [0] 초기화 않됨, [1] 초기화 완료
        //short   scanEnable;                 // ESIO 계전기 통신 활성화 ... [0] scan 금지, [1] SCAN 허용
                
        short   online;                     // MPU  통신상태
        byte    cpuStatus;                  // CPU 상태정보 => READ
        byte    rcvStatus;                  // 수신 MPU 상태정보 => READ
        
        //byte    sndSeqNo;
        byte    rcvSeqNo;
        
        short   scanIndex;                  // ESIO 계전기 Index
        short   scanMaxNum;                 // ESIO 등록된 계전기 수
        short   scanDevice[MAX_DEVICE];     // ESIO 계전기 리스트
        
        short   cntFlag;                    // 대상 계전기 제어 Flag
        short   cntDev;                     // 대상 계전기 번호 [1..64]        
        short   cntPoint;                   // 대상 포인트 [1..256]            
        short   cntTCF;                     // 제어상태, [1] TRIP, [2] CLOSE   
        
        short   dbChanged;                  // ESIO 데이터베이스 변경 여부
        
        short   dbMpuUpdate;                // ESIO 데이터베이스 : MPU CFG 변경   
        short   dbEsioUpdate;               // ESIO 데이터베이스 : ESIO CFG 변경    
        short   dbModbusUpdate;             // ESIO 데이터베이스 : MODBUS DB 변경
        short   dbScanUpdate;               // ESIO 데이터베이스 : SCAN DB 변경
        short   dbDeviceUpdate;             // ESIO 데이터베이스 : DEVICE DB 변경
        short   dbPointUpdate;              // ESIO 데이터베이스 : 포인트 DB 변경
        
        /* ------------------------------------ */
        /*  TCP/IP 통신 : DEVICE 별 구조체      */
        /* ------------------------------------ */
        struct sockaddr_in	serv_addr;
        struct sockaddr_in	clientPtr;
        
        short   SocketFd;           /* for Client */
        //short   SocketID;           /* for Server */
        short   connectStatus;
        short   connectFail;
        //short   comFailTick;
        
        int     threadActive[2];
        int     acceptPORT[2];
        int     portResetCount;
        
        char    mpuIPAddr[16];  
        short   mpuPort;
        
        byte    mpuRxbuf[2][4096];
        byte    mpuTxbuf[2][4096];
        
        
        
    } __attribute__ ((packed)) ESIO_CONFIG;
        
        
/* ---------------------------------------------------- */
/* STURECTURE 정의 : POINTBUF                           */
/* 1. DI/DO/AI등 CU내 포인트 운영을 위한 스트럭쳐       */
/* ---------------------------------------------------- */   
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */     
        byte    ptNameStr[40];      // SDP POINT : 포인트 이름

        byte    devNo;              // SDP POINT : device 번호 [1..32] */
        word    devPt;              // SDP POINT : device 포인트 번호 [1..1024] */
        byte    devType;            // SDP POINT : devic TYPE  */    

        byte	iccpType;		    // SDP POINT : ICCP 포인트 TYPE, SDI/SDO/SAI/DDI/DAI/QDI/QAI/TDI/TAI/DEV
		word	iccpIndex;	        // SDP POINT : ICCP 포인트 인덱스, [0: 미지정, 1 ~ 1024]
        byte	iccpRes1;	        // SDP POINT : ICCP 예비
        byte	iccpRes2;	        // SDP POINT : ICCP 예비
        
        byte    ptType;             // SDP POINT : device 포인트 TYPE  */     
        byte    ptConfig;           // SDP POINT : Point Config
        
        word    dbmax;              // SDP POINT :Point MAX Scale 값
        word    pointOffset;        // SDP POINT :Point Offset Scale 값
        byte    pointDelta;         // SDP POINT :Point Delts Scale 값
        
        word    localIndex;
        
     	word	modBase;			// SDP POINT :MODBUS base Address
     	word	modIndex;		    // SDP POINT :MODBUS index
             
        byte    dbport;             // SDP POINT :주장치 PORT 번호  [1..16]
        byte    dbpoint;            // SDP POINT :주장치 POINT 번호 [1..64] 

        word    hostIndex[MAX_HOST];       // SDP POINT : 상위 호스트#1 Index 번호 [1..1024]

		byte    onStr[10];      	    // SDP POINT : ON  상태 이름
		byte    offStr[10];      	// SDP POINT : OFF 상태 이름
        
        
        /* ------------------------ */
        /*  APP-프로그램 참조       */
        /* ------------------------ */
        byte    runStatus;          // SIM : 계전기 동작상태 
        byte    config;             // device 포인트 : 포인트 사용여부 ... 
        byte    costick;            /* Point previous status */   
        byte    cos;
        byte    debounce;           // Point Debounce Time [ms]     
        word    cntrTime;           // Point Control Delay Time [ms]  
                   
       	/* ------------------------ */
        /* POINT 운영 정보 ...      */
        /* ------------------------ */
        byte    status;             /* Point status */   
        byte    oldsts;             /* 이전 상태값 */
        int     pointData;          /* Analog Point Data */
        float	floatData;			/* FLOAT 데이터 */
        
        struct timeval	updateTime;		// 포인트 정보 Update Time
        
    } __attribute__ ((packed)) POINT_BUF;



/* ---------------------------------------------------- */
/* SCAN PORT 별 DEVICE Config                           */
/* ---------------------------------------------------- */
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;                // SDP SCAN : Channel 사용유무
        byte	targetID;               // SDP SCAN : ESIO Target Module-ID, 0:사용않함, 1: MPU, 2: ESIO1, 3:ESIO2, 4:ESIO3, 5:ESIO4, 6:SIO
        
        byte    protocol;               // SDP SCAN : SCAN 통신 프로토콜 
        byte    comMode;                // SDP SCAN : SCAN 통신모드
        byte	comPort;			    // SDP SCAN : SCAN 통신포트
        byte    comSpeed;               // SDP SCAN : SCAN 통신속도

        byte	comDelay;			    // SDP SCAN : SCAN 통신간격
        byte	offCount;			    // SDP SCAN : SCAN Offline Count
        byte    chgMode;                // SDP SCAN : SCAN Change Mode
        
        byte    function1;              // SDP SCAN : SCAN Function#1
        byte    function2;              // SDP SCAN : SCAN Function#2
        byte    function3;              // SDP SCAN : SCAN Function#3
        
        byte	scanNameStr[20];		// SDP SCAN : SCAN Name String
        
        /* ------------------------ */
        /*  APP-프로그램 참조       */
        /* ------------------------ */
        short   scanid;                 // SDP SCAN : SCAN 구조체 id
        short   devNum;                 // SDP SCAN : SCAN 할당 계전기 수
        short   device[MAX_DEVICE];     // SDP SCAN : SCAN 할당 계전기 LIST
        
        short   cntrDevID;              // SDP SCAN : SCAN 제어대상 계전기 번호
	    short   cntrFlag;               // SDP SCAN : SCAN 제어 요청
	    short   cntrComTick;            // SDP SCAN : SCAN 제어 Timeout
	    
	    short   scaninit;
	    
        /* ------------------------ */
        /* SCAN 통신 Buffer         */
        /* ------------------------ */
        byte    sndBuffer[256];
        byte    rcvBuffer[512];

                                    
    } __attribute__ ((packed)) SCAN_CONFIG;
        
        
/* ---------------------------------------------------- */
/* STURECTURE 정의 : MODBUS_READ/WRITE Block            */
/*  - MODBUS Protocol 구성정보                          */
/* ---------------------------------------------------- */   
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;           	// READ Block : 사용여부 
        byte    opcode;           	// READ Block : OPCODE 
        word    baseAddr;   		// READ Block : BASE Address 
        word    baseOffset;   		// READ Block : Offset Address
		word	dataNum;			// READ Block : Read Point Number
		byte	dataType;			// READ Block : Read Data Type
		byte	dataFormat;			// READ Block : Data Format 
		byte	swepEnb;            // READ Block : SWEP 지정
		byte    function;           // READ Block : Function#

		word	commIndex;          /* READ-Block 별 통신회수 제어 */
		
    } __attribute__ ((packed)) MODBUS_READ;
    
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;           	// WRITE Block : 사용여부 
        byte    opcode;           	// WRITE Block : OPCODE 
        word    baseAddr;   		// WRITE Block : BASE Address 
        word    baseOffset;   		// WRITE Block : Offset Address
		word	dataNum;			// WRITE Block : Read Point Number
		byte	dataType;			// WRITE Block : Read Data Type
		byte	dataFormat;			// WRITE Block : Data Format
		word	tripData;           // WRITE Block : TRIP Control Data
		word	closeData;          // WRITE Block : CLOSE Control Data

    } __attribute__ ((packed)) MODBUS_WRITE;    

typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte    useFlag;           	// TIME Block : 사용여부 
        byte    opcode;           	// TIME Block : OPCODE 
        word    baseAddr;   		// TIME Block : BASE Address 
        word    baseOffset;   		// TIME Block : Offset Address
		word	dataNum;			// TIME Block : Read Point Number
		byte	dataType;			// TIME Block : Read Data Type
		byte	dataFormat;			// TIME Block : Data Format
		word	reserved;           // TIME Block : TRIP Control Data

    } __attribute__ ((packed)) MODBUS_TIME;    


/* ---------------------------------------------------- */
/* STURECTURE 정의 : SDP_DEVICE, 계전기 운영  Buffer    */
/*  - 계전기/장치별 운영 Buffer                         */
/* ---------------------------------------------------- */  
typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */
        byte	devNameStr[40];		    // SDP DEVICE : 계전기 Name String
        byte    scan;                   // SDP DEVICE : 계전기 사용유무
        byte    comDevID;               // SDP DEVICE : 기능모듈내 계전기 ID 
        byte    comDevIndex;            // SDP DEVICE : 기능모듈내 계전기 Index 
        byte    scanPort;               // SDP DEVICE : 계전기 통신 포트
        byte    type;           	    // SDP DEVICE : 계전기 TYPE
		byte	dualENB;			    // SDP DEVICE : 계전기 - 계전기 이중화 여부
		byte	modbusFileNo;		    // SDP DEVICE : 계전기 - MODBUS 프로파일
		
		word    devDiPoint;             // SDP DEVICE : 장치별 DI 포인트 수 
        word    devDoPoint;             // SDP DEVICE : 장치별 DO 포인트 수 
        word    devAiPoint;             // SDP DEVICE : 장치별 AI 포인트 수 
        word    devAoPoint;             // SDP DEVICE : 장치별 AO 포인트 수 
        word    devCntPoint;            // SDP DEVICE : 장치별 Binary Counter 포인트 수
        
		char	ipString1[16];		    // SDP DEVICE : HOST IP Address
		char	ipString2[16];		    // SDP DEVICE : HOST IP Address
        word    netPort;			    // SDP DEVICE : HOST TCPIP Port번호 */
        short   function1;              // SDP DEVICE : SCAN Function#1
        
        /* ------------------------ */
        /*  APP-프로그램 참조       */
        /* ------------------------ */
        short   ioid;                   // SDP DEVICE : 계전기 ID 
        short   hostid;                 // SDP DEVICE : CU ID
        short   targetID;               // ESIO Target ID, 0:사용않함, 1: SIO, 2:ESIO1, 3:ESIO2, 4:ESIO3, 5:ESIO4, 6:RTU, 7:MPU1, 8:MPU2
        short   sensorID;               // 가스밀도용 센서ID
        
        short   devStatus;              // SDP : 계전기 동작상태 
        byte    rcvIIN[2];              // SDP DEVICE : 계전기 DNP 상태정보    
        
        short   init_status;            // SDP DEVICE : DEVICE 초기화 상태
        short   init_analog;            // SDP DEVICE : DEVICE 초기화 상태
        short   online;                 // SDP DEVICE : DEVICE Online 상태
        short	offlineTick;            // SDP DEVICE : DEVICE Offline Count Tick
        short	offCount;			    // SDP DEVICE : 통신 Offline 기준
        
        short   reqAckDelay;
        
        word    comSndCount;            // SDP DEVICE : 계전기 통신 - Send Count
        word    comRcvCount;            // SDP DEVICE : 계전기 통신 - Recv Count
        word    oldRcvCount;            // SDP DEVICE : 계전기 통신 - 이전 Recv Count
		
		short	simDumpInit;			// SIM : 계전기 포인트 상태 Dump Flag
        //short	reportDIpt;
        //short	reportAIpt;
        short	regDiPointNum;			// 계전기 등록 DI 포인트 수
        short	regAiPointNum;			// 계전기 등록 AI 포인트 수
        
        
        short   selectReq;              // SDP DEVICE : SELECT 제어요청
        short   operateReq;             // SDP DEVICE : OPERATE 제어요청
        short   cntPoint;               // SDP DEVICE : 제어 포인트 [0...]
        short   cntTCF;                 // SDP DEVICE : 제어 상태, [0x80] TRIP, [0x40]CLOSE
        short   cntrType;               // SDP DEVICE : 계전기 제어방식 : Pulse/Latch ... 
        
        //byte    vmeSoeRear;             /* VMEBUS : SOE 정보 Rear 포인터 */
        short   stsByteCnt;             /* CPU 이중화연계 : 상태정보 수 */
        
        byte    cursts[MAX_DI_DATA_NUM];        /* DEVICE 별 포인트 정보 : 상태포인트 Current */
        byte    df[MAX_DI_DATA_NUM];            /* DEVICE 별 포인트 정보 : 상태포인트 Old */
        //byte    mpuLinkSts[MAX_DI_DATA_NUM];    /* MPU 전송용 상태 데이터용 버퍼 */
        
        
        POINT_BUF       diPtBuf[MAX_DEV_DI_POINT];      // 전자식배전반 DI 상태포인트
	    POINT_BUF       doPtBuf[MAX_DEV_DO_POINT];      // 전자식배전반 DO 상태포인트
	    POINT_BUF       aiPtBuf[MAX_DEV_AI_POINT];      // 전자식배전반 AI 상태포인트
	    //POINT_BUF       countBuf[MAX_DEV_COUNT_POINT];  // 전자식배전반 AO 포인트
	    //POINT_BUF       aoPtBuf[MAX_DEV_AO_POINT];      // 전자식배전반 AO 포인트
	    
	    
	    /* ---------------------------- */
        /*  DNP 통신 관련               */
        /* ---------------------------- */
        short   rcvAC;       
        short   rcvACfunc; 
        short   rcvACseq; 
        short   rcvObj;
        short   rcvVar;
        short   rcvIndex;
        short   rcvQcode;

        word    rcvStart;
        word    rcvStop;
        short   rcvQnum;
        short   rcvDataPos;
        
        short   sndFlag;
        short   appFailTick;
        short   timeOutTick;
        short   comFailTick;
        
	    short   devRestart;        
	    short   timeSyncReq;            // SDP DEVICE : TIME SYNC Flag
        short   eventDumpReq;
        short   classReq;
        short   class1Req;
        short   class2Req;
        short   class3Req;
        short   statusDump;		        // SDP DEVICE : Status Input Dump Flag
        short   analogDump;		        // SDP DEVICE : Analog Input Dump Flag
        short   countDump;              // SDP DEVICE : Count Input Dump Flag
        short   anaOutDump;
        
        
        short   aoVpoint;
        short   aoVpointData;        
        short   anaVOutSelect;      /* V0 Reset 관련 */
        short   anaVOutOpr;

        /* [Common]  Reference variable */
        short   sndLinkSts;
        short   rcvLinkSts;
        short   sndFcbBit;
        short   sndFcvBit;
        short   rcvFcbBit;
        short   rcvFcvBit;
                
        short   ACseqFail;                
        short   acSndSeqno;
        short   thSndSeqno;
        
        /* ---------------------------- */
        /*  MODBUS 통신 관련            */
        /* ---------------------------- */
        short   faultResetReq;          /* MODBUS : Remote-Reset Request */
        short   modbusRcvOK;            /* MODBUS Block별 Index */   
        short   modbusRcvFail;          // MODBUS Rcv Frame Error 
        short   reqDataCount;           // MODBUS 수신요청 데이터 수 
        short   reqDataOpcode;          // MODBUS 수신요청 OPCODE 
        
        short	readIndex;				// MODBUS READ-BLOCK Index
        short	writeIndex;
        short	defaultRead;			// MODBUS Default READ
        
        /* PNC-Tech 고장점 시각동기 관련 */
        short	timeIndex;
        short	timeInfoFlag;
        
        int		timeInfoAddr[8];
        short	timeInfoData[8];
        
        MODBUS_READ		readBlock[8];
        MODBUS_WRITE	writeBlock[4];
        MODBUS_READ		timeBlock;
        
	    /* ------------------------ */
        /* 지멘스(TCPIP) 이중화 관련*/
        /*  - MODBUS 프로토콜 관련  */
        /* ------------------------ */
        short   activeMode;             // 계전기 통신 Active Flag... 
        short   activeFlag;             // 계전기 통신 Active Flag... 
        short   activeTick;
        
        short	activeChan;
        short   dualComFlag;		// 계전기 CPU 이중화 여부
        
        
        /* ------------------------------------ */
        /*  TCP/IP 통신 : DEVICE 별 구조체      */
        /* ------------------------------------ */
        struct sockaddr_in	serv_addr;

        short   SocketFd;           /* for Client */
        short   newSocketFd;        /* for Client */
        short   connectStatus;
        short   connectFail;

#if 0        
        /* MODBUS 통신 Buffer Define */
        byte    sndBuffer[256];
        byte    rcvBuffer[512];
        
        /* DNP 통신 Buffer Define */
        DNP_APP_FRAME   rcvAppFrame;
        DNP_APP_FRAME   sndAppFrame;
        DNP_DATA_LINK   dataLinkFrame;
#endif
        
        /* -------------------------------- */
        /*  계전기별 통신포트 구성          */
        /* -------------------------------- */
        TTY_DESC        *ttyPort;
        
    } __attribute__ ((packed)) SDP_DEVICE;


/* ==================================================== */
/* STURECTURE 정의 : 61850 계전기 운영  Buffer          */
/* ==================================================== */

typedef struct
    {
        byte    eventCode;                              // SOE 정보구분자 , 0x13 : 계전기 Online, 0x14: 계전기 Offline, 0x20 : 상태포인트 SOE
        byte    devNo;                                  // SOE 발생 계전기 번호, [1...64]
        word    pointNo;                                // SOE 발생 포인트 번호, [1..1024]
        byte    state;                                  // SOE 발생 상태정보
        byte    esioNo;                                 // SOE 발생 ESIO 번호, [1..6]
        //byte    reserve1;
        //byte    reserve2;
        
        struct timeval	updateTime;		                // SOE 발생 시각정보, timeval
        
    } __attribute__ ((packed)) SOEQ_ENTRY;  
    
typedef struct
    {
        byte    front;                                  // [초기화/WRITE-61850] 61850 계전기 : SOE-Queue Front ... [0...255]
        byte    rear;                                   // [초기화/WRITE-LHS]   61850 계전기 : SOE-Queue Rear  ... [0...255]
        
        byte    overflow;                               // [초기화/WRITE-61850] 61850 계전기 : SOE Queue Overflow 상태 ... 0 : 없음, 1 : Overflow
        byte    report;                                 // [초기화/WRITE-LHS]                : SOE Queue Report 값

        SOEQ_ENTRY  Queue[256 + 2];
        
    } __attribute__ ((packed)) SOE_QUEUE;               // [초기화/WRITE-61850] 61850 계전기 : SOE-Queue 정보


typedef struct
    {
        /* ------------------------ */
        /*  DATABASE 구성내용       */
        /* ------------------------ */     
//        byte    devNo;                                  // [초기화/WRITE-61850] 61850 계전기 : device 번호 [1..64] 
//        word    devPt;                                  // [초기화/WRITE-61850] 61850 계전기 : device 포인트 번호 [1..1024] 

//        byte    ptType;                                 // [초기화/WRITE-61850] 61850 계전기 : device 포인트 TYPE, 0 : 미지정, 1 : 상태(DI), 2 : 계측(AI)
//        byte    ptConfig;                               // [초기화/WRITE-61850] 61850 계전기 : Point 사용유무, 0 : 사용않함, 1 : 사용
        byte    config61850;                            // [초기화/WRITE-61850] 61850 계전기 : 61850 계전기/포인트  DB 등록 상태, 0: 미지정, 1: 지정
		//char	name[130];      
       	/* ------------------------ */
        /* POINT 운영 정보 ...      */
        /* ------------------------ */

        /* 포인트 데이터 영역 */
        struct timeval  updateTime;                     // [초기화/WRITE-61850] 61850 계전기 : 포인트 정보 갱신시간, 0 = 미지정(Offline)
        
        byte    state;                                  // [초기화/WRITE-61850] 61850 계전기 : 상태 Point status, 0 :OPEN(TRIP), 1: CLOSE   
        
        /* -------------------------------- */
        /*	한빛이디에스 시험 : 이벤트 정보 => long 형태	*/
        /*	- 61850 데이터 형식 : double			*/
        /* -------------------------------- */
        //float	floatData;			                    // [초기화/WRITE-61850] 61850 계전기 : 계측 Point FLOAT 데이터
        double	floatData;			                    // [초기화/WRITE-61850] 61850 계전기 : 계측 Point FLOAT 데이터
        
    } __attribute__ ((packed)) POINT_61850;


/* ==================================================== */
/* STURECTURE 정의 : 61850 계전기 운영  Buffer          */
/*  - 계전기/장치별 운영 Buffer                         */
/* ==================================================== */
#define	DEV_STATUS_ONLINE_BIT	0x0001
#define	DEV_STATUS_RUN_BIT		0x0010
#define	DEV_STATUS_FAIL_BIT		0x0040

typedef struct
    {
        short   ioid;                                   // 61850 계전기 ID .. [1/2...64 ]
        short   config61850;                            // [초기화/WRITE-61850] 61850 계전기 DB 등록 상태, 0: 미지정, 1: 지정 */

        short   src_ioid;                               // MPU 와 통신용 ID ...[1,2... 64]        
        
		char	name[130];      
        char	ipAddress[16];
        /* -------------------------------- */
        /*  계전기 - 동작관련 상태영역      */
        /* -------------------------------- */		
        short   devStatus;                              // [초기화/WRITE-61850] 61850 rPwjsrl 계전기 동작상태 정보.. 
                                                        //     BIT0: Online[1]  / Offline[0] : Communication Status
                                                        //     BIT4: Run[1]     / Stop[0]
                                                        //     BIT6: DevFail[1] / DevOk[0] : Report/Polling 등이 비정상 동작일 경우.
                                        
        short   online;                                 // [초기화/WRITE-61850] 61850 계전기 Online 상태 ... [0] Offline, [1] Online

        word    comSndCount;                            // [초기화/WRITE-61850] 61850 계전기 통신 - Send Count, MAX 1024, 순차적 증가
        word    comRcvCount;                            // [초기화/WRITE-61850] 61850 계전기 통신 - Recv Count, MAX 1024, 순차적 증가

        short   devDiLastNo;                             // 61850 계전기 DI 등록 포인트 수 ...
        short   devAiLastNo;                             // 61850 계전기 AI 등록 포인트 수     
        short   devDoLastNo;                             // 61850 계전기 DO 등록 포인트 수 
        
        /* -------------------------------- */
        /*  61850 계전기 - 제어정보 연계    */
        /* -------------------------------- */		
        short   controlReq;                             // [초기화/WRITE-LHS] 61850 계전기 : 제어요청 ....[0] 제어없음, [1] 제어 있음
        short   cntPoint;                               // [초기화/WRITE-LHS] 61850 계전기 : 제어 포인트 번호 [1...1024]
        short   cntTcf;                                 // [초기화/WRITE-LHS] 61850 계전기 : 제어 상태, [1] TRIP, [2]CLOSE
        
        /* -------------------------------- */
        /*  계전기 - 상태/계측정보 영역     */
        /* -------------------------------- */		
        byte    stsByteCnt;                             // [초기화/WRITE-61850] 61850 계전기 : 상태정보 Byte  수 
        byte    cursts[MAX_DI_DATA_NUM];                // [초기화/WRITE-61850] 61850 계전기 : 계전기 상태 포인트 Dump 정보 (Byte 형식)
        
        POINT_61850     diPtBuf[MAX_DEV_DI_POINT];      // [초기화/WRITE-61850] 61850 계전기 : 계전기별 상태(DI) 포인트 정보
	    POINT_61850     aiPtBuf[MAX_DEV_AI_POINT];      // [초기화/WRITE-61850] 61850 계전기 : 계번기별 계측(AI) 포인트 정보
        POINT_61850     doPtBuf[MAX_DEV_DO_POINT];      // [초기화/WRITE-61850] 61850 계전기 : 계전기별 상태(DO) 포인트 정보

    } __attribute__ ((packed)) DEVICE_61850;

#endif	// RTUBUF_HEADER_INCLUDED
