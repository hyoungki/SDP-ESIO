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
 
//
// Include File
//
#include    "localLib.h"

// CRC Code Table
unsigned short crcTable[256] = {
	0x0000,  0xc0c1,  0xc181,  0x0140,  0xc301,  0x03c0, 0x0280, 0xc241,
	0xc601,  0x06c0,  0x0780,  0xc741,  0x0500,  0xc5c1, 0xc481, 0x0440,
	0xcc01,  0x0cc0,  0x0d80,  0xcd41,  0x0f00,  0xcfc1, 0xce81, 0x0E40,
	0x0a00,  0xcac1,  0xcb81,  0x0b40,  0xc901,  0x09c0, 0x0880, 0xc841,
	0xd801,  0x18c0,  0x1980,  0xd941,  0x1b00,  0xdbc1, 0xda81, 0x1a40,
	0x1e00,  0xdec1,  0xdf81,  0x1f40,  0xdd01,  0x1dc0, 0x1c80, 0xdc41,
	0x1400,  0xd4c1,  0xd581,  0x1540,  0xd701,  0x17c0, 0x1680, 0xd641,
	0xd201,  0x12c0,  0x1380,  0xd341,  0x1100,  0xd1c1, 0xd081, 0x1040,
	0xf001,  0x30c0,  0x3180,  0xf141,  0x3300,  0xf3c1, 0xf281, 0x3240,
	0x3600,  0xf6c1,  0xf781,  0x3740,  0xf501,  0x35c0, 0x3480, 0xf441,
	0x3c00,  0xfcc1,  0xfd81,  0x3d40,  0xff01,  0x3fc0, 0x3e80, 0xfe41,
	0xfa01,  0x3ac0,  0x3b80,  0xfb41,  0x3900,  0xf9c1, 0xf881, 0x3840,
	0x2800,  0xe8c1,  0xe981,  0x2940,  0xeb01,  0x2bc0, 0x2a80, 0xea41,
	0xee01,  0x2ec0,  0x2f80,  0xef41,  0x2d00,  0xedc1, 0xec81, 0x2c40,
	0xe401,  0x24c0,  0x2580,  0xe541,  0x2700,  0xe7c1, 0xe681, 0x2640,
	0x2200,  0xe2c1,  0xe381,  0x2340,  0xe101,  0x21c0, 0x2080, 0xe041,
	0xa001,  0x60c0,  0x6180,  0xa141,  0x6300,  0xa3c1, 0xa281, 0x6240,
	0x6600,  0xa6c1,  0xa781,  0x6740,  0xa501,  0x65c0, 0x6480, 0xa441,
	0x6c00,  0xacc1,  0xad81,  0x6d40,  0xaf01,  0x6fc0, 0x6e80, 0xae41,
	0xaa01,  0x6ac0,  0x6b80,  0xab41,  0x6900,  0xa9c1, 0xa881, 0x6840,
	0x7800,  0xb8c1,  0xb981,  0x7940,  0xbb01,  0x7bc0, 0x7a80, 0xba41,
	0xbe01,  0x7ec0,  0x7f80,  0xbf41,  0x7d00,  0xbdc1, 0xbc81, 0x7c40,
	0xb401,  0x74c0,  0x7580,  0xb541,  0x7700,  0xb7c1, 0xb681, 0x7640,
	0x7200,  0xb2c1,  0xb381,  0x7340,  0xb101,  0x71c0, 0x7080, 0xb041,
	0x5000,  0x90c1,  0x9181,  0x5140,  0x9301,  0x53c0, 0x5280, 0x9241,
	0x9601,  0x56c0,  0x5780,  0x9741,  0x5500,  0x95c1, 0x9481, 0x5440,
	0x9c01,  0x5cc0,  0x5d80,  0x9d41,  0x5f00,  0x9fc1, 0x9e81, 0x5e40,
	0x5a00,  0x9ac1,  0x9b81,  0x5b40,  0x9901,  0x59c0, 0x5880, 0x9841,
	0x8801,  0x48c0,  0x4980,  0x8941,  0x4b00,  0x8bc1, 0x8a81, 0x4a40,
	0x4e00,  0x8ec1,  0x8f81,  0x4f40,  0x8d01,  0x4dc0, 0x4c80, 0x8c41,
	0x4400,  0x84c1,  0x8581,  0x4540,  0x8701,  0x47c0, 0x4680, 0x8641,
	0x8201,  0x42c0,  0x4380,  0x8341,  0x4100,  0x81c1, 0x8081, 0x4040};




/* 배열 myclock[]로부터 시간차를 구해서 리턴한다 */
int calclock (struct timespec inclock, struct timespec outclock) 
{
    int    nano_time, temp, temp_n;
    int    milisec;
    
    //sec = outclock.tv_sec - inclock.tv_sec;
    
    if (outclock.tv_sec >= inclock.tv_sec) 
    {
        temp = outclock.tv_sec - inclock.tv_sec;
        temp_n = outclock.tv_nsec - inclock.tv_nsec;
        nano_time = 1000000000 * temp + temp_n;
        //nano_time = 100000000 * temp + temp_n/10;
    } 
    else 
    {
        temp = outclock.tv_sec - inclock.tv_sec - 1;
        temp_n = 1000000000 + outclock.tv_nsec - inclock.tv_nsec;
        nano_time = 1000000000 * temp + temp_n;
        //nano_time = 100000000 * temp + temp_n/10;
    }

    // usec time
    milisec = nano_time / 1000000;
    
    //return milisec*10;      // 10ms 단위 전송
    return (milisec);       // ms 단위 전송
}

/**
**  print Transmition Message
**/
void prndat(const char *msg, char *str, int  len)
{
    int i;
    int size;
    byte digit;
    char *bfptr;
    char buf[80];

	size = strlen(msg) ;
    memcpy(buf, msg,size);
    bfptr = buf + size;

    for(;len > 0;)
    {
        for(i = 0; i < 20; i++)
        {
            *bfptr++ = ' ';
            digit = (*str >> 4) & 0x0f;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            digit = *str++ & 0xf;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            if(--len <= 0) break;
        }
        *bfptr = '\0';
        printf("%s\n\r", buf);
        for(i = 0, bfptr = buf; i < size; i++, bfptr++) *bfptr = ' ';
    } /* for */

}

/**
**  print Transmition Message
**/
void prn_rcv(msg, str, len)
char   *msg, *str;
int len;
{
    int i;
    int size;
    byte digit;
    int first;
    char *bfptr;
    char buf[80];

    size = strlen(msg);
    
    memcpy(buf, msg, size);
    bfptr = buf + size;
    first = 1;

    for(;len > 0;)
    {
        for(i = 0; i < 16; i++)
        {
            *bfptr++ = ' ';
            digit = (*str >> 4) & 0x0f;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            digit = *str++ & 0xf;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            if(--len <= 0) break;
            if((first == 1) && (i == 9))
            {
                first = 0;
                break;
            }
        }
        *bfptr = '\0';
        printf("%s\n", buf);
        for(i = 0, bfptr = buf; i < size; i++, bfptr++) *bfptr = ' ';
    } /* for */

}

/**
**  print Transmition Message
**/
void prn_snd(msg, str, len)
char   *msg, *str;
int len;
{
    int i;
    int size;
    byte digit;
    int first;
    char *bfptr;
    char buf[80];

    memcpy(buf, msg, size = strlen(msg));
    bfptr = buf + size;
    first = 1;

    for(;len > 0;)
    {
        for(i = 0; i < 18; i++)
        {
            *bfptr++ = ' ';
            digit = (*str >> 4) & 0x0f;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            digit = *str++ & 0xf;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            if(--len <= 0) break;
            if((first == 1) && (i == 9))
            {
                first = 0;
                break;
            }
        }
        *bfptr = '\0';
        printf("%s\n", buf);
        for(i = 0, bfptr = buf; i < size; i++, bfptr++) *bfptr = ' ';
    } /* for */

}


// ===================================================
#ifdef	NETWORK_CONSOLE   
// ===================================================

//
// 모듈:	DumpBuffer()
//
void DumpDNP_rcv(CONSOLE_INFO *debug, ...)
{
	int		i, size;
	byte 	digit;
	word	front;
	int		length;
	int		bufsize;
	int first;
	char	*stamp;
	unsigned char	*buffer;
	va_list	argList;
    char *bfptr;
    char buf[256];

	if(debug->active == 0)	return ;
	for(i=0;i < 5; i++)
	{
		if(debug->writeFlag == SET)	    usleep(100);
		else break;		
	}
	
	va_start(argList, debug);
	
	stamp  = va_arg(argList, char *);
	buffer = va_arg(argList, unsigned char *);
	length = va_arg(argList, int);	

	size = strlen(stamp) ;
    memcpy(buf, stamp,size);
    bfptr = buf + size;

	debug->writeFlag = SET;

	first = 1;

    for(;length > 0;)
    {
        for(i = 0; i < 16; i++)
        {
            *bfptr++ = ' ';
            digit = (*buffer >> 4) & 0x0f;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            digit = *buffer++ & 0xf;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            if(--length <= 0) break;
            if((first == 1) && (i == 9))
            {
                first = 0;
                break;
            }
        }
        *bfptr++ = '\n';
        *bfptr = '\0';
        //printf("lib> %s", buf);
        bufsize = strlen(buf);
        front = debug->front & LOG_MASK_NUM;
        memcpy( debug->logData[front], buf, bufsize+1);
		debug->front = (front+1) & LOG_MASK_NUM;
        
        for(i = 0, bfptr = buf; i < size; i++, bfptr++) *bfptr = ' ';
    } /* for */
        	
	va_end(argList);
	
	debug->writeFlag = RESET;	
	//usleep(500);
	
}


//
// 모듈:	DumpBuffer()
//
void DumpDNP_snd(CONSOLE_INFO *debug, ...)
{
	int		i, size;
	byte 	digit;
	word	front;
	int		length;
	int		bufsize;
	int first;
	char	*stamp;
	unsigned char	*buffer;
	va_list	argList;
    char *bfptr;
    char buf[256];

	if(debug->active == 0)	return ;
	for(i=0;i < 5; i++)
	{
		if(debug->writeFlag == SET)	    usleep(100);
		else break;		
	}
	
	va_start(argList, debug);
	
	stamp  = va_arg(argList, char *);
	buffer = va_arg(argList, unsigned char *);
	length = va_arg(argList, int);	

	size = strlen(stamp) ;
    memcpy(buf, stamp,size);
    bfptr = buf + size;

	debug->writeFlag = SET;

	first = 1;

    for(;length > 0;)
    {
        for(i = 0; i < 18; i++)
        {
            *bfptr++ = ' ';
            digit = (*buffer >> 4) & 0x0f;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            digit = *buffer++ & 0xf;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            if(--length <= 0) break;
            if((first == 1) && (i == 9))
            {
                first = 0;
                break;
            }
        }
        *bfptr++ = '\n';
        *bfptr = '\0';
        //printf("lib> %s", buf);
        bufsize = strlen(buf);
        front = debug->front & LOG_MASK_NUM;
        memcpy( debug->logData[front], buf, bufsize+1);
		debug->front = (front+1) & LOG_MASK_NUM;
        
        for(i = 0, bfptr = buf; i < size; i++, bfptr++) *bfptr = ' ';
    } /* for */
        	
	va_end(argList);
	
	debug->writeFlag = RESET;	
	//usleep(500);
	
}

//
// 모듈:	Debug()
//
void Debug(CONSOLE_INFO *debug, ...)
{
	int	 i;
	char	*format;
	int		length;
	word	front;
	byte	buffer[256];
	va_list	argList;

	if(debug->active == 0)	return ;
		
	for(i=0;i < 5; i++)
	{
		if(debug->writeFlag == SET)	usleep(100);
		else break;		
	}
		
	debug->writeFlag = SET;
			
	front = debug->front & LOG_MASK_NUM;
	va_start(argList, debug);
	
	format = va_arg(argList, char *);
	vsprintf( (char *) buffer, format, argList);
	
	//vprintf(format, argList);
	va_end(argList);
	
	length = strlen((char *) buffer);
	buffer[length++] = '\0';
	memcpy( debug->logData[front], buffer, length);
	debug->front = (front+1) & LOG_MASK_NUM;
	
	debug->writeFlag = RESET;	
	//usleep(500);
	
}

//
// 모듈:	DumpBuffer()
//
void DumpBuff(CONSOLE_INFO *debug, ...)
{
	int		i, size;
	byte 	digit;
	word	front;
	int		length;
	int		bufsize;
	char	*stamp;
	unsigned char	*buffer;
	va_list	argList;
    char *bfptr;
    char buf[256];

 
	if(debug->active == 0)	return ;
	for(i=0;i < 5; i++)
	{
		if(debug->writeFlag == SET)	    usleep(100);
		else break;		
	}
	
	va_start(argList, debug);
	
	stamp  = va_arg(argList, char *);
	buffer = va_arg(argList, unsigned char *);
	length = va_arg(argList, int);	

	size = strlen(stamp) ;
    memcpy(buf, stamp,size);
    bfptr = buf + size;

	debug->writeFlag = SET;
    	
    for(;length > 0;)
    {
        for(i = 0; i < 20; i++)
        {
            *bfptr++ = ' ';
            digit = (*buffer >> 4) & 0x0f;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            digit = *buffer++ & 0xf;
            *bfptr++ = (digit < 10) ? digit + '0'       /* '0' to '9' */
                                    : digit + 'A' - 10  /* 'A' to 'F' */;
            if(--length <= 0) break;
        }
        *bfptr++ = '\n';
        *bfptr = '\0';
        //printf("lib> %s", buf);
        bufsize = strlen(buf);
        front = debug->front & LOG_MASK_NUM;
        memcpy( debug->logData[front], buf, bufsize+1);
		debug->front = (front+1) & LOG_MASK_NUM;
        
        for(i = 0, bfptr = buf; i < size; i++, bfptr++) *bfptr = ' ';
    } /* for */
	
	va_end(argList);
	
	debug->writeFlag = RESET;	
	//usleep(500);
	
}

// ===================================================
#else
// ===================================================

//
// 모듈:	DumpBuffer()
//
void DumpDNP_rcv(CONSOLE_INFO *debug, ...)
{
	//int		i, size;
	//byte 	digit;
	//word	front, rear;
	int		length;
	//int		bufsize;
	//int first;
	char	*stamp;
	unsigned char	*buffer;
	va_list	argList;
    //char *bfptr;
    //char buf[256];

	va_start(argList, debug);
	
	stamp  = va_arg(argList, char *);
	buffer = va_arg(argList, unsigned char *);
	length = va_arg(argList, int);	

	va_end(argList);

	prn_rcv( stamp, (char *) buffer, length);
	
	
}


//
// 모듈:	DumpBuffer()
//
void DumpDNP_snd(CONSOLE_INFO *debug, ...)
{
	//int		i, size;
	//byte 	digit;
	//word	front, rear;
	int		length;
	//int		bufsize;
	//int first;
	char	*stamp;
	unsigned char	*buffer;
	va_list	argList;
    //char *bfptr;
    //char buf[256];

	va_start(argList, debug);
	
	stamp  = va_arg(argList, char *);
	buffer = va_arg(argList, unsigned char *);
	length = va_arg(argList, int);	

	va_end(argList);

	prn_snd( stamp, (char *) buffer, length);
	
}

//
// 모듈:	Debug()
//
void Debug(CONSOLE_INFO *debug, ...)
{
	//int	 i;
	char	*format;
	//int		level;
	//int		length;
	//word	front, rear;
	//byte	buffer[256];
	va_list	argList;

	va_start(argList, debug);
	
	format = va_arg(argList, char *);
	//vsprintf( buffer, format, argList);
	vprintf(format, argList);
	va_end(argList);
}

//
// 모듈:	DumpBuffer()
//
void DumpBuff(CONSOLE_INFO *debug, ...)
{
	//int		i, size;
	//byte 	digit;
	//word	front, rear;
	int		length;
	//int		bufsize;
	char	*stamp;
	unsigned char	*buffer;
	va_list	argList;
    //char *bfptr;
    //char buf[256];

	va_start(argList, debug);
	
	stamp  = va_arg(argList, char *);
	buffer = va_arg(argList, unsigned char *);
	length = va_arg(argList, int);	

	va_end(argList);

	prndat( stamp, (char *) buffer, length);
	
}


/* NETWORK_CONSOLE ... end */
// ===================================================
#endif
// ===================================================





/*
 *  LED Display 
 */
void outputLED(int fda, int	pos, int value)
{
}

//
// 모듈:	GetTod()
//
struct tm *GetTod(void)
{
	time_t	clock;

	time(&clock);
	return(localtime(&clock));
}

//
// 모듈:	SetClock()
//
void SetClock(struct timeval *tv)
{
	struct timezone	tz;

	gettimeofday(tv, &tz);
}

//
// 모듈:	SetWord()
//
void SetWord(char *ptr, int value, int mode)
{
	switch (mode)
	{
	case 1:
		*ptr = value / 256;
		*(ptr+1) = value % 256;
		break;
	default:
		*ptr = value % 256;
		*(ptr+1) = value / 256;
		break;
	}
}

//
// 모듈:	GetClock()
//
int GetClock(struct timeval *prev)
{
	int		elapsedTime;
	struct timeval	tv;
	struct timezone	tz;
	
	gettimeofday(&tv, &tz);
	tv.tv_sec -= prev->tv_sec;
	prev->tv_sec = 0;
	if (tv.tv_sec < 0)
		return(0);
	elapsedTime = tv.tv_sec * 1000000 + tv.tv_usec - prev->tv_usec;
	
	elapsedTime = (elapsedTime / 1000); // ms 
	return(elapsedTime);
}

//
// 모듈:	GetMsecDelta()
//
int GetMsecDelta(struct timeval *prev, struct timeval *cur)
{
	int		secDelta, usecDelta;

	secDelta = cur->tv_sec - prev->tv_sec;
	if (secDelta < 0)
		return(0);
	usecDelta = cur->tv_usec - prev->tv_usec;
	return(secDelta * 1000 + usecDelta/ 1000);
}


//
// 모듈: ExecCommand()
//
int ExecCommand(char *format, ...)
{
	int	retVal=0;
    char    command[MESSAGE_LEN];
	va_list argList;

	va_start(argList, format);
	vsprintf(command, format, argList);
	va_end(argList);
	
	retVal = system(command);
	
	return (retVal);
	//printf("exec> %s\n", command);
}




/*
 *  Generate LRC code
 */
byte genlrc(byte *buf, int bfcnt)
{
    byte LRC = 0;
    for(;bfcnt > 0; bfcnt--, buf++) LRC ^= *buf;
    return(LRC);
}

/*
 *  Generate chksum
 */
word gensum(byte *buf, int bfcnt)
{
    word   chksum = 0;
    for(;bfcnt > 0; bfcnt--, buf++) chksum += *buf;
    return(chksum);
}

/*
 *  Generate chksum
 */
byte	genSumByte(byte *buf, int bfcnt)
{
    byte	chksum = 0;
    
    for(;bfcnt > 0; bfcnt--, buf++) chksum += *buf;
    
    return(chksum);
}



//
// 모듈:	GenLrc()
//
BYTE GenLrc(byte *buffer, int length)
{
	BYTE	lrcCode;

	for (lrcCode = 0; length > 0; length--, buffer++)
		lrcCode ^= *buffer;
	return(lrcCode);
}

//
// 모듈:	GenCrc()
//
WORD GenCrc(char *buffer, int length)
{
	WORD	crcCode;

	for(crcCode = 0; length > 0; length--, buffer++)
		crcCode = (crcCode >> 8) ^ crcTable[(crcCode ^ *buffer) & 0x00FF];
	return(crcCode);
}

word genCrc_modbus (byte *ptr, int len)
{
	word   crc = 0xffff;
	int    idx;

	for (idx = 0; idx < len; idx++, ptr++)
		crc = ((crc >> 8) & 0x00ff) ^ crcTable[(crc & 0xff)^*ptr];

	return(crc);
}

//
// 모듈: IsProcessActive()
//
int IsProcessActive(int pid)
{
	if (pid <= 1)
		return(-1);
	if (kill(pid, 0) < 0)
		return(-1);
	return(0);
}

//
// 모듈: KillProcess()
//
void KillProcess(pid_t pid, int wait)
{
	time_t	curClock, start;

    printf("[KillProcess] pid=%d...wait=%d!\n", pid, wait);
    
	if (pid <= 1)
		return;
	if (kill(pid, SIGTERM) < 0)
		return;

	curClock = time(&start);		// Start Time
	while (curClock < start + wait)
	{
		usleep(100000);			// 100 msec wait
		if (kill(pid, 0) < 0)
			return;
		time(&curClock);
	}
	
	kill(pid, SIGKILL);
}


//
// 모듈: StartProcess()
//
int StartProcess(char *name, char *argument)
{
#ifdef  ROM_VERSION
	char	*homeDir = "/mnt/bin";
#else
	char	*homeDir = ".";
#endif
	int	retVal=0;
	
	// Get Home Directory
	//if ((homeDir = getenv("AES_HOME")) == NULL || strlen(name) == 0)
	//	return;
    
	if (strlen(argument))
	{		
		retVal = ExecCommand("%s/%s %s &", homeDir, name, argument);
	}
	else	
	{
		retVal = ExecCommand("%s/%s &", homeDir, name);
	}
	
	return (retVal);
	//printf("wdt> Start Process : %s/%s &\n", homeDir, name);
	    
}

//
// 모듈: UpdateProcessInfo()
//
void UpdateProcessInfo(TASK_INFO *prcPtr, int state, pid_t pid)
{
	prcPtr->active = state;
	//prcPtr->wdtCount = prcPtr->wdt = 0;
	prcPtr->wdtCount = 0;
	prcPtr->pid = (state ? pid : 0);
	if (state)
		prcPtr->maxEtime = 0;
}

//
// 모듈: ShmCreate()
//
int ShmCreate(SHM_DESC *sp)
{
	if ((sp->id = shmget(sp->key, sp->size, 0x1B6)) < 0)
	{
		if (errno == ENOENT)
		{
			if ((sp->id = shmget(sp->key, sp->size, 0x1B6|IPC_CREAT)) < 0)
		    {	    
		        printf("shm>> 1) shmget() error.... %d\n", sp->id);
				return(-1);
			}
		}
		else    
		{
		    printf("shm>> 2) shmget() error.... %d\n", sp->id);
		    return(-1);
		}
	}

	if ((sp->address = (char *)shmat(sp->id, 0, 0x1B6)) == (char *)-1)
    {	
		return(-1);
	}
	return(0);
}

//
// 모듈:	ShmDetach()
//
void ShmDetach(SHM_DESC *sp)
{
	if (sp->id <= 0)
		return;
	shmdt(sp->address);
}

//
// 모듈: ShmCheck()
//
int ShmCheck(SHM_DESC *sp)
{
	if ((sp->id = shmget(sp->key, sp->size, 0x1B6)) < 0)
		return(-1);

	if ((sp->address = (char *)shmat(sp->id, 0, 0x1B6)) == (char *)-1)
		return(-1);
	return(0);
}

//
// 모듈:	ShmDelete()
//
void ShmDelete(SHM_DESC *sp)
{
	if (sp->id < 0 || sp->address == NULL)
		return;
	if (shmdt(sp->address) < 0)
		return;

	shmctl(sp->id, IPC_RMID, (struct shmid_ds *) 0);
}

#if 0
//
// 모듈:	QueueCreate()
//
int QueueCreate(MSG_DESC *qp)
{
	if ((qp->id = msgget(qp->key, 0x1B6)) >= 0)
		return(0);

	if (errno == ENOENT)
	{
		if ((qp->id = msgget(qp->key, 0x1B6|IPC_CREAT)) >= 0)
			return(0);
	}
	return(-1);
}

//
// 모듈:	QueueCreateList()
//
int QueueCreateList(MSG_DESC *qp, int count)
{
	int		idx;
	
	for (idx = 0; idx < count; idx++, qp++)
	{
		if (QueueCreate(qp) < 0)
			return(0);
	}
	return(1);
}

//
// 모듈:	QueueCheck()
//
int QueueCheck(MSG_DESC *qp)
{
	if ((qp->id = msgget(qp->key, 0x1B6)) >= 0)
		return(0);
	return(-1);
}

//
// 모듈:	QueueDelete()
//
void QueueDelete(MSG_DESC *qp)
{
	if (qp->id < 0)
		return;
	(void)msgctl(qp->id, IPC_RMID, (struct msqid_ds *)0);
}

//
// 모듈:	QueueDeleteList()
//
void QueueDeleteList(MSG_DESC *qp, int count)
{
	int		idx;

	for (idx = 0; idx < count; idx++, qp++)
		QueueDelete(qp);
}

//
//  모듈:	QueueSnd()
//
int QueueSnd(MSG_DESC *qp, char *buffer, int length)
{
	if (length <= 0 || length > MSG_LENGTH)
		return(-3);

	if (qp->id < 0)
		return(-2);

	qp->mBuffer.mType = 1;
	memcpy(qp->mBuffer.mText, buffer, length);
	if (msgsnd(qp->id, &qp->mBuffer, length, IPC_NOWAIT) < 0)
		return(-1);
	return(0);
}

//
// 모듈:	QueueRcv()
//
int QueueRcv(MSG_DESC *qp, char *buffer, int length)
{
	int		rxCount;

	if (qp->id < 0)
		return(-2);

	if ((rxCount = msgrcv(qp->id, &qp->mBuffer, length, 0, IPC_NOWAIT)) < 0)
	{
		if ((errno != ENOMSG) && (errno != EINTR))
			return(-1);
		return(0);
	}

	memcpy(buffer, qp->mBuffer.mText, rxCount);
	return(rxCount);
}

//
// 모듈:	QueueClear()
//
void QueueClear(MSG_DESC *qp)
{
	if (qp->id < 0)
		return;
	while (1)
	{
		if (msgrcv(qp->id, &qp->mBuffer, MSG_LENGTH, 0, IPC_NOWAIT) < 0)
			break;
	}
}
#endif

//
// 모듈:    CheckPort()
//
void rtsControl_860(TTY_DESC *tp, int   mode)
{
	byte	baudrate;

    if(mode == RTS_OFF)
	{
	    baudrate = 0x00;
   	    //printf("SET RTS HIGH (INACTIVE)\n");
		ioctl(tp->id, SCC_IOC_RTSCTRL, &baudrate);
    }
    else if(mode == RTS_ON)
    {
	    baudrate = 0x02;
	    //printf("SET RTS LOW (ACTIVE)\n");
		ioctl(tp->id, SCC_IOC_RTSCTRL, &baudrate);
	}		
}


//
// 모듈:    CheckPort()
//
void rtsControl(TTY_DESC *tp, int   mode)
{
	int	modem_status;
	int    rts_mode;
	
	ioctl(tp->id, TIOCMGET, &modem_status);

    if(mode == RTS_ON)
	{
	    rts_mode = modem_status | TIOCM_RTS;
		ioctl(tp->id, TIOCMSET, &rts_mode);
    }
    else if(mode == RTS_OFF)
    {
	    rts_mode = modem_status & ~TIOCM_RTS;
		ioctl(tp->id, TIOCMSET, &rts_mode);
	}		
	
}

int checkCTS(TTY_DESC *tp)
{
	int	modem_status;
	//int    rts_mode;
	
	ioctl(tp->id, TIOCMGET, &modem_status);

    if(modem_status & TIOCM_CTS)
    {
        //printf("CTS ON (INACTIVE)\n");
        return (1);
    }
    else
    {
        //printf("*CTS OFF (INACTIVE)\n");
        return (0);
    }
    
}

//
// 모듈:	PortAttribute()
//
int PortAttribute(TTY_DESC *tp)
{
	unsigned short	newAttribute;
	struct termio	newArg;

	newAttribute = CREAD;

	switch (tp->baudRate)
	{
	case 1200: newAttribute |= B1200; break;
	case 2400: newAttribute |= B2400; break;
	case 4800: newAttribute |= B4800; break;
	case 19200: newAttribute |= B19200; break;
	case 38400: newAttribute |= B38400; break;
	case 57600: newAttribute |= B57600; break;
	case 115200: newAttribute |= B115200; break;
	case 9600:
	default: newAttribute |= B9600; break;
	}

	switch (tp->dataBits)
	{
	case 7: newAttribute |= CS7;break;
	case 8:
	default: newAttribute |= CS8; break;
	}

	switch (tp->stopBits)
	{
	case 2: newAttribute |= CSTOPB; break;
	case 1:
	default: break;
	}

	switch (tp->parity)
	{
	case 'E': newAttribute |= PARENB; break;
	case 'O': newAttribute |= (PARENB | PARODD); break;
	case 'N':
	default: break;
	}

	if (ioctl(tp->id, TCGETA, (struct termio *) &newArg) < 0)
		return(-1);
	newArg.c_iflag &= ~IXON;
	newArg.c_iflag &= ~INLCR;
	newArg.c_iflag &= ~ICRNL;
	newArg.c_iflag &= ~BRKINT;
	
	newArg.c_iflag &= ~IXOFF;
	
	if (tp->dataBits != 8)
		newArg.c_iflag |= ISTRIP;
	else	newArg.c_iflag &= ~ISTRIP;
	newArg.c_oflag &= ~XTABS;
	newArg.c_oflag &= ~ONLCR;
	newArg.c_oflag &= ~OPOST;
	newArg.c_cflag = newAttribute;

	newArg.c_cc[4] = 1;		// MIN  = 1
	newArg.c_cc[5] = 5;		// TIME = 500
	newArg.c_cflag |= CLOCAL;
	newArg.c_cflag |= HUPCL;
	
	newArg.c_cflag &= ~CRTSCTS;
	
	newArg.c_lflag &= ~ECHO;
	newArg.c_lflag &= ~IEXTEN;
	newArg.c_lflag &= ~ICANON;
	newArg.c_lflag &= ~ISIG;
	
	//newArg.c_cflag |= IXANY;
	
	if (ioctl(tp->id, TCSETA, (struct termio *) &newArg) < 0)
		return(-1);
		
	return(0);
}

//
// 모듈:	PortClose()
//
void PortClose(TTY_DESC *tp)
{
	if (tp->id > 1)         close(tp->id);
	if (tp->fId != NULL)    fclose(tp->fId);
		
	tp->id = 0;
	tp->fId = NULL;
}

#if 0
//
// 모듈:	PortRxReady()
//
int PortRxReady(int fd, struct timeval *timeOut)
{
	fd_set	readFds;

	FD_ZERO(&readFds);
	FD_SET(fd, &readFds);
	return(select(MAX_TTY_DEVICE, &readFds, 0, 0, timeOut));
}
#endif

//
// 모듈:	PortOpen()
//
int PortOpen(TTY_DESC *tp)
{
	PortClose(tp);

	if ((tp->id = open(tp->name, O_RDWR)) < 0)
	{
	    printf("portOpen > err#1\n");
		return(-1);
    }
    
	if (PortAttribute(tp) < 0)
	{
	    printf("portOpen > err#2\n");
		PortClose(tp);
		return(-1);
	}

	if ((tp->fId = fopen(tp->name, "w+")) == NULL)
	{
	    printf("portOpen > err#3\n");
		return(-1);
	}
	
	//printf("portOpen > %s OK\n", tp->name);
	return(0);
}

//
// 모듈:	PortWrite()
//
int PortWrite(TTY_DESC *ttyDesc, byte *buffer, int length)
{
	int		txCount;

//return (0);

    /* NET860-1000 외부 8530 포트 초기화 시 ... */
    if(ttyDesc->config == ASYNC_8530)
    {
    	if ((txCount = write( ttyDesc->id, buffer, length)) < 0)
    	{
	    	//printf("[*] Port8530 Write Error....%s, length=%d\n", ttyDesc->name, length);

            PortClose(ttyDesc);
            PortInitial(ttyDesc);

    		return(NOK);
	    }
    
	    return(txCount);        
    }
    else
    {   
    	if ((txCount = fwrite(buffer, 1, length, ttyDesc->fId)) < 0) 
    	{
    	    PortClose(ttyDesc);
            pause(100);
            PortInitial(ttyDesc);
            pause(100);
            
    	    fflush(ttyDesc->fId);
    	    return(NOK);
	    }
	    fflush(ttyDesc->fId);
	    return(txCount);
    }
    
}

//
// 모듈:	PortRead()
//
int PortRead(TTY_DESC *tp, byte *bfptr, int count)
{
	//int i;
	//int	size,rcount;
	//int	timeOut;
	int	rxCount;
	//fd_set	readFds, writeFds, expFds;

#if 0
	FD_ZERO(&readFds);
	FD_SET(tp->id, &readFds);

	retValue = select(MAX_TTY_DEVICE, &readFds, 0, 0, &tp->timeOut);

	if (retValue < 0)	return (-1);
	if (retValue == 0)	return (0);
#endif
    
    pause(count * 5);
    
	if ((rxCount = read(tp->id, bfptr, count)) < 0)	return (-1);

	return(rxCount);

}

/***************************************************
*	FUNCTION : asyncRead()
*	: Async Port Read with TIMEOUT delay 
***************************************************/
int	asyncRead( OPR_MSG *opr, TTY_DESC	*tp,  byte *bfptr, int count)
{
	int i;
	int	size,rcount;
	int	timeOut;
	int	rxCount;
	//fd_set	readFds, writeFds, expFds;

    /* NET860-1000 외부 8530 포트 초기화 시 ... */
    if(tp->config == ASYNC_8530)
    {
        pause(count);
	    if ((rxCount = read(tp->id, bfptr, count)) < 0)	
	    {
	        return (-1);
	    }
	    return(rxCount);
    }
    else
    {
    	for(timeOut = count + 6, rcount = 0; timeOut > 0; timeOut--)
	    {
		    if(ioctl(tp->id, FIONREAD, (int) &size) == -1)
    		{
	    		printf("async: error [%d]... port=%s\n", size, tp->name);
		    	return (-1);
		    }

    		if(size > 0)
	    	{
		    	/* printf("size:%d\n", size); */
			    for(i=0; i< size; i++)
    			{
	    			read(tp->id, &bfptr[rcount], 1); 	
		    		if(opr->lineDebug == tp->chid) 
		    		{
		    		    printf("[%02x]", bfptr[rcount]);
		    		}
			    	rcount++;
				    if(rcount == count)	return (rcount);
			    }
		    }

		    usleep(2000);
	    }

	    return(rcount);
    }
    
}

//
// 모듈:	SetPortInfo()
//
void SetPortInfo(TTY_DESC *tp, PORT_INFO *ptr)
{
	char    parity[3] = {'N', 'O', 'E'};
	int     parityIdx;

	sprintf(tp->name, "%s", ptr->name);
	tp->baudRate = ptr->baudRate;
	tp->dataBits = ptr->byteSize;
	tp->stopBits = ptr->stopBit;
	parityIdx = ptr->parity > 2 ? 0 : ptr->parity;
	tp->parity = parity[parityIdx];
}


//
// 모듈:    CheckPort()
//
int harrisPortInitial(TTY_DESC *ttyDesc)
{
	byte	baudrate;

    //return (0);
    
    /* NET860-1000 외부 8530 포트 초기화 시 ... */
    if(ttyDesc->config == ASYNC_8530)
    {
        if ((ttyDesc->id = open(ttyDesc->name, O_RDWR)) < 0)
        {
            printf(">> *PORT %s Open Error \n", ttyDesc->name);
            pause(1000);
            return(NOK);
        }
    	else
	    {
		    //printf("PORT %s  id = %d initial OK\n", ttyDesc->name, ttyDesc->id);
            switch ( ttyDesc->baudRate)
            {
            case 2400 :     baudrate=SCC2400; break ;                
            case 9600 :     baudrate=SCC9600; break ;
            case 19200 :    baudrate=SCC19200 ; break ;
            case 38400 :    baudrate=SCC38400 ; break ;
            case 57600 :    baudrate=SCC57600 ; break ;
            default :
                baudrate = 0 ;
            }

            /* Comm Baudrate config... */
            if ( ioctl(ttyDesc->id, SCC_IOC_SET_BAUDRATE, &baudrate) < 0)
		    {
		        printf(">> ***PORT %s  Baudrate = %d initial Fail\n", ttyDesc->name, ttyDesc->baudRate);
		        return (0);
    		}
            
            //printf(">> PORT %s  Baudrate = %d initial OK\n", ttyDesc->name, ttyDesc->baudRate);

        }
	}
    else if(ttyDesc->config == ASYNC_860T)
    {
    	//SetPortInfo(&rfPort);
    	if(PortOpen( ttyDesc) < 0)
	    {
    		printf(">> *PORT %s initial Failed\n", ttyDesc->name);
	    	return(-1);
    	}
	    else
    	{
	    	//printf(">> PORT %s initial OK\n", ttyDesc->name);
	    }
    }	
	return(OK);
}


//
// 모듈:    CheckPort()
// 2026-04-24 오전 11:25:08 hkkim
// 정싱이면 return fd
// 실패면 < 0
//
int PortInitial(TTY_DESC *ttyDesc)
{
	byte	baudrate;

    //return (0);
    
    /* NET860-1000 외부 8530 포트 초기화 시 ... */
    if(ttyDesc->config == ASYNC_8530)
    {
        if ((ttyDesc->id = open(ttyDesc->name, O_RDWR)) < 0)
        {
            printf("ERR-%s >> %s Open Error \n",__FUNCTION__, ttyDesc->name);
            pause(1000);
            return(LIOW_NOK);
        }
    	else
	    {
		    //printf("PORT %s  id = %d initial OK\n", ttyDesc->name, ttyDesc->id);
            switch ( ttyDesc->baudRate)
            {
            case 2400 :     baudrate=SCC2400; break ;                
            case 9600 :     baudrate=SCC9600; break ;
            case 19200 :    baudrate=SCC19200 ; break ;
            case 38400 :    baudrate=SCC38400 ; break ;
            case 57600 :    baudrate=SCC57600 ; break ;
            default :
                baudrate = 0 ;
            }

            /* Comm Baudrate config... */
            if ( ioctl(ttyDesc->id, SCC_IOC_SET_BAUDRATE, &baudrate) < 0)
		    {
		        printf("ERR-%s>> %s  Baudrate = %d initial Fail\n",__FUNCTION__,ttyDesc->name, ttyDesc->baudRate);
		        return (-LIOW_NOK);
    		}
            
            //printf(">> PORT %s  Baudrate = %d initial OK\n", ttyDesc->name, ttyDesc->baudRate);

        }
	}
    else if(ttyDesc->config == ASYNC_860T)
    {
    	//SetPortInfo(&rfPort);
    	if(PortOpen( ttyDesc) < 0)
	    {
    		printf("ERR-%s>> *PORT %s initial Failed\n", __FUNCTION__,ttyDesc->name);
	    	return( LIOW_NOK);
    	}
	    else
    	{
	    	//printf(">> PORT %s initial OK\n", ttyDesc->name);
	    }
    }	
	return(ttyDesc->id);
}


//
// 모듈:	FileOpen()
//
int FileOpen(FILE_DESC *fp)
{
	if ((fp->id = fopen(fp->name, "rb+")) == NULL)
	{
		if (fp->mode == FIO_NORMAL)
			return(-2);
	
		if ((fp->id = fopen(fp->name, "wb+")) == NULL)
			return(-1);
	}
	return(0);
}

//
// 모듈:	FileRead()
//
int FileRead(FILE_DESC *fp, char *buffer, int length)
{
	int		rdLen, tally;
	char	*ptr;

	tally = 0;
	ptr = buffer;
	while (tally < length)
	{
		rdLen = ((length - tally) > FIO_BUFLEN) ? FIO_BUFLEN : length - tally;
		if (fread(ptr, rdLen, 1, fp->id) <= 0)
			return(-1);
		tally += rdLen;
		ptr += rdLen;
		if (tally < length)
			usleep(FIO_DELAY_TIME);
	}
	return(0);
}
                 
                 
//
// 모듈:	FileWrite()
//
int FileWrite(FILE_DESC *fp, char *buffer, int length)
{
	int		tally, wtLen;
	char	*ptr;

	tally = 0;
	ptr = buffer;
	while (tally < length)
	{
		wtLen = ((length - tally) > FIO_BUFLEN) ? FIO_BUFLEN : length - tally;
		if (fwrite(ptr, wtLen, 1, fp->id) != 1)
			return(-1);
		tally += wtLen;
		ptr += wtLen;
		if (tally < length)
			usleep(FIO_DELAY_TIME);
	}
	return(0);
}

//
// 모듈:	FileClose()
//
void FileClose(FILE_DESC *fp)
{
	fclose(fp->id);
	fp->id = NULL;
}


#if 0
//
// 모듈:	FileSave()
//
int FileSave(FILE_DESC *fp, char *buffer, int length)
{
	if (FileOpen(fp) < 0)
		return(-1);
	if (FileWrite(fp, buffer, length) < 0)
	{
		FileClose(fp);
		return(-2);
	}
	FileClose(fp);
	return(0);
}

//
// 모듈:	FileReadFiled()
//
int FileReadField(FILE_DESC *fp, char *buffer, int offset, int length)
{
	int		rdLen, tally;
	char	*ptr;

	if ((fp->id = fopen(fp->name, "rb+")) < 0)
		return(-1);
	if (fseek(fp->id, (long)offset, SEEK_SET) < 0)
	{
		fclose(fp->id);
		return(-2);
	}
	tally = 0;
	ptr = buffer;
	while (tally < length)
	{
		rdLen = ((length - tally) > FIO_BUFLEN) ? FIO_BUFLEN : length - tally;
		if (fread(ptr, rdLen, 1, fp->id) <= 0)
		{
			fclose(fp->id);
			return(-1);
		}
		tally += rdLen;
		ptr += rdLen;
		if (tally < length)
			usleep(FIO_DELAY_TIME);
	}
	fclose(fp->id);
	return((tally != length) ? -3 : 0);
}

//
// 모듈:	FileWriteField()
//
int FileWriteField(FILE_DESC *fp, char *buffer, int offset, int length)
{
	int		tally, wtLen;
	char	mode[8], *ptr;
	
	sprintf(mode, "%s", offset ? "rb+" : "wb");
	if ((fp->id = fopen(fp->name, mode)) == NULL)
		return(-1);
	if (fseek(fp->id, (long)offset, SEEK_SET) < 0)
	{
		fclose(fp->id);
		return(-2);
	}
	tally = 0;
	ptr = buffer;
	while (tally < length)
	{
		wtLen = ((length - tally) > FIO_BUFLEN) ? FIO_BUFLEN : length - tally;
		if (fwrite(ptr, wtLen, 1, fp->id) != 1)
		{
			fclose(fp->id);
			return(-3);
		}
		tally += wtLen;
		ptr += wtLen;
		if (tally < length)
			usleep(FIO_DELAY_TIME);
	}
	fclose(fp->id);
	return(0);
}

//
// 모듈:	FileSaveDirect()
//
int FileSaveDirect(FILE_DESC *fp, char *buffer, int length)
{
	if ((fp->id = fopen(fp->name, "wb+")) == NULL)
		return(-1);
	if (fwrite(buffer, length, 1, fp->id) < 0)
		return(-1);
	fclose(fp->id);
	return(0);
}

//
// 모듈:	GetFileStat()
//
int GetFileStat(char *fileName, struct stat *state)
{
	memset((char *)state, 0, sizeof (struct stat));
	return(stat(fileName, state));
}

#endif

/* ============================================================ */
/*  사용자 정의 Library Function ...                            */
/* ============================================================ */
//
// 모듈:	dbFileWrite()
//
int dbFileWrite_ESIO(RTU_DATABASE *localDB)
{
	//char	*buffer;                   
	int     dbsize;
	word	romsum;

#ifdef	SLAVE_BOARD_TEST
    FILE_DESC	romdbFile1 = {NULL, FIO_NORMAL, ROMDB_STRING2 };
#else
    FILE_DESC	romdbFile1 = {NULL, FIO_NORMAL, ROMDB_STRING };
#endif
	    
    
    /* 데이터베이스 저장 ... */
    romsum = gensum((byte *)localDB, sizeof(RTU_DATABASE) - 2);
    localDB->chksum[0] = romsum;
    localDB->chksum[1] = romsum >> 8;
    
	dbsize = sizeof(RTU_DATABASE);
	
	/* Database File #1 Open */
	if (FileOpen(&romdbFile1) < 0)
	{
		printf("[WR] %s Database File Open Error...!\n", romdbFile1.name);
		//romDBFail();
		/* Database File Create.... */
		//FileOpen(&romdbFile2);
		return (-1);
	}
	
    printf("[WR] %s Database File Open OK...(size=%d) !\n", romdbFile1.name, dbsize);   

    /* 내부변수의 내용을 Database화일로 Write 한다. */
    if(FileWrite(&romdbFile1, (char *) localDB, dbsize) < 0)
    {         
		return (-1);        
    }

    FileClose(&romdbFile1);  
    
    return (1);
    
}


//
// 모듈:	dbFileRead()
//
int dbFileRead_ESIO(RTU_DATABASE *localDB)
{
	char	*buffer;                   
	int     dbsize;

#ifdef	SLAVE_BOARD_TEST
    FILE_DESC	romdbFile1 = {NULL, FIO_CREATE, ROMDB_STRING2};
#else
	FILE_DESC	romdbFile1 = {NULL, FIO_CREATE, ROMDB_STRING};
#endif

	dbsize = sizeof(RTU_DATABASE);
    buffer = (char *) localDB;	

    /* Database File #1 Read ... */	
	if (FileOpen(&romdbFile1) < 0)
	{
		printf("[DB] %s Database File Open Error...!\n", romdbFile1.name);
		return (-1);
	}
	
    //printf("[DB] %s Database File Open OK...!\n", romdbFile1.name);

    /* Database 화일의 내용을 Read 하여 내부 변수로 지정함 */                     
    if(FileRead(&romdbFile1, buffer, dbsize) < 0)
    {         
        //romDBFail();
		return (-1);        
    }

	FileClose(&romdbFile1); 
	
	return (1);
	
}


/*
* ====================================================
*   MPU 용 History-Queue
* ====================================================
*/
void logEvent_MPU(SHM_MEMORY *pShm, int logid, int ioid, int point, int state, int hostid, struct timeval *soeTime)
{
    word    front;
    
    word        chksum;
    SYSLOG_FORM *log;
    OPR_MSG     *opr;
    HISTORY_QUE *shmHque;
    
    opr     = (OPR_MSG *) &pShm->opr_msg;
    shmHque = (HISTORY_QUE *) &pShm->localHistoryQ; 

    /* -------------------------------- */
	/* (1) HISTORY-QUEUE 이벤트 로깅    */    
	/* -------------------------------- */
    front = shmHque->front & HISTORY_QUE_MASK;
    log = (SYSLOG_FORM *) &shmHque->queue[front];        
    
    if(opr->eventLogging == SET)    pause(100);
    opr->eventLogging = SET;
    
    /* -------------------------------- */
    /* 로그 시간정보 추출               */
    /* -------------------------------- */
    gettimeofday(&log->logTime, NULL);
    
    //if(logid == ENT_SOE)    memcpy( (byte *) &log->soeTime, (byte *) soeTime, sizeof(struct timeval));          // SOE 생성 시각정보
    //else                    memcpy( (byte *) &log->soeTime, (byte *) &log->logTime, sizeof(struct timeval));     
   
    if(logid == ENT_SOE)
    {
    	if(soeTime != NULL)    memcpy( (byte *) &log->soeTime, (byte *) soeTime, sizeof(struct timeval));
   	}
            
    log->logid  = logid;        // 이벤트 코드 구분 : SOE/Control/System ...
    log->ioid   = ioid;         // 대상 계전기 번호 [1...112]
    log->point  = point;        // 대상 포인트 번호 [1...1024]
    log->state  = state;        // 이벤트/제어 상태
    log->hostid = hostid;       // 발생주체, 0: MPU, 1 ~ 8 : HOST

    front = (front + 1) & HISTORY_QUE_MASK;
    shmHque->front = front & HISTORY_QUE_MASK;
    
    if(shmHque->front == 0)    shmHque->overlab = 1;

    /* Calc... Chksum */
    chksum = gensum((byte *) shmHque, sizeof(HISTORY_QUE) - 2);
    shmHque->chksum = chksum;
    
    opr->eventLogging = RESET;

}


/*
*   CPU(8248) SRAM/MRAM Access 
*    - buffer clear[zero]
*/
void bzero8248(byte *buf, int size)
{
    for(; size > 0; size--,buf++) 
    {
        *buf = 0;
    }
}


/*
*   CPU(8248) SRAM/MRAM Access 
*    - memcpy( des, src, size)
*/
void memcpy8248(volatile byte *des, volatile byte *src, int size)
{
    int i;
    
    for(i=0; i < size; i++)
    {
        des[i] = (byte ) src[i];
    }
}


//
// 모듈:	FileOpen()
//
int logFileOpen(FILE_DESC *fp)
{
	if ((fp->id = fopen(fp->name, "a+")) == NULL)
	{
	    printf("*[%s] log File Open Error...!\n", fp->name);

        if ((fp->id = fopen(fp->name, "ab+")) == NULL)
	    return(-1);
	}
	//printf("*[%s] log File Open OK...!\n", fp->name);
	return(0);
}

//
// 모듈:	FileClose()
//
void logFileClose(FILE_DESC *fp)
{
	fclose(fp->id);
	fp->id = NULL;
	
	//printf("*[%s] log File Close...!\n", fp->name);
}
