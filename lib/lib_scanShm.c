#include "localLib.h"


/*------------------------------------------------------------------------- */
/*  Function Name : storeAI_FLOAT_ESIO()                                     */
/*    - FLOAT 계측값을 계전기/포인트 정보 Update                            */
/* ------------------------------------------------------------------------ */
int storeAI_FLOAT_ESIO(SHM_MEMORY *shmPtr, int devid, int point, float fdata, byte *lowdata)
{
    int     pointIndex;

    CONSOLE_INFO	*console= NULL;
    OPR_MSG         *opr    = NULL;
    
    SDP_DEVICE      *dev;
    POINT_BUF       *aiPoint;

    console = (CONSOLE_INFO *) &shmPtr->console; 
    opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    
    /* DEVICE Offline 처리 */
    if((devid < 0) || (devid >= MAX_DEVICE))    	return (0);
   	if((point < 0) || (point >= MAX_DEV_AI_POINT))  return (0);
   		
    dev = (SDP_DEVICE *) &shmPtr->deviceCFG[devid];

    /* -------------------------------- */
    /* 지멘스 통신상태 : Active Flag    */
    /* -------------------------------- */
    if((dev->activeMode == SET) && (dev->activeFlag == RESET))
    {
        if(opr->msgDebug == devid)
        Debug(console,"dev%2d> store_AI_FLOAT... Active Status -> *RESET* ... return\n", devid+1);   
        return (0); 
    }
    
    /* ---------------------------------------- */
    /* 2014.12.11 수신된 POINT 예외처리...      */
    /* ---------------------------------------- */
    if(point >= MAX_DEV_AI_POINT)
    {
        Debug(console,"*** dev=%2d, func: storeAI_FLOAT_ESIO() - *Invalid AI-Point (%d) !\n", devid+1, point);
        return (0); 
    }
    
    /* Point Buffer Assign ... */
    pointIndex = point;
    aiPoint = (POINT_BUF *) &dev->aiPtBuf[pointIndex];
    
    if(aiPoint->config != SET)  return 0;
        
    aiPoint->status     = 0x01;
    aiPoint->pointData  = fdata;    /* Float Data ASSIGN */
    aiPoint->floatData  = fdata;

    gettimeofday(&aiPoint->updateTime, NULL);       //계측정보 갱신시간 추출...timeval  Form      
    
    return (0);
    
}   
        
static	byte	bitcode[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

/* ======================================================== */
/*   ESIO 전송 이벤트 저장...                               */
/*  - 전력감시, 원격진단, 전력품질 부 SOE 생성              */
/* ======================================================== */
int  storeESIO_event(SHM_MEMORY *shmPtr, SOEQ_ENTRY *devEvent)
{
    int     devNo, ioid, pointNo;
    byte    front, rear, status;
    int     index1, index2;
    int     eventNum;
    char    buffer[256];
    
    CONSOLE_INFO	*console= NULL;
    OPR_MSG         *opr    = NULL;
    SOE_QUEUE       *soeQueue= NULL;
    
    console = (CONSOLE_INFO *) &shmPtr->console; 
    opr     = (OPR_MSG *)   &shmPtr->opr_msg;
    soeQueue = (SOE_QUEUE *) &shmPtr->soeQueue;
    
    //bzero(buffer, sizeof(buffer));
    
    SOEQ_ENTRY  *event;
    SDP_DEVICE  *dev;
    POINT_BUF   *diPoint;
    
    struct tm *curTime;
    int milisec;
    
    /*--------------------------------------- */
    /* 이벤트로부터 SOE 이벤트 시간 추출                 */
    /*--------------------------------------- */
    curTime = localtime(&devEvent->updateTime.tv_sec);
    milisec = devEvent->updateTime.tv_usec / 1000;

    /*--------------------------------------- */
    /*  EVENT TYPE 에 따른 정보 처리...             */
    /*--------------------------------------- */ 
    devNo   = devEvent->devNo - 1;          // 계전기 번호....1,2...64
    pointNo = devEvent->pointNo - 1;        // 계전기-포인트 번호....1,2...1024
    status  = devEvent->state;              // 계전기-상태정보
    
    /* DEVICE Offline 처리 */
    if((devNo < 0) || (devNo >= MAX_DEVICE))    		return (0);
   	if((pointNo < 0) || (pointNo >= MAX_DEV_DI_POINT))  return (0);
   		
    
    /* ----------------------------------- */
    /* DEVICE 별 포인트 정보 Update...          */
    /* ----------------------------------- */               
    dev = (SDP_DEVICE *) &shmPtr->deviceCFG[devNo];
    ioid = dev->comDevID;       // 계전기 통신ID

	/* 초기 상태덤프 후 ... 이벤트 발생 */
	if(dev->init_status == RESET)   return (0);
	
	/* ---------------------------------- */
    /* ESIO - Event 저장/Update...          */
    /* ---------------------------------- */ 
    logEvent_MPU(shmPtr, devEvent->eventCode, devNo+1, pointNo+1, status, 0, &devEvent->updateTime);
    
    if(devEvent->eventCode == ENT_SOE)
    {      
        diPoint = (POINT_BUF *) &dev->diPtBuf[pointNo];
        diPoint->status = status;    
        memcpy( (byte *) &diPoint->updateTime, (byte *) &devEvent->updateTime, sizeof(struct timeval));
    
        /* ------------------------------------ */
        /* 계전기 상태포인트 영역 Update... */
        /* ------------------------------------ */
        index1 = pointNo / 8;
        index2 = pointNo % 8;
    	if(status)   dev->cursts[index1] |= bitcode[index2];	
        else         dev->cursts[index1] &= (~bitcode[index2]);	 
    }

	/* ---------------------------- */
	/* SCAN-TASK 절체후 이벤트 발생금지...	*/
	/* ---------------------------- */
    if(opr->scanInitial)	return (0);
    	          
    /* ---------------------------------------------- */
    /* ESIO-Event 저장                                  */
    /* ---------------------------------------------- */
    front = soeQueue->front & 0xff;
    rear  = soeQueue->rear & 0xff;
    event = (SOEQ_ENTRY *) &soeQueue->Queue[front];
    memcpy( (byte *) event, (byte *) devEvent, sizeof(SOEQ_ENTRY));
    
    front = (front + 1) & 0xff;
    soeQueue->front  = front;
    eventNum = (front - rear + 256) % 256;

	if (eventNum >= 255)    soeQueue->overflow = 1;

	/* ---------------------------------------------- */
    /* LOG File 저장                                    */
    /* ---------------------------------------------- */
    if(devEvent->eventCode == ENT_SOE)
    {
        if(opr->soeDebug)
        Debug(console, ">>SOE (f/r=%03d/%03d) code=%02x, dev=%2d[%2d], pt=%3d, sts=%d ...[%4d/%2d/%2d %02d:%02d:%02d-%d ms] %s\n", 
            soeQueue->front, soeQueue->rear, devEvent->eventCode, devNo+1, ioid, pointNo+1, status,
            curTime->tm_year + 1900, curTime->tm_mon +1, curTime->tm_mday, curTime->tm_hour, curTime->tm_min, curTime->tm_sec, milisec, diPoint->ptNameStr);
        
        sprintf(buffer, "SOE: dev=%2d[%2d], point=%3d, state=%d ... [%4d-%02d-%02d %02d:%02d:%02d-%03d ms] %s", 
            devNo+1, ioid, pointNo+1, status, curTime->tm_year + 1900, curTime->tm_mon +1, curTime->tm_mday, 
            curTime->tm_hour, curTime->tm_min, curTime->tm_sec, milisec, diPoint->ptNameStr);    
    }
    else
    {
        if(opr->soeDebug)
        Debug(console, ">>DEV (f/r=%03d/%03d) code=%02x, dev=%2d[%2d], pt=%3d, sts=%d ...[%4d/%2d/%2d %02d:%02d:%02d-%d ms] %s \n", 
            soeQueue->front, soeQueue->rear, devEvent->eventCode, devNo+1, ioid, pointNo+1, status,
            curTime->tm_year + 1900, curTime->tm_mon +1, curTime->tm_mday, curTime->tm_hour, curTime->tm_min, curTime->tm_sec, milisec, dev->devNameStr);

        sprintf(buffer, "DEV: dev=%2d[%2d], state=%d, %s", devNo, ioid, status,  dev->devNameStr);
    }
    
    LogFile_MPU(shmPtr, devEvent->eventCode, buffer, strlen(buffer));
    
    return (0);
}
