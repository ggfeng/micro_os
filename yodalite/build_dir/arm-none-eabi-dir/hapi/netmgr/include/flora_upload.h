#ifndef __FLORA_EVENT_H
#define __FLORA_EVENT_H
#ifdef __cplusplus
extern "C" {
#endif

#include  <hapi/flora_agent.h>

extern int  network_flora_init(void); 
extern void network_flora_exit(void);
extern void network_flora_send_msg(uint8_t *buf);
extern void network_flora_return_msg(flora_call_reply_t reply, uint8_t *buf);

#ifdef __cplusplus
}
#endif
#endif
