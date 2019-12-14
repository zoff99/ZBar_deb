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
#include <assert.h>
#include <errno.h>

#include "window.h"
#include "posix.h"
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

static inline int proc_sleep (int timeout)
{
    assert(timeout > 0);
    struct timespec sleepns, remns;
    sleepns.tv_sec = timeout / 1000;
    sleepns.tv_nsec = (timeout % 1000) * 1000000;
    while(nanosleep(&sleepns, &remns) && errno == EINTR)
    {
        sleepns = remns;
    }
    return(1);
}

/* used by poll interface.  lock is already held */
static int proc_video_handler (zbar_processor_t *proc, int i)
{
    _zbar_mutex_lock(&proc->mutex);
    _zbar_processor_lock(proc);
    _zbar_mutex_unlock(&proc->mutex);

    zbar_image_t *img = NULL;

    if (proc->streaming)
    {
        /* not expected to block */
        img = zbar_video_next_image(proc->video);
        if(img)
        {
            uint32_t format = zbar_image_get_format(img);
            unsigned width = zbar_image_get_width(img);
            unsigned height = zbar_image_get_height(img);
            const uint8_t *data = zbar_image_get_data(img);
                zprintf(1, "captured image: %d x %d %.4s @%p\n",
                        width, height, (char*)&format, data);

            // ------------- put "img" to framebuffer here !! -------------
            // ------------- put "img" to framebuffer here !! -------------
            // ------------- put "img" to framebuffer here !! -------------
            // ------------- put "img" to framebuffer here !! -------------
            // ------------- put "img" to framebuffer here !! -------------

            if (framebuffer_mappedmem != NULL)
            {
                if (framebuffer_screensize > 1)
                {
                    // clear FB
                    memset(framebuffer_mappedmem, 0x0, framebuffer_screensize);
                    // put data into FB
                    memcpy(framebuffer_mappedmem, data, (framebuffer_screensize - 1) );
                }
            }

            // ------------- put "img" to framebuffer here !! -------------
            // ------------- put "img" to framebuffer here !! -------------
            // ------------- put "img" to framebuffer here !! -------------
            // ------------- put "img" to framebuffer here !! -------------
            // ------------- put "img" to framebuffer here !! -------------

            _zbar_process_image(proc, img);
        }
    }

    _zbar_mutex_lock(&proc->mutex);
    _zbar_processor_unlock(proc, 0);
    _zbar_mutex_unlock(&proc->mutex);

    if(img)
    {
        zbar_image_destroy(img);
    }

    return(0);
}

static inline void proc_cache_polling (processor_state_t *state)
{
    zprintf(1, "%s\n", __func__);

    /* make a thread-local copy of polling data */
    int n = state->thr_polling.num = state->polling.num;
    alloc_polls(&state->thr_polling);
    memcpy(state->thr_polling.fds, state->polling.fds,
           n * sizeof(struct pollfd));
    memcpy(state->thr_polling.handlers, state->polling.handlers,
           n * sizeof(poll_handler_t*));
}

static int proc_kick_handler (zbar_processor_t *proc,
                              int i)
{
    zprintf(1, "%s\n", __func__);

    processor_state_t *state = proc->state;
    zprintf(5, "kicking %d fds\n", state->polling.num);

    unsigned junk[2];
    int rc = read(state->kick_fds[0], junk, 2 * sizeof(unsigned));

    assert(proc->threaded);
    _zbar_mutex_lock(&proc->mutex);
    proc_cache_polling(proc->state);
    _zbar_mutex_unlock(&proc->mutex);
    return(rc);
}

static inline int proc_poll_inputs (zbar_processor_t *proc,
                                    int timeout)
{
    zprintf(1, "%s\n", __func__);

    processor_state_t *state = proc->state;
    if(state->pre_poll_handler)
        state->pre_poll_handler(proc, -1);

    poll_desc_t *p = &state->thr_polling;
    assert(p->num);
    int rc = poll(p->fds, p->num, timeout);
    if(rc <= 0)
        /* FIXME detect and handle fatal errors (somehow) */
        return(rc);
    int i;
    for(i = p->num - 1; i >= 0; i--)
        if(p->fds[i].revents) {
            if(p->handlers[i])
                p->handlers[i](proc, i);
            p->fds[i].revents = 0; /* debug */
            rc--;
        }
    assert(!rc);
    return(1);
}


// ------------------------------------

int _zbar_event_init (zbar_event_t *event)
{
    zprintf(1, "%s\n", __func__);

    event->state = 0;
    event->pollfd = -1;
#ifdef HAVE_LIBPTHREAD
    pthread_cond_destroy(&event->cond);
#endif

    return(0);
}

void _zbar_event_destroy (zbar_event_t *event)
{
    zprintf(1, "%s\n", __func__);

    event->state = -1;
    event->pollfd = -1;
#ifdef HAVE_LIBPTHREAD
    pthread_cond_destroy(&event->cond);
#endif
}

void _zbar_event_trigger (zbar_event_t *event)
{
    zprintf(1, "%s\n", __func__);

    event->state = 1;
#ifdef HAVE_LIBPTHREAD
    pthread_cond_broadcast(&event->cond);
#endif
    if(event->pollfd >= 0)
    {
        unsigned i = 0; /* unused */
        if(write(event->pollfd, &i, sizeof(unsigned)) < 0)
        {
            perror("");
        }
        event->pollfd = -1;
    }
}

/* lock must be held */
int _zbar_event_wait (zbar_event_t *event,
                      zbar_mutex_t *lock,
                      zbar_timer_t *timeout)
{
    zprintf(1, "%s\n", __func__);
    int rc = 0;

    while(!rc && !event->state)
    {
        if(!timeout)
        {
            rc = pthread_cond_wait(&event->cond, lock);
        }
        else
        {
            struct timespec *timer;
# if _POSIX_TIMERS > 0
            timer = timeout;
# else
            struct timespec tmp;
            tmp.tv_sec = timeout->tv_sec;
            tmp.tv_nsec = timeout->tv_usec * 1000;
            timer = &tmp;
# endif
            rc = pthread_cond_timedwait(&event->cond, lock, timer);
        }
    }

    /* consume/reset event */
    event->state = 0;

    if(!rc)
    {
        return(1); /* got event */
    }

    if(rc == ETIMEDOUT)
    {
        return(0); /* timed out */
    }

    return(-1); /* error (FIXME save info) */
}

int _zbar_thread_start (zbar_thread_t *thr,
                        zbar_thread_proc_t *proc,
                        void *arg,
                        zbar_mutex_t *lock)
{
    zprintf(1, "%s\n", __func__);

    if(thr->started || thr->running)
    {
        return(-1/*FIXME*/);
    }

    thr->started = 1;
    _zbar_event_init(&thr->notify);
    _zbar_event_init(&thr->activity);

    int rc = 0;
    _zbar_mutex_lock(lock);

    if (pthread_create(&thr->tid, NULL, proc, arg) ||
       _zbar_event_wait(&thr->activity, lock, NULL) < 0 ||
       !thr->running)
    {
        thr->started = 0;
        _zbar_event_destroy(&thr->notify);
        _zbar_event_destroy(&thr->activity);
        /*rc = err_capture_num(proc, SEV_ERROR, ZBAR_ERR_SYSTEM,
          __func__, "spawning thread", rc);*/
        rc = -1/*FIXME*/;
    }
    _zbar_mutex_unlock(lock);

    return(rc);
}

int _zbar_thread_stop (zbar_thread_t *thr,
                       zbar_mutex_t *lock)
{
    zprintf(1, "%s\n", __func__);

    if(thr->started)
    {
        thr->started = 0;
        _zbar_event_trigger(&thr->notify);

        while(thr->running)
        {
            /* FIXME time out and abandon? */
            _zbar_event_wait(&thr->activity, lock, NULL);
        }

        pthread_join(thr->tid, NULL);
        _zbar_event_destroy(&thr->notify);
        _zbar_event_destroy(&thr->activity);
    }

    return(0);
}

int _zbar_processor_input_wait (zbar_processor_t *proc,
                                zbar_event_t *event,
                                int timeout)
{
    zprintf(1, "%s\n", __func__);

    processor_state_t *state = proc->state;
    if(state->thr_polling.num)
    {
        if(event)
        {
            _zbar_mutex_lock(&proc->mutex);
            event->pollfd = state->kick_fds[1];
            _zbar_mutex_unlock(&proc->mutex);
        }
        return(proc_poll_inputs(proc, timeout));
    }
    else if(timeout)
    {
        return(proc_sleep(timeout));
    }

    return(-1);
}

int _zbar_processor_init (zbar_processor_t *proc)
{
    zprintf(1, "%s\n", __func__);

    processor_state_t *state = proc->state = calloc(1, sizeof(processor_state_t));
    state->kick_fds[0] = state->kick_fds[1] = -1;

    if(proc->threaded)
    {
        /* FIXME check errors */
        if(pipe(state->kick_fds))
        {
            return(err_capture(proc, SEV_FATAL, ZBAR_ERR_SYSTEM, __func__,
                               "failed to open pipe"));
        }
        add_poll(proc, state->kick_fds[0], proc_kick_handler);
        proc_cache_polling(proc->state);
    }

    return(0);
}

int _zbar_processor_cleanup (zbar_processor_t *proc)
{
    zprintf(1, "%s\n", __func__);

    processor_state_t *state = proc->state;
    if(proc->threaded) {
        close(state->kick_fds[0]);
        close(state->kick_fds[1]);
        state->kick_fds[0] = state->kick_fds[1] = -1;
    }
    if(state->polling.fds) {
        free(state->polling.fds);
        state->polling.fds = NULL;
        if(!proc->threaded)
            state->thr_polling.fds = NULL;
    }
    if(state->polling.handlers) {
        free(state->polling.handlers);
        state->polling.handlers = NULL;
        if(!proc->threaded)
            state->thr_polling.handlers = NULL;
    }
    if(state->thr_polling.fds) {
        free(state->thr_polling.fds);
        state->thr_polling.fds = NULL;
    }
    if(state->thr_polling.handlers) {
        free(state->thr_polling.handlers);
        state->thr_polling.handlers = NULL;
    }
    free(proc->state);
    proc->state = NULL;
    return(0);
}

int _zbar_processor_enable (zbar_processor_t *proc)
{
    zprintf(1, "%s\n", __func__);

    int vid_fd = zbar_video_get_fd(proc->video);

    if(vid_fd < 0)
    {
        return(0);
    }

    if(proc->streaming)
    {
        add_poll(proc, vid_fd, proc_video_handler);
    }
    else
    {
        remove_poll(proc, vid_fd);
    }
    /* FIXME failure recovery? */
    return(0);
}

// ------------------------------------

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

