/*
 * (C) Copyright 2019 Rokid Corp.
 * Zhu Bin <bin.zhu@rokid.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <hardware/platform.h>

static struct platform_device_driver*  open_device[MAX_OPEN_DEV_NUM] = {0};
static struct platform_device_driver *platform_device_driver_chain = NULL;

#define yodalite_malloc malloc
#define yodalite_free free

 /**
 * @brief  bind pdevice and pdriver together into platform_device_driver chain if the name in pdevice and pdriver is same
 * @param  pdevice: the pointer to the platform_device
 * @param  pdriver: the pointer to the platform_driver
 * @retval STATUS_OK for function running well and other value for function running fault
 */
 int platform_device_driver_bind(struct platform_device *pdevice, struct platform_driver *pdriver)
{
    struct platform_device_driver *pdev;
    struct platform_device_driver *pnew_dev;
    int ret = STATUS_OK;

    if(!pdevice || !pdriver)
        return -STATUS_EINVAL;
    if(strcmp(pdevice->name, pdriver->name))
        return -STATUS_EINVAL;
    pnew_dev = (struct platform_device_driver*)yodalite_malloc(sizeof(struct platform_device_driver));
    if(!pnew_dev)
        return -STATUS_EPERM;
    memset(pnew_dev, 0, sizeof(struct platform_device_driver));
    pnew_dev->pdev = pdevice;
    pnew_dev->pdrv = pdriver;
    if(pnew_dev->pdrv->probe)
        ret = pnew_dev->pdrv->probe(pnew_dev->pdev);
    if(ret != STATUS_OK)
        return -STATUS_EACCES;
    if(!platform_device_driver_chain) {
        platform_device_driver_chain = pnew_dev;
        return STATUS_OK;
    }
    pdev = platform_device_driver_chain;
    while(pdev->next) {
        pdev = pdev->next;
    }
    pdev->next = pnew_dev;
    pnew_dev->prev = pdev;
    return STATUS_OK;
}
/*
int platform_device_chain_remove(void)
{
    return STATUS_OK;
}

int platform_device_chain_suspend(void)
{
    return STATUS_OK;
}

int platform_device_chain_resume(void)
{
    return STATUS_OK;
}
*/
int platform_device_open(const char *dev_name, unsigned flag)
{
    struct platform_device_driver *dev;

    dev = platform_device_driver_chain;
    while(dev) {
        if(!strcmp(dev->pdev->name, dev_name)) {
            if(dev->open_cnt < 0)
                return -STATUS_EINVAL;
            dev->open_cnt = dev->open_cnt + 1;
            if(dev->id)
                return dev->id;
            else {
                int i;
                for(i=1; i<MAX_OPEN_DEV_NUM; i++) {
                    if(!open_device[i]) {
                        open_device[i] = dev;
                        dev->id = i;
                        return i;
                    }
                }
                if(i>=MAX_OPEN_DEV_NUM)
                    return -STATUS_EACCES;
            }
        }
        dev = dev->next;
    }
    return -STATUS_ENODEV;
}

int platform_device_read(int fd, char *buf, unsigned size)
{
    int ret;
    struct platform_device_driver *pdev;

    pdev = open_device[fd];
    if(!pdev)
        return -STATUS_ENODEV;
    if(pdev->pdrv->fops.read)
        ret = pdev->pdrv->fops.read(buf, size);
    return ret;
}

int platform_device_write(int fd, char *buf, unsigned size)
{
    int ret;
    struct platform_device_driver *pdev;

    pdev = open_device[fd];
    if(!pdev)
        return -STATUS_ENODEV;
    if(pdev->pdrv->fops.write)
        ret = pdev->pdrv->fops.write(buf, size);
    return ret;
}

int platform_device_ioctl(int fd, unsigned cmd, void *pvalue)
{
    int ret;
    struct platform_device_driver *pdev;

    pdev = open_device[fd];
    if(!pdev)
        return -STATUS_ENODEV;
    if(pdev->pdrv->fops.ioctl)
        ret = pdev->pdrv->fops.ioctl(cmd, pvalue);
    return ret;
}

int platform_device_close(int fd)
{
    int ret;
    struct platform_device_driver *pdev;

    pdev = open_device[fd];
    if(!pdev)
        return -STATUS_ENODEV;
    if(pdev->open_cnt <= 0)
        return -STATUS_EINVAL;
    pdev->open_cnt = pdev->open_cnt - 1;
    if(!pdev->open_cnt){
        pdev->id = 0;
        open_device[fd] = NULL;
    }
    return STATUS_OK;
}


