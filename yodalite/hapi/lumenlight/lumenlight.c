
/*
* author: tian.fan@rokid.com
*/

#include <yodalite_autoconf.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include <hapi/lumenlight.h>

#define LUMENLIGHT_LOG  printf
#define yodalite_malloc malloc
#define yodalite_free free

static int m_frameSize;
static int m_ledCount;
static int m_pixelFormat;
static int m_fps;
static ledarray_device_t *m_leds = NULL;

static int getFrameSize(void)
{
    return m_frameSize;
}

static int getLedCount(void)
{
    return m_ledCount;
}

static int getPixelFormat(void)
{
    return m_pixelFormat;
}

static int getFps(void)
{
    return m_fps;
}

static int lumen_draw(unsigned char* buf, int len) {
    if(len > m_frameSize){
        len = m_frameSize;
    }
    int ret = m_leds->draw(m_leds, buf, len);
    return ret;
}

static ledarray_device_t *chkDev (void) {

    //ALOGV ("%s", __FUNCTION__);
    struct hw_module_t* hw_mod;
    ledarray_device_t *leds = NULL;
    int ret;

    hw_get_module (LED_ARRAY_HW_ID, hw_mod);
    ret = hw_mod->methods->open (hw_mod, NULL, (struct hw_device_t **) &leds);
    if (ret < 0) {
        LUMENLIGHT_LOG ("failed to fetch hw module: led_array\n");
        goto lumen_dev_init_fail;
    }
    return leds;

lumen_dev_init_fail:

    if (leds)
        leds->dev_close (leds);
    return NULL;
}

static void initialize (void) {
    m_leds = chkDev();
    m_frameSize = m_leds->led_count * m_leds->pxl_fmt;
    m_ledCount = m_leds->led_count;
    m_fps = m_leds->fps;
    m_pixelFormat = m_leds->pxl_fmt;
    LUMENLIGHT_LOG ("=== led light info ===\nf size: %d\nled count: %d\nfps: %d\npxl format: %d\n=====\n",
        m_frameSize,
        m_ledCount,
        m_fps,
        m_pixelFormat);
    return;
}

static void lumen_set_enable(bool cmd) {
    if (cmd)
        m_leds->set_enable(m_leds,1);
    else
        m_leds->set_enable(m_leds,0);
}

struct LumenLight* newLumenLight (void) {
    struct LumenLight *plumenlight = NULL;
    LUMENLIGHT_LOG ("LumenLight ctor\n");
    plumenlight = (struct LumenLight*)yodalite_malloc(sizeof(struct LumenLight));
    if(plumenlight) {
        initialize();
        plumenlight->lumen_set_enable = lumen_set_enable;
        plumenlight->lumen_draw = lumen_draw;
        plumenlight->getFrameSize = getFrameSize;
        plumenlight->getLedCount = getLedCount;
        plumenlight->getPixelFormat = getPixelFormat;
        plumenlight->getFps = getFps;
    }
    return plumenlight;
}

int destroyLumenLight (struct LumenLight* plumenlight) {
    LUMENLIGHT_LOG ("LumenLight dtor\n");
    yodalite_free(plumenlight);
    return 0;
}

