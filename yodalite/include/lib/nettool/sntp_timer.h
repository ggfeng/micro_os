#ifndef _SNTP_TIMER_H_
#define _SNTP_TIMER_H_

#ifndef timeval
struct timeval
{
  unsigned long tv_sec;
  unsigned long tv_usec

};
#endif

extern int yodalite_gettimeofday(struct timeval *tv,void *tz);

#undef  gettimeofday
#define gettimeofday yodalite_gettimeofday

#endif

