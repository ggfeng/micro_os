#ifndef __LUMENEFFECTS_H__
#define __LUMENEFFECTS_H__

#include <hapi/lumenlight.h>
typedef unsigned char u8;

#define MAX_LED_NUM 100

#define FUNC_IDLE 0

#define ENDDEGREE_NOW -1
#define ENDDEGREE_WAIT -2

struct LumenEffects {
    void (*InitLayers)(int speed);
    bool (*LayerAddBackground)(u8* color);
    bool (*LayerAddGroup)(bool dir,u8 poi,u8 len,u8* color);
    bool (*LayerAddGroupPoint)(bool dir,u8 poi,u8 len,u8* color);
    bool (*LayerAddPoint)(u8 led_num,u8* color);
    void (*DelLayers)(void);

    bool (*LayersSetSpeed)(int speed);
    bool (*LayersSetLight)(u8 light);
    bool (*LayersShow)(void);

    bool (*EffectFadeIn)(bool dir,u8 poi,u8 len,u8* color,int speed);
    bool (*EffectFadeOut)(bool dir,u8 poi,u8 len,u8* color,int speed);
    bool (*EffectPoint)(int degree);
    bool (*EffectPoint1)(int degree,u8* color_point,u8 len,u8* color_len,int speed);
    bool (*EffectStartRound)(void);
    bool (*EffectStartRound1)(int start_degree);
    bool (*EffectStartRound2)(int start_degree,int end_degree,bool dir,u8 len,u8* color,int speed,int acc_spd,int max_spd);
    bool (*EffectEndRound)(void);
    bool (*EffectEndRound1)(int end_degree);
    bool (*EffectStop)(void);
};

extern struct LumenEffects* newLumenEffects(void);
extern int destroyLumenEffects(struct LumenEffects* plumeneffects);

#endif
