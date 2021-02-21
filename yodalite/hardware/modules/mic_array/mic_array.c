/*************************************************************************
        > File Name: mic_array.c
        > Author:
        > Mail:
        > Created Time: Mon May  4 14:22:33 2015
 ************************************************************************/
#include <yodalite_autoconf.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include <hardware/platform.h>
#include <hardware/hardware.h>
#include <hardware/mic_array.h>

#define MIC_CHANNEL CONFIG_BOARD_MIC_CHANNEL

#define MODULE_NAME "mic_array"
#define MODULE_AUTHOR "jiaqi@rokid.com"

#define MIC_SAMPLE_RATE CONFIG_MIC_SAMPLE_RATE
#define PCM_CARD CONFIG_BOARD_SOUND_CARDID
#define PCM_DEVICE CONFIG_BOARD_CAPTURE_DEVICEID
#define FRAME_COUNT MIC_SAMPLE_RATE / 100 * MIC_CHANNEL * 4

#define MIC_LOG    printf

static struct pcm_config pcm_config_in = {
    .channels = MIC_CHANNEL,
    .rate = MIC_SAMPLE_RATE,
    .period_size = 1024,
    .period_count = 8,
    .format = PCM_FORMAT_S32_LE,
};

static struct mic_array_device_ex mic_array;
static char frame_buf[FRAME_COUNT];

static int mic_array_device_open(const struct hw_module_t* module,const char* name, struct hw_device_t** device);

static int mic_array_device_close(struct hw_device_t* device);

static int mic_array_device_start_stream(struct mic_array_device_t* dev);

static int mic_array_device_stop_stream(struct mic_array_device_t* dev);

static int mic_array_device_finish_stream(struct mic_array_device_t* dev);

static int mic_array_device_read_stream(struct mic_array_device_t* dev, char* buff, unsigned int frame_cnt);

static int mic_array_device_config_stream(struct mic_array_device_t* dev, int cmd, char* cmd_buff);

static int mic_array_device_get_stream_buff_size(struct mic_array_device_t* dev);

static int mic_array_device_resume_stream(struct mic_array_device_t* dev);

static int mic_array_device_find_card(const char* snd);

static struct hw_module_methods_t mic_array_module_methods = {
    .open = mic_array_device_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM(MIC_ARRAY_HARDWARE_MODULE_ID) = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = MIC_API_VERSION,
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = HAL_MODULE_ID(MIC_ARRAY_HARDWARE_MODULE_ID),
    .name = MODULE_NAME,
    .author = MODULE_AUTHOR,
    .methods = &mic_array_module_methods,
};

static int mic_array_device_find_card(const char* snd)
{
    if (snd == NULL)
        return -1;
    return 0;
}

static int mic_array_device_open(const struct hw_module_t* module,
    const char* name, struct hw_device_t** device)
{
    int i = 0;
    struct mic_array_device_ex* dev_ex = NULL;
    struct mic_array_device_t* dev = NULL;
    dev_ex = &mic_array;
    dev = (struct mic_array_device_t*)dev_ex;

    if (!dev_ex) {
        MIC_LOG("MIC_ARRAY: FAILED TO ALLOC SPACE");
        return -1;
    }

    memset(dev, 0, sizeof(struct mic_array_device_ex));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (hw_module_t*)module;
    dev->common.close = mic_array_device_close;
    dev->start_stream = mic_array_device_start_stream;
    dev->stop_stream = mic_array_device_stop_stream;
    dev->finish_stream = mic_array_device_finish_stream;
    dev->resume_stream = mic_array_device_resume_stream;
    dev->read_stream = mic_array_device_read_stream;
    dev->config_stream = mic_array_device_config_stream;
    dev->get_stream_buff_size = mic_array_device_get_stream_buff_size;
    dev->find_card = mic_array_device_find_card;

    dev->channels = MIC_CHANNEL;
    dev->sample_rate = MIC_SAMPLE_RATE;
    dev->bit = pcm_format_to_bits(pcm_config_in.format);
    dev->pcm = NULL;
    dev->frame_cnt = FRAME_COUNT;
    MIC_LOG("alloc frame buffer size %d", dev->frame_cnt);
    dev_ex->buffer = frame_buf;
    *device = &(dev->common);
    return 0;
}

static void resetBuffer(struct mic_array_device_ex* dev) { dev->pts = 0; }

static int mic_array_device_close(struct hw_device_t* device)
{
    MIC_LOG("pcm close");

    struct mic_array_device_t* mic_array_device = (struct mic_array_device_t*)device;
    struct mic_array_device_ex* dev_ex = (struct mic_array_device_ex*)mic_array_device;

    if (dev_ex != NULL) {
        dev_ex = NULL;
    }
    return 0;
}

static int mic_array_device_start_stream(struct mic_array_device_t* dev)
{
    int card = PCM_CARD;
    struct pcm* pcm = NULL;

    pcm = pcm_open(card, PCM_DEVICE, PCM_IN, &pcm_config_in);
    if (!pcm || !pcm_is_ready(pcm)) {
        MIC_LOG("Unable to open PCM device %u (%d)\n", card, PCM_DEVICE);
        if (pcm != NULL) {
            pcm_close(pcm);
            pcm = NULL;
        }
        return -1;
    }
    dev->pcm = pcm;
    return 0;
}

static int mic_array_device_stop_stream(struct mic_array_device_t* dev)
{
    if (dev->pcm != NULL) {
        pcm_close(dev->pcm);
        dev->pcm = NULL;
    }
    return 0;
}

static int mic_array_device_finish_stream(struct mic_array_device_t* dev)
{
    MIC_LOG("finish stream is no use");
    return -1;
}

static int read_frame(struct mic_array_device_t* dev, char* buffer)
{
#if defined(ANDROID) || defined(__ANDROID__)
    return pcm_read(dev->pcm, buffer, dev->frame_cnt);
#else
    int ret = pcm_read(dev->pcm, buffer, dev->frame_cnt);
    if (ret > 0)
    return 0;
    else
    return -1;
#endif
}

static int read_left_frame(
    struct mic_array_device_ex* dev, char* buff, int left)
{
    int ret = 0;
    if (dev->pts == 0) {
        if ((ret = read_frame(&(dev->mic_array), dev->buffer)) != 0) {
            MIC_LOG("read_frame error %d", ret);
            resetBuffer(dev);
            return ret;
        }
        memcpy(buff, dev->buffer, left);
        memcpy(dev->buffer, dev->buffer + left, dev->mic_array.frame_cnt - left);
        dev->pts = dev->mic_array.frame_cnt - left;
    } else {
        if (dev->pts >= left) {
            memcpy(buff, dev->buffer, left);
            dev->pts -= left;
            if (dev->pts != 0) {
                memcpy(dev->buffer, dev->buffer + left, dev->pts);
            }
        } else {
            memcpy(buff, dev->buffer, dev->pts);
            left -= dev->pts;
            if ((ret = read_frame(&(dev->mic_array), dev->buffer)) != 0) {
                MIC_LOG("read_frame error %d", ret);
                resetBuffer(dev);
                return ret;
            }
            memcpy(buff + dev->pts, dev->buffer, left);
            memcpy(dev->buffer, dev->buffer + left,dev->mic_array.frame_cnt - left);
            dev->pts = dev->mic_array.frame_cnt - left;
        }
    }
    return 0;
}

static int mic_array_device_read_stream(
    struct mic_array_device_t* dev, char* buff, unsigned int frame_cnt)
{
    struct pcm* pcm = dev->pcm;
    struct mic_array_device_ex* dev_ex = (struct mic_array_device_ex*)dev;
    char* target = NULL;

    int ret = 0;
    int left = 0;
    int size = dev->frame_cnt;
    if (size <= 0) {
        MIC_LOG("frame cnt lt 0");
        return -1;
    }

    if (buff == NULL) {
        MIC_LOG("null buffer");
        return -1;
    }

    if (frame_cnt >= size) {
        int cnt = frame_cnt / size;
        int i;
        left = frame_cnt % size;

        if (dev_ex->pts > left) {
            --cnt;
        }
        if (dev_ex->pts > 0) {
            memcpy(buff, dev_ex->buffer, dev_ex->pts);
        }

        for (i = 0; i < cnt; i++) {
            if ((ret = read_frame(dev, buff + dev_ex->pts + i * size)) != 0) {
                MIC_LOG("read_frame error %d", ret);
                resetBuffer(dev_ex);
                return ret;
            }
        }
        if (frame_cnt - (dev_ex->pts + cnt * size) == 0) {
            dev_ex->pts = 0;
            return ret;
        }
        //      ALOGE("-------------------cnt : %d, left : %d, cache :
        //%d, frame_cnt : %d", cnt, left, dev_ex->pts, frame_cnt);
        target = buff + dev_ex->pts + cnt * size;
        left = frame_cnt - (dev_ex->pts + cnt * size);
        dev_ex->pts = 0;
    } else {
        target = buff;
        left = frame_cnt;
    }

    if ((ret = read_left_frame(dev_ex, target, left)) != 0) {
        MIC_LOG("read left frame return %d, pcm read error", ret);
        resetBuffer(dev_ex);
        return ret;
    }

    return ret;
}

static int mic_array_device_config_stream(struct mic_array_device_t* dev, int cmd, char* cmd_buff)
{
    return -1;
}

static int mic_array_device_get_stream_buff_size(struct mic_array_device_t* dev)
{
    return dev->frame_cnt;
}

static int mic_array_device_resume_stream(struct mic_array_device_t* dev)
{
    MIC_LOG("not implmentation");
    return -1;
}
