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
 * I/O stream
 * ========================================================================== */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include "dn_event.h"
#include "dn_buffer.h"
#include "dn_log.h"
#include "dn_assert.h"

/* ========================================================================== *
 * I/O type for buffer, this is private
 * ========================================================================== */
typedef enum DN_IOType
{
  DN_SOCK2BUFF = 1,
  DN_BUFF2SOCK,
} DN_IOType_t;

/* ========================================================================== *
 * Make fd no blocking
 * ========================================================================== */
int
DN_NoBlock (const int fd)
{
  DN_LogMode_t mode;
  int flags;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);

  if (-1 == (flags = fcntl (fd, F_GETFL, 0)))
    {
      DN_LOG (mode, MSG_E, "fcntl failed: %s.\n", strerror (errno));
      return -1;
    }

  flags |= O_NONBLOCK;
  if (-1 == fcntl (fd, F_SETFL, flags))
    {
      DN_LOG (mode, MSG_E, "fcntl failed: %s.\n", strerror (errno));
      return -1;
    }

  return 1;
}

/* ========================================================================== *
 * I/O on event buffer, this is private
 * ========================================================================== */
int
DN_BuffIO (DN_IOEvent_t * const ioev, const int fd, const int size,
           const DN_IOType_t type)
{
  DN_LogMode_t mode;
  char *cache;
  int len, dataLen;
  int again;
  int ret;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);
  DN_ASSERT_RETURN (size > 0, "size is illegal.\n", -1);

  if (!(cache = (char *)malloc (size)))
    {
      DN_LOG (mode, MSG_E, "malloc failed %s,\n", strerror (errno));
      abort ();
    }

  do
    {
      if (DN_SOCK2BUFF == type)
        {
          len = recv (fd, cache, size, 0);
        }
      else if (DN_BUFF2SOCK == type)
        {
          dataLen = size;

          if (-1 == DN_ReadBuff (&ioev->buff.out, cache, &dataLen))
            {
              DN_LOG (mode, MSG_E, "DN_ReadBuff failed.\n");
              ret = -1;
              goto END;
            }

          if (0 == dataLen)
            {
              ret = 1;
              goto END;
            }

          len = send (fd, cache, size, 0);
        }
      else
        {
          DN_LOG (mode, MSG_E, "type is illegal.\n");
          ret = -1;
          goto END;
        }

      if (-1 == len)
        {
          if (EWOULDBLOCK == errno || EAGAIN == errno || EINTR == errno)
            {
              break;
            }

          DN_LOG (mode, MSG_E, "%s failed: %s.\n",
                                DN_SOCK2BUFF == type ? "recv" : "send",
                                strerror (errno));
          ret = -1;
          goto END;
        }
      else if (0 == len)
        {
          /* Socket has closed */
          break;
        }
      else if (len == size)
        {
          again = 1;
        }
      else
        {
          again = 0;
        }

      dataLen = len;

      if (DN_SOCK2BUFF == type)
        {
          if (-1 == DN_WriteBuff (&ioev->buff.in, cache, &dataLen))
            {
              DN_LOG (mode, MSG_E, "DN_WriteBuff failed.\n");
              ret = -1;
              goto END;
            }
          if (dataLen < len)
            {
              DN_LOG (mode, MSG_E, "DN_WriteBuff: no more space to write.\n");
              /* Read for socket buffer has finished,
               * but event buffer has no more space */
              ret = 0;
              goto END;
            }
        }
    }
  while (again);

  ret = 1;

END:
  if (cache)
    {
      free (cache);
      cache = NULL;
    }

  return ret;
}

/* ========================================================================== *
 * Read data from socket buffer to event (in) buffer
 * ========================================================================== */
int
DN_Sock2Buff (DN_IOEvent_t * const ioev, const int fd, const int size)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);
  DN_ASSERT_RETURN (size > 0, "size is illegal.\n", -1);

  if (-1 == DN_BuffIO (ioev, fd, size, DN_SOCK2BUFF))
    {
      DN_LOG (mode, MSG_E, "DN_IOBuff failed.\n");
      return -1;
    }

  return 0;
}

/* ========================================================================== *
 * Write data from event (out) buffer to socket buffer
 * ========================================================================== */
int
DN_Buff2Sock (DN_IOEvent_t * const ioev, const int fd, const int size)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);
  DN_ASSERT_RETURN (size > 0, "size is illegal.\n", -1);

  if (-1 == DN_BuffIO (ioev, fd, size, DN_BUFF2SOCK))
    {
      DN_LOG (mode, MSG_E, "DN_IOBuff failed.\n");
      return -1;
    }

  return 0;
}

