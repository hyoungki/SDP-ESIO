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
#include    "cli.h"

extern	int	 termExec;

extern	RTC 		*rtc;
extern	OPR_MSG     *opr;
extern	CONSOLE_INFO	*console;

extern	THREAD_ENTRY    debugTHREAD;

/* ------------------------------------------------------- */
/*  TTMS THREAD Start Routine ....             */
/* ------------------------------------------------------- */
void    *debug_thread(int  *arg)
{
	word	front, rear;

	printf(" %s DEBUG-MAIN PROCESS Activated ... !\n", TARGET_NAME);		
	
	console->front = 0;
	console->rear  = 0;
	console->active = SET;
	
	while(termExec)
	{
		pause(10);
		
		front = console->front & LOG_MASK_NUM;
		rear  = console->rear & LOG_MASK_NUM;
		
		while(front != rear)
		{
			//usleep(1000);
			printf("%s", console->logData[rear]);
			console->rear = (rear + 1) & LOG_MASK_NUM;
			
			front = console->front & LOG_MASK_NUM;
			rear  = console->rear & LOG_MASK_NUM;
		}
		
	}

	console->active = RESET;
	return (0);
}
