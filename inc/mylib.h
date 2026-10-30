
#ifndef _MY_LIB_H_
#define  _MY_LIB_H_


#ifdef TK_EXTERN
#undef TK_EXTERN 
#endif


#ifdef _LIB_MYLIB_C_
#define TK_EXTERN 
#else
#define TK_EXTERN extern
#endif 





/***********************************
  utils 
************************************/


TK_EXTERN  void sleep_ms(long ms) ;


/***********************************
 network
************************************/


TK_EXTERN int validate_ip(const char *str) ;
TK_EXTERN int check_already_running(const char *app_name) ;


/*
time 

*/

TK_EXTERN char *sprt_timeval (char *out, int outSize, struct timeval *pTime);

#endif 