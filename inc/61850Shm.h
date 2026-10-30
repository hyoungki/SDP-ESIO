/*
 * ==============================================================
 * System TARGET : ACE Control SDP-2000 Ver 1.0   
 * Target CPU    : MPC8248, VME6U
 * Main Factors  : 스마트급전용 SDP 자장치 
 *     - MPU 이중화 구성
 *     - ESOP/SIO 연계 : 전력감시, 원격진단, 전력품질, 고장점 등
 *     - 통합 시뮬레이터 : 2013/07/08 V8.2 적용 
 * --------------------------------------------------------------
 * System DESIGN : SANE-SYSTEM   .... by  Lee Ho-Sang
 * Initial-DATA  : 2016,03,25
 * Last Updated  : 2016,03,25
 * ==============================================================
 */

#ifndef	D61850SHM_HEADER_INCLUDED
#define	D61850SHM_HEADER_INCLUDED

//
// Include Files
//
#include "localLib.h"
#include "filelock.h"

/*
 * shared memory LOCK/UNLOCK
 */
#define	D61850_SHM_LOCK_FNAME		"/tmp/61850shm.lock"

extern int d61850_shm_lock_fd;     /* shm lock file : global variable */

#define D61850_SHM_LOCK_READ() \
	file_lock_read	(d61850_shm_lock_fd, 0, 1)
#define D61850_SHM_LOCK_WRITE() \
	file_lock_write	(d61850_shm_lock_fd, 0, 1)
#define D61850_SHM_UNLOCK() \
	file_unlock		(d61850_shm_lock_fd, 0, 1)


/*
 *	proto-type definition for lib_61850Shm.c
*/
int	d61850Shm_Init (SHM_MEMORY *p);
void d61850Shm_End (void);

int d61850Shm_SetDevice61850Config (int max, short *config, char name[MAX_DEVICE][130], char ip[MAX_DEVICE][16]);
int d61850Shm_SetDo61850Config (byte devNo, int max, short *config, char name[MAX_DEV_DO_POINT][130]);
int d61850Shm_SetDi61850Config (byte devNo, int max, short *config, char name[MAX_DEV_DI_POINT][130]);
int d61850Shm_SetAi61850Config (byte devNo, int max, short *config, char name[MAX_DEV_AI_POINT][130]);

int d61850Shm_UpdateDiData (byte devNo, word pointNo, struct timeval *pUpdateTime, byte data);
int d61850Shm_UpdateAiData (byte devNo, word pointNo, struct timeval *pUpdateTime, double data);

int d61850Shm_PushSoe (SOEQ_ENTRY *pSoe);
int d61850Shm_PopSoe (SOEQ_ENTRY *pSoe);
int d61850Shm_GetControlData (byte devNo, short *cntPoint, short *cntTcf);
int d61850Shm_SetControlData (byte devNo, short cntPoint, short cntTcf);

int d61850Shm_SetDeviceStatus (byte devNo, short devStatus);
int d61850Shm_AddCommCount (byte devNo, int send, int recv);

#endif	// D61850SHM_HEADER_INCLUDED
