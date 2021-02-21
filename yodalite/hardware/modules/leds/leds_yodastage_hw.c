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
#include <hardware/leds.h>

#define LED_PATH "rokid, led-stage"
#define MAX_LED_GROUP_NUM CONFIG_BOARD_LED_NUMS
#define MAX_BRIGHTNESS    255
#define MAX_PATH_LEN      128
#define LEDS_LOG    printf

static int brightness = MAX_BRIGHTNESS;
static unsigned char api_enabled = 1;
static struct ledarray_device_t led_array;

#define CHK_IF_ENABLED() \
	do { \
		if (!api_enabled) { \
			LEDS_LOG("disabled\n"); \
			return (-STATUS_EPERM); \
		} \
	} while(0)

#define CHK_IF_OUT_OF_RANGE(br) \
	do { \
		if (br < 0 || br > 255) { \
			LEDS_LOG("out of range\n"); \
			return (-STATUS_EINVAL); \
		} \
	} while(0)

#define CHK_IF_BUSY() \
	do { \
		if (led_is_busy()) { \
			LEDS_LOG("busy\n"); \
			return (-STATUS_EBUSY); \
		} \
	} while(0)

static int write_pixls(unsigned char *buf, int len)
{
    int fd;
    int ret;

    fd = platform_device_open(LED_PATH, DEV_WRONLY);
    if (fd < 0) {
	return -1;
    }

    ret = platform_device_write(fd, buf, len);
    platform_device_close(fd);

    if (ret < 0) {
	return -1;
    }

    return 0;
}

static int led_is_busy()
{
    int busy = 0;
    int ret;
    int fd;

    fd = platform_device_open(LED_PATH, DEV_RDONLY);
    if (fd < 0) {
        return -STATUS_ENODEV;
    }
    ret = platform_device_ioctl(fd, CHK_DEV_STATUS, &busy);
    platform_device_close(fd);
    if (ret) {
        return ret;
    }
    return busy;
}

static int led_enable_set(struct ledarray_device_t *dev __unused, int cmd)
{
    api_enabled = cmd;
    return 0;
}

static int led_brightness_set(struct ledarray_device_t *dev __unused,
			      unsigned char br)
{
    CHK_IF_ENABLED();
    brightness = br;
    return 0;
}

#ifdef BOARD_ROKID_LED_PATCH_FOR_ERR_DIRECTION_1STPOS
static inline int led_index_relocate(int index)
 {
    index = ((MAX_LED_GROUP_NUM - index)%MAX_LED_GROUP_NUM); // reverse.
    index = ((index + 3) % MAX_LED_GROUP_NUM); // first led pos need offset 9.
    return index;
}

static void led_reverse_mem(unsigned char *dst, const unsigned char *src, unsigned n)
{
    unsigned src_idx;
    unsigned dst_idx;
    const unsigned led_nums = n/3;

    for (src_idx = 0; src_idx < led_nums; ++src_idx) {
        dst_idx = led_index_relocate(src_idx);
	const unsigned src_offset = src_idx * 3;
	const unsigned dst_offset = dst_idx * 3;
        dst[dst_offset] = src[src_offset];
        dst[dst_offset+1] = src[src_offset+1];
        dst[dst_offset+2] = src[src_offset+2];
    }
}
#endif

static void rgb2bgr(unsigned char *tmp_buf, int len)
{
    int i;
    unsigned char swap;

    for (i = 0; i < len; i += 3) {
	swap = tmp_buf[i];
	tmp_buf[i] = tmp_buf[i + 2];
	tmp_buf[i + 2] = swap;
    }
}

static int led_draw(struct ledarray_device_t *dev,
		    unsigned char *buf, int len)
{
    int ret = 0;
    char path[MAX_PATH_LEN];
    unsigned char tmp_buf[MAX_LED_GROUP_NUM * PXL_FMT_RGB] = {0};

    CHK_IF_ENABLED();
    CHK_IF_BUSY();

    if (len < 0 || len > MAX_LED_GROUP_NUM * PXL_FMT_RGB || (len % 3)) {
	LEDS_LOG("invalid argument: len(%d)\n", len);
	return -STATUS_EINVAL;
    }

#ifdef BOARD_ROKID_LED_PATCH_FOR_ERR_DIRECTION_1STPOS
    led_reverse_mem(tmp_buf, buf, len);
#else
    memcpy(tmp_buf, buf, len);
#endif
    rgb2bgr(tmp_buf, len);
    ret = write_pixls(tmp_buf, len);
    if (ret)
	return ret;
    return 0;
}

static int led_dev_close(struct ledarray_device_t *device)
{
    return 0;
}

static int led_dev_open(const hw_module_t * module,
			const char *name __unused, hw_device_t ** device)
{
    struct ledarray_device_t *ledadev = &led_array;

    ledadev->common.tag = HARDWARE_DEVICE_TAG;
    ledadev->common.module = (hw_module_t *) module;
    ledadev->common.version = HARDWARE_DEVICE_API_VERSION(1, 0);


    ledadev->dev_code = NANA;
    ledadev->led_count = MAX_LED_GROUP_NUM;
    ledadev->pxl_fmt = PXL_FMT_RGB;

    ledadev->dev_close = led_dev_close;
    ledadev->draw = led_draw;
    //ledadev->draw_one = led_draw_one;
    ledadev->set_brightness = led_brightness_set;
    ledadev->set_enable = led_enable_set;
    //ledadev->set_current_reating = led_current_set;

    *device = (struct hw_device_t *) ledadev;
    return 0;
}

static struct hw_module_methods_t led_module_methods = {
    .open = led_dev_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM(LED_ARRAY_HW_ID) = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = LEDS_API_VERSION,
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = HAL_MODULE_ID(LED_ARRAY_HW_ID),
    .name = "ROKID LEDS HAL: The Coral solution.",
    .author = "Rokid Platform Foundation",
    .methods = &led_module_methods,
};
