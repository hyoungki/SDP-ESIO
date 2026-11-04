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

#ifndef	COMMON_HEADER_INCLUDED

#define	COMMON_HEADER_INCLUDED


/* ------------------------------------------------ */
/* ROM Version 또는 NFS Version 선택                */
/* ------------------------------------------------ */
#if 1
#define ROM_VERSION             100        
#define NETWORK_CONSOLE         200     // Network - CONSOLE   
#endif

#define LOGFILE_DISABLE         200     // Network - CONSOLE   
//#define TEST_61850         		200     // Network - CONSOLE   
//#define	ESIO_TEST_MODE		100


/* ------------------------------------------------ */
/* ESIO Network 초기화 : VMEBUS 선택                */
/* ------------------------------------------------ */
#define VME_NETWORK_INITIAL       100

/* ------------------------------------------------ */
/* SDP 자장치 - ESIO 관련 상수정의                  */
/* ------------------------------------------------ */
//#define DATE_STRING         "2017.11.15"
//#define VERSION_STRING      "Ver17.11.15"




#define TARGET_NAME         ">>SDP-ESIO-V3 : "


#define DATE_STRING         "2024.10.30"
#define VERSION_STRING      "Ver24.10.30"

//#define TARGET_NAME         ">>SH-ESIO-V1 : "
#define SIMULATOR_NAME      "SDP-Simulator V1.1.5, 2017-01-04"


#define ROMDB_STRING      "/mnt/bin/DB-SDP-ESIO"   
#define ROMDB_STRING2     "/mnt/bin/DB-SDP-ESIO-1"                


#define _PACK_ __attribute__((__packed__))      // Structure Data Size Packed...

//
// Macro
//
#if	0	/* _CHOIBC_DELETE */
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))
#endif	/* _CHOIBC_DELETE */
#define pause(x)    usleep(x*1000) // x is in mS *

//
// General Data Type
//
typedef unsigned char   BYTE;
typedef unsigned char   UCHAR;
typedef unsigned short  WORD;

typedef unsigned char   byte;
typedef unsigned char   uchar;
typedef unsigned short  word;



#define ETH0            0
#define ETH1            1
#define ETH2            2


//
// General Constant Data Type
//
#define	NO 		        0
#define	YES		        1

#define	RESET 	        0
#define	SET		        1

#define	TRUE 	        1
#define	FAIL	        -1
#define FALSE           0

#define	OK 		        1
#define	NOK		        0

#define	ON 		        1
#define	OFF		        0

#define ERROR           -1
#define CONNECT         1

//
// Return Codes
//
#define	E_OK		    0
#define	E_ERR			-1
#define	E_SKIP			-1
#define	E_WAIT			-2


#define BIT0            0x01
#define BIT1            0x02
#define BIT2            0x04
#define BIT3            0x08
#define BIT4            0x10
#define BIT5            0x20
#define BIT6            0x40
#define BIT7            0x80

#define AUTO_ENABLE     0x10
#define AUTO_DISABLE    0x11
#define DATA7           0x12
#define DATA8           0x13

#define FIOFLUSH        0x01
#define QUIT            0x03
#define EXIT            0x18
#define FIOFLUSH        0x01
#define RTS_ON          0x02
#define RTS_OFF         0x03


/* ------------------------------------------------ */
/*  Process Index                                   */
/*  PROCESS 별 ID 부여 (실제 운영되는 프로세서 정의 */
/* ------------------------------------------------ */
#define	MAX_PROCESS		    4

#define	MPUIF_PROCESS	    0       /* MPU 정보연계 통신용 Process */
#define	SIM_PROCESS	        1       /* SIMULATOR 정보연계 통신용 Process */
#define	SCAN_PROCESS	    2       /* RTU SCAN Process */

#define IEC61850_PROCESS    2       /* 61850 계전기 통신 Process */
#define NTP_CLIENT_PROCESS  3       /* NTP 통신 Process */


#define	CLI_PROCESS		MAX_PROCESS
#define	WDT_PROCESS	    (MAX_PROCESS + 1)       /* WDT     통신용 Process */


//
// IPC : Shared Memory 관련
//
#define	SHM_KEY			(key_t)100

#define	MAX_QUEUE				4
#define	LINK_QUEUE		(key_t)100          // 센서노드 수집 Queue
#define	SND_QUEUE		(key_t)101          // 정보제공 Queue
#define	HST_QUEUE		(key_t)102          // HOST Queue
#define	SIM_QUEUE		(key_t)103          // Simulator Queue


#define	PLATENUM_LEN		12
#define	CARINFO_LEN			18
#define	RAWIMAGE_HEADER		16
#define	PRCNAME_LEN			32
#define	STAMP_LEN			32
#define	FILENAME_LEN		64
#define	MESSAGE_LEN			128
#define	SHORTMSG_LEN		16
#define	CAMMSG_LEN			16
#define	RES_X				1024
#define	RES_Y				1024
#define	RES_YH				(RES_Y/2)
#define	TOTAL_IMAGE			(RES_X * RES_Y)
#define	RAWIMAGE_LEN		(TOTAL_IMAGE + RAWIMAGE_HEADER)
#define	PIXEL_BLACK			0
#define	PIXEL_GRAY_NOISE	115
#define	PIXEL_GRAY			125
#define	PIXEL_WHITE			255
#define	MAX_GRAY_LEVEL		256
#define	MAX_SEGMENT			10
#define	NEW_SEGCOUNT		9
#define	OLD_SEGCOUNT		8
#define	MULTI_THRESHOLD		15
#define	RAW_EXT_NAME		"raw"
#define	MAX_LINES			6



//2026-05-27 오전 9:44:27
#define ESIO_NET_MAX  4
#define MPU_NET_MAX   4 
/* ----------------------------- */
/* SHARERAM 관련 Structure 외부선언 */
/* ----------------------------- */ 
typedef struct 
{
	int		id;
	key_t	key;
	int		size;
	char	*address;
} SHM_DESC;

/* ----------------------------- */
/* THREAD 관련 Structure 외부선언 */
/* ----------------------------- */
#define	TASK_NAME_LEN	16

typedef struct 
{
	char	name[TASK_NAME_LEN];
	pthread_t		tid;
	int		priority;
	int		options;
	void	*entryPt;
	void	*arg;

} THREAD_ENTRY;
                
                
/* ----------------------------- */
/* TTYDESC 관련 Structure 외부선언 */
/* ----------------------------- */ 
#define	MAX_TTY_DEVICE		64
#define	MAX_TTY_DEVICE		64
#define	FILENAME_LEN		64
#define	MESSAGE_LEN			128


#define ASYNC_8530          1           // ASYNC PORT : 8530 기반
#define ASYNC_860T          2           // ASYNC PORT : 860 기반

typedef struct {
    int     chid;
	int		id;
	FILE	*fId;
	int		config;             /* device 속성 정의 (외부8530, 내부 ASYNC) */
	int     commMode;
	int		baudRate;
	int		dataBits;
	int		stopBits;
	int     lineDebug;
	char	parity;
	char	name[FILENAME_LEN];
	struct timeval  timeOut;
} TTY_DESC;


/* ----------------------------- */
/* MESSAGE 관련 Structure 외부선언 */
/* ----------------------------- */ 
#define	MSG_LENGTH		1024

typedef struct {
	long	mType;
	char	mText[MSG_LENGTH];
} MSG_BUFFER;

typedef struct {
	key_t	key;
	int		id;
	MSG_BUFFER	mBuffer;
} MSG_DESC;

/* ----------------------------- */
/* FILE 관련 Structure 외부선언 */
/* ----------------------------- */  
#define	FIO_NORMAL			0
#define	FIO_APPEND			1
#define	FIO_CREATE			2

#define	FIO_BUFLEN			(256 * 1024)
#define	FIO_DELAY_TIME		(1000)

typedef struct {
	FILE	*id;
	int		mode;
	char	name[FILENAME_LEN];
} FILE_DESC;


//
// 구조체 선언
//
#define	WDT_WAIT_COUNT			10

typedef struct
{
	char	name[PRCNAME_LEN];
	char	argument[MESSAGE_LEN];
	short	check;
	char	waitCount;
} PROCESS_DESC;


#endif		// COMMON_HEADER_INCLUDED

//
// End of common.h
//
