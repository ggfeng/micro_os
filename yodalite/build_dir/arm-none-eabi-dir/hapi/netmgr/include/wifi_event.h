

#ifndef __WIFI_EVENT_H
#define __WIFI_EVENT_H
#include <stdint.h>
#include "network_report.h"
#include <hapi/flora_agent.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_FAILED_TIMES 3

typedef struct 
{
  int flag ;
  uint32_t result;
  uint32_t reason;
}tRESULT;

extern int wifi_start_monitor(void);
extern void wifi_stop_monitor(void);
extern int wifi_respond_status(flora_call_reply_t reply);
#ifdef __cplusplus
}
#endif
#endif
