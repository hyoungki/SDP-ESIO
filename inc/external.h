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

extern  SHM_MEMORY	    *shmPtr;

extern  RTC             *rtc;
extern  OPR_MSG         *opr;
extern  RTU_DATABASE    *rtudb;                     // SDP 데이터베이스
extern  CONSOLE_INFO	*console;

extern  HISTORY_QUE     *hque;                      // CONSOLE 용 이벤트

extern  SOE_QUEUE       *soeQueue;                  // ESIO 장치 SOE-Queue
extern  ESIO_CONFIG     *esioCFG;                   // ESIO 장치 Config
extern  MPU_CONFIG      *mpuCFG;                    // MPU Network 구성정보

extern  POINT_BUF       *pointCFG[MAX_DEV_POINT];   // SDP 포인트 Config 정보
extern  SCAN_CONFIG     *scanCFG[MAX_SCAN_PORT];    // 하위계전기 SCAN Config
extern  SDP_DEVICE      *deviceCFG[MAX_DEVICE];     // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
extern  DEVICE_61850    *device61850[MAX_DEVICE];       // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)

extern  TIME_SYNC_INFO	*timeSync;                  // 61850 Time-Server 연동

extern  byte    errmsg[NUMBER_OF_EVENT][20];
