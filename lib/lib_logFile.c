#include    "localLib.h"
#include    "eventmsg.h"

#define	ESIO_LOG_HOME	"/mnt/debug"
#define	ESIO_LOG_NAME	"esio.log"
#define	ESIO_LOG_SIZE	409600	// 400 kbytes
// logging file이 filesize 보다 크면, 백업을 만든다.

int  LogFile_MPU (SHM_MEMORY *shmPtr, int code, char *outstr, int osize)
{
    int     pos;
    char    buffer[256];
	char    fname[64], fnameOld[64], *fopen_type;
	struct stat stat_buf;
	FILE *fp;

    struct timeval  logT;
    struct tm *cTime;
    int 	milisec;
    int		readyTick=0;

	OPR_MSG         *opr    = NULL;
    
    opr     = (OPR_MSG *)   &shmPtr->opr_msg;

#ifdef	LOGFILE_DISABLE
	return (0);
#endif    
    
    /* ------------------------------------------------ */
	/*	*LOG 화일 WRITE 대기 (저장중 대기)						*/    	
	/* ------------------------------------------------ */
	if(opr->logFileWrite == SET)	
	{
		while(opr->logFileWrite)
		{
			pause(100);
			if(++readyTick > 5)
			{
				printf("log-file> *** LOG-Write Timeout...RETURN !\n");
				opr->logFileWrite = RESET;
				return (0);	
			}
		}	
	}

	/* LOG화일 저장중 Flag */
	opr->logFileWrite = SET;
	
    //bzero(buffer, sizeof(buffer));
    
    /* SOE 이벤트 시간 추출 */
    gettimeofday(&logT, NULL);
    cTime = localtime(&logT.tv_sec);
    milisec = logT.tv_usec / 1000;
    
    sprintf(buffer, "%4d-%02d-%2d %02d:%02d:%02d.%03d  ", 
        cTime->tm_year + 1900, cTime->tm_mon +1, cTime->tm_mday, cTime->tm_hour, cTime->tm_min, cTime->tm_sec, milisec);
  
    pos = strlen(buffer);
    sprintf(&buffer[pos], "%s  %s \n", errmsg[code], outstr);

	/* LOG-FILE 관련 */
	sprintf (fname, "%s/%s", ESIO_LOG_HOME, ESIO_LOG_NAME);
	
	/* check filesize */
	if (stat (fname, &stat_buf) >= 0)
    {
		if (stat_buf.st_size > ESIO_LOG_SIZE)
		{
			sprintf (fnameOld, "%s.old", fname);
			if (rename (fname, fnameOld) < 0)
			{
				printf ("file : rename (%s --> %s) error, %d : %s\n",
						fname, fnameOld, errno, strerror(errno));
			}
			fopen_type = "w";
		}
		else
			fopen_type = "a+";
	}
	else
	{
        //printf ("file : stat (%s) error, %d : %s\n",
        //        fname, errno, strerror(errno));
		fopen_type = "w";
    }

	if ((fp = fopen (fname, fopen_type)) == NULL)
	{
		printf ("log-file> *** file open (%s,%s) error,  %d : %s\n", fname, fopen_type, errno, strerror(errno));
		
		/* LOG화일 저장중 Flag */
		opr->logFileWrite = RESET;
		return (-1);
	}
	
	fprintf (fp, "%s", buffer);
	fclose(fp);

	/* LOG화일 저장중 Flag */
	opr->logFileWrite = RESET;
		
	return (1);
}
