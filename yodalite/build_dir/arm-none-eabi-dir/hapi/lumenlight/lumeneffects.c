#include <yodalite_autoconf.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include<osal/pthread.h>
#include <hapi/lumeneffects.h>


#define LUMENEFFECT_LOG  printf
#define yodalite_malloc malloc
#define yodalite_free free


static struct LumenLight *_lumenlight;
static u8 led_data[3*MAX_LED_NUM];
volatile int _speed;
volatile int _flag_stop;
volatile int _end_degree;

static pthread_t _lumenthread;
static pthread_mutex_t _lumenmutex;
static int ref_count = 0;

struct fade_param {
    bool dir;
    u8 poi;
    u8 len;
    u8* color;
    int speed;
};
static struct fade_param effect_fadein, effect_fadeout;

struct point_param {
    int degree;
    u8* color_point;
    u8 len;
    u8* color_len;
    int speed;
};
static struct point_param effect_point;

struct round_param {
    int start_degree;
    int end_degree;
    bool dir;
    u8 len;
    u8* color;
    int speed;
    int acc_spd;
    int max_spd;
};
static struct round_param effect_round;

static void InitLayers(int speed)
{
    _speed = speed;
    _lumenlight->lumen_set_enable(true);
}

static bool LayerAddBackground(u8* color)
{
    int i;
    for (i = 0; i < _lumenlight->getLedCount(); i++){
        led_data[i*3] = color[0];
        led_data[i*3+1] = color[1];
        led_data[i*3+2] = color[2];
    }
    return true;
}

static bool LayerAddGroup(bool dir,u8 poi,u8 len,u8* color)
{
    int i;
    if(dir){
        for(i = 0; i < len; i++){
            led_data[(poi+i)%12*3] = color[i*3];
            led_data[(poi+i)%12*3+1] = color[i*3+1];
            led_data[(poi+i)%12*3+2] = color[i*3+2];
        }
    }
    else{
        for(i = 0; i < len; i++){
            led_data[(poi-i+12)%12*3] = color[i*3];
            led_data[(poi-i+12)%12*3+1] = color[i*3+1];
            led_data[(poi-i+12)%12*3+2] = color[i*3+2];
        }
    }

    return true;
}

static bool LayerAddGroupPoint(bool dir,u8 poi,u8 len,u8* color)
{
    int i;
    if(dir){
        for(i = 0; i < len; i++){
            led_data[(poi+i)%12*3] = color[0];
            led_data[(poi+i)%12*3+1] = color[1];
            led_data[(poi+i)%12*3+2] = color[2];
        }
    }
    else{
        for(i = 0; i < len; i++){
            led_data[(poi-i+12)%12*3] = color[0];
            led_data[(poi-i+12)%12*3+1] = color[1];
            led_data[(poi-i+12)%12*3+2] = color[2];
        }
    }

    return true;
}

static bool LayerAddPoint(u8 led_num,u8* color)
{
    led_data[led_num*3] = color[0];
    led_data[led_num*3+1] = color[1];
    led_data[led_num*3+2] = color[2];

    return true;
}

static void DelLayers(void)
{
    _lumenlight->lumen_set_enable(false);
}

static bool LayersSetSpeed(int speed)
{
    _speed = speed;
}

static bool LayersSetLight(u8 light)
{
    int i;
    for (i = 0; i < _lumenlight->getFrameSize(); i++){
        led_data[i] = led_data[i]*light/255;
    }
}

static bool LayersShow(void)
{
    _lumenlight->lumen_draw(led_data, sizeof(led_data));
}

static void* _EffectFadeIn(void* fadein)
{
    u8 color_b[3] = {0,0,0};
    struct fade_param *peffect_fadein = fadein;
    bool dir = peffect_fadein->dir;
    u8 poi = peffect_fadein->poi;
    u8 len = peffect_fadein->len;
    u8* color = peffect_fadein->color;
    int speed = peffect_fadein->speed;
    int i;

    InitLayers(speed);
    for (i = 0; i < len; i++)
    {
        LayerAddBackground(color_b);
        if(dir){
            LayerAddGroup(!dir,(poi+i)%12,i+1,color);
        }
        else{
            LayerAddGroup(!dir,(poi-i+12)%12,i+1,color);
        }
        LayersShow();
        usleep(speed);
        if(_flag_stop){
            _flag_stop = 0;
            return NULL;
        }
    }
    return NULL;
}

static bool EffectFadeIn(bool dir,u8 poi,u8 len,u8* color,int speed)
{
    _flag_stop = 1;
    pthread_join(_lumenthread, NULL);
    _flag_stop = 0;
    effect_fadein.dir = dir;
    effect_fadein.poi = poi;
    effect_fadein.len = len;
    effect_fadein.color = color;
    effect_fadein.speed = speed;
    pthread_create(&_lumenthread, NULL, _EffectFadeIn, &effect_fadein);
}

static void* _EffectFadeOut(void* fadeout)
{
    static u8 color_b[3] = {0,0,0};
    struct fade_param *peffect_fadeout = fadeout;
    bool dir = peffect_fadeout->dir;
    u8 poi = peffect_fadeout->poi;
    u8 len = peffect_fadeout->len;
    u8* color = peffect_fadeout->color;
    int speed = peffect_fadeout->speed;
    int i;

    InitLayers(speed);
    for (i = 0; i < len; i++)
    {
        LayerAddBackground(color_b);
        LayerAddGroup(!dir,poi,len-i-1,color + 3*i+3);
        LayersShow();
        usleep(speed);
        if(_flag_stop){
            _flag_stop = 0;
            return NULL;
        }
    }

    return NULL;
}

static bool EffectFadeOut(bool dir,u8 poi,u8 len,u8* color,int speed)
{
    _flag_stop = 1;
    pthread_join(_lumenthread, NULL);
    _flag_stop = 0;
    effect_fadeout.dir = dir;
    effect_fadeout.poi = poi;
    effect_fadeout.len = len;
    effect_fadeout.color = color;
    effect_fadeout.speed = speed;
    pthread_create(&_lumenthread, NULL, _EffectFadeOut, &effect_fadeout);
}

static void* _EffectPoint(void* point)
{
    struct point_param *peffect_point = point;
    int degree = peffect_point->degree;
    u8* color_point = peffect_point->color_point;
    u8 len = peffect_point->len;
    u8* color_len = peffect_point->color_len;
    int speed = peffect_point->speed;
    static u8 color_b[3] = {0,0,0};
    u8 poi = (degree/30)%12;
    static u8 color_b0[3] = {10,10,200};
    int i;

    if(degree == -1)
    {
        InitLayers(speed);
        LayerAddBackground(color_b0);
        LayersShow();
        return NULL;
    }

    InitLayers(speed);
    for (i = 0; i < (6+len); i++){
        LayerAddBackground(color_b);
        if(i < 6){
            LayerAddGroup(false,(poi+6+i)%12,i<len?i:len,color_len);
            LayerAddGroup(true,(poi+18-i)%12,i<len?i:len,color_len);
        }
        else{
            LayerAddGroup(false,poi,len-i+6,color_len+i-6);
            LayerAddGroup(true,poi,len-i+6,color_len+i-6);
        }
        LayerAddPoint(poi,color_point);
        LayersShow();
        usleep(speed);
        if(_flag_stop){
            _flag_stop = 0;
            return NULL;
        }
    }

    return NULL;
}

static bool EffectPoint1(int degree,u8* color_point,u8 len,u8* color_len,int speed)
{
    _flag_stop = 1;
    pthread_join(_lumenthread, NULL);
    _flag_stop = 0;
    effect_point.degree = degree;
    effect_point.color_point = color_point;
    effect_point.len = len;
    effect_point.color_len = color_len;
    effect_point.speed = speed;
    pthread_create(&_lumenthread, NULL, _EffectPoint, &effect_point);
}

static bool EffectPoint(int degree)
{
    static u8 color_point[3] = {100,100,100};
    static u8 color_len[12] = {30,30,30,10,10,10,4,4,4,1,1,1};

    EffectPoint1(degree,color_point,4,color_len,50000);
}

static void* _EffectStartRound(void* round)
{
    pthread_mutex_lock(&_lumenmutex);
    struct round_param *peffect_round = round;
    int start_degree = peffect_round->start_degree;
    int end_degree = peffect_round->end_degree;
    bool dir = peffect_round->dir;
    u8 len = peffect_round->len;
    u8* color = peffect_round->color;
    int speed = peffect_round->speed;
    int acc_spd = peffect_round->acc_spd;
    int max_spd = peffect_round->max_spd;
    static u8 color_b[3] = {0,0,0};
    u8 poi = (start_degree/30)%12;
    int i = 0;
    u8 end_poi;

    _end_degree = end_degree;
    pthread_mutex_unlock(&_lumenmutex);
    InitLayers(speed);
    while(ENDDEGREE_WAIT == _end_degree){
        LayerAddBackground(color_b);
        if(dir){
            LayerAddGroup(!dir,poi,i,color);
            poi = poi >= 11 ? 0:poi+1;
        }
        else{
            LayerAddGroup(!dir,poi,i,color);
            poi = poi <= 0 ? 11:poi-1;
        }
        i = i >= len ? i : i+1;
        LayersShow();
        usleep(_speed);
        if(_speed >= max_spd){
            _speed -= acc_spd;
        }
        if(_flag_stop){
            _flag_stop = 0;
            return NULL;
        }
    }
    if(ENDDEGREE_NOW == _end_degree){
        end_poi = poi;
    }
    else{
        end_poi = (_end_degree/30)%12;
    }
    while(end_poi != poi){
        LayerAddBackground(color_b);
        if(dir){
            LayerAddGroup(!dir,poi,i,color);
            poi = poi >= 11 ? 0:poi+1;
        }
        else{
            LayerAddGroup(!dir,poi,i,color);
            poi = poi <= 0 ? 11:poi-1;
        }
        i = i >= len ? i : i+1;
        LayersShow();
        usleep(speed);
        if(_flag_stop){
            _flag_stop = 0;
            return NULL;
        }
    }
    for (i = 0; i < len; i++)
    {
        LayerAddBackground(color_b);
        LayerAddGroup(!dir,poi-1,len-i-1,color + 3*i+3);
        LayersShow();
        usleep(speed);
        if(_flag_stop){
            _flag_stop = 0;
            return NULL;
        }
    }

    return NULL;
}

static bool EffectStartRound2(int start_degree,int end_degree,bool dir,u8 len,u8* color,int speed,int acc_spd,int max_spd)
{
    _flag_stop = 1;
    pthread_join(_lumenthread, NULL);
    _flag_stop = 0;
    effect_round.start_degree = start_degree;
    effect_round.end_degree = end_degree;
    effect_round.dir = dir;
    effect_round.len = len;
    effect_round.color = color;
    effect_round.speed = speed;
    effect_round.acc_spd = acc_spd;
    effect_round.max_spd = max_spd;
    pthread_create(&_lumenthread, NULL, _EffectStartRound, &effect_round);
}

static bool EffectStartRound(void)
{
    static u8 len = 5;
    static u8 color[15] = {30,30,200,15,15,100,10,10,50,4,4,10,1,1,1};;

    EffectStartRound2(0,ENDDEGREE_WAIT,true,5,color,80000,1000,70000);
}

static bool EffectStartRound1(int start_degree)
{
    static u8 len = 5;
    static u8 color[15] = {100,100,100,30,30,30,10,10,10,4,4,4,1,1,1};

    EffectStartRound2(start_degree,ENDDEGREE_WAIT,true,5,color,70000,1000,50000);
}

static bool EffectEndRound1(int end_degree)
{
    usleep(1000);
    pthread_mutex_lock(&_lumenmutex);
    _end_degree = end_degree;
    pthread_mutex_unlock(&_lumenmutex);
}

static bool EffectEndRound(void)
{
    EffectEndRound1(ENDDEGREE_NOW);
}

static bool EffectStop(void)
{
    static u8 color_bkg[3] = {0,0,0};

    _flag_stop = 1;
    pthread_join(_lumenthread, NULL);
    _flag_stop = 0;
    LayerAddBackground(color_bkg);

    LayersShow();
}

struct LumenEffects* newLumenEffects(void)
{
    struct LumenEffects *plumeneffects = NULL;
    _speed = 70000;
    plumeneffects = (struct LumenEffects*)yodalite_malloc(sizeof(struct LumenEffects));
    if(plumeneffects) {
        plumeneffects->InitLayers = InitLayers;
        plumeneffects->LayerAddBackground = LayerAddBackground;
        plumeneffects->LayerAddGroup = LayerAddGroup;
        plumeneffects->LayerAddGroupPoint = LayerAddGroupPoint;
        plumeneffects->LayerAddPoint = LayerAddPoint;
        plumeneffects->DelLayers = DelLayers;
        plumeneffects->LayersSetSpeed = LayersSetSpeed;
        plumeneffects->LayersSetLight = LayersSetLight;
        plumeneffects->LayersShow = LayersShow;
        plumeneffects->EffectFadeIn = EffectFadeIn;
        plumeneffects->EffectFadeOut = EffectFadeOut;
        plumeneffects->EffectPoint = EffectPoint;
        plumeneffects->EffectPoint1 = EffectPoint1;
        plumeneffects->EffectStartRound = EffectStartRound;
        plumeneffects->EffectStartRound1 = EffectStartRound1;
        plumeneffects->EffectStartRound2 = EffectStartRound2;
        plumeneffects->EffectEndRound = EffectEndRound;
        plumeneffects->EffectEndRound1 = EffectEndRound1;
        plumeneffects->EffectStop = EffectStop;
    }
    if(!ref_count) {
        _lumenlight = newLumenLight();
        pthread_mutex_init (&_lumenmutex, NULL);
    }
    ref_count++;
    return plumeneffects;
}

int destroyLumenEffects(struct LumenEffects* plumeneffects)
{
    ref_count--;
    DelLayers();
    if(!ref_count) {
        pthread_mutex_destroy(&_lumenmutex);
        destroyLumenLight(_lumenlight);
    }
    _flag_stop = 1;
    pthread_join(_lumenthread, NULL);
}

