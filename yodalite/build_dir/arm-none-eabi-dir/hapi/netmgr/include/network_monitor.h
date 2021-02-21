#ifndef _NETWORK_MONITOR_
#define _NETWORK_MONITOR_

#include "network_report.h"

extern void monitor_report_status(void);
extern int monitor_get_status(tNETWORK_STATUS *status);
extern int monitor_respond_status(flora_call_reply_t reply);
extern int network_start_monitor(void);
extern void network_stop_monitor(void);
#endif
