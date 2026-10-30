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

#ifndef	SHMMEMORY_HEADER_INCLUDED

#define	SHMMEMORY_HEADER_INCLUDED

//
// Include Files
//
#include "common.h"


typedef struct
{
	pid_t	pid;
	int		define;
	int		check;
	int		active;
	int     debug;
	int     restart;
	int     initial;
	int		wdtEnable;
	int     wdtCount;
	int		lastWdt;
	int		prevEtime;
	int		maxEtime;
	char	name[16];
#if 0	// 2026-04-27 오후 3:46:35
} __attribute__ ((packed)) TASK_INFO;
#else 
}  TASK_INFO;
#endif 

#define	PORTNAME_LEN		32

typedef struct
{
	char	name[PORTNAME_LEN];
	int		baudRate;
	BYTE	byteSize;
	BYTE	stopBit;
	BYTE	parity;
	BYTE	resetCom;
#if 0	// 2026-04-27 오후 3:46:40
} __attribute__ ((packed)) PORT_INFO;
#else
} PORT_INFO;
#endif 

#if 1	// 2016.07.17 ChoiBC Modify
typedef struct
{
	// config info
	int		period;			/* Period of Time Sync, 0 means no time sync, minutes */
	BYTE	ip[2][4];		/* IP of first/second  Time Sync Server */
	// Dynamic
	time_t	aliveTime;		// alive time of ntpClient, task set every seconds
	int		valid;			// 0인 경우, 아래 상태 데이터는 무효임.
	time_t	timeSyncTime;	// Time Sync Try time
	int		activeNo;		// 0(SNTP Server 1), 1(SNTP Server 2)
#if 1   // 2022.03 ChoiBC - “ClockFailure” in SNTP time server
	short   success;		// 1:SUCCESS, 0:FAILURE
	short   clockFailure;
#else
	int		success;		// 1:SUCCESS, 0:FAILURE
#endif
	//char	errorMsg[256];	// 실패 메세지, not used
} __attribute__ ((packed)) TIME_SYNC_INFO;

#endif

/* ---------------------------------------------------- */
/* STURECTURE 정의 : SHM_MEMORY                         */
/* - SDP 자장치 : 공유메모리                            */
/* ---------------------------------------------------- */  

typedef struct
{
	TASK_INFO	    taskInfo[MAX_PROCESS + 2];
	
	RTC             rtc; 
	OPR_MSG         opr_msg;                            
	
	CONSOLE_INFO	console;

    MPU_CONFIG      mpuConfig;                          // MPU Config
    ESIO_CONFIG     esioConfig;                         // ESIO 장치 Config
    
	//POINT_BUF       pointCFG[MAX_DEV_POINT];            // SDP 포인트 Config 정보
	//POINT_BUF       devPtBuf[MAX_DEV_POINT];            // 장치 포인트용 구조체
	
	SCAN_CONFIG     scanCFG[MAX_SCAN_PORT];             // 하위계전기 SCAN Config
	SDP_DEVICE      deviceCFG[MAX_DEVICE];              // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)

    //RTU_DATABASE    esioDatabase;                        // SDP 데이터베이스
    RTU_DATABASE    rtuDatabase;                        // SDP 데이터베이스
    
    HISTORY_QUE     localHistoryQ;                      // CONSOLE 용 LOCAL Event (SOE...)
    
#if 1	// 2016.07.17 ChoiBC Modify
	TIME_SYNC_INFO	timeSync;
#endif
	
    /* -------------------------------- */
    /* 61850 계전기 관련 영역           */
    /* 2016.07.30 LHS Add               */	
    /* -------------------------------- */
    
    DEVICE_61850    device61850[MAX_DEVICE];      // 61850 계전기/장치 구조체
    SOE_QUEUE       soeQueue;

#if 0    
} __attribute__ ((packed)) SHM_MEMORY;
#else
}   SHM_MEMORY;
#endif 

#endif		// SHMMEMORY_HEADER_INCLUDED

//
// End of shmMemory.h
//
