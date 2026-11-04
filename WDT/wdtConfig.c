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
#include	"external.h"


/*----------------------------------------------------------------------------
* Function Name : pointParaConfig()
* 수행내용: 데이터베이스중 Runtime 포인트 정보를 DB에서 읽어 온다.
* ArgList :
*   1. ioid - 초기화 되고자 하는 모듈번호
*   2. buf  - 초기화 되는 데이터베이스상의 Pointer Address
* Return  :  
---------------------------------------------------------------------------- */   
int pointParaConfig()
{
    int     i;
    int     devNo, devPt;
    word    pointMax, pointOffset, localIndex;
    word    modBase, modIndex;
    byte    dbport, dbpoint, pointDelta;
    short   iccpType, iccpRes1, iccpRes2;
    word    iccpIndex;
    
    DB_POINT_BUF    *devPoint;
    POINT_BUF       *ptBuf;
    SDP_DEVICE      *dev;
    
    /* 계전기별 포인트 정보 초기화 */
    for(devNo=0; devNo < MAX_DEVICE; devNo++)
    {
        /* -------------------------------------------- */
        /* 계전기 데이터베이스 초기화                   */
        /* -------------------------------------------- */
        dev   = (SDP_DEVICE *) deviceCFG[devNo];  
        dev->regDiPointNum = 0;
        dev->regAiPointNum = 0;
    }
    
    /* -------------------------------------------------------- */
    /*  데이터베이스 상의 포인트 DB 초기화...(max 4096)         */
    /* -------------------------------------------------------- */
    for(i = 0; i < MAX_DBASE_POINT; i++)
    {
        /* DB 상의 포인트 구조체 ... */
        devPoint = (DB_POINT_BUF *) &rtudb->pointBuf[i];

        devNo = devPoint->devNo;                                        // 장치 ID [1..64]
        devPt = (devPoint->devPt[0]*256) + devPoint->devPt[1];          // 장치-포인트 [1...4096]
        
        /* ---------------------------------------- */
        /* 입력상의 오류 검증...                    */
        /* ---------------------------------------- */
        if((devNo < 1) || (devNo > MAX_DEVICE))         continue;
        if((devPt < 1) || (devPt > MAX_DEV_DI_POINT))   continue;
        if(devPoint->devType == NULL_DEV)   			continue;

        /* ------------------------------------------------ */
        /*  계전기 정보 추출 : 계전기별 포인트 구성정보     */
        /* ------------------------------------------------ */
        devNo = devNo - 1;
        devPt = devPt - 1;
        
        dev   = (SDP_DEVICE *) deviceCFG[devNo]; 
     
        /* ICCP 관련 포인트 정보 추출 */
        iccpType    = devPoint->iccpPointType;		// SDP POINT : ICCP 포인트 TYPE, SDI/SDO/SAI/DDI/DAI/QDI/QAI/TDI/TAI/DEV
        iccpIndex   = (devPoint->iccpPointIndex[0]*256) + devPoint->iccpPointIndex[1]; 
        iccpRes1    = devPoint->reserved1;
        iccpRes2    = devPoint->reserved2;
        
        /* 계전기 관련 포인트 정보 추출 */
        pointMax    = (devPoint->pointMax[0]*256) + devPoint->pointMax[1];          // SDP POINT :Point MAX Scale 값
        pointOffset = (devPoint->pointOffset[0]*256) + devPoint->pointOffset[1];    // SDP POINT :Point Offset Scale 값
        pointDelta  = devPoint->pointDelta;                                         // SDP POINT :Point Delts Scale 값
        
        localIndex  = (devPoint->localIndex[0]*256) + devPoint->localIndex[1];      // SDP POINT : Local Index
        
        modBase  = (devPoint->modBase[0]*256) + devPoint->modBase[1];               // SDP POINT :MODBUS base Address
        modIndex  = (devPoint->modIndex[0]*256) + devPoint->modIndex[1];            // SDP POINT :MODBUS index
        
        dbport   = devPoint->port;            // SDP POINT :주장치 PORT 번호  [1..16]
        dbpoint  = devPoint->point;           // SDP POINT :주장치 POINT 번호 [1..64] 
        
        /* ------------------------------------------------ */
        /* 계전기/장치 TYPE에 따른 ... 포인트 초기화        */
        /* ------------------------------------------------ */
        if((devPoint->devType == VDI_POINT) || (devPoint->devType == DI_POINT))
        {
            ptBuf = (POINT_BUF *) &dev->diPtBuf[devPt];
            
            ptBuf->config = SET;
            ptBuf->devNo = devNo + 1;               // SDP POINT : device 번호 [1..32] 
            ptBuf->devPt = devPt + 1;               // SDP POINT : device 포인트 번호 [1..1024]
            
            ptBuf->ptType   = devPoint->ptType;     // SDP POINT : device 포인트 TYPE   
            ptBuf->ptConfig = devPoint->ptConfig;   // SDP POINT : Point Config
            
            /* ICCP 관련 포인트 정보 초기화 */
            ptBuf->iccpType  = iccpType;
            ptBuf->iccpIndex = iccpIndex;
            ptBuf->iccpRes1  = iccpRes1;
            ptBuf->iccpRes2  = iccpRes2;
            
            dev->regDiPointNum++;	// 계전기별 DI 등록 포인트 수 

#if 0            
            /* -------------------------------------------- */
            /* 지멘스 계전기 : 계전기 통신 Active Flag      */
            /* -------------------------------------------- */
            if(dev->type == TYPE_MOD_SIEMENS)
            {      
                if(ptBuf->ptType == ACT_PT)
                {
                    Debug(console, "----------------------------------\n");
                    Debug(console, " ==> SIEMENS-Device (%2d) Com Active Flag ....!\n", devNo+1);
                    Debug(console, "----------------------------------\n");
                    
                    dev->activeMode = SET;
                    dev->activeFlag = SET; 
                }
            }
#endif            
        }
        else if((devPoint->devType == VDO_POINT) || (devPoint->devType == DO_POINT))
        {
            ptBuf = (POINT_BUF *) &dev->doPtBuf[devPt];
            ptBuf->config = SET; 
            ptBuf->devNo = devNo + 1;
            ptBuf->devPt = devPt + 1;           

            ptBuf->ptType   = devPoint->ptType;     // SDP POINT : device 포인트 TYPE   
            ptBuf->ptConfig = devPoint->ptConfig;   // SDP POINT : Point Config
            
            /* ICCP 관련 포인트 정보 초기화 */
            ptBuf->iccpType  = iccpType;
            ptBuf->iccpIndex = iccpIndex;
            ptBuf->iccpRes1  = iccpRes1;
            ptBuf->iccpRes2  = iccpRes2;
        }    
        else if((devPoint->devType == VAI_POINT) || (devPoint->devType == AI_POINT))
        {
            ptBuf = (POINT_BUF *) &dev->aiPtBuf[devPt];
            ptBuf->config = SET;
            ptBuf->devNo = devNo + 1;
            ptBuf->devPt = devPt + 1;

            ptBuf->ptType   = devPoint->ptType;     // SDP POINT : device 포인트 TYPE   
            ptBuf->ptConfig = devPoint->ptConfig;   // SDP POINT : Point Config
  
            /* ICCP 관련 포인트 정보 초기화 */
            ptBuf->iccpType  = iccpType;
            ptBuf->iccpIndex = iccpIndex;
            ptBuf->iccpRes1  = iccpRes1;
            ptBuf->iccpRes2  = iccpRes2;
            
            dev->regAiPointNum++;	// 계전기별 AI 등록 포인트 수 
            
        }
#if 0        
        else if((devPoint->devType == RAO_POINT) || (devPoint->devType == AO_POINT))
        {
            ptBuf = (POINT_BUF *) &dev->aoPtBuf[devPt];	
            ptBuf->config = SET;
            ptBuf->devNo = devNo + 1;
            ptBuf->devPt = devPt + 1;
            
            /* ICCP 관련 포인트 정보 초기화 */
            ptBuf->iccpType  = iccpType;
            ptBuf->iccpIndex = iccpIndex;
            ptBuf->iccpRes1  = iccpRes1;
            ptBuf->iccpRes2  = iccpRes2;
        }
#endif
        else    
        {
            continue;
        }            
        
        /* ------------------------------------------------ */
        /*  포인트별 Information : 포인트 구성정보          */
        /* ------------------------------------------------ */
        ptBuf->devType  = devPoint->devType;                        // SDP POINT : devic TYPE  */ 
        ptBuf->ptType   = devPoint->ptType;                         // SDP POINT : device 포인트 TYPE  */    
        ptBuf->ptConfig = devPoint->ptConfig;                       // SDP POINT : Point Config
        ptBuf->dbmax    = pointMax;                                 // SDP POINT :Point MAX Scale 값
        ptBuf->pointOffset  = pointOffset;                          // SDP POINT :Point MAX Scale 값
        ptBuf->pointDelta   = pointDelta;                           // SDP POINT :Point MAX Scale 값
        ptBuf->localIndex   = localIndex;                           // SDP POINT :Point MAX Scale 값
        
        
        ptBuf->debounce = ptBuf->dbmax & 0xff;
        ptBuf->cntrTime = ptBuf->dbmax;
        
        if(ptBuf->debounce <= 5)   		ptBuf->debounce = 5;   
        if(ptBuf->cntrTime <= 500)   	ptBuf->cntrTime = 500;   
        
        ptBuf->dbport  = dbport;
        ptBuf->dbpoint = dbpoint;
        
        ptBuf->modBase  = modBase;                                  // SDP POINT :MODBUS base Address
        ptBuf->modIndex = modIndex;                                 // SDP POINT :MODBUS index
        
        /* 포인트/ON/OFF 이름 정보 */
        memcpy( (byte *) ptBuf->ptNameStr, (byte *) devPoint->ptNameStr, 40);
        memcpy( (byte *) ptBuf->onStr, (byte *) devPoint->onStr, 10);
        memcpy( (byte *) ptBuf->offStr, (byte *) devPoint->offStr, 10);

        /* DB 상의 운영 포인트 수 */
        opr->max_sdpPoint = i+1;    
    }

    
    printf("wdt> => POINT Config End...(CU Point = %2d)!\n", opr->max_sdpPoint);
    
    return (0);
}



/**************************************************************** 
*   FUNCTION : scanParaConfig() 
*   - DATABASE parameter initial 
****************************************************************/ 
/* 이 함수 사용안 함.. 그래서 deviceParaConfig 중복되나 보다. */ 

int scanParaConfig() 
{ 
    int     index;
    int     scanid;
    
    DB_SCAN_CONFIG		*dbScan;
    SCAN_CONFIG         *scan;

    for(scanid=0; scanid < MAX_SCAN_PORT; scanid++)
    {
        dbScan = (DB_SCAN_CONFIG *) &rtudb->scanConfig[scanid];
        scan   = (SCAN_CONFIG *) scanCFG[scanid];                
        
        scan->useFlag    = dbScan->useFlag;              // SDP SCAN : Channel 사용유무
        scan->targetID  = dbScan->targetID;             // SDP SCAN : ESIO Target Module-ID, 0:사용않함, 1: ESIO1, 2:ESIO2, 3:ESIO3, 4:ESIO4, 5:SIO
        
        scan->protocol  = dbScan->protocol;             // SDP SCAN : SCAN 통신 프로토콜 
        scan->comMode   = dbScan->comMode;              // SDP SCAN : SCAN 통신모드
        scan->comPort   = dbScan->comPort;			    // SDP SCAN : SCAN 통신포트
        scan->comSpeed  = dbScan->comSpeed;             // SDP SCAN : SCAN 통신속도

        scan->comDelay  = dbScan->comDelay;			    // SDP SCAN : SCAN 통신간격
        scan->offCount  = dbScan->offCount;			    // SDP SCAN : SCAN Offline Count
        scan->chgMode   = dbScan->chgMode;              // SDP SCAN : SCAN Change Mode
        
        scan->function1 = dbScan->function1;            // SDP SCAN : SCAN Function#1
        scan->function2 = dbScan->function2;            // SDP SCAN : SCAN Function#2
        scan->function3 = dbScan->function3;            // SDP SCAN : SCAN Function#3
        
        memcpy((byte *)&scan->scanNameStr[0], (byte *) &dbScan->scanNameStr[0], 20);       // SDP SCAN : SCAN Name String
        
        
        /* SCAN : 할당된 계전기 초기화 ... */    	
        /* deviceParaConfig에서 SCAN에 넣는다. */
        scan->devNum       = 0;
        for(index=0; index< MAX_DEVICE; index++)    scan->device[index] = 0;
        
    }
        
    printf("wdt> => SCAN Parameter init...!\n");
    
    return (0);
}

/**************************************************************** 
*   FUNCTION : deviceParaConfig() 
*   - DATABASE parameter initial 
****************************************************************/ 
int deviceParaConfig() 
{ 
    int     i;
    int     ioid;
    int     devid;
    int     scanid;
    int     index;
    int		state;
    
    DB_SDP_DEVICE	*dbMOD;
    SDP_DEVICE      *dev;
    DEVICE_61850    *dev61850;
    SCAN_CONFIG     *scan;
    
    MODBUS_READ		*readBlock;
    MODBUS_WRITE	*writeBlock;
    MODBUS_READ		*timeBlock;
    
    DB_MODBUS_READ	*readDB;
    DB_MODBUS_WRITE	*writeDB;
    DB_MODBUS_READ	*timeDB;
    
    DB_MODBUS_PROFILE	*modbusDB;
    DB_SCAN_CONFIG		*dbScan;
    
    /* ------------------------ */
    /* ESIO 할당 계전기 초기화  */
    /* ------------------------ */
    for(i=0; i < MAX_DEVICE; i++)   esioCFG->scanDevice[i] = 0;
    esioCFG->scanIndex = 0;
    
    /* ------------------------------------------------ */
    /*  SCAN 구조체 초기화                              */
    /* ------------------------------------------------ */
    for(scanid=0; scanid < MAX_SCAN_PORT; scanid++)
    {
        /* SCAN 구조체 초기화 */
        //bzero8248((byte *) scanCFG[scanid], sizeof(SCAN_CONFIG));
        
        dbScan = (DB_SCAN_CONFIG *) &rtudb->scanConfig[scanid];
        scan   = (SCAN_CONFIG *) scanCFG[scanid];                
        
        // targetID : scan을 사용하는 ESIO ID
        scan->targetID  = dbScan->targetID;             // SDP SCAN : ESIO Target Module-ID, 0:사용않함, 1: MPU, 2: ESIO1, 3:ESIO2, 4:ESIO3, 5:ESIO4, 6:SIO
       
        //printf("scan init... %d\n", scanid);
        
        /* ------------------------------------ */
        /* 해당 ESIO 에 속한 SCAN 구조체        */
        /* ------------------------------------ */
        if(scan->targetID == opr->esioID)
        {
            scan->useFlag   = dbScan->useFlag;              // SDP SCAN : Channel 사용유무
            scan->protocol  = dbScan->protocol;             // SDP SCAN : SCAN 통신 프로토콜 
            scan->comMode   = dbScan->comMode;              // SDP SCAN : SCAN 통신모드
            scan->comPort   = dbScan->comPort;			    // SDP SCAN : SCAN 통신포트
            scan->comSpeed  = dbScan->comSpeed;             // SDP SCAN : SCAN 통신속도

            scan->comDelay  = dbScan->comDelay;			    // SDP SCAN : SCAN 통신간격
            scan->offCount  = dbScan->offCount;			    // SDP SCAN : SCAN Offline Count
            scan->chgMode   = dbScan->chgMode;              // SDP SCAN : SCAN Change Mode
        
            scan->function1 = dbScan->function1;            // SDP SCAN : SCAN Function#1
            scan->function2 = dbScan->function2;            // SDP SCAN : SCAN Function#2
            scan->function3 = dbScan->function3;            // SDP SCAN : SCAN Function#3
        
            memcpy((byte *)&scan->scanNameStr[0], (byte *) &dbScan->scanNameStr[0], 20);       // SDP SCAN : SCAN Name String
        
            /* -------------------------------- */
            /* SCAN : 할당된 계전기 초기화 ...  */   
            /* -------------------------------- */ 	
            scan->devNum       = 0;
            // 밑 device 처리할 때 ..처리됨.
            for(index=0; index< MAX_DEVICE; index++)    scan->device[index] = 0;
            
        }
    }
     
    /* ------------------------------------------------ */
    /*  계전기 구조체 초기화                            */
    /* ------------------------------------------------ */    
    for(ioid=0; ioid < MAX_DEVICE; ioid++)
    {
        /* -------------------------------------------- */
        /* 계전기 데이터베이스 초기화                   */
        /* -------------------------------------------- */
        //bzero8248((byte *) deviceCFG[ioid], sizeof(SDP_DEVICE));
        
        dbMOD = (DB_SDP_DEVICE *) &rtudb->deviceConfig[ioid];
        dev   = (SDP_DEVICE *) deviceCFG[ioid];                
        
        dev->scanPort     = dbMOD->scanPort;            // SDP DEVICE : 계전기 통신 포트
        
        //printf("device init... %d... scanport=%d \n", ioid, dev->scanPort);
        
        /* -------------------------------------------- */
        /* ESIO 별 계전기 할당 정보 초기화              */
        /* -------------------------------------------- */
        if((dev->scanPort > 0) && (dev->scanPort <= MAX_SCAN_PORT))
        {   
            /* SCAN 구조체내 할당 계전기  지정 */    
            scan = (SCAN_CONFIG *) scanCFG[dev->scanPort - 1];
            scan->device[scan->devNum++] = ioid;            // 계전기 번호, 0,1,2... 64
            
            dev->targetID = scan->targetID;     // ESIO Target ID, 0:사용않함, 1: SIO, 2:ESIO1, 3:ESIO2, 4:ESIO3, 5:ESIO4, 6:RTU, 7:MPU1, 8:MPU2
            
            /* -------------------------------------------- */
            /* SCAN 포트가 해당 ESIO 보드에 할당된 경우...  */
            /* -------------------------------------------- */        
            if(scan->targetID == opr->esioID)                   // 0: 사용않함, 1: MPU, 2: ESIO1, 3: ESIO2, 4: ESI3, 5: ESIO4, 6: SIO
            {      
                /* ESIO 계전기 리스트에 추가 */
                esioCFG->scanDevice[esioCFG->scanIndex] = ioid + 1;
                esioCFG->scanIndex = (esioCFG->scanIndex + 1) & 0x3f;
                esioCFG->scanMaxNum = esioCFG->scanIndex;       
            }
            else
            {
                continue;
            }
        }
        else
        {
            continue;
        }               
        
        /* -------------------------------------------- */
        /* SCAN 포트가 해당 ESIO 보드에 할당된 경우...  */
        /* -------------------------------------------- */

        /* SCAN 구조체내 할당 계전기  지정 */    
        scan = (SCAN_CONFIG *) scanCFG[dev->scanPort - 1];
            
        memcpy((byte *)&dev->devNameStr[0], (byte *) &dbMOD->devNameStr[0], 40);       // SDP DEVICE : 계전기 Name String
        
        dev->scan         = dbMOD->useFlag;             // SDP DEVICE : 계전기 사용유무
        dev->comDevID     = dbMOD->comDevID;            // SDP DEVICE : 기능모듈내 계전기 ID 
        dev->comDevIndex  = dbMOD->comDevIndex;         // SDP DEVICE : 기능모듈내 계전기 Index    
        dev->scanPort     = dbMOD->scanPort;            // SDP DEVICE : 계전기 통신 포트
        dev->type         = dbMOD->type;           	    // SDP DEVICE : 계전기 TYPE
        dev->dualENB      = dbMOD->dualENB;			    // SDP DEVICE : 계전기 - 계전기 이중화 여부
        dev->modbusFileNo = dbMOD->modbusFileNo;		// SDP DEVICE : 계전기 - MODBUS 프로파일
		
		
        dev->devDiPoint = (dbMOD->di_ptnum[0]*256) + dbMOD->di_ptnum[1];       // SDP DEVICE : 장치별 DI 포인트 수 
        dev->devDoPoint = (dbMOD->do_ptnum[0]*256) + dbMOD->do_ptnum[1];       // SDP DEVICE : 장치별 DO 포인트 수 
        dev->devAiPoint = (dbMOD->ai_ptnum[0]*256) + dbMOD->ai_ptnum[1];       // SDP DEVICE : 장치별 AI 포인트 수 
        dev->devAoPoint = (dbMOD->ao_ptnum[0]*256) + dbMOD->ao_ptnum[1];       // SDP DEVICE : 장치별 AO 포인트 수 
        dev->devCntPoint= (dbMOD->cnt_ptnum[0]*256)+ dbMOD->cnt_ptnum[1];     // SDP DEVICE : 장치별 Count 포인트 수 
		
        dev->netPort= (dbMOD->netPort[0]*256) + dbMOD->netPort[1];      // SDP DEVICE : HOST TCPIP Port번호 */
        dev->function1 = dbMOD->function1;		                        // SDP DEVICE : SCAN Function#1
		
		/* 계전기별 ACK 응답시간 조정 */
        if(scan->comMode != COM_TCPIP)  dev->reqAckDelay = dev->netPort;
        else                            dev->reqAckDelay = 0;
            
        /*  계전기 Network 정보 */
        memcpy((byte *)&dev->ipString1[0], (byte *) &dbMOD->ipString1[0], 16);       // SDP DEVICE : 계전기 IP-Address
        memcpy((byte *)&dev->ipString2[0], (byte *) &dbMOD->ipString2[0], 16);       // SDP DEVICE : 계전기 IP-Address
        
        /* -------------------------------------------- */
        /*	2023.11.13 계전기 CPU 이중화 통신정의 여부 			*/
        /* -------------------------------------------- */
        state = inet_addr(dev->ipString2);	    		/* IP conversion & assign */;
        if(state == -1)	dev->dualComFlag = 0;
        else			dev->dualComFlag = SET;	
        
        /* -------------------------------------------- */
        /* 계전기 운영정보 초기화                       */
        /* -------------------------------------------- */
        dev->ioid   = ioid + 1 ;           			            /* Target IO Address */
        dev->ioid   = dev->comDevID;                            // ESIO 모듈별 ... 계전기 통신 ID 
        
        dev->hostid = 50;  
        
        /* -------------------------------------------- */
		/* 	장치별 최대 포인트 제한 					*/
		/* -------------------------------------------- */
		if(dev->devDiPoint >= MAX_DEV_DI_POINT)		dev->devDiPoint = MAX_DEV_DI_POINT;		// 장치당 최대 DI 포인트 
		if(dev->devDoPoint >= MAX_DEV_DO_POINT)		dev->devDoPoint = MAX_DEV_DO_POINT;		// 장치당 최대 DO 포인트 
		if(dev->devAiPoint >= MAX_DEV_AI_POINT)		dev->devAiPoint = MAX_DEV_AI_POINT;		// 장치당 최대 AI 포인트 
		if(dev->devCntPoint >= MAX_DEV_COUNT_POINT)	dev->devCntPoint = MAX_DEV_COUNT_POINT;	// 장치당 최대 Binary Counter 포인트 

        /* -------------------------------------------- */
		/* 	PROTOCOL 구성정보 초기화 					*/
		/* -------------------------------------------- */
		if((dbMOD->modbusFileNo > 0) && (dbMOD->modbusFileNo <= MAX_MODBUS_PROFILE)) 
		{
		    /* -------------------------------------------- */
    		/* 	MODBUS 데이터베이스 구성정보 				*/					
	    	/* -------------------------------------------- */		
		    modbusDB = ( DB_MODBUS_PROFILE *) &rtudb->modbusProfile[dev->modbusFileNo - 1];

    		/* READ BLOCK 초기화 */
	    	for(i=0; i < 8; i++)
		    {
			    readBlock = (MODBUS_READ *) &dev->readBlock[i];
    			readDB	  = (DB_MODBUS_READ *) &modbusDB->readBlock[i];
			
	    		readBlock->useFlag 	 = readDB->useFlag;
		    	readBlock->opcode 	 = readDB->opcode;
			    readBlock->baseAddr  = (readDB->baseAddr[0]*256) + readDB->baseAddr[1];

    			readBlock->baseOffset= (readDB->baseOffset[0]*256) + readDB->baseOffset[1];
	    		readBlock->dataNum 	 = (readDB->dataNum[0]*256) +  readDB->dataNum[1];
		    	readBlock->dataType	 = readDB->dataType;
			    readBlock->dataFormat= readDB->dataFormat;
    			readBlock->swepEnb   = readDB->swepEnb;      // Read Block 별 SWEP 여부 지정 
	    		readBlock->function  = readDB->function;     // Reserved
			
		    	readBlock->commIndex = 0;
		    }
		
    		/* WRITE BLOCK 초기화 */
	    	for(i=0; i < 4; i++)
		    {
    			writeBlock = (MODBUS_WRITE *) &dev->writeBlock[i];
	    		writeDB	  = (DB_MODBUS_WRITE *) &modbusDB->writeBlock[i];
			
    			writeBlock->useFlag   = writeDB->useFlag;
	    		writeBlock->opcode 	  = writeDB->opcode;
		    	writeBlock->baseAddr  = (writeDB->baseAddr[0]*256) + writeDB->baseAddr[1];
			    writeBlock->baseOffset= (writeDB->baseOffset[0]*256) + writeDB->baseOffset[1];
    			writeBlock->dataNum   = (writeDB->dataNum[0]*256) +  writeDB->dataNum[1];
	    		writeBlock->dataType  = writeDB->dataType;
		    	writeBlock->dataFormat= writeDB->dataFormat;
			
    			writeBlock->tripData  = (writeDB->tripData[0]*256) + writeDB->tripData[1];
	    		writeBlock->closeData = (writeDB->closeData[0]*256) + writeDB->closeData[1];
		    }
		
			/* ---------------------------------------- */
    		/* TIME BLOCK 초기화 							*/
    		/* ---------------------------------------- */
	    	timeBlock = (MODBUS_READ *) &dev->timeBlock;
		    timeDB	  = (DB_MODBUS_READ *) &modbusDB->timeBlock;
			
    		timeBlock->useFlag 	 = timeDB->useFlag;
	    	timeBlock->opcode 	 = timeDB->opcode;
		    timeBlock->baseAddr  = (timeDB->baseAddr[0]*256) + timeDB->baseAddr[1];
    		timeBlock->baseOffset= (timeDB->baseOffset[0]*256) + timeDB->baseOffset[1];
	    	timeBlock->dataNum   = (timeDB->dataNum[0]*256) +  timeDB->dataNum[1];
    		timeBlock->dataType	 = timeDB->dataType;
	    	timeBlock->dataFormat= timeDB->dataFormat;
		    
		    /* PNC-Tech 고장점표정반 */
		    if(dev->type == TYPE_LOC_TYPE1)
		   	{
		   		dev->timeIndex   = 0;
		   		
		   		dev->timeInfoAddr[0] = timeBlock->baseAddr;			/* 40027: YEAR 영역 	*/
		   		dev->timeInfoAddr[1] = timeBlock->baseAddr + 1;		/* 40028: Month 영역 	*/
		   		dev->timeInfoAddr[2] = timeBlock->baseAddr + 2;		/* 40029: Day 영역 	*/
		   		dev->timeInfoAddr[3] = timeBlock->baseAddr + 3;		/* 40030: Hour 영역 	*/
		   		dev->timeInfoAddr[4] = timeBlock->baseAddr + 4;		/* 40031: Min 영역 	*/
		   		dev->timeInfoAddr[5] = timeBlock->baseAddr + 5;		/* 40032: Sec 영역 	*/
		   		dev->timeInfoAddr[6] = timeBlock->baseAddr + 6;		/* 40033: Milisec 영역*/
		   		
		   	}
		   	 	
        }
        
        /* -------------------------------------------- */
        /*  61850 계전기 정보 초기화                    */
        /* -------------------------------------------- */
        if(opr->esioID == SDP_ESIO_61850)
        {
            devid = dev->comDevID;                      // SDP DEVICE : 기능모듈내 계전기 ID          
            if((devid > 0) && (devid < MAX_DEVICE))
            {
                dev61850 = (DEVICE_61850 *) device61850[devid-1];
                dev61850->src_ioid = dev->ioid;         // MPU 통신용 IOID 
            }
        }
	    	    
    } // for(ioid=0; ioid < MAX_DEVICE; ioid++)
       
    printf("wdt> => DEVICE Parameter init...!\n");
    
    return (0);
}


/*
*   MPU 구성정보 Config 
*/
void mpuParaConfig()
{
    int i;
    DB_MPU_CONFIG		*dbMPU;
    DB_MNET_ENTRY	    *dbMnet;
    
    MPU_NET_ENTRY       *mpuNet;

    /* 데이터베이스 DB 포인터 초기화 */
    dbMPU = (DB_MPU_CONFIG *) &rtudb->mpuConfig;

    memcpy( (byte *) &mpuCFG->sdpNameStr[0], (byte *) &dbMPU->sdpNameStr[0], 20);   // MPU Parameter : 현장 이름
    
    /* ------------------------------------ */
    /* 주 MPU : Network 구성정보 초기화     */
    /* ------------------------------------ */
    for(i=0; i < 3; i++)
    {
        dbMnet = (DB_MNET_ENTRY *) &dbMPU->master_netCfg[i];
        mpuNet = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[i];
        
        mpuNet->useFlag = dbMnet->useFlag;
        memcpy( (byte *) &mpuNet->ipAddr[0], (byte *) &dbMnet->ipAddr[0], 16); 
        memcpy( (byte *) &mpuNet->gwAddr[0], (byte *) &dbMnet->gwAddr[0], 16); 
        memcpy( (byte *) &mpuNet->subMask[0], (byte *) &dbMnet->subMask[0], 16); 
    }
    
    /* ------------------------------------ */
    /* 부 MPU : Network 구성정보 초기화     */
    /* ------------------------------------ */
    for(i=0; i < 3; i++)
    {
        dbMnet = (DB_MNET_ENTRY *) &dbMPU->slave_netCfg[i];
        mpuNet = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[i];
        
        mpuNet->useFlag = dbMnet->useFlag;
        memcpy( (byte *) &mpuNet->ipAddr[0], (byte *) &dbMnet->ipAddr[0], 16); 
        memcpy( (byte *) &mpuNet->gwAddr[0], (byte *) &dbMnet->gwAddr[0], 16); 
        memcpy( (byte *) &mpuNet->subMask[0], (byte *) &dbMnet->subMask[0], 16); 
    }
    
    mpuCFG->dualMpu     = dbMPU->dualMpu;           // MPU Parameter : CPU 이중화 운영모드
    mpuCFG->dualModule  = dbMPU->dualModule;        // MPU Parameter : Module 이중화 운영모드 
    
    mpuCFG->scuUseFlag  = dbMPU->scuUseFlag;        // MPU Parameter : SCU 모듈 사용유무
    mpuCFG->mmiUseFlag  = dbMPU->mmiUseFlag;        // MPU Parameter : MMI 모듈 사용유무
    
    mpuCFG->statusDump  = dbMPU->statusDump;        // MPU Parameter : STATUS Dump 주기
    mpuCFG->analogDump  = dbMPU->analogDump;        // MPU Parameter : ANALOG Dump 주기
    mpuCFG->debounce    = dbMPU->debounce;          // MPU Parameter : Function 코드#1
    mpuCFG->dbCheck     = dbMPU->dbCheck;           // MPU Parameter : Function 코드#2
    mpuCFG->hostWDT     = dbMPU->hostWDT;           // MPU Parameter : Function 코드#3
    mpuCFG->iccpFlag    = dbMPU->iccpFlag;          // MPU Parameter : Function 코드#4
    
    mpuCFG->func1 = dbMPU->func1;                   // MPU Parameter : Function 코드#1
    mpuCFG->func2 = dbMPU->func2;                   // MPU Parameter : Function 코드#2
    mpuCFG->func3 = dbMPU->func3;                   // MPU Parameter : Function 코드#3
    mpuCFG->func4 = dbMPU->func4;                   // MPU Parameter : Function 코드#4
    mpuCFG->func5 = dbMPU->func5;                   // MPU Parameter : Function 코드#5
    mpuCFG->func6 = dbMPU->func6;                   // MPU Parameter : Function 코드#6
    
    
    /* 하위 계전기 : 상태/아날로그 수집주기 */
    opr->vmeNetworkInit = mpuCFG->scuUseFlag;		// MPU Parameter : VME Network Config	
    opr->stsDumpPeriod = mpuCFG->statusDump;        // MPU Parameter : STATUS Dump 주기
    opr->anaDumpPeriod = mpuCFG->analogDump;        // MPU Parameter : ANALOG Dump 주기
    
    opr->vmeNetworkInit = SET;
    opr->soeDebounce   = mpuCFG->debounce;          // SOE Check... Debounce Time       
    
    /* MODBUS SOE-Debounce Count */
    if(opr->soeDebounce < 1)    opr->soeDebounce = 1;
    if(opr->soeDebounce > 4)    opr->soeDebounce = 4;

    /* -------------------------------------------- */		
    /* CU 운영 파라메타 이상값 보정                 */
    /* -------------------------------------------- */
    if(opr->stsDumpPeriod < 3)      opr->stsDumpPeriod   = 3;
    if(opr->stsDumpPeriod > 60)     opr->stsDumpPeriod   = 60;    
        
    if(opr->anaDumpPeriod < 3)      opr->anaDumpPeriod   = 3;
    if(opr->anaDumpPeriod > 60)     opr->anaDumpPeriod   = 60;    
        
    printf("wdt> => MPU Parameter init...!\n");

}



/*
*   ESIO 구성정보 Config 
*/
int esioParaConfig()
{
    int j;
    DB_ESIO_CONFIG		*dbESIO;
    DB_ENET_ENTRY	    *dbEnet;
    DB_PORT_ENTRY	    *dbEport;
    
    ESIO_NET_ENTRY      *esioNet;
    ESIO_PORT_ENTRY     *esioPort;

    /* 데이터베이스 DB 포인터 초기화 */
    dbESIO = (DB_ESIO_CONFIG *) &rtudb->esioConfig[opr->boardID];   // ESIO별 DB 구조체 참조
        
    esioCFG->useFlag    = dbESIO->useFlag;                // ESIO Parameter : ESIO 사용유무
    esioCFG->targetID   = dbESIO->targetID;               // ESIO Parameter : ESIO Target Module-ID, 0:사용않함, 1: ESIO1, 2:ESIO2, 3:ESIO3, 4:ESIO4, 5:SIO
    esioCFG->linkMth    = dbESIO->linkMth;                // ESIO Parameter : MPU 연계방식
    esioCFG->comDelay   = dbESIO->comDelay;               // ESIO Parameter : MPU 통신 지연 (10ms)
    
    memcpy( (byte *) &esioCFG->esioNameStr[0], (byte *) &dbESIO->esioNameStr[0], 20);   // ESIO 장치이름
    
    /* ------------------------------------ */
    /* 주 ESIO : Network 구성정보 초기화    */
    /* ------------------------------------ */
    // 2026-05-26 오후 7:57:06 
    // 2026-11-03 오전 9:35:21  4->ESIO_NET_MAX
    for(j=0; j < ESIO_NET_MAX; j++)
    {
        dbEnet = (DB_ENET_ENTRY *) &dbESIO->mstNetConfig[j];
        esioNet= (ESIO_NET_ENTRY *)&esioCFG->mstNetConfig[j];
    
        esioNet->useFlag = dbEnet->useFlag;
        memcpy( (byte *) &esioNet->ipAddr[0], (byte *) &dbEnet->ipAddr[0], 16); 
        memcpy( (byte *) &esioNet->gwAddr[0], (byte *) &dbEnet->gwAddr[0], 16); 
        memcpy( (byte *) &esioNet->subMask[0], (byte *) &dbEnet->subMask[0], 16); 
    }
    
    /* ------------------------------------ */
    /* 예비 ESIO : Network 구성정보 초기화  */
    /* ------------------------------------ */
    // 2026-05-26 오후 7:57:19
    // 2026-11-03 오전 9:35:21  4->ESIO_NET_MAX
    for(j=0; j < ESIO_NET_MAX; j++)
    {
        dbEnet = (DB_ENET_ENTRY *) &dbESIO->slvNetConfig[j];
        esioNet= (ESIO_NET_ENTRY *)&esioCFG->slvNetConfig[j];
    
        esioNet->useFlag = dbEnet->useFlag;
        memcpy( (byte *) &esioNet->ipAddr[0], (byte *) &dbEnet->ipAddr[0], 16); 
        memcpy( (byte *) &esioNet->gwAddr[0], (byte *) &dbEnet->gwAddr[0], 16); 
        memcpy( (byte *) &esioNet->subMask[0], (byte *) &dbEnet->subMask[0], 16); 
    }
    
    /* ------------------------------------ */
    /* ESIO : PORT 구성정보 초기화          */
    /* ------------------------------------ */
    for(j=0; j < 8; j++)
    {
        dbEport = (DB_PORT_ENTRY *) &dbESIO->portConfig[j];
        esioPort= (ESIO_PORT_ENTRY *)&esioCFG->portConfig[j];
    
        esioPort->useFlag   = dbEport->useFlag;             // PORT# 사용유무                                       
        esioPort->function  = dbEport->function;            // PORT# Function                                       
        esioPort->comMode   = dbEport->comMode;             // PORT# 통신모드, 0:RS232, 1:MODEM, 2:RS485, 3:TCPIP   
        esioPort->comSpeed  = dbEport->comSpeed;            // PORT# 통신속도, [0] 1200 ~ [7] 115200                
        
        memcpy( (byte *) &esioPort->portNameStr[0], (byte *) &dbEport->portNameStr[0], 20);     // PORT# 포트이름
    }  
        
    printf("wdt> => ESIO Parameter init...!\n"); 
    return (0);          
}


/*----------------------------------------------------------------------------
* Function Name : readRTUdb()
* 수행내용: 데이터베이스내용의 Check 및 초기화 수행 함수
* ArgList :
* Return  :  
---------------------------------------------------------------------------- */    
int readRTUdb()
{
    word    chksum;
    word    romsum;

    /* --------------------------- */
    /*   Check chksum              */
    /* --------------------------- */
    chksum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
    romsum = rtudb->chksum[0] + rtudb->chksum[1]*256;

    if(chksum != romsum)
    {
        printf("\n=============================\n");
        printf("[*] *** Chksum Error : %4x (%4x)\n", romsum ,chksum);
        printf("=============================\n");

        romsum = gensum((byte *)rtudb, sizeof(RTU_DATABASE) - 2);
        rtudb->chksum[0] = romsum;
        rtudb->chksum[1] = romsum >> 8;

        dbFileWrite_ESIO(rtudb);
    }

    /* ---------------------------- */
    /*   MPU 구성정보 Config        */
    /* ---------------------------- */
    mpuParaConfig();
    pause(20);
    
    /* ---------------------------- */
    /*   ESIO 구성정보 Config        */
    /* ---------------------------- */
    esioParaConfig();
    pause(20);
    
    /* ---------------------------- */
    /*   DEVICE Database Config       */
    /* ---------------------------- */
    deviceParaConfig();
    pause(20);
                
    /* ---------------------------- */
    /*   Read point_config          */
    /* ---------------------------- */
    pointParaConfig();                 
    pause(20);

    printf("[*] RTU Database initial OK... !\n\n");
    printf("ESIO DB Size = %d\n", sizeof(RTU_DATABASE));
    
    return (1);
}

/* ............ end of " config.c " ....................................... */

