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
#include    "external.h"

extern  int CheckEOT(void);
extern  int getDecWord();

#if 0
/* --------------------------------------- */
/* ESIO : SCAN Task : 프로토콜 타입 정의 */
/* --------------------------------------- */
#define SCAN_PROT_NULL      0
#define SCAN_PROT_DNP       1		    // SCAN# 프로토콜 타입 : DNP
#define SCAN_PROT_MODBUS    2           // SCAN# 프로토콜 타입 : MODBUS
#define SCAN_PROT_BUS       3           // SCAN# 프로토콜 타입 : BUS
#define SCAN_PROT_REMOTE1   4           // SCAN# 프로토콜 타입 : 원격진단#1 : 유중가스 분석장치 (DGA)
#define SCAN_PROT_REMOTE2   5           // SCAN# 프로토콜 타입 : 원격진단#2 : 가스밀도
#define SCAN_PROT_REMOTE3   6           // SCAN# 프로토콜 타입 : 원격진단#3 : 
#define SCAN_PROT_EQM1      7           // SCAN# 프로토콜 타입 : 전력품질#1 :  
#define SCAN_PROT_EQM2      8           // SCAN# 프로토콜 타입 : 전력품질#2
#define SCAN_PROT_EQM3      9           // SCAN# 프로토콜 타입 : 전력품질#3
#define SCAN_PROT_61850     10          // SCAN# 프로토콜 타입 : 61850
#define SCAN_PROT_LOCATOR1  11          // SCAN# 프로토콜 타입 : 고장점#1
#define SCAN_PROT_LOCATOR2  12          // SCAN# 프로토콜 타입 : 고장점#2
#define SCAN_PROT_TYPE1     13          // SCAN# 프로토콜 타입 : 예비
#define SCAN_PROT_TYPE2     14          // SCAN# 프로토콜 타입 : 예비
#define SCAN_PROT_TYPE3     15          // SCAN# 프로토콜 타입 : 예비
#endif

static char pointTypeStr[16][8]= { {"--- "},{"COS "}, {"SOE "}, {"ALL "}, {"ACC "}, {"SBO "},
                                   {"DCO "},{"1MA "}, {"20MA"}, {"10V "}, {"RALL"}, {"RDEV"}, {"MCPU"},{"SCPU"},{"CNTR"}};
                                    
static char scanComMode[4][8]   = {{"RS232"}, {"MODEM"}, {"RS485"}, {"TCPIP"}};
static char scanPortType[8][8]  = {{" COM1"}, {" COM2"},{" COM3"}, {" COM4"}, {" COM5"}, {" COM6"}, {" COM7"}, {" COM8"} };
static char scanModType[10][8]  = {{"*NOT "}, {" SIO "},{"ESIO1"}, {"ESIO2"}, {"ESIO3"}, {"ESIO4"}, {" RTU "}, {" MPU1"}, {" MPU2"} };

static char scanProtType[16][8] = {{" *NOT "}, {"  DNP "}, {"MODBUS"}, {"  BUS "}, {"RM-DGA"}, {"RM-GAS"}, {"REMOTE"}, {"EL-EQ1"},{"EL-EQ2"},
                                   {"EL-EQ3"}, {"61850 "}, {"LOCAT1"}, {"LOCAT2"}, {"*RESE1"}, {"*RESE2"}, {"*RESE3"} };
                                    
static char speedType[8][10]    = {{"1200B"}, {"2400B"}, {"4800B"}, {"9600B"},{"19200"},{"38400"},{"57600"}, {"11.52K"}};
static char scanChgMode[4][10]  = {{"*NOT"}, {" ALL"}, {"EACH"}, {"****"}};
static char scanCfgType[20][10] = {{"*NO"}, {"S01"}, {"S02"}, {"S03"},{"S04"},{"S05"},{"S06"},{"S07"},{"S08"},
                                   {"S09"}, {"S10"}, {"S11"}, {"S12"},{"S13"},{"S14"},{"S15"},{"S16"},{"***"}};

static char scanDevType[32][12] = {{"  *NULL  "},{"RTU-DIM  "},{"RTU-DOM  "},{"RTU-AIM  "},{"RTU-AOM  "},{"DNP-GIPAM"},{"DNP-SIEMN"},{"DNP-HIMAP"},{"DNP-FRTU "},{"DNP-PROPC"},
                                   {"DNP-VIPAM"},{"DNP-LBS  "},{"MOD-LG   "},{"MOD-SIEMN"},{"MOD-SUB  "},{"LOC-TYPE1"},{"LOC-TYPE2"},{"REM-UHF  "},{"REM-DGA  "},{"REM-LA   "},
                                   {"REM-TYPE1"},{"REM-TYPE2"},{"REM-TYPE3"},{"REM-TYPE4"},{"EQM-TYPE1"},{"EQM-TYPE2"},{"EQM-TYPE3"},{"EQM-TYPE4"},{"61850-D#1"},{"61850-D#2"},
                                   {"61850-D#3"} };
                                    
//static char devTypeStr[32][8]  = { {"------ "}, {" DIM   "}, {" DOM   "}, {" AIM   "}, {" AOM   "}, {" RDI   "}, {" RDO   "}, {" RAI   "}, 
//                                   {" RAO   "}, {" DEV   "}, {"SDP-STS"}, {"CPU-RUN"}, {"CPU-STS"}, {"RTU-RUN"}, {"RTU-STS"}, {" CU-RUN"}, 
//                                   {" CU-STS"}, {"DIG-RUN"}, {"DIG-STS"}, {"EQM-RUN"}, {"EQM-STS"}, {"IEC-RUN"}, {"IEC-STS"}};

static char devTypeStr[32][8]  = { {"------ "}, {" DIM   "}, {" DOM   "}, {" AIM   "}, {" AOM   "}, {" VDI   "}, {" VAI   "}, {" VDO   "}, 
                                   {" DEV   "}, {"SDP-STS"}, {"CPU-RUN"}, {"CPU-STS"}, {"RTU-RUN"}, {"RTU-STS"}, {" CU-RUN"}, {" CU-STS"}, 
                                   {"DIG-RUN"}, {"DIG-STS"}, {"EQM-RUN"}, {"EQM-STS"}, {"IEC-RUN"}, {"IEC-STS"}};
                                    
static char iccpPTypeStr[16][8]= { {"---"},{"SDI"}, {"SDO"}, {"SAI"}, {"DDI"}, {"DAI"}, {"QDI"},{"QAI"}, {"TDI"}, {"TAI"}, {"DEV"}};
                                    
static char onoffType[2][8]    = {{"OFF"},   {"ON"}};
static char cpuTypeStr[2][12]  = {{"MASTER-CPU"}, {"SLAVE-CPU"}};

static char modbusType[8][8]   = {{"NUL"}, {"DIM"}, {"AIM"}, {"DOM"},{"AOM"}};
static char modbusFormat[8][10]= {{"BIT  "}, {"BYTE "}, {"SHORT"},{"WORD "},{"INT  "}, {"UINT "}, {"FLOAT"}};
                                    
//
// 모듈:	CmdHistory()
//
void CmdDispVERSION(void)
{
	// 명령 형식을 점검한다.
	if (CheckEOT() < 0)	return;

    printf("\n*** Display SDP-ESIO History ***\n");
    printf(" 1. Target Name : [%s] \n", TARGET_NAME);
    printf(" 2. Manufacture : SANE System Co.,Ltd\n");
    printf(" 3. Model Name  : %s\n", VERSION_STRING);
    printf(" 4. Simulator   : %s\n", SIMULATOR_NAME);
    printf(" 5. Update Date : %s\n", DATE_STRING);
    printf(" 6. Print  Date : %4d/%2d/%2d %02d:%02d:%02d\n", ((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
    printf("[프로그램 Update]------------------------------------\n");
    printf("[2016/04/22 - ACE-SDP-ESIO Version 1.0 \n");
    printf("[2020/03/20 - CPU Change/Re-Initial socket > 60....SCAN-Researt ...! \n");
    printf("[2020/05/15 - WDT: TASK-Restart 실패시 재기동 ...! \n");
    printf("-----------------------------------------------------\n");
    printf("[2020/06/05 - Shared Memory 최적화 ...! \n");
    printf("[2020/06/05 - NTP Time-Sync => 통신방식변경...! \n");
    printf("-----------------------------------------------------\n");
    printf("[2020/11/04 - 상태반전 적용...MPU Point-Dump 보완...! \n");
    printf("-----------------------------------------------------\n");
    printf("[2021/03/20 - ACE-SDP-ESIO Version 3.0 \n");
    printf("[2021/03/20 - Change Standby Mode => 61850-TASK Restart \n");
    printf("[2021/04/02 - ANALOG Type 변환 : Time_t 정보=> 2000년 1월 1일 00:00:00 기준 \n");
    printf("[2022/06/29 - DEVICE Point 용량(AI-256, DO-200) 변경  \n");
    printf("[2023/06/20 - MODBUS Control Opcode (0x05, 0x06) 동일처리 \n");
    printf("[2023/11/13 - 하위계전기 TCPIP 통신-CPU 이중화 절체로직 \n");
    printf("[2024/04/23 - DNP통신부 - 아날로그 음수처리  \n");
    printf("[2024/10/30 - MODBUS 수집 SOE - 반전포인트 처리 \n");
    printf("\n");

}

/*----------------------------------------------------------------------------
* Function Name : CmdDispRelayCfg()
* 수행내용: CCU내의 모듈구성정보를 Console로 출력하는 함수
* ArgList :
* Return  :  
---------------------------------------------------------------------------- */  
void CmdDispRelayCfg_61850()
{
    short i;
    
    DEVICE_61850    *dev61850;

    printf("\n\n----------------------------------------------------------------------------\n");
    printf(" %s : 61850-RELAY Configuration ...\n", TARGET_NAME );
    printf("  No.  SCAN  ONL  STS IP-Address      NAME\n");
    printf("----------------------------------------------------------------------------\n");
    for(i = 0; i < MAX_DEVICE; i++)
    {
        dev61850 = (DEVICE_61850 *) device61850[i];
    
        if(dev61850->config61850 == 0)  continue;
        printf(" %02d.     %d    %d   %02x   %s  %s \n", 
            i + 1, dev61850->config61850, dev61850->online, dev61850->devStatus, dev61850->ipAddress, dev61850->name);
    }
    
}



/*----------------------------------------------------------------------------
* Function Name : CmdDispRelayCfg()
* 수행내용: CCU내의 모듈구성정보를 Console로 출력하는 함수
* ArgList :
* Return  :  
---------------------------------------------------------------------------- */  
void CmdDispRelayCfg()
{
    short i;
    
    SDP_DEVICE  *dev;
    SCAN_CONFIG *scan;
    
    printf("\n============================================================================\n");
    printf(" %s : SCAN Configuration ...\n", TARGET_NAME);
    printf("  No.  ACT  PROTO  MODE   TARGET PORT  SPEED [DELAY OFFT] CH-M  FT1 FT2 FT3  NAME \n");
    printf("============================================================================\n");
    
    for(i = 0; i < MAX_SCAN_PORT; i++)
    {
    	scan = (SCAN_CONFIG *) scanCFG[i];
        
        if(scan->useFlag == 0)  continue;

        printf("  %02d.   %d  %s  %s   %5s  %5s  %s   %02d   %02d   %s    %d  %d  %d    %s\n", 
            i + 1,  scan->useFlag, scanProtType[scan->protocol], scanComMode[scan->comMode], scanModType[scan->targetID], scanPortType[scan->comPort], 
            speedType[scan->comSpeed], scan->comDelay, scan->offCount, scanChgMode[scan->chgMode], scan->function1, scan->function2, scan->function3, scan->scanNameStr);
    }
    
    printf("\n\n----------------------------------------------------------------------------\n");
    printf(" %s : RELAY Configuration ...(Max SDP Point = %d)\n", TARGET_NAME, opr->max_sdpPoint);
    printf(" No. SCN ID Target PORT  Type      USEF [ DI/ DO/ AI/ AO] MOD ONL [SND /RCV]  PRE [IP_ADDR     Port]\n");
    printf("----------------------------------------------------------------------------\n");
    for(i = 0; i < MAX_DEVICE; i++)
    {
        dev = (SDP_DEVICE *) deviceCFG[i];
        
        if((dev->type == 0) && (dev->scan == 0))    continue;
        
        scan = (SCAN_CONFIG *) scanCFG[dev->scanPort - 1];

        printf(" %02d. %s %02d %s %s  %s  %02d  [%3d/%3d/%3d/%3d] %2d  %02x  %04d-%04d %4d [%s,%4d] %s \n", 
            i + 1, scanCfgType[dev->scanPort], dev->comDevID, scanModType[scan->targetID], 
            scanPortType[scan->comPort], scanDevType[dev->type], dev->scan, 
            dev->devDiPoint, dev->devDoPoint, dev->devAiPoint, dev->devAoPoint, 
            dev->modbusFileNo, 
            dev->online,
            dev->comSndCount,
            dev->comRcvCount,
            dev->oldRcvCount,
            dev->ipString1,
            dev->netPort,
            dev->devNameStr);
    }

    /* -------------------------------------------- */
    /*  61850 계전기 구성정보                       */
    /* -------------------------------------------- */
    if(opr->esioID == SDP_ESIO_61850)
    {
        CmdDispRelayCfg_61850();
    }
    
    printf("----------------------------------------------------------------------------\n");
    printf("\n... Configuration End \n\n");

}


/*----------------------------------------------------------------------------
* Function Name : CmdDispMPUCfg()
* 수행내용: MCU 구성정보 Desplay

* 2026-11-02 오후 7:17:16 network 3->4개로

---------------------------------------------------------------------------- */  
void  CmdDispMPUCfg()
{
    MPU_NET_ENTRY   *mNet1, *mNet2,*mNet3, *sNet1, *sNet2,*sNet3;
    MPU_NET_ENTRY   *mNet4, *sNet4;
    
    mNet1 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[0];
    mNet2 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[1];
    mNet3 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[2];
// 2026-11-02 오후 7:18:07
    mNet3 = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[3];
    
    sNet1 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[0];
    sNet2 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[1];
    sNet3 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[2];
//    
    sNet3 = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[3];
    
    printf("\n============================================================================\n");
    printf(" %s : MCU Configuration  : %s \n", TARGET_NAME, mpuCFG->sdpNameStr);
    printf("============================================================================\n");
    printf("[*] DUAL-CPU Run Mode  = %s , CPU = %s\n", onoffType[mpuCFG->dualMpu], cpuTypeStr[opr->cpuMode]);
    printf("[*] DUAL-RTU Run Mode  = %s \n", onoffType[mpuCFG->dualModule]);
    printf("[*] USE MMI Module     = %s \n", onoffType[mpuCFG->mmiUseFlag]);
    printf("[*] USE SCU Module     = %s \n", onoffType[mpuCFG->scuUseFlag]);
    printf("[*] Status Dump Period = %2d [sec]\n", mpuCFG->statusDump);
    printf("[*] Analog Dump Period = %2d [sec]\n", mpuCFG->analogDump);
    printf("[*] Modbus Debounce T  = %2d \n", mpuCFG->debounce);
    printf("[*] WDT DB-Check Flag  = %2d \n", mpuCFG->dbCheck);
    printf("[*] HOST Comm-WDT Flag = %2d \n", mpuCFG->hostWDT);
    printf("[*] ICCP-HOST Flag     = %2d \n", mpuCFG->iccpFlag);
    printf("[*] Function Code#     = %02d %02d %02d %02d %02d %02d\n", mpuCFG->func1, mpuCFG->func2, mpuCFG->func3, mpuCFG->func4, mpuCFG->func5, mpuCFG->func6);
    printf("============================================================================\n");
    printf("[*] Master Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);
    printf("           Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet2->useFlag, mNet2->ipAddr, mNet2->gwAddr, mNet2->subMask);
    printf("           Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n\n", mNet3->useFlag, mNet3->ipAddr, mNet3->gwAddr, mNet3->subMask);
    printf("           Network#4 Info(IP/GW/SUB): use = %d, %s/%s/%s\n\n", mNet4->useFlag, mNet4->ipAddr, mNet4->gwAddr, mNet4->subMask);
    printf("[*] Slave  Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet1->useFlag, sNet1->ipAddr, sNet1->gwAddr, sNet1->subMask);
    printf("           Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet2->useFlag, sNet2->ipAddr, sNet2->gwAddr, sNet2->subMask);
    printf("           Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet3->useFlag, sNet3->ipAddr, sNet3->gwAddr, sNet3->subMask);
    printf("           Network#4 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet4->useFlag, sNet4->ipAddr, sNet4->gwAddr, sNet4->subMask);
    printf("============================================================================\n\n");
    
}


/*----------------------------------------------------------------------------
* Function Name : CmdDispESIOCfg()
* 수행내용: ESIO 구성정보 Desplay
---------------------------------------------------------------------------- */  
void CmdDispESIOCfg()
{
    short  id;
    ESIO_NET_ENTRY   *mNet1, *mNet2,*mNet3, *sNet1, *sNet2,*sNet3;
    ESIO_PORT_ENTRY  *port;

    ESIO_NET_ENTRY   *mNet4, *sNet4;    
    
    mNet1 = (ESIO_NET_ENTRY *) &esioCFG->mstNetConfig[0];
    mNet2 = (ESIO_NET_ENTRY *) &esioCFG->mstNetConfig[1];
    mNet3 = (ESIO_NET_ENTRY *) &esioCFG->mstNetConfig[2];
    mNet4 = (ESIO_NET_ENTRY *) &esioCFG->mstNetConfig[3];
    
    sNet1 = (ESIO_NET_ENTRY *) &esioCFG->slvNetConfig[0];
    sNet2 = (ESIO_NET_ENTRY *) &esioCFG->slvNetConfig[1];
    sNet3 = (ESIO_NET_ENTRY *) &esioCFG->slvNetConfig[2];
    sNet4 = (ESIO_NET_ENTRY *) &esioCFG->slvNetConfig[3];
    
    printf("\n============================================================================\n");
    printf("  ESIO Configuration  : %s \n", esioCFG->esioNameStr);
    printf("============================================================================\n");
    printf("[*] ESIO USE Flag  = %s \n", onoffType[esioCFG->useFlag]);
    printf("[*] ESIO Target ID = %s \n", scanModType[esioCFG->targetID]);
    printf("[*] ESIO LINK Mth  = %d \n", esioCFG->linkMth);
    printf("[*] ESIO ComDelay  = %d \n", esioCFG->comDelay);
    printf("----------------------------------------------------------------------------\n");
    printf("[*] Master Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet1->useFlag, mNet1->ipAddr, mNet1->gwAddr, mNet1->subMask);
    printf("           Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet2->useFlag, mNet2->ipAddr, mNet2->gwAddr, mNet2->subMask);
    printf("           Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet3->useFlag, mNet3->ipAddr, mNet3->gwAddr, mNet3->subMask);
    printf("           Network#4 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   mNet4->useFlag, mNet4->ipAddr, mNet4->gwAddr, mNet4->subMask);
    printf("[*] Slave  Network#1 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet1->useFlag, sNet1->ipAddr, sNet1->gwAddr, sNet1->subMask);
    printf("           Network#2 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet2->useFlag, sNet2->ipAddr, sNet2->gwAddr, sNet2->subMask);
    printf("           Network#3 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet3->useFlag, sNet3->ipAddr, sNet3->gwAddr, sNet3->subMask);
    printf("           Network#4 Info(IP/GW/SUB): use = %d, %s/%s/%s\n",   sNet4->useFlag, sNet4->ipAddr, sNet4->gwAddr, sNet4->subMask);
    printf("----------------------------------------------------------------------------\n");
    
    printf("  No.  SCAN  Func   MODE  SPEED    NAME \n");
    
    for(id=0; id < MAX_ESIO_PORT; id++)
    {
        port = (ESIO_PORT_ENTRY *) &esioCFG->portConfig[id];
        printf( "  %02d    %d     %d    %s  %s  : %s\n", id, port->useFlag, port->function, 
            scanComMode[port->comMode], speedType[port->comSpeed], port->portNameStr);   
    }
    
}


/*----------------------------------------------------------------------------
* Function Name : CmdDispMODBUSCfg()
---------------------------------------------------------------------------- */  
void CmdDispMODBUSCfg()
{
    short i, j;
    word	baseAddr,baseOffset, dataNum,  tripData, closeData;
    
    DB_MODBUS_PROFILE   *dbBlock;
	DB_MODBUS_READ	    *readBlock, *timeBlock;
	DB_MODBUS_WRITE	    *writeBlock;

    	
    printf("\n %s : MODBUS  Configuration ...\n", TARGET_NAME);
    printf("BLK  No. USE  OPC  Addr  Offset Data  Type  Format ... Trip Close   [Swep:Func]\n");
    for(i = 0; i < MAX_MODBUS_PROFILE; i++)
    {
    	printf("-----------------------------------------------------------\n");
        dbBlock = (DB_MODBUS_PROFILE *) &rtudb->modbusProfile[i];
        
        for(j=0; j < 8; j++)
        {
        	readBlock = (DB_MODBUS_READ *) &dbBlock->readBlock[j];
        	baseAddr = readBlock->baseAddr[0]*256 + readBlock->baseAddr[1];
        	baseOffset = readBlock->baseOffset[0]*256 + readBlock->baseOffset[1];
        	dataNum = readBlock->dataNum[0]*256 + readBlock->dataNum[1]	;
        	if(readBlock->useFlag == 0) continue;
        	
        	
        	printf("%02d.  R%d   %d   %2x   %06d %06d  %02d    %s   %s ... %04x %04x   [%2d/%2d]\n", 
            	i + 1, j+1, 
            	readBlock->useFlag, readBlock->opcode, baseAddr, baseOffset, dataNum, 
            	modbusType[readBlock->dataType], modbusFormat[readBlock->dataFormat], 0, 0, readBlock->swepEnb, readBlock->function);
        }
        
        for(j=0; j < 4; j++)
        {
        	writeBlock = (DB_MODBUS_WRITE *) &dbBlock->writeBlock[j];
        	baseAddr = writeBlock->baseAddr[0]*256 + writeBlock->baseAddr[1];
        	tripData = writeBlock->tripData[0]*256 + writeBlock->tripData[1];
        	closeData= writeBlock->closeData[0]*256 + writeBlock->closeData[1];
        	dataNum = writeBlock->dataNum[0]*256 + writeBlock->dataNum[1]	;
        	if(writeBlock->useFlag == 0) continue;
        		
        	printf("%02d.  W%d   %d   %2x   %06d %06d  %03d    %s   %s ... %04x %04x\n", 
            	i + 1, j+1, 
            	writeBlock->useFlag, writeBlock->opcode, baseAddr, baseOffset, dataNum, 
            	modbusType[writeBlock->dataType], modbusFormat[writeBlock->dataFormat], tripData, closeData);
        }
        
        timeBlock = (DB_MODBUS_READ *) &dbBlock->timeBlock;
       	baseAddr = timeBlock->baseAddr[0]*256 + timeBlock->baseAddr[1];
       	dataNum = timeBlock->dataNum[0]*256 + timeBlock->dataNum[1]	;
        	
       	if(timeBlock->useFlag == 0) continue;
        		
       	printf("%02d.  T%d   %d   %2x   %06d %06d  %03d    %s   %s ... %04x %04x\n", 
           	i + 1, 1, 
            timeBlock->useFlag, timeBlock->opcode, baseAddr, baseOffset, dataNum, 
            modbusType[timeBlock->dataType], modbusFormat[timeBlock->dataFormat], 0, 0);
            	
    }

    printf("-----------------------------------------------------------\n");
    printf("... Configuration End \n");
}


/*----------------------------------------------------------------------------
* Function Name : dispDevice()
* 수행내용: 각 계전기 별 수집된 DI/AI/AO 정보를 표출함
* ArgList :
*   1. port - 출력하고자 하는 계전기 번호
* Return  :  
---------------------------------------------------------------------------- */  
int dispDevice(int devid)
{
    int     i, j;
    int     point;
    int     ptStart, ptStop;
    char    ch;
    byte    msg[128];
    byte    *bfptr;
    
    SDP_DEVICE  *dev;
    POINT_BUF *diPoint;
    POINT_BUF *aiPoint;
    DEVICE_61850    *dev61850;
    POINT_61850     *diPt61850;
    POINT_61850     *aiPt61850;
    
    
    /*  check Invalid DEVICE # */
    if((devid < 0) || (devid >= MAX_DEVICE))
    {
        printf("*Invalid devid (%d) ...!\n", devid);
        return (0);
    }
    
    ch = 0;
    while(ch != 'q')
    {
        if(opr->esioID == SDP_ESIO_61850)
        {
            dev61850 = (DEVICE_61850 *) device61850[devid];
            
            printf("\n[DEVICE Information] ------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n", 
        	    ((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
            printf(" Device Address   : %2d\n", dev61850->ioid);
            //printf(" [*] Device TYPE  : [%s]\n", scanDevType[dev61850->type]);
            printf(" [*] Point Config : [DI/DO/AI] - %2d %2d %2d \n", dev61850->devDiLastNo, dev61850->devDoLastNo,dev61850->devAiLastNo);
 
     		/* ------------------------ */ 
            /* Status Bit Display ... 	*/
            /* ------------------------ */ 
            ptStart = 0;
            //ptStop  = dev->devDiPoint;
            ptStop  = dev61850->devDiLastNo;
            //ptStop  = 32;
            
            point   = (ptStop - ptStart) % 32;
            printf("=> DI Data : [%d - %d] \n", ptStart, ptStop);
    
            for(i = ptStart, j=0, bfptr = msg; i < ptStop; i++, j++)
            {
                diPt61850 = (POINT_61850 *) &dev61850->diPtBuf[i];
                *bfptr++ = ' ';
                *bfptr++ = '0' + diPt61850->state;      

                if((j & 0x1f) == 0x1f)
                {
                    *bfptr = '\0';
                    printf(" %s\n", msg);
                    bfptr = msg;
                }
                else if((j & 7) == 7)
                {
                    *bfptr = ' ';
                    bfptr++;
                    *bfptr = '-';
                    bfptr++;
                }
            } /* for */
    
            if(point)
            {
                //bfptr--;
                *bfptr = '\0';
                printf(" %s\n", msg);
            }

            /* ------------------------ */               
            /* ANALOG Input Display ... */
            /* ------------------------ */ 
            ptStart = 0;
            ptStop  = dev61850->devAiLastNo;
            //ptStop  = 16;
            
            printf("=> AI Data : [%d - %d] \n", ptStart, ptStop);
            for(i = ptStart,j=1; i < ptStop; i++, j++)
            {
                aiPt61850 = (POINT_61850 *) &dev61850->aiPtBuf[i];
                printf("%05.2f ", aiPt61850->floatData);
                
                if(j== 10) 
                {
                    j = 0;
                    printf("\n");
                }            
            }
            printf("\n");        
        
            ch = getchar();
        
            continue;
        }
        
        dev = (SDP_DEVICE *) deviceCFG[devid];

        printf("\n[DEVICE Information] ------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n", 
        	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);
        printf(" Device Address   : %2d\n", dev->ioid);
        printf(" [*] Device TYPE  : [%s]\n", scanDevType[dev->type]);
        printf(" [*] Point Config : [DI/DO/AI/Count] - %2d %2d %2d %2d \n", dev->devDiPoint, dev->devDoPoint,dev->devAiPoint, dev->devAoPoint);
 
 		/* ------------------------ */ 
        /* Status Bit Display ... 	*/
        /* ------------------------ */ 
        ptStart = 0;
        ptStop  = dev->devDiPoint;
        point   = (ptStop - ptStart) % 32;
        printf("=> DI Data : [%d - %d] \n", ptStart, ptStop);
    
        for(i = ptStart, j=0, bfptr = msg; i < ptStop; i++, j++)
        {
            diPoint = (POINT_BUF *) &dev->diPtBuf[i];
            *bfptr++ = ' ';
            *bfptr++ = '0' + diPoint->status;      

            if((j & 0x1f) == 0x1f)
            {
                *bfptr = '\0';
                printf(" %s\n", msg);
                bfptr = msg;
            }
            else if((j & 7) == 7)
            {
                *bfptr = ' ';
                bfptr++;
                *bfptr = '-';
                bfptr++;
            }
        } /* for */
    
        if(point)
        {
            //bfptr--;
            *bfptr = '\0';
            printf(" %s\n", msg);
        }
               
        /* ------------------------ */               
        /* ANALOG Input Display ... */
        /* ------------------------ */ 
        ptStart = 0;
        ptStop  = dev->devAiPoint;
        printf("=> AI Data : [%d - %d] \n", ptStart, ptStop);
        for(i = ptStart,j=1; i < ptStop; i++, j++)
        {
            aiPoint = (POINT_BUF *) &dev->aiPtBuf[i];
            printf("%05.2f ", aiPoint->floatData);
                
            if(j== 10) 
            {
                j = 0;
                printf("\n");
            }            
        }
        printf("\n");        
        
        ch = getchar();
    }

    printf("\n"); 
    return (0);   
}


/*----------------------------------------------------------------------------
* Function Name : dispDevStatus()
* 수행내용: 각 계전기 별 수집된 DI 정보를 표출함
* ArgList :
*   1. port - 출력하고자 하는 계전기 번호
* Return  :  
---------------------------------------------------------------------------- */  
void dispDevStatus(int devid, int point)
{
    int     i, j;
    int     ptStart, ptStop;
    char    ch=0;
    byte    msg[128];
    byte    *bfptr;
    
    SDP_DEVICE  *dev;
    POINT_BUF   *diPoint;
    
    printf("[DEVICE-%02d / Status POINT-%02d]\n", devid+1, point + 1);

    dev = (SDP_DEVICE *) deviceCFG[devid];
    printf("\n[DEVICE Information] ---------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n", 
        	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);   
    printf(" Device Address   : %2d\n", dev->ioid);
    printf(" [*] Device TYPE  : [%s]\n", scanDevType[dev->type]);
    printf(" [*] Point Config : [DI/DO/AI/Count] - %2d %2d %2d %2d \n", dev->devDiPoint, dev->devDoPoint,dev->devAiPoint, dev->devAoPoint);
    printf("--------------------------------------------------------------------\n");    

    while(ch != 'q')
    {
        /* ------------------------ */ 
        /* Status Bit Display ... 	*/
        /* ------------------------ */ 
        ptStart = point;
        ptStop  = point + 8;
        printf("=> STATUS Data [%3d - %3d] : ", ptStart+1, ptStop);
    
        for(i = ptStart, j=0, bfptr = msg; i < ptStop; i++, j++)
        {
            diPoint = (POINT_BUF *) &dev->diPtBuf[i];
            *bfptr++ = ' ';
            *bfptr++ = '0' + diPoint->status;      

            if((j & 0x07) == 0x07)
            {
                *bfptr = '\0';
                printf(" %s ", msg);
                bfptr = msg;
            }
            else if((j & 3) == 3)
            {
                *bfptr = ' ';
                bfptr++;
                *bfptr = '-';
                bfptr++;
            }
        } /* for */
        
        ch = getchar();
    }
 
    printf("\n\n");
}

/*----------------------------------------------------------------------------
* Function Name : dispDevStatus()
* 수행내용: 각 계전기 별 수집된 DI 정보를 표출함
* ArgList :
*   1. port - 출력하고자 하는 계전기 번호
* Return  :  
---------------------------------------------------------------------------- */  
void dispDevAnalog(int devid, int point)
{
    int     i, j;
    int     ptStart, ptStop;
    char    ch=0;
    
    SDP_DEVICE  *dev;
    POINT_BUF   *aiPoint;
    
    printf("[DEVICE-%02d / Analog POINT-%02d]\n", devid+1, point + 1);

    dev = (SDP_DEVICE *) deviceCFG[devid];
    printf("\n[DEVICE Information] ---------------------------[%4d/%2d/%2d %02d:%02d:%02d]\n", 
        	((rtc->year%100) + 2000), rtc->month, rtc->day, rtc->hour, rtc->min, rtc->sec);    
    printf(" Device Address   : %2d\n", dev->ioid);
    printf(" [*] Device TYPE  : [%s]\n", scanDevType[dev->type]);
    printf(" [*] Point Config : [DI/DO/AI/Count] - %2d %2d %2d %2d \n", dev->devDiPoint, dev->devDoPoint,dev->devAiPoint, dev->devAoPoint);
    printf("--------------------------------------------------------------------\n");    

    while(ch != 'q')
    {
        
        /* ------------------------ */               
        /* ANALOG Input Display ... */
        /* ------------------------ */ 
        ptStart = point;
        ptStop  = point + 8;
        printf("=> ANALOG Data [%3d - %3d] : ", ptStart+1, ptStop);
        
        for(i = ptStart,j=0; i < ptStop; i++, j++)
        {
            aiPoint = (POINT_BUF *) &dev->aiPtBuf[i];
            printf("%05.2f ", aiPoint->floatData);
                
            if(j== 10) 
            {
                j = 0;
                printf("\n");
            }
            
        }
        
        ch = getchar();
    }
 
    printf("\n\n");
}


/*----------------------------------------------------------------------------
* Function Name : CmdDispPOINTConfig()
* 수행내용: CCU내의 모듈에 대한 포인트 구성에 대한 정보를 Console로 출력하는 함수
* ArgList :
*   1. ioid - 출력하고자 하는 모듈 번호
* Return  :  
---------------------------------------------------------------------------- */   
void CmdDispPOINTConfig_61850()
{
    int     ioid, i,ch;
    //int     devNo, devPt;
    //int     ptType, devType, ptConfig;
    //int		port1, point1;
    //int     dnp_index[MAX_HOST];
    //int     dbmax, dbhostpt, modBase, modIndex;
    //int     iccpType, iccpIndex;
    //int     rev1, rev2;
    //byte    ptname[32];
    
    //POINT_BUF *ptBuf;
    //DB_POINT_BUF    *devPoint;
    DEVICE_61850    *dev61850;
    POINT_61850     *pt61850;

    
    printf("[61850 INFO] -------------------\n");
    printf(" PT-TYPE   No.  [ dNo dPt]  SCAN  NAME \n");
    printf("---------------------------------\n");
    for(ioid = 0; ioid < MAX_DEVICE; ioid++)
    {
        dev61850 = (DEVICE_61850 *) device61850[ioid];
        if(dev61850->config61850 == 0 ) continue;
         
        /* ---------------------------------------- */
        /*  61850 계전기 - DI 포인트 정보           */
        /* ---------------------------------------- */
        for(i=0; i < MAX_DEV_DI_POINT; i++)
        {
            pt61850 = (POINT_61850 *) &dev61850->diPtBuf[i];
            if(pt61850->config61850 == 0) continue;
            printf("  STATUS  %04d    %2d  %04d   %d  \n", i+1, ioid+1, i+1, pt61850->config61850);
        }
        
        /* ---------------------------------------- */
        /*  61850 계전기 - AI 포인트 정보           */
        /* ---------------------------------------- */
        for(i=0; i < MAX_DEV_AI_POINT; i++)
        {
            pt61850 = (POINT_61850 *) &dev61850->aiPtBuf[i];
            if(pt61850->config61850 == 0) continue;
            
            printf("  ANALOG  %04d    %2d  %04d   %d \n", i+1, ioid+1, i+1, pt61850->config61850);
        }             

        /* ---------------------------------------- */
        /*  61850 계전기 - DO 포인트 정보           */
        /* ---------------------------------------- */
        for(i=0; i < MAX_DEV_DO_POINT; i++)
        {
            pt61850 = (POINT_61850 *) &dev61850->doPtBuf[i];
            if(pt61850->config61850 == 0) continue;

            printf(" CONTROL  %04d    %2d  %04d   %d \n", i+1, ioid+1, i+1, pt61850->config61850);
        }                     

        printf("---------------------------------[q]... quit!\n");
        
        ch=getchar();
        if(ch == 'q')  break;
        printf("[61850 INFO] -------------------\n");
        printf(" PT-TYPE   No.  [ dNo dPt]  SCAN  NAME \n");
        printf("---------------------------------\n");
    }
    
    printf("\n");
    
} 


/*----------------------------------------------------------------------------
* Function Name : CmdDispPOINTConfig()
* 수행내용: CCU내의 모듈에 대한 포인트 구성에 대한 정보를 Console로 출력하는 함수
* ArgList :
*   1. ioid - 출력하고자 하는 모듈 번호
* Return  :  
---------------------------------------------------------------------------- */   
void CmdDispPOINTConfig()
{
    int     i,ch, count=0 ;
    int     devNo, devPt;
    int     ptType, devType, ptConfig;
    int		port1, point1;
    int     dnp_index[MAX_HOST];
    int     dbmax, dbhostpt, modBase, modIndex;
    int     iccpType, iccpIndex;
    //int     rev1, rev2;
    byte    ptname[40];
    
    //POINT_BUF *ptBuf;
    DB_POINT_BUF    *devPoint;

    printf("\n%s : RTUDB Point Configure ...\n", TARGET_NAME);

    printf("---------------------------------------------------------------------------------------------------------------\n");
    printf(" No.  [ dNo dPt]  TYPE  PT-TY  ICCP-INDX  M-base M-inx Cfg DBMAX [PO PT] LOC HST1 HST2 HST3 HST4 HST5 \n");
    printf("---------------------------------------------------------------------------------------------------------------\n");
    
    for(i = 0; i < MAX_DBASE_POINT; i++)
    {
        devPoint = (DB_POINT_BUF *) &rtudb->pointBuf[i];
        devNo = devPoint->devNo;
        devPt = (devPoint->devPt[0]*256) + devPoint->devPt[1];
        
        //if((devNo < 1) || (devNo > MAX_MODULE))     	continue;
        //if((devPt < 1) || (devPt > MAX_DEV_DI_POINT))   continue;
        //if(devPoint->devType > DEV_POINT)   continue;

        bzero8248(ptname, 40);
        
        /* DNP-HOST Information */
        devType    = devPoint->devType;
        ptType     = devPoint->ptType;
        ptConfig   = devPoint->ptConfig;
        
        /* ICCP-HOST 구성정보 */
        iccpType   = devPoint->iccpPointType;
        iccpIndex  = (devPoint->iccpPointIndex[0]*256) + devPoint->iccpPointIndex[1];
        //rev1       = devPoint->reserved1;
        //rev2       = devPoint->reserved2;
        
        modBase    = (devPoint->modBase[0]*256) + devPoint->modBase[1];
        modIndex   = (devPoint->modIndex[0]*256) + devPoint->modIndex[1];
        dbmax      = (devPoint->pointMax[0]*256) + devPoint->pointMax[1];
        port1      = devPoint->port;
        point1     = devPoint->point;
        
        dbhostpt   = (devPoint->localIndex[0]*256) + devPoint->localIndex[1];
        dnp_index[0]= (devPoint->hostIndex[0][0]*256) + devPoint->hostIndex[0][1];
        dnp_index[1]= (devPoint->hostIndex[1][0]*256) + devPoint->hostIndex[1][1];
        dnp_index[2]= (devPoint->hostIndex[2][0]*256) + devPoint->hostIndex[2][1];
        dnp_index[3]= (devPoint->hostIndex[3][0]*256) + devPoint->hostIndex[3][1];
        dnp_index[4]= (devPoint->hostIndex[4][0]*256) + devPoint->hostIndex[4][1];
        
        memcpy(ptname, devPoint->ptNameStr, 40);
        ptname[39] = '\0';
        
        printf(" %03d   [%2d %4d] %7s %4s |%4s %4d| %6d  %3d  %2d   %4d  %2d %2d   %2d %4d %4d %4d %4d %4d  %s \n",
            i+1, devNo,  devPt,     
            devTypeStr[devType], pointTypeStr[ptType], iccpPTypeStr[iccpType], iccpIndex, modBase, modIndex, ptConfig, dbmax, 
            port1, point1, dbhostpt, 
            dnp_index[0], dnp_index[1], dnp_index[2],dnp_index[3],dnp_index[4], ptname);
                    
        
        if(++count == 32)
        {
            count = 0;
            printf("[q]... quit!\n");
            ch=getchar();
            if(ch == 'q')  break;

    printf("---------------------------------------------------------------------------------------------------------------\n");
    printf(" No.  [ dNo dPt]  TYPE  PT-TY  ICCP-INDX  M-base M-inx Cfg DBMAX [PO PT] LOC HST1 HST2 HST3 HST4 HST5 \n");
    printf("---------------------------------------------------------------------------------------------------------------\n");

        }
                    
    }
    printf("\n");

} 

