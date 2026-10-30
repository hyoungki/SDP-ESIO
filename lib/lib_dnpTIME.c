

#include    "localLib.h"




/* ============================================================ */
/*  DNP-TIME 관련 Function                                      */
/* ============================================================ */

/*
    convert Byte ordering ... 
    DNP little 로 왔다. 즉  leaset 오고 most오낟  dnptime td 와 반드 
    
*/
void convertRTC(byte *src, byte *des)
{
// hkkim
#if 0 // PPC    
    src[0] = des[5];
    src[1] = des[4];
    src[2] = des[3];
    src[3] = des[2];
    src[4] = des[1];
    src[5] = des[0];

#else // ARM
    src[5] = des[5];
    src[4] = des[4];
    src[3] = des[3];
    src[2] = des[2];
    src[1] = des[1];
    src[0] = des[0];    
#endif

}


word dayOfCurrent( int year, int month, int day)
{
    word curday;
#if 0    
  return(((pDateTime->month * 275) / 9) -
         (((pDateTime->month + 9) / 12) * (1 + (((pDateTime->year%4) + 2) / 3))) +
         pDateTime->day - 30);
#endif         

    curday = (((month * 275) / 9) - (((month + 9) / 12) * (1 + (((year%4) + 2) / 3))) +  day - 30);
    
    return (curday);         
}

/*****************************************************************************/
/* Triangle MicroWorks, Inc.                              Copyright (c) 1997 */
/*****************************************************************************/
/*                                                                           */
/* FUNCTION: pdpdtime_dateTimeToMSSince70() - converts date/time structure   */
/*                                            to 6 byte count of ms since    */
/*                                            Jan 1, 1970                    */
/*                                                                           */
/* CAVEAT:  This function is only valid for dates on or after 1/1/70, and    */
/*          since this function assumes every fourth year is a leap year, it */
/*          is only valid through 2/28/2100.                                 */
/* 현재의 RTC 정보를 DNP 형태의 시간정보(6BYTE) 로 변경한다.                */
/*****************************************************************************/

void     makeDnpTimeByte(                  /* converts date/time structure to */
                                          /* 6 byte count of ms since Jan 1, */
                                          /* 1970                            */
        PDPDTIME_MS_SINCE_70 *pMsSince70, /* destination to put ms since     */
                                          /* 1970                            */
        PDPDTIME_DATE_TIME   *pDateTime)  /* source date/time structure to   */
                                          /* convert                         */
{
    word    curday;
  unsigned long mostSignificant;
  unsigned long leastSignificant;

  /* first calculate the number of days in the years from 1/1/70 to the year */
  /* preceeding the current one, assuming every fourth year was a leap year  */
  /* (valid from 1970-2099)                                                  */

  mostSignificant = ((pDateTime->year - 1970) * 365) +
                    ((pDateTime->year - 1969)/4);

  /* now add in the days in the current year up to, but not including the    */
  /* current day.                                                            */

  curday = dayOfCurrent(pDateTime->year,pDateTime->month,pDateTime->day);
  
  /*mostSignificant += dtimeComputeDayOfYear(pDateTime) - 1;*/
  mostSignificant += curday - 1;

  /* now multiply by 24 to put this in units of hours and add in the number  */
  /* of hours in the current day up to, but not including the current hour.  */

  mostSignificant = (mostSignificant*24) + pDateTime->hour;

  /* now multiply by 60 to put this in units of minutes and add in the       */
  /* number of minutes in the current hour up to, but not including the      */
  /* current minute.                                                         */

  mostSignificant = (mostSignificant*60) + pDateTime->minute;

  /* mostSignificant now holds the number of minutes since 1/1/70.  We will  */
  /* compute the seconds and milliseconds in the current second into a       */
  /* separate leastSignificant unsigned short.  Since its maximum value is   */
  /* 60,000 which is close to 64K, the math is easier to convert to the 6    */
  /* byte DNP format.                                                        */

  leastSignificant = (pDateTime->second * 1000) + pDateTime->millisecond;

  leastSignificant += (mostSignificant & 0xffff)*60000;
  mostSignificant   = ((mostSignificant >> 16)*60000) + (leastSignificant >> 16);

  pMsSince70->leastSignificant = (word) leastSignificant;
  pMsSince70->mostSignificant  = mostSignificant;
}

/*****************************************************************************/
/* Triangle MicroWorks, Inc.                              Copyright (c) 1997 */
/*****************************************************************************/
/*                                                                           */
/* FUNCTION: pdpdtime_msSince70ToDateTime() - converts ms count since 1/1/70 */
/*                                            to date/time structure         */
/*                                                                           */
/* CAVEAT:  This function is only valid for dates on or after 1/1/70, and    */
/*          since this function assumes every fourth year is a leap year, it */
/*          is only valid through 2/28/2100.                                 */
/* DNP 형태의 시간정보(6BYTE) 를 RTC 형태의 데이터로 변경한다.               */
/*****************************************************************************/

void makeDnpTimeRTC(                         /* converts ms count since 1/1/70 */
                                           /* to date/time structure         */
        PDPDTIME_DATE_TIME   *pDateTime,   /* destination to put date/time   */
                                           /* structure                      */
        PDPDTIME_MS_SINCE_70 *pMsSince70)  /* source to convert              */
{
  /* We need to do a bunch of divisions and modulos, but we can't            */
  /* necessarily divide a 6 byte number, so first we need to convert the     */
  /* structure into units we can handle.  Specifically we will convert it    */
  /* into two variables: a least significant number that hold the number of  */
  /* seconds in the current minute, and a most siginficant variable that     */
  /* holds the number of minutes since 1/1/70.  (Note this is the reverse    */
  /* process of the conversion in the function above.)                       */
    word    curday;
  unsigned long temp_value;
  unsigned long mostSignificant;
  unsigned long leastSignificant;

  temp_value       = (pMsSince70->mostSignificant%60000)<<16;
  mostSignificant  = ((pMsSince70->mostSignificant/60000)<<16) +
                     ((temp_value + pMsSince70->leastSignificant)/60000);
  leastSignificant =  (temp_value + pMsSince70->leastSignificant)%60000;

  pDateTime->millisecond = (word) (leastSignificant % 1000);
  pDateTime->second      = (byte)  (leastSignificant / 1000);
  pDateTime->minute      = (byte)  (mostSignificant  % 60);
  mostSignificant        =          mostSignificant  / 60;  /* now holds hours */
  pDateTime->hour        = (byte)  (mostSignificant  % 24);
  mostSignificant        =          mostSignificant  / 24;  /* now holds days */

  /* Take a guess at years, but this number could be too large because of */
  /* leap years */
  pDateTime->year        = (word) ((mostSignificant / 365) + 1970);

  while(1) /* Continue adjusting the guessed year until we get it right. For */
           /* dates within the next 365*4=1460 years (the number of years it */
           /* will take to generate a year's worth of leap days), this       */
           /* should only take one iteration.                                */
  {
    /* now calculate the number of days in the years from 1/1/70 to the year */
    /* preceeding the guessed one, assuming every fourth year was a leap     */
    /* year (valid from 1970-2099)                                           */

    temp_value = ((((unsigned long) pDateTime->year) - 1970) * 365) +
                 ((((unsigned long) pDateTime->year) - 1969)/4);

    /* If this computed number of days based on the year guess is greater    */
    /* than the actual number of days (in mostSignificant), then our guess   */
    /* was wrong.  Several examples: 1/1/70 is day 0; 12/31/70 is day 364;   */
    /* 12/31/71 is day 729 = 365 (for 1970) + 364 (12/31 is day 364 of       */
    /* 1971); 12/31/72 is day 1095 = 365 (for 1970) + 365 (for 1971) + 365   */
    /* (1972 is a leap year with 366 days, so 12/31 is day 365 of 1972).     */
    /* The guess year for 12/31/72 would be 1973 (because 1095/365 = 3), but */
    /* the computed number of days (in temp_value) for this guess would be   */
    /* 1096, which is greater than the actual number of days.                */
    if (temp_value > mostSignificant)
    {
      pDateTime->year--;
    }
    else
    {
      break;  /* no more year adjustment is necessary; we guessed correctly */
    }
  }

  /* Now subtract all the days up until the computed year, leaving the       */
  /* number of days in the computed year. Add one to the count of days       */
  /* within the year because dtimeComputeDayOfYear() will compute Jan 1 as   */
  /* day 1, not day 0.                                                       */
  mostSignificant = mostSignificant  - temp_value + 1; /* now contains days  */
                                                       /* since Jan 1.       */

  /* As we did for years, take a guess at the month and day of month, and    */
  /* then correct it by subtracting days until the computed day of year is   */
  /* the same as the actual day of the year.                                 */

  pDateTime->month = (byte) (mostSignificant/28 + 1);
  pDateTime->day   = (byte) (mostSignificant%28 + 1);

  curday = dayOfCurrent(pDateTime->year,pDateTime->month,pDateTime->day);
  
  while(curday != mostSignificant)
  {
    pDateTime->day--;
    if (pDateTime->day == 0)
    {
      pDateTime->month--;
      pDateTime->day = 31;
    }
    curday = dayOfCurrent(pDateTime->year,pDateTime->month,pDateTime->day);
  }
}
