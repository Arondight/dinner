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
 * Run CGI
 * ========================================================================== */
#ifndef __DN_CGI__
#define __DN_CGI__

#include "dn_event.h"
#include "dn_request.h"

typedef struct DN_CGIInfo
{
  char *path;
  char *args;
  DN_RequestType_t request;
} DN_CGIInfo_t;

/* Exec a CGI program */
int DN_ExecCGI (const int fd, DN_CGIInfo_t * const info,
                DN_IOEvent_t * const ioev);

#endif

