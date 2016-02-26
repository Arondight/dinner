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
 * Deal requests
 * ========================================================================== */
/* For memmem { */
#define _GNU_SOURCE
/* } */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "dn_io.h"
#include "dn_file.h"
#include "dn_cgi.h"
#include "dn_event.h"
#include "dn_request.h"
#include "dn_response.h"
#include "dn_log.h"
#include "dn_assert.h"

/* ========================================================================== *
 * Discard every thing of a request
 * ========================================================================== */
int
DN_DiscardRequest (const int fd)
{
  DN_LogMode_t mode;
  char cache[DN_BUFFER_LEN];

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);

  /* Discard whole request */
  while (1)
    {
      if (-1 == recv (fd, cache, sizeof (cache), 0))
        {
          break;    /* If EWOULDBLOCK or EAGAIN, break */
        }
    }

  return 1;
}

/* ========================================================================== *
 * Accept a request
 * Return 0 if can not get whole HTTP head.
 * ========================================================================== */
int
DN_AcceptRequest (const DN_IOEventHandlerArgs_t args)
{
  char request[DN_DEFAULT_MAX_REQUEST_LEN];
  char url[DN_DEFAULT_BUFFER_LEN];
  char path[MAXPATHLEN];
  DN_IOEvent_t *ioev;
  DN_CGIInfo_t *info;
  DN_RequestType_t type;
  DN_LogMode_t mode;
  struct stat status;
  int epfd, fd;
  int cgi;
  int buffLen, urlLen, pathLen;
  int requestLen, urlPos;
  int ret;

  DN_LOGMODE (&mode);

  ioev = args.ioev;
  epfd = args.epfd;
  fd = args.fd;

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (epfd > -1, "epfd is illegal.\n", -1);
  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);

  while (1)
    {
      if (-1 == (ret = DN_Sock2Buff (ioev, fd, DN_BUFFER_LEN)))
        {
          DN_LOG (mode, MSG_E, "DN_Sock2Buff failed.\n");
          return -1;
        }

      /* Continue reading untill we read whole HTTP request head.
       * It is ok here, before invoke DN_ReadBuff, begin is always 0 */
      if (memmem (DN_BUFFER_BEGIN_ADDR (ioev->buff.in),
                  DN_BUFFER_DATA_SIZE (ioev->buff.in),
                  DN_HTTP_HEADER_END_MARK, DN_HTTP_HEADER_END_MARK_LEN))
        {
          break;
        }

      /* If buffer is full, but we can not find end flag of HTTP head, quit */
      if (!ret)
        {
          return 0;
        }
    }

  buffLen = DN_BUFFER_END_POS (ioev->buff.in);

  /* Get request method */
  memset (request, 0, sizeof (request));
  for (requestLen = DN_BUFFER_BEGIN_POS (ioev->buff.in);
       !isspace (ioev->buff.in.buff[requestLen]) && requestLen < buffLen;
       ++requestLen)
    {
      /* Do noting */
    }
  if (-1 == DN_ReadBuff (&ioev->buff.in, request, &requestLen))
    {
      DN_LOG (mode, MSG_E, "DN_ReadBuff failed.\n");
      return -1;
    }

  /* Strip space after request method */
  for (urlLen = DN_BUFFER_BEGIN_POS (ioev->buff.in);
       isspace (ioev->buff.in.buff[urlLen]) && urlLen < buffLen;
      ++urlLen)
    {
      /* Do noting */
    }
  urlLen -= DN_BUFFER_BEGIN_POS (ioev->buff.in);
  if (-1 == DN_ReadBuff (&ioev->buff.in, url, &urlLen))
    {
      DN_LOG (mode, MSG_E, "DN_ReadBuff failed.\n");
      return -1;
    }

  /* Get request url */
  memset (url, 0, sizeof (url));
  for (urlLen = DN_BUFFER_BEGIN_POS (ioev->buff.in);
       !isspace (ioev->buff.in.buff[urlLen]) && urlLen < buffLen;
       ++urlLen)
    {
      /* Do nothing */
    }
  urlLen -= DN_BUFFER_BEGIN_POS (ioev->buff.in);
  if (-1 == DN_ReadBuff (&ioev->buff.in, url, &urlLen))
    {
      DN_LOG (mode, MSG_E, "DN_ReadBuff failed.\n");
      return -1;
    }

  /* Handle request */
  cgi = 0;
  urlPos = 0;

  /* GET */
  if (!strncasecmp ("get", request, 3))
    {
      if (DN_DEFAULT_BUFFER_LEN < (urlLen = strlen (url)))
        {
          DN_LOG (mode, MSG_E, "array out of bounds");
          return -1;
        }

      for (urlPos = 0; (urlPos < urlLen) && ('?' != url[urlPos]); ++urlPos)
        {
          /* Do nothing */
        }

      if ('?' == url[urlPos])
        {
          cgi = 1;
          url[urlPos] = 0;
        }

      type = DN_RequestType_GET;
    }
  /* POST */
  else if (!strncasecmp ("post", request, 4))
    {
      cgi = 1;
      type = DN_RequestType_POST;
    }
  /* Unimplemented method */
  else
    {
      ioev->info.type = DN_SEND400;
      goto FINISH;
    }

  /* Make path */
  strncpy (path, &url[1], MAXPATHLEN);

  pathLen = strlen (path);

  if (!pathLen || (pathLen > 0 && '/' == path[pathLen - 1]))
    {
      strncat (path, "index.html", MAXPATHLEN);
    }

  /* 404 */
  if (-1 == stat (path, &status))
    {
      ioev->info.type = DN_SEND404;
      goto FINISH;
    }

  if (S_IFDIR == (status.st_mode & S_IFMT))
    {
      strncat (path, "/index.html", MAXPATHLEN);
    }

  if ((status.st_mode & S_IXUSR) || (status.st_mode & S_IXGRP)
                                 || (status.st_mode & S_IXOTH))
    {
      cgi = 1;
    }

  if (cgi)
    {

      if (!(info = (DN_CGIInfo_t *)malloc (sizeof (DN_CGIInfo_t))))
        {
          DN_LOG (mode, MSG_E, "malloc failed: %s.\n", strerror (errno));
          abort ();
        }
      info->path = strdup (path);
      info->args = strdup (&url[urlPos + 1]);
      info->request = type;
      ioev->info.type = DN_RUNCGI;
      ioev->info.arg = info;
    }
  else
    {
      ioev->info.type = DN_SENDFILE;
      ioev->info.arg = strdup (path);
    }

FINISH:
  if (-1 == DN_SetEvent (ioev, (DN_IOEventHandler)DN_SendResponse))
    {
      DN_LOG (mode, MSG_E, "DN_SetEvent failed.\n");
      return -1;
    }

  if (-1 == DN_AddEvent (epfd, EPOLLOUT, ioev))
    {
      DN_LOG (mode, MSG_E, "DN_AddEvent failed.\n");
      return -1;
    }

  return 1;
}

