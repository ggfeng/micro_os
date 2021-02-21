#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "hardware/platform.h"
#include "hardware/pal_asound.h"
int player_destory(int fd);
void play_local(int src_type);
int player_pause(int fd);
int player_resume(int fd);
int player_stop(int fd);
int player_rerun(int fd);
int play_url_init(char * url);
int  play_pcm_data(int pcm_t);
int play_local_init(int src_type);

int player_init(void);



#endif
