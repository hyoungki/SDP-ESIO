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

/* -------------------------------------------- */
/*  SYSTEM-EVENT 코드 정의                      */
/* -------------------------------------------- */
#define ENT_NOT_DEFINED     0x00

#define ENT_CU_RESTART      0x01        // CU 재기동시
#define ENT_ROMDB_FAIL      0x02        // 데이터베이스 이상시
#define ENT_RESET_USER      0x03        // 사용자 재기동
#define ENT_RESET_WDT       0x04        // WDT에 의한 재기동
#define ENT_MASTER_ACT      0x05        // MASTER-CPU 동작
#define ENT_SLAVE_ACT       0x06        // SLAVE-CPU 동작
#define ENT_MAN_CHG_A       0x07        // 이중화 절체-A
#define ENT_MAN_CHG_B       0x08        // 이중화 절체-B
#define ENT_CHG_AUTO        0x09        // 자동모드 절체
#define ENT_CHG_MANUAL      0x0a        // Manual모드 절체
#define ENT_LOCAL_TIME      0x0b        // 시스템 시각동기
#define ENT_SIM_TIMESET     0x0c        // 추가 : 시뮬레이터 시각동기
#define ENT_HOST_TIMESET    0x0d        // HOST 시각동기
#define ENT_CHANGE_CPU      0x0e        // 추가 : 이중화 절체-CPU
#define ENT_WDT_HOST        0x0f        // 추가 : HOST 통신이상 RESET

#define ENT_ICCP_CNTR       0x10        // ICCP-관제에 의한 제어
#define ENT_HOST_CNTR       0x11        // HOST-관제에 의한 제어
#define ENT_SIM_CNTR        0x12        // 시뮬레이터에 의한 제어
#define ENT_USER_CNTR       0x13        // 사용자 자체 제어
#define ENT_LINK_CNTR       0x14        // SLAVE-CPU에 의한 제어
#define ENT_ESIO_CNTR       0x15        // SLAVE-CPU에 의한 제어

#define ENT_SOE             0x20        // 계전기 SOE 이벤트
#define ENT_DEVICE_SOE      0x21        // DEVICE 포인트 SOE 이벤트

#define ENT_ICCP_ONLINE     0x30        // ICCP-HOST 통신 Online
#define ENT_ICCP_OFFLINE    0x31        // ICCP-HOST 통신 Offline
#define ENT_HOST_ONLINE     0x32        // HOST 통신 Online
#define ENT_HOST_OFFLINE    0x33        // HOST 통신 Offline
#define ENT_DEV_ONLINE      0x34        // 하위계전기 통신 Online
#define ENT_DEV_OFFLINE     0x35        // 하위계전기 통신 Offline
#define ENT_ESIO_ONLINE     0x36        // HOST 통신 Online
#define ENT_ESIO_OFFLINE    0x37        // HOST 통신 Offline
#define ENT_MMI_ONLINE      0x38        // 하위계전기 통신 Online
#define ENT_MMI_OFFLINE     0x39        // 하위계전기 통신 Offline
#define ENT_SCU_ONLINE      0x3a        // 하위계전기 통신 Online
#define ENT_SCU_OFFLINE     0x3b        // 하위계전기 통신 Offline
#define ENT_LINK_ONLINE     0x3c        // 하위계전기 통신 Online
#define ENT_LINK_OFFLINE    0x3d        // 하위계전기 통신 Offline
#define ENT_VME_ONLINE      0x3e        // VMEBUS Online
#define ENT_VME_OFFLINE     0x3f        // VMEBUS Offline

#define ENT_VME_INSTALL     0x40        // VMEBUS Online
#define ENT_VME_UNINSTALL   0x41        // VMEBUS Offline

#define NUMBER_OF_EVENT     (0x50 + 2)

typedef struct
        {
        //word    year;           // 2000 base
        //byte    month;
        //byte    day;
        //byte    hour;
        //byte    min;
        //byte    sec;
        //word    milisec;
        struct timeval  logTime;    // MPU 로깅 시간정보
        struct timeval  soeTime;    // LOCAL 이벤트 시간정보                   
        
        byte    logid;              // 이벤트 코드 구분 : SOE/Control/System ...
        byte    ioid;               // 대상 계전기 번호 [1...112]
        word    point;              // 대상 포인트 번호 [1...1024]
        byte    state;              // 이벤트/제어 상태
        byte    hostid;             // 발생주체, 0: MPU, 1 ~ 8 : HOST
        
        } __attribute__ ((packed)) SYSLOG_FORM;

        
/* ---------------------------------------------------- */
/*  MPU 내부 운영 History-Queue                         */
/* ---------------------------------------------------- */
#define HISTORY_QUE_MAX      1024
#define HISTORY_QUE_MASK     1023

typedef struct
{
    word    front;
    word    overlab;
    word    clear;
    //word    temp;

    SYSLOG_FORM   queue[HISTORY_QUE_MAX];        
    word    chksum;                         /*   check word           */
        
} __attribute__ ((packed)) HISTORY_QUE;



#define	LOG_MAX_NUM		256
#define	LOG_MASK_NUM	255

typedef struct
{

	short	active;
	word	front;
	word	rear;
	
	byte	logData[LOG_MAX_NUM+2][256];

	short	writeFlag;
	//short	temp;
		
} CONSOLE_INFO;
