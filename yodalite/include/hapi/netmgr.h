#ifndef _NETMGR_H_
#define _NETMGR_H_

#define NETWORK_STATUS_NAME      "network.status"
#define NETWORK_COMMAND_NAME     "network.command"
#define FLORA_AGET_NET_MANAGER_URI  "unix:/var/run/flora.sock#net_manager"

extern int hapi_netmgr_exit(void);
extern int hapi_netmgr_init(void);

#endif
