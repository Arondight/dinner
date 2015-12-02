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
 * Warpper for say ()
 * ========================================================================== */
#ifndef __DN_LOG_H__
#define __DN_LOG_H__

#include "libgen.h"
#include "dn_say.h"
#include "dn_config.h"

#ifndef DN_DEBUG
  #include <"DN_DEBUG" is needed but not defined>
#endif
#ifdef DN_DEBUG_LOG
  #include <"DN_DEBUG_LOG" has a duplicate definition>
#endif
#define DN_DEBUG_LOG (DN_DEBUG)

typedef DN_SayMode_t DN_LogMode_t;

#define DN_LOGMODE(p) (DN_SayMode (p))
#define DN_TRUELOGMODE(p) (DN_GetSayMode (p))

#if DN_DEBUG_LOG
  #define DN_LOG(m,t,f,...)   \
    ((DN_Say ((m), (t), "%4d of %s:\t(%s) - ",  \
                        __LINE__, basename (__FILE__), __FUNCTION__)),  \
     (DN_Say ((m), (t), (f), ##__VA_ARGS__)))
#else
  #define DN_LOG(m,t,f,...)   \
    (DN_Say ((m), (t), (f), ##__VA_ARGS__))
#endif

#endif

