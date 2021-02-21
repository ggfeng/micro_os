#ifndef _LUMEN_LIGHT_H
#define _LUMEN_LIGHT_H
#include <stdio.h>
#include <hardware/hardware.h>
#include <hardware/leds.h>

#define bool    int
#define true    1
#define false   0
struct LumenLight {
    /* set the interface lumen_draw be usrful or not */
    void (*lumen_set_enable)(bool cmd);

    /* draw all led to show the color buff you set
     * buf[m_ledCount * m_pixelFormat]; len = sizeof(buf);
     * if m_pixelFormat == 3, means RGB
     * you can set buf[ledNum] = 0xff;  for the red component
     * you can set buf[ledNum + 1] = 0xff  for the green component
     * you can set buf[ledNum + 2] = 0xff  for the blue component
     * ledNum: from 0 to  (m_ledCount - 1)*/
    int (*lumen_draw)(unsigned char* buf, int len);

    /* read framesize
     * framesize = m_ledCount *  m_pixelFormat */
    int (*getFrameSize)(void);

    /* read led count */
    int (*getLedCount)(void);

    /* read pixel format */
    int (*getPixelFormat)(void);

    /* read the max fps that the lumenlight suggests */
    int (*getFps)(void);
};

extern struct LumenLight* newLumenLight (void);
extern int destroyLumenLight (struct LumenLight* plumenlight);

#endif
