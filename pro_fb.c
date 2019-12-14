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

#include "processor.h"


int _zbar_processor_open (zbar_processor_t *proc,
                          char *name,
                          unsigned w,
                          unsigned h)
{
    zprintf(1, "%s\n", __func__);
    return(0);
}

int _zbar_processor_close (zbar_processor_t *proc)
{
    zprintf(1, "%s\n", __func__);
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
