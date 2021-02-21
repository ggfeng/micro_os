#include <stdio.h>

#ifdef YODALITE
#include <osal/pthread.h>
#else
#include <pthread.h>
#endif

#include "activation.h"

#include "network.h"


// extern int network_status = 0;
// extern pthread_mutex_t _network_mutex;
// extern pthread_cond_t  _network_cond;

int activation_init(void)
{
    network_int();
    //player_init();
   // dispatcher_init();
    return 0;
}
