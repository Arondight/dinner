/* ========================================================================== *
 * Copyright (c) 2015-2016 秦凡东(Qin Fandong)
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
 * I/O stream
 * ========================================================================== */
#ifndef __DN_IO_H__
#define __DN_IO_H__

#include "dn_event.h"

/* Set fd to non-block */
int DN_NoBlock (const int fd);
/* Write data from event (out) buffer to socket buffer */
int DN_Buff2Sock (DN_IOEvent_t * const ioev, const int fd, const int size);
/* Read data from socket buffer to event (in) buffer */
int DN_Sock2Buff (DN_IOEvent_t * const ioev, const int fd, const int size);

#endif

