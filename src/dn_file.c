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
 * Serve file
 * ========================================================================== */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "dn_event.h"
#include "dn_io.h"
#include "dn_request.h"
#include "dn_log.h"
#include "dn_assert.h"
#include "dn_config.h"

/* ========================================================================== *
 * Send file include a response header
 * ========================================================================== */
int
DN_SendFile (const int fd, const char * const path, DN_IOEvent_t * const ioev)
{
  FILE *fh;
  char cache[DN_DEFAULT_BUFFER_LEN];
  DN_LogMode_t mode;
  int len, writeLen;
  int ret;
  int again;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);
  DN_ASSERT_RETURN (path, "path is NULL.\n", -1);

  /* Discard all data in socket buffer, or a RST will be seed */
  if (-1 == DN_DiscardRequest (fd))
    {
      DN_LOG (mode, MSG_E, "DN_DiscardRequest failed.\n");
      /* Do nothing */
    }

  /* Send response header */
  if (-1 == DN_ResponsePage (fd, DN_SEND200))
    {
      DN_LOG (mode, MSG_E, "DN_ResponsePage failed.\n");
      return -1;
    }

  if (!(fh = fopen (path, "r")))
    {
      DN_LOG (mode, MSG_E, "fopen failed: %s.\n", strerror (errno));
      return -1;
    }

  DN_LOG (mode, MSG_I, "serve file \"%s\".\n", path);

  /* Then send context of file */
  again = 1;
  do
    {
      if ((len = fread (cache, 1, sizeof (cache), fh)) < sizeof (cache))
        {
          again = 0;
        }

      writeLen = len;
      while (1)
        {
          if (-1 == DN_WriteBuff (&ioev->buff.out, cache, &writeLen))
            {
              DN_LOG (mode, MSG_E, "DN_WriteBuff failed.\n");
              ret = -1;
              goto END;
            }

          /* Write all data of buffer to socket buffer */
          if (-1 == DN_Buff2Sock (ioev, fd, writeLen))
            {
              DN_LOG (mode, MSG_E, "DN_Buff2Sock failed.\n");
              ret = -1;
              goto END;
            }

          len -= writeLen;
          writeLen = len;

          /* Break if no more data to write */
          if (!writeLen)
            {
              break;
            }
        }
    }
  while (again);

  ret = 1;

END:
  if (fh)
    {
      fclose (fh);
    }

  return ret;
}

