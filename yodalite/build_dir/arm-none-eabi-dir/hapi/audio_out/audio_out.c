#include "flora_player.h"
#include "audio_out.h"
static void play_project()
{
   flora_player_init();
   audio_player_init();
}

int audio_out_init(void)
{
   play_project();
   return 0;
}


