//==============================================================================
//
// File:	lib_61850Shm.c
//
//==============================================================================

//
// Include File
//
#include    "61850Shm.h"

static SHM_MEMORY *pShm = NULL;

int d61850_shm_lock_fd = -1;     /* shm lock file : global variable */

/*
 *	Shared memory lock file open
 */
int	d61850Shm_Init (SHM_MEMORY *p)
{
	pShm = p;

	/* initial file lock */
	if ((d61850_shm_lock_fd = open (D61850_SHM_LOCK_FNAME, O_CREAT | O_RDWR, 0666)) < 0)
	{
		printf( "61850 lock file(%s) open error <%d:%s>\n",
				D61850_SHM_LOCK_FNAME, errno, strerror(errno) );
		return( NOK );
	}

    return 0;
}

/*
 *	Shared memory lock file close
 */
void d61850Shm_End (void)
{
	if (d61850_shm_lock_fd != -1)
	{
		close (d61850_shm_lock_fd);
		d61850_shm_lock_fd = -1;
	}
}

/*
 * Set All Device config61850
 */
int d61850Shm_SetDevice61850Config (int max, short *config, char name[MAX_DEVICE][130], char ip[MAX_DEVICE][16])
{
	int i;
	DEVICE_61850 *pP;

	if (max != MAX_DEVICE)
	{
		printf ("d61850Shm_SetDevice61850Config : max(%d) != MAX_DEVICE(%d) error\n", max, MAX_DEVICE);
		return -1;
	}
	if (pShm == NULL)
		return -1;
	D61850_SHM_LOCK_WRITE();

	pP = &pShm->device61850[0];
	for (i=0; i<max; i++, pP++)
	{
		if (config[i])
		{
			pP->config61850 = 1;
			memcpy (pP->name, name[i], sizeof(pP->name));
			memcpy (pP->ipAddress, ip[i], sizeof(pP->ipAddress));
		}
		else
		{
			//pP->config61850  = 0;
			//pP->name[0]      = 0;
			//pP->ipAddress[0] = 0;
			memset (pP, 0, sizeof(*pP));
		}
	}

	pShm->esioConfig.modInit_OK = SET;

	D61850_SHM_UNLOCK();
	return 0;
}

/*
 * Set All Do Point config61850
 */
int d61850Shm_SetDo61850Config (byte devNo, int max, short *config, char name[MAX_DEV_DO_POINT][130])
{
	int i;
	short LastNo = -1;
	POINT_61850 *pP;

	if (devNo < 1 || devNo > MAX_DEVICE)
	{
		printf ("d61850Shm_SetDo61850Config : devNo(%d) error\n", devNo);
		return -1;
	}
	if (max != MAX_DEV_DO_POINT)
	{
		printf ("d61850Shm_SetDo61850Config : max(%d) != MAX_DEV_DO_POINT(%d) error\n",
			max, MAX_DEV_DO_POINT);
		return -1;
	}
	if (pShm == NULL)
		return -1;
	D61850_SHM_LOCK_WRITE();

	pP = &pShm->device61850[devNo-1].doPtBuf[0];
	for (i=0; i<max; i++, pP++)
	{
		if (config[i])
		{
			pP->config61850 = 1;
			/* ????? POINT_61850 구조체 : name 영역 미사용 */
			//memcpy (pP->name, name[i], sizeof(pP->name));
			if (LastNo < i)
				LastNo = i;
		}
		else
		{
			pP->config61850 = 0;
			/* ????? POINT_61850 구조체 : name 영역 미사용 */
			//pP->name[0] = 0;
		}
	}
	pShm->device61850[devNo-1].devDoLastNo = LastNo + 1;

	D61850_SHM_UNLOCK();
	return 0;
}

/*
 * Set All Di Point config61850
 */
int d61850Shm_SetDi61850Config (byte devNo, int max, short *config, char name[MAX_DEV_DI_POINT][130])
{
	int i;
	short LastNo = -1;
	POINT_61850 *pP;

	if (devNo < 1 || devNo > MAX_DEVICE)
	{
		printf ("d61850Shm_SetDi61850Config : devNo(%d) error\n", devNo);
		return -1;
	}
	if (max != MAX_DEV_DI_POINT)
	{
		printf ("d61850Shm_SetDi61850Config : max(%d) != MAX_DEV_DI_POINT(%d) error\n",
			max, MAX_DEV_DI_POINT);
		return -1;
	}
	if (pShm == NULL)
		return -1;
	D61850_SHM_LOCK_WRITE();

	pP = &pShm->device61850[devNo-1].diPtBuf[0];
	for (i=0; i<max; i++, pP++)
	{
		if (config[i])
		{
			pP->config61850 = 1;
			/* ????? POINT_61850 구조체 : name 영역 미사용 */
			//memcpy (pP->name, name[i], sizeof(pP->name));
			if (LastNo < i)
				LastNo = i;
		}
		else
		{
			pP->config61850 = 0;
			/* ????? POINT_61850 구조체 : name 영역 미사용 */
			//pP->name[0] = 0;
		}
	}
	pShm->device61850[devNo-1].devDiLastNo = LastNo + 1;

	D61850_SHM_UNLOCK();
	return 0;
}

/*
 * Set All Ai Point config61850
 */
int d61850Shm_SetAi61850Config (byte devNo, int max, short *config, char name[MAX_DEV_AI_POINT][130])
{
	int i;
	short LastNo = -1;
	POINT_61850 *pP;
	

	if (devNo < 1 || devNo > MAX_DEVICE)
	{
		printf ("d61850Shm_SetAi61850Config : devNo(%d) error\n", devNo);
		return -1;
	}
	if (max != MAX_DEV_AI_POINT)
	{
		printf ("d61850Shm_SetAi61850Config : max(%d) != MAX_DEV_AI_POINT(%d) error\n",
			max, MAX_DEV_AI_POINT);
		return -1;
	}
	if (pShm == NULL)
		return -1;
		

    
	D61850_SHM_LOCK_WRITE();

	pP = &pShm->device61850[devNo-1].aiPtBuf[0];
	for (i=0; i<max; i++, pP++)
	{
		if (config[i])
		{
			pP->config61850 = 1;
			/* ????? POINT_61850 구조체 : name 영역 미사용 */
			//memcpy (pP->name, name[i], sizeof(pP->name));
			if (LastNo < i)
				LastNo = i;
		}
		else
		{
			pP->config61850 = 0;
			/* ????? POINT_61850 구조체 : name 영역 미사용 */
			//pP->name[0] = 0;
		}
	}
	pShm->device61850[devNo-1].devAiLastNo = LastNo + 1;

	D61850_SHM_UNLOCK();
	
	
	    
	return 0;
}

/*
 * Update Di Point Data
 */
int d61850Shm_UpdateDiData (byte devNo, word pointNo, struct timeval *pUpdateTime, byte data)
{
	POINT_61850	*pPnt;
    OPR_MSG         *opr;
    
	if (devNo < 1 || devNo > MAX_DEVICE || pointNo < 1 || pointNo > MAX_DEV_DI_POINT)
	{
		printf ("d61850Shm_UpdateDiData : devNo(%d) or pointNo(%d) error\n", devNo, pointNo);
		return -1;
	}
	if (pShm == NULL)
		return -1;
    
    opr      = (OPR_MSG *)   &pShm->opr_msg;
	
	D61850_SHM_LOCK_WRITE();

	pPnt = &pShm->device61850[devNo-1].diPtBuf[pointNo-1];
	memcpy (&pPnt->updateTime, pUpdateTime, sizeof(pPnt->updateTime));
	pPnt->state = data ? 1 : 0;

	D61850_SHM_UNLOCK();
	
	if(opr->ntpDebug)
    printf("61850> ... Update... DI ... dev=%d, point=%d, stats=%d\n", devNo, pointNo, data);	       
	    
	return 0;
}

/*
 * Update Ai Point Data
 */
int d61850Shm_UpdateAiData (byte devNo, word pointNo, struct timeval *pUpdateTime, double data)
{
	POINT_61850	*pPnt;
	OPR_MSG         *opr;

	if (devNo < 1 || devNo > MAX_DEVICE || pointNo < 1 || pointNo > MAX_DEV_AI_POINT)
	{
		printf ("d61850Shm_UpdateAiData : devNo(%d) or pointNo(%d) error\n", devNo, pointNo);
		return -1;
	}
	if (pShm == NULL)
		return -1;
		
	opr      = (OPR_MSG *)   &pShm->opr_msg;
			
	D61850_SHM_LOCK_WRITE();

	pPnt = &pShm->device61850[devNo-1].aiPtBuf[pointNo-1];
	memcpy (&pPnt->updateTime, pUpdateTime, sizeof(pPnt->updateTime));
	
	/* -------------------------------- */
    /*	한빛이디에스 시험 : 이벤트 정보 => long 형태	*/
    /*	- 61850 데이터 형식 : double			*/
    /* -------------------------------- */
	//pPnt->floatData = (float) data;
	pPnt->floatData = data;

	D61850_SHM_UNLOCK();
	
	if(opr->ntpDebug)
    printf("61850> ... Update... AI ... dev=%d, point=%d, data = %4.2f \n", devNo, pointNo,  data);	       
	    
	return 0;
}

/*
 * 61850 계전기.... Push SOE Event
 */
int d61850Shm_PushSoe (SOEQ_ENTRY *pSoe)
{
    int     ioid, pointNo;
    byte    front, rear;
    int     eventNum;

    SOE_QUEUE       *soeQueue;
    OPR_MSG         *opr;
    CONSOLE_INFO	*console;
    DEVICE_61850    *dev61850;
    
    struct tm *curTime;
    int milisec;
    
	if (pShm == NULL)
		return -1;
	
	D61850_SHM_LOCK_WRITE();
		
    /* ------------------------------------ */
    /*  실제 계전기 번호로 변환...          */
    /* ------------------------------------ */
    ioid    = pSoe->devNo;         // 61850 계전기 번호....
    pointNo = pSoe->pointNo;       // 61850 계전기-포인트 번호....
    
    dev61850 = (DEVICE_61850 *) &pShm->device61850[ioid-1];
    
    /* 확인... ???????????????????? */
    pSoe->devNo = dev61850->src_ioid;

    /* ------------------------------------ */
    /*  MPU 전송 SOE 정보 생성...          */
    /* ------------------------------------ */
    opr      = (OPR_MSG *)   &pShm->opr_msg;
	soeQueue = &pShm->soeQueue;
	front = soeQueue->front & 0xff;
    rear  = soeQueue->rear & 0xff;
    
   
    /* SOE-Event 저장 */
    memcpy (&soeQueue->Queue[front], pSoe, sizeof(SOEQ_ENTRY));
    soeQueue->front  = (soeQueue->front + 1) & 0xff;
    eventNum = (front - rear + 256) % 256;

	if (eventNum >= 255)
		soeQueue->overflow = 1;

	D61850_SHM_UNLOCK();

    /* ------------------------------------ */
    /*  SOE Debugging.... Console           */
    /* ------------------------------------ */
    if(opr->soeDebug)
    {   
        console = (CONSOLE_INFO *)   &pShm->console;
        
        /* SOE 이벤트 시간 추출 */
        curTime = localtime(&pSoe->updateTime.tv_sec);
        milisec = pSoe->updateTime.tv_usec / 1000;
        
        dev61850  = (DEVICE_61850 *) &pShm->device61850[ioid - 1];
        
        Debug(console, ">>SOE-61850 (f/r=%03d/%03d) code=%02x, dev=%2d[%2d], pt=%3d, sts=%d ...[%4d/%2d/%2d %02d:%02d:%02d-%d ms] \n", 
            soeQueue->front, soeQueue->rear, pSoe->eventCode, pSoe->devNo, ioid, pointNo, pSoe->state,
            curTime->tm_year + 1900, curTime->tm_mon +1, curTime->tm_mday, curTime->tm_hour, curTime->tm_min, curTime->tm_sec, milisec);
    }     

	return 0;
}

/*
 * Pop SOE Event
 */
int d61850Shm_PopSoe (SOEQ_ENTRY *pSoe)
{
    byte    front, rear;
	SOE_QUEUE *soeQueue;
    int     eventNum;

	if (pShm == NULL)
		return -1;

	D61850_SHM_LOCK_WRITE();

	soeQueue = &pShm->soeQueue;
	front = soeQueue->front & 0xff;
    rear  = soeQueue->rear & 0xff;
    eventNum = (front - rear + 256) % 256;
	if (eventNum > 0)
	{
		/* SOE-Event Pop */
		memcpy (pSoe, &soeQueue->Queue[rear], sizeof(SOEQ_ENTRY));
		soeQueue->rear  = (soeQueue->rear + 1) & 0xff;
	}
	D61850_SHM_UNLOCK();

	return (eventNum ? 1 : 0);
}

/*
 *	Control Data를 가져온다.
 *	  return 0 : 제어없음
 *		     1 : 제어 있음
 *			 -1 : Error
 */
int d61850Shm_GetControlData (byte devNo, short *cntPoint, short *cntTcf)
{
	int ret;
	DEVICE_61850 *pDev;

	if (devNo < 1 || devNo > MAX_DEVICE)
	{
		printf ("d61850Shm_GetControlData : devNo(%d) error\n", devNo);
		return -1;
	}
	if (pShm == NULL)
		return -1;

	D61850_SHM_LOCK_WRITE();

	pDev = &pShm->device61850[devNo-1];
	if (pDev->controlReq)
	{
		if (pDev->cntPoint < 1 || pDev->cntPoint > MAX_DEV_DO_POINT ||
			(pDev->cntTcf != 1 && pDev->cntTcf != 2))
		{
			printf ("d61850Shm_GetControlData : devNo(%d) cntPoint(%d) cntTcf(%d) error\n",
				devNo, pDev->cntPoint, pDev->cntTcf);
			ret = -1;
		}
		else
		{
			*cntPoint = pDev->cntPoint;
			*cntTcf   = pDev->cntTcf;
			ret = 1;
		}
		pDev->controlReq = 0;
	}
	else
		ret = 0;

	D61850_SHM_UNLOCK();

	return ret;

}

/*
 *	Control Data를 설정한다.
 *	  return 0 : OK
 *			 -1 : Error
 */
int d61850Shm_SetControlData (byte devNo, short cntPoint, short cntTcf)
{
	DEVICE_61850 *pDev;

	if (devNo < 1 || devNo > MAX_DEVICE ||
		cntPoint < 1 || cntPoint > MAX_DEV_DO_POINT ||
		(cntTcf != 1 && cntTcf != 2))
	{
		printf ("d61850Shm_SetControlData : devNo(%d) cntPoint(%d) cntTcf(%d) error\n",
			devNo, cntPoint, cntTcf);
		return -1;
	}
	if (pShm == NULL)
		return -1;

	D61850_SHM_LOCK_WRITE();

	pDev = &pShm->device61850[devNo-1];
	if (pDev->controlReq)
	{
		printf ("d61850Shm_SetControlData : devNo(%d) cntPoint(%d) cntTcf(%d) - control in progress error\n",
			devNo, cntPoint, cntTcf);
		D61850_SHM_UNLOCK();
		return -1;
	}
	pDev->cntPoint = cntPoint;
	pDev->cntTcf   = cntTcf;
	pDev->controlReq = 1;

	D61850_SHM_UNLOCK();

	return 0;
}

/*
 *	디바이스 상태를 설정한다.
 *	  return 0 : OK
 *			 -1 : Error
 */
int d61850Shm_SetDeviceStatus (byte devNo, short devStatus)
{
	short online;
	short onlineOld;
	//SOEQ_ENTRY sEntry;
	DEVICE_61850 *pDev;
    //SDP_DEVICE  *dev;
	if (devNo < 1 || devNo > MAX_DEVICE)
	{
		printf ("d61850Shm_SetDeviceStatus : devNo(%d) error\n", devNo);
		return -1;
	}
	if (pShm == NULL)
		return -1;

	D61850_SHM_LOCK_WRITE();

	pDev = &pShm->device61850[devNo-1];
	pDev->devStatus = devStatus;
	onlineOld = pDev->online;
	online = pDev->online = (devStatus & DEV_STATUS_ONLINE_BIT) ? 1 : 0;

#if 0
    dev = (SDP_DEVICE *) &pShm->deviceCFG[devNo-1];
    
    dev->online = pDev->online;
    if(dev->online) dev->devStatus |= 0x01;
    else            dev->devStatus &= (~0x01);             // Online-Bit RESET    
#endif
        
//printf("\n=================================\n");
//printf("61850> Device(%d)  Online (shm).... %d\n", devNo, pDev->online);
//printf("=================================\n");

	D61850_SHM_UNLOCK();

#if 0
    /* ------------------------------------------------------------ */
    /*  계전기 Online/Offline 이벤트는... wdtMain() 에서 생성...    */
    /* ------------------------------------------------------------ */
	if (onlineOld != online)
	{
		sEntry.eventCode = online ? ENT_DEV_ONLINE : ENT_DEV_OFFLINE;
		sEntry.devNo     = devNo;
		sEntry.pointNo   = 0;
		sEntry.state     = online;
		gettimeofday (&sEntry.updateTime, NULL);

		d61850Shm_PushSoe (&sEntry);
	}
#endif

	return 0;
}

/*
 *	계전기 통신 수를 설정한다.
 *	  return 0 : OK
 *			 -1 : Error
 */
int d61850Shm_AddCommCount (byte devNo, int send, int recv)
{
	DEVICE_61850 *pDev;

	if (devNo < 1 || devNo > MAX_DEVICE)
	{
		printf ("d61850Shm_SetCommCount : devNo(%d) error\n", devNo);
		return -1;
	}
	if (pShm == NULL)
		return -1;

	D61850_SHM_LOCK_WRITE();

	pDev = &pShm->device61850[devNo-1];
	if (send)
	{
		if (++pDev->comSndCount > 1000)
			pDev->comSndCount = 1;
	}
	if (recv)
	{
		if (++pDev->comRcvCount > 1000)
			pDev->comRcvCount = 1;
	}

	D61850_SHM_UNLOCK();

	return 0;
}

