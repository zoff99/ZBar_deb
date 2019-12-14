/*------------------------------------------------------------------------
 *  Copyright 2008-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <linux/fb.h>

#include "window.h"
#include "processor.h"

int global_framebuffer_device_fd = 0;
const char *framebuffer_device = "/dev/fb0";
struct fb_var_screeninfo var_framebuffer_info;
struct fb_fix_screeninfo var_framebuffer_fix_info;
size_t framebuffer_screensize = 0;
unsigned char *framebuffer_mappedmem = NULL;

void fb_fill_black()
{
    if (framebuffer_mappedmem != NULL)
    {
        memset(framebuffer_mappedmem, 0x0, framebuffer_screensize);
    }
}

void fb_fill_xxx()
{
    if (framebuffer_mappedmem != NULL)
    {
        memset(framebuffer_mappedmem, 0xa3, framebuffer_screensize);
    }
}


int _zbar_processor_open (zbar_processor_t *proc,
                          char *name,
                          unsigned w,
                          unsigned h)
{
    zprintf(1, "%s\n", __func__);

    global_framebuffer_device_fd = open(framebuffer_device, O_RDWR);

    if (ioctl(global_framebuffer_device_fd, FBIOGET_VSCREENINFO, &var_framebuffer_info))
    {
        zprintf(1, "%s ERROR FBIOGET_VSCREENINFO\n", __func__);
    }

    if (ioctl(global_framebuffer_device_fd, FBIOGET_FSCREENINFO, &var_framebuffer_fix_info))
    {
        zprintf(1, "%s ERROR FBIOGET_FSCREENINFO\n", __func__);
    }

    // map framebuffer to user memory
    framebuffer_screensize = (size_t)var_framebuffer_fix_info.smem_len;
    framebuffer_mappedmem = NULL;
    framebuffer_mappedmem = (unsigned char *)mmap(NULL,
                                            (size_t)framebuffer_screensize,
                                            PROT_READ | PROT_WRITE,
                                            MAP_SHARED,
                                            global_framebuffer_device_fd, 0);

    if (framebuffer_mappedmem == NULL)
    {
        zprintf(1, "%s Failed to mmap Framebuffer\n", __func__);
    }
    else
    {
        zprintf(1, "%s mmap Framebuffer: %p\n", __func__, framebuffer_mappedmem);
    }

    return(0);
}

int _zbar_processor_close (zbar_processor_t *proc)
{
    zprintf(1, "%s\n", __func__);

    if (framebuffer_mappedmem != NULL)
    {
        munmap(framebuffer_mappedmem, (size_t)framebuffer_screensize);
        framebuffer_mappedmem = NULL;
    }

    if (global_framebuffer_device_fd > 0)
    {
        close(global_framebuffer_device_fd);
        global_framebuffer_device_fd = 0;
    }

    return(0);
}

int _zbar_processor_set_visible (zbar_processor_t *proc,
                                 int vis)
{
    zprintf(1, "%s\n", __func__);
    // fill framebuffer to see if it works
    fb_fill_xxx();
    return(0);
}

int _zbar_processor_set_size (zbar_processor_t *proc,
                              unsigned width,
                              unsigned height)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_processor_invalidate (zbar_processor_t *proc)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}
