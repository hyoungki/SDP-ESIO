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

#ifndef	WISDOM_HEADER_INCLUDED

#define	WISDOM_HEADER_INCLUDED

//
// Include Files
//
#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<stdarg.h>
#include 	<signal.h>
#include    <time.h>
#include 	<termios.h>
#include 	<sys/types.h>
#include 	<sys/time.h>
//#include 	<sys/io.h>
#include 	<sys/ioctl.h>
#include 	<sys/ipc.h>
#include 	<sys/shm.h>
#include 	<sys/signal.h>
#include 	<sys/stat.h>
#include 	<sys/socket.h>
#include    <sys/mman.h>
#include 	<arpa/inet.h>
#include    <linux/rtc.h>  // hello world 

#include 	<errno.h>
#include	<pthread.h>
#include 	<fcntl.h>
#include 	<unistd.h>
#include 	<ctype.h>
#include 	<math.h>

#include    "common.h"

#include    "m860io.h"
#include    "vmesio.h"

#include    "op_dnp.h"                  // DNP 구조체 및 관련 Header
#include    "op_modbus.h"
#include    "op_rtusim.h"               // SIMULATOR 관련 Header

#include    "rtudb.h"
#include    "rtubuf.h"

#include    "usrtype.h"
#include    "logdb.h"
#include    "shmMemory.h"
#include    "extfunc.h"

// 2026-05-21 오전 9:50:19 
#include "includes.h"



/* ------------------------------------------------ */
/*  영상검지기용 860 Board : NET860-500             */
/*  860 Internal ASYNC -PORT Define ...             */
/* ------------------------------------------------ */
#define DEVICE_HERO_IO      "/dev/hera-io"
#define DEVICE_EX_IO        "/dev/ex-io"

#define FID_CONSOLE_PORT  	"/dev/ttyS0"
//#define FID_LINK_PORT  	    "/dev/ttyCPM1"	    /* CPU 링크연계 */
//#define FID_SCU_PORT  	    "/dev/ttyCPM2"	    /* 절체장치 연계 */
//#define FID_RFU_PORT  	    "/dev/ttyCPM3"	    /* 예비용 포트 연계 */
//#define FID_MMI_PORT  	    "/dev/ttyCPM4"	    /* MMI 장치 연계포트 */

/*  External ASYNC -PORT Define ... */
#if 0 // 2026-04-11 오후 5:25:57 hkkiim
#define FID_SCAN1_PORT	    "/dev/ttyS2"		/* 하위 계전기#1  */
#define FID_SCAN2_PORT	    "/dev/ttyS3"		/* 하위 계전기#2  */
#define FID_SCAN3_PORT	    "/dev/ttyS4"		/* 하위 계전기#3  */
#define FID_SCAN4_PORT	    "/dev/ttyS5" 		/* 하위 계전기#4  */
#define FID_SCAN5_PORT	    "/dev/ttyS6" 		/* 하위 계전기#4  */
#define FID_SCAN6_PORT	    "/dev/ttyS7"		/* 하위 계전기#1  */
#define FID_SCAN7_PORT	    "/dev/ttyS8"		/* 하위 계전기#2  */
#define FID_SCAN8_PORT	    "/dev/ttyS1"		/* 하위 계전기#3  */


#else

#define FID_SCAN1_PORT	    "/dev/ttyESIO0"		/* 하위 계전기#1  */
#define FID_SCAN2_PORT	    "/dev/ttyESIO1"		/* 하위 계전기#2  */
#define FID_SCAN3_PORT	    "/dev/ttyESIO2"		/* 하위 계전기#3  */
#define FID_SCAN4_PORT	    "/dev/ttyESIO3"		/* 하위 계전기#4  */
#define FID_SCAN5_PORT	    "/dev/ttyESIO4"		/* 하위 계전기#4  */
#define FID_SCAN6_PORT	    "/dev/ttyESIO5"		/* 하위 계전기#1  */

#define FID_SCAN7_PORT	    "/dev/noDevice"		/* 하위 계전기#1  */
#define FID_SCAN8_PORT	    "/dev/noDevice"		/* 하위 계전기#1  */


#endif 





/* 전체 통신포트에 대한 ID */
#define FID_CONSOLE     0
#define FID_LINK        1
#define FID_SCU         2
#define FID_RFU         3
#define FID_MMI         4

/* ------------------------------------------------ */
/*  하위 계전기용 통신포트 ...(총 10개)             */
/* ------------------------------------------------ */
#define FID_SCAN1       0       // CPU 내장 SCAN #1
#define FID_SCAN2       1       // CPU 내장 SCAN #2
#define FID_SCAN3       2       // CPU 내장 SCAN #3
#define FID_SCAN4       3       // CPU 내장 SCAN #4

#define FID_SCAN5       4
#define FID_SCAN6       5
#define FID_SCAN7       6
#define FID_SCAN8       7
#define FID_SCAN9       8
#define FID_SCAN10      9


#define MAX_CPUPORT     5

#define VME_COM1        10      // VME SIO Port #1
#define VME_COM2        11      // VME SIO Port #2
#define VME_COM3        12      // VME SIO Port #3
#define VME_COM4        13      // VME SIO Port #4
#define VME_COM5        14      // VME SIO Port #5
#define VME_COM6        15      // VME SIO Port #6
#define VME_COM7        16      // VME SIO Port #7
#define VME_COM8        17      // VME SIO Port #8




#endif

//
// End of aesLib.h
//
