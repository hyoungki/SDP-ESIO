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

/***************************************************
*	FUNCTION : asyncRead()
*	: Async Port Read with TIMEOUT delay 
***************************************************/
int	netReadLine( register int fd,  byte *bfptr, int count)
{
	int i;
	int	size,rcount;
	int	timeOut;

	for(timeOut = count + 2, rcount = 0; timeOut > 0; timeOut--)
	{
		if(ioctl(fd, FIONREAD, (int) &size) == -1)
		{
			printf("net> async: error [%d]\n", size);
			return (0);
		}

		if(size > 0)
		{
			/* printf("size:%d\n", size); */
			for(i=0; i< size; i++)
			{
				read(fd, &bfptr[rcount], 1); 	
				//if(opr.lineDebug) printf("[%2x]", bfptr[rcount]);
				rcount++;
				if(rcount == count)	return (rcount);
			}
		}

		usleep(10);
	}

	return(rcount);

}


/*******************************************************************************
 *
 * 모듈명:  _readLine()
 *
 ******************************************************************************/

int _readLine(register int fd, register char *ptr, register int maxlen)
{
	int n, rc;
	char c;

	for(n=1;n<maxlen;n++)
	{
		if((rc = read(fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			//printf("[%d]", rc);
		}
		else if(rc == 0)
		{
		    //printf("[%d]", rc);
			if(n == 1) return(0);
			else break;
		}
		else 
		{
		    //printf("[%d]", rc);
		    return(-1);
        }		    
		
	}
	
	*ptr = 0;
	return(n-1);
}

/*******************************************************************************
 *
 * 모듈명:  _writen()
 *
 ******************************************************************************/

int _writen(register int fd, register char *ptr, register int nbytes)
{
	int nleft, nwritten;

	nleft = nbytes;
	while(nleft > 0)
	{
		nwritten = write(fd, ptr, nleft);
		if(nwritten <= 0) return(nwritten);
		nleft -= nwritten;
		ptr += nwritten;
	}
	return(nbytes-nleft);
}


/* write */
/*
 timeout is mS Unit
 
 n 만큼 읽을 수도 있고
 0 연결이 끊어 질 수도 있고
 -1 timout
 -2 Eror가 발생
 
*/
int TKwrite(int fd, byte *buf, int size , int tm)
{
  int retval;
  fd_set  rset;      
  struct timeval timeout;
  FD_ZERO(&rset);
  FD_SET(fd,&rset);  

   //debug("%s\n\r",__FUNCTION__); 
  
  if (tm)
  {

    if ( tm < 1000)
    {
        timeout.tv_usec = tm*1000 ;
        timeout.tv_sec =  0 ;
    }
    else
    {
        timeout.tv_usec = 0;
        timeout.tv_sec =  tm/1000 ;          
    }
    
    retval =select(fd+1,NULL,&rset,NULL,&timeout);  
    
  }
  else   /* wait for ever */
  retval =select(fd+1,&rset,NULL,NULL,NULL);
  
  //debug("TKwrite %d-%d ms\n",retval, tm);
  
  if ( retval == ERROR) 
  {
        perror("write select");
        
        return -2;
   }
  else if ( retval == 0 ) 
  {
  	 perror("write imeout");
  	 return -1 ; /* timeout */
  }
 
  if ( FD_ISSET(fd, &rset))
  {
        /* TPC retval=0 means End-of-Connetion */
        retval = write(fd, buf, size);
        return retval ; 
  }
  else return -2; /* ???? */
    
     
} 

/***********************************************
*     block인 경우는 1회 write로 성공한다.
*	 nonblock인 경우 확인 요 
* 
*   return  n > 0 byte to be written
*           n -1  write eror such as PIPE BROKEN
************************************************/

int  tkWriteTCP(int sockfd, byte *txBuffer, int size,int timeout)
{
    int retval ;
	int totalSize ;
	int remains;
	byte *txData;
		
	totalSize = 0;
	remains = size;
		
	txData = txBuffer ;
	
    while ( remains > 0)
	{
  		//if (( retval = TKwrite(sockfd, txData, remains,timeout)) < 0)	  
	    if(( retval = write(sockfd, txData, remains)) < 0)
	  	{
            printf("tkWriteTCP %s\r\n",strerror(errno)); 	
			return ERROR;	  				
	  	}
	  		
	  	remains -= retval;
	  	totalSize += retval;
	  	txData +=retval;	  			  		  		  	
    }
	  
	return totalSize;
}


/*
 timeout is mS Unit
 
 n 만큼 읽을 수도 있고
 0 연결이 끊어 질 수도 있고
 -1 timout
 -2 Eror가 발생
 
*/
int TKread(int fd, char *buf, int size , int tm)
{
    int retval;
    fd_set  rset;      
    struct timeval timeout;

    FD_ZERO(&rset);
    FD_SET(fd,&rset);  

    //debug("%s\n\r",__FUNCTION__); 
  
    if (tm)
    {
        if ( tm < 1000)
        {
            timeout.tv_usec = tm*1000 ;
            timeout.tv_sec =  0 ;
        }
        else
        {
            timeout.tv_usec = 0;
            timeout.tv_sec =  tm/1000 ;          
        }
        retval =select(fd+1,&rset,NULL,NULL,&timeout);  
    }
    else   /* wait for ever */
        retval =select(fd+1,&rset,NULL,NULL,NULL);
  
    if ( retval == ERROR) 
    {
        if ( errno == EINTR)
			return -1;
	    else 
            printf("sim> read select %s\r\n",strerror(errno)); 			
        return -2;
    }
    else if ( retval == 0 ) 
    {
  //	 perror("read timeout");
  	    return -1 ; /* timeout */
    }
 
    if ( FD_ISSET(fd, &rset))
    {
        /* TPC retval=0 means End-of-Connetion */
        retval = read(fd, buf, size);
        return retval ; 
    }
    else return -2; /* ???? */
     
}
 
 /* from serial_test.cpp*/
/* 2008-03-10 오후 7:02:26 update for retval =0*/
/* read can be smaller than requested */
/******************************************************
*
* return  n > 0 read byte
*         n = 0 close socket
*         n = -1 timeout     or   EINTR
*         n = -2 system error  except EINTR
* to-do 사실은 TKread 마다 timeout을 제외해야 된다.
*******************************************************/

int TKreadn (int fd, char *buf, int size , int timeout)
{
  int totalSize;
  char *ptr = buf;
  int retval ; 
  
  totalSize = 0 ;
  
  while ( totalSize < size)
  {
     retval = TKread(fd, ptr,size,timeout);

     if ( retval <0 ) return retval ;    /* timeout or error */ 
     if ( retval ==0) return totalSize ;          
     totalSize +=retval;
     ptr +=retval ;
     
     if ( totalSize >= size) return totalSize ;  
        
  }
        
  return totalSize;
}

/* End of netFunc.c */
