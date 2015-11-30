/* ========================================================================== *
 * Copyright (c) 2015 秦凡东(Qin Fandong)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ========================================================================== *
 * Serve files
 * ========================================================================== */
#ifndef __DN_FILES__
#define __DN_FILES__

#include "dn_event.h"

/* Send file header */
int DN_SendFileHeader (const int fd);
/* Send file include a response header */
int DN_SendFile (const int fd, const char * const path,
                 DN_IOEvent_t * const ioev);

#endif

