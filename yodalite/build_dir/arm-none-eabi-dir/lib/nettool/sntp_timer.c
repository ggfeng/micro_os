#include <yodalite_autoconf.h>

#if CONFIG_SNTP_TIMER == 1 
#ifndef CONFIG_LIBC_ENABLE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#else
#include <lib/libc/yodalite_libc.h>
#endif

#include "osal/time.h"
#include "net/sntp/sntp.h"
#include "lib/nettool/sntp_timer.h"

static unsigned long g_sys_time_base = 0;
static unsigned int g_sys_time_flag = 0;

static sntp_time*get_current_time(void)
{
  if (sntp_request(NULL) != 0) {
     printf("error:request time fail\n");
	 return NULL;
  }

  return (sntp_time *)sntp_obtain_time();
}

static unsigned long mktime (unsigned int year, unsigned int mon,unsigned int day, unsigned int hour,unsigned int min, unsigned int sec)
{
  if (0 >=(int)(mon-=2)){
     mon += 12;
     year -=1;
  }

  return((((unsigned long)(year/4-year/100+year/400+367*mon/12+day)+year*365-719499)*24+hour)*60+min)*60+sec;
}

static int get_sys_time_base(unsigned long *base)
{
   sntp_time * time; 
   unsigned long seconds;
   struct timespec sys_time;

   if((time = get_current_time())== NULL){
        printf("get_current_time fail\n");
		return -1;
   }

  printf("sntp(%u  %u  %u ",time->year, time->mon, time->day);
  printf("%u:%u:%u %u)\n", time->hour, time->min, time->sec, time->week);

   seconds = mktime(2000+time->year,time->mon,time->day,time->hour,time->min,time->sec);

   if (clock_gettime(CLOCK_REALTIME, &sys_time) == -1) {
        printf("clock_gettime fail\n");
        return -1;
   }

   printf("total seconds:%ld\n",seconds);

   seconds -= sys_time.tv_sec;

   *base = seconds;

   printf("total seconds:%ld,sys_time.sec:%ld\n",seconds,sys_time.tv_sec);

   return 0;
}


int yodalite_gettimeofday(struct timeval *tv,void *tz){

	unsigned long base;
    struct timespec sys_time;

     if(g_sys_time_flag == 0){
	   if(get_sys_time_base(&base) == 0){
	       g_sys_time_base  = base;
           g_sys_time_flag  = 1;
	   }else{
	    printf("error:get_sys_time_base\n");
		return -1;
	   }
	   printf("sys_time_base:%ld\n",g_sys_time_base);
   }

   if (clock_gettime(CLOCK_REALTIME, &sys_time) == -1) {
        printf("clock_gettime fail\n");
        return -1;
   }

   tv->tv_sec = g_sys_time_base + sys_time.tv_sec; 
   tv->tv_usec = sys_time.tv_nsec /1000; 

   printf("tv_sec:%ld,tv_usec:%ld\n",tv->tv_sec,tv->tv_usec);
   return 0;
}
#endif
