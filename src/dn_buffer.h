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
 * Ring buffer
 * ========================================================================== */
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdint.h>

#define DN_BUFFER_END_POS(b)  \
 ((b).stat.end)

#define DN_BUFFER_END_ADDR(b)  \
 (&(b).buff[(b).stat.end & ((b).stat.size - 1)])

#define DN_BUFFER_BEGIN_POS(b)  \
 ((b).stat.begin)

#define DN_BUFFER_BEGIN_ADDR(b)  \
 (&(b).buff[(b).stat.begin & ((b).stat.size - 1)])

#define DN_BUFFER_SIZE(b)  \
 ((b).stat.size)

#define DN_BUFFER_DATA_SIZE(b)  \
 ((b).stat.end - (b).stat.begin)

typedef struct DN_BuffStat
{
  /* *MUST* be 32 bits, DO NOT use unsigned int, size_t or *uint_fast32_t* */
  uint32_t size;
  /* } */
  int begin;
  int end;
} DN_BuffStat_t;

typedef struct DN_Buff
{
  char *buff;
  DN_BuffStat_t stat;
} DN_Buff_t;

/* Resize (or create) an buffer
 * Size may not be the real size of buffer, use buffer.stat.size */
int DN_ResizeBuff (DN_Buff_t * const buff, const uint32_t size);
/* Create an buffer
 * Size may not be the real size of buffer, use buffer.stat.size */
int DN_CreateBuff (DN_Buff_t * const buff, const uint32_t size);
/* Destory an buffer */
int DN_DelBuff (DN_Buff_t * const buff);
/* Write data to buffer */
int DN_WriteBuff (DN_Buff_t * const buff, const char * const data,
                  int * const size);
/* Read data from buffer */
int DN_ReadBuff (DN_Buff_t * const buff, char * const data, int * const size);

#endif

