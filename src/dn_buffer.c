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
#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "dn_buffer.h"
#include "dn_log.h"
#include "dn_assert.h"

/* ========================================================================== *
 * Adjust size to it's next power of 2
 * This is private
 * ========================================================================== */
int
DN_BuffSizeAdjust (uint32_t * const size)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (size, "size is NULL.\n", -1);

  --(*size);
  *size |= *size >> 1;
  *size |= *size >> 2;
  *size |= *size >> 4;
  *size |= *size >> 8;
  *size |= *size >> 16;
  ++(*size);

  return 1;
}

/* ========================================================================== *
 * Resize (or create) an buffer
 * Size may not be the real size of buffer, use buffer.stat.size
 * ========================================================================== */
int
DN_ResizeBuff (DN_Buff_t * const buff, const uint32_t size)
{
  DN_LogMode_t mode;
  uint32_t buffSize;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (buff, "buff is NULL.\n", -1);

  buffSize = size;

  DN_BuffSizeAdjust (&buffSize);

  /* buff->buff can be NULL here */
  if (!(buff->buff = (char *)realloc (buff->buff, size)))
    {
      DN_LOG (mode, MSG_E, "realloc failed: %s.\n", strerror (errno));
      return -1;
    }

  buff->stat.size = buffSize;
  buff->stat.begin = buff->stat.end = 0;

  return 1;
}

/* ========================================================================== *
 * Create an buffer
 * Size may not be the real size of buffer, use buffer.stat.size
 * ========================================================================== */
int
DN_CreateBuff (DN_Buff_t * const buff, const uint32_t size)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (buff, "buff is NULL.\n", -1);

  if (-1 == DN_ResizeBuff (buff, size))
    {
      DN_LOG (mode, MSG_E, "DN_ResizeBuff failed.\n");
      return -1;
    }

  return 1;
}

/* ========================================================================== *
 * Destory an buffer
 * ========================================================================== */
int
DN_DelBuff (DN_Buff_t * const buff)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (buff, "buff is NULL.\n", -1);

  if (buff->buff)
    {
      free (buff->buff);
    }

  memset (buff, 0, sizeof (DN_Buff_t));

  return 1;
}

/* ========================================================================== *
 * Write data to buffer
 * ========================================================================== */
int
DN_WriteBuff (DN_Buff_t * const buff, const char * const data, int * const size)
{
  DN_LogMode_t mode;
  int dataSize;
  int writeSpace, tailSpaceUsed;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (buff, "buff is NULL.\n", -1);
  DN_ASSERT_RETURN (data, "data is NULL.\n", -1);
  DN_ASSERT_RETURN (size, "size is NULL.\n", -1);
  DN_ASSERT_RETURN (*size > -1, "size is illegal.\n", -1);

  dataSize = *size;

  if (buff->stat.size == buff->stat.end - buff->stat.begin)
    {
      return 0;
    }

  writeSpace = buff->stat.size - buff->stat.end + buff->stat.begin;
  writeSpace = writeSpace < dataSize ? writeSpace : dataSize;

  tailSpaceUsed = buff->stat.size - (buff->stat.end & (buff->stat.size - 1));
  tailSpaceUsed = tailSpaceUsed < writeSpace ? tailSpaceUsed : writeSpace;

  /* Write to tail space */
  memcpy (buff->buff + (buff->stat.end & (buff->stat.size - 1)), data,
          tailSpaceUsed);

  /* Write to head space (if needed) */
  memcpy (buff->buff, data + tailSpaceUsed, writeSpace - tailSpaceUsed);

  buff->stat.end += writeSpace;

  *size = writeSpace;

  return 1;
}

/* ========================================================================== *
 * Read data from buffer
 * ========================================================================== */
int
DN_ReadBuff (DN_Buff_t * const buff, char * const data, int * const size)
{
  DN_LogMode_t mode;
  int dataSize;
  int readSpace, readSpaceUsed;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (buff, "buff is NULL.\n", -1);
  DN_ASSERT_RETURN (data, "data is NULL.\n", -1);
  DN_ASSERT_RETURN (size, "size is NULL.\n", -1);
  DN_ASSERT_RETURN (*size > -1, "size is illegal.\n", -1);

  dataSize = *size;

  readSpace = buff->stat.end - buff->stat.begin;
  readSpace = readSpace < dataSize ? readSpace : dataSize;

  readSpaceUsed = buff->stat.size - (buff->stat.begin & (buff->stat.size - 1));
  readSpaceUsed = readSpaceUsed < readSpace ? readSpaceUsed : readSpace;

  /* Read data space */
  memcpy (data, buff->buff + (buff->stat.begin & (buff->stat.size - 1)),
          readSpaceUsed);

  /* Read head space (if needed)
   * If buff.stat.end % buff.stat.size < buff.stat.begin then we need do this
   */
  memcpy (data + readSpaceUsed, buff->buff, readSpace - readSpaceUsed);

  buff->stat.begin += readSpace;

  if (buff->stat.begin == buff->stat.end)
    {
      buff->stat.begin = buff->stat.end = 0;
    }

  *size = readSpace;

  return 1;
}

