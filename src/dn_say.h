/* ========================================================================== *
 * Copyright (c) 2015 秦凡东 (Qin Fandong)
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
 * Write line to stream
 * ========================================================================== */
#ifndef __DN_SAY_H__
#define __DN_SAY_H__

typedef enum
{
  MODE_UNKNOWN = 1,
  MODE_FILE = 1 << 1,
  MODE_OUT = 1 << 2,
} DN_SayMode_t;

typedef enum
{
  MSG_I = 0,
  MSG_W,
  MSG_E,
} DN_SayLevel_t;

/* Get write mode */
int DN_SayMode (DN_SayMode_t * const mode);
/* Write line to stream */
int DN_Say (const DN_SayMode_t mode, const DN_SayLevel_t level,
         const char * const str, ...);

#endif

