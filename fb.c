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

#include "window.h"

int _zbar_window_attach (zbar_window_t *w,
                         void *display,
                         unsigned long win)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_expose (zbar_window_t *w,
                         int x,
                         int y,
                         int width,
                         int height)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_resize (zbar_window_t *w)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_clear (zbar_window_t *w)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_begin (zbar_window_t *w)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_end (zbar_window_t *w)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_draw_marker (zbar_window_t *w,
                              uint32_t rgb,
                              point_t p)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_draw_polygon (zbar_window_t *w,
                               uint32_t rgb,
                               const point_t *pts,
                               int npts)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_draw_text (zbar_window_t *w,
                            uint32_t rgb,
                            point_t p,
                            const char *text)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_fill_rect (zbar_window_t *w,
                            uint32_t rgb,
                            point_t org,
                            point_t size)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_window_draw_logo (zbar_window_t *w)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}
