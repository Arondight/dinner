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
 * Deal requests
 * ========================================================================== */
#ifndef __DN_REQUEST_H__
#define __DN_REQUEST_H__

#include "dn_event.h"

typedef enum DN_RequsetType
{
  DN_RequestType_UNKOWN = 0,
  DN_RequestType_POST,
  DN_RequestType_GET,
} DN_RequestType_t;

/* Send meesage of a unkown (and unimplemented) request */
int DN_ErrRequest (const int fd);
/* Discard every thiDN_IOEvent_t *ioev, ng of a request */
int DN_DiscardRequest (const int fd);
/* Accept a request */
int DN_AcceptRequest (const int epfd, const int fd, const int events,
                      DN_IOEvent_t * const ioev, void const * arg);

#endif

