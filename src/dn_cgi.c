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
 * Run CGI
 * ========================================================================== */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "dn_request.h"
#include "dn_cgi.h"
#include "dn_io.h"
#include "dn_assert.h"
#include "dn_log.h"

/* ========================================================================== *
 * Exec a CGI program
 * ========================================================================== */
int
DN_ExecCGI (const int fd, DN_CGIInfo_t * const info, DN_IOEvent_t * const ioev)
{
  DN_LogMode_t mode;
  int cgiIn[2];
  int cgiOut[2];
  char envRqst[1 << 5];
  char envQueryStr[1 << 14];
  char envContentLen[1 << 14];
  char *str;
  pid_t pid;
  int status;
  int size, strLen, contentLen;
  int index;
  int ret;
  /* FIXME: Will content length behind with a certain space { */
  const char contentLenStr[] = "Content-Length: ";
  /* } */
  const int contentLenStrLen = strlen (contentLenStr);
  const int envRqstLen = 1 << 5;
  const int envMaxLen = 1 << 14;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);
  DN_ASSERT_RETURN (info, "info is NULL.\n", -1);
  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (info->path, "info->path is NULL.\n", -1);
  DN_ASSERT_RETURN (info->args, "info->args is NULL.\n", -1);

  contentLen = 0;
  str = NULL;
  memset (envRqst, 0, envRqstLen);
  memset (cgiIn, 0, sizeof (cgiIn));
  memset (cgiOut, 0, sizeof (cgiOut));

  /* Check request type */
  switch (info->request)
    {
    case DN_RequestType_GET:
      if (-1 == DN_DiscardRequest (fd))
        {
          DN_LOG (mode, MSG_E, "DN_DiscardRequest failed.\n");
          ret = 0;
          goto FINISH;
        }
      strncpy (envRqst, "REQUEST_METHOD=GET", envRqstLen);
      break;

    case DN_RequestType_POST:
      /* Here we have read all HTTP head to event buffer */
      if (!(str = (char *)memmem (DN_BUFFER_BEGIN_ADDR (ioev->buff.in),
                                  DN_BUFFER_DATA_SIZE(ioev->buff.in),
                                  contentLenStr, contentLenStrLen)))
        {
          if (-1 == DN_ResponsePage (fd, DN_SEND400))
            {
              DN_LOG (mode, MSG_E, "DN_ResponsePage failed.\n");
              ret = 0;
              goto FINISH;
            }
        }
      else
        {
          str += contentLenStrLen;
        }
      strncpy (envRqst, "REQUEST_METHOD=POST", envRqstLen);
      break;

    case DN_RequestType_UNKOWN:
      /* Fall through */
    default:
      DN_LOG (mode, MSG_E, "unkown request type.\n");
      ret = -1;
      goto FINISH;
    }

  /* Get content length */
  if (DN_RequestType_POST == info->request)
    {
      for (index = 0;
           !isspace (str[index])
              && index < DN_BUFFER_END_ADDR (ioev->buff.in) - str;
           ++index)
        {
          /* Do nothing */
        }

      ret = str[index];
      str[index] = 0;
      contentLen = atoi (str);
      str[index] = ret;
      str = &str[index];

      /* Discard HTTP header in event buffer */
      if (!(str = (char *)memmem (str, DN_BUFFER_END_ADDR (ioev->buff.in) - str,
                                  DN_HTTP_HEADER_END_MARK,
                                  DN_HTTP_HEADER_END_MARK_LEN)))
        {
          DN_LOG (mode, MSG_E, "can not find end mark of HTTP header.\n");
          goto FINISH;
        }

      str += DN_HTTP_HEADER_END_MARK_LEN;
      ioev->buff.in.stat.begin = str - ioev->buff.in.buff;
    }

  if (-1 == DN_ResponsePage (fd, DN_SEND200CGI))
    {
      DN_LOG (mode, MSG_E, "DN_ResponsePage failed.\n");
      ret = -1;
      goto FINISH;
    }

  if (-1 == pipe (cgiIn))
    {
      DN_LOG (mode, MSG_E, "pipe failed: %s.\n", strerror (errno));
      ret = -1;
      goto FINISH;
    }

  if (-1 == pipe (cgiOut))
    {
      DN_LOG (mode, MSG_E, "pipe failed: %s.\n", strerror (errno));
      ret = -1;
      goto FINISH;
    }

  ret = 1;

  if ((pid = vfork ()) < 0)
    {
      DN_LOG (mode, MSG_E, "vfork failed: %s.\n", strerror (errno));
      ret = -1;
      goto FINISH;
    }

  /* Child */
  if (!pid)
    {
      dup2 (cgiOut[1], STDOUT_FILENO);
      dup2 (cgiIn[0], STDIN_FILENO);
      close (cgiOut[0]);
      close (cgiOut[1]);

      putenv (envRqst);

      if (DN_RequestType_GET == info->request)
        {
          snprintf (envQueryStr, envMaxLen, "QUERY_STRING=%s", info->args);
          putenv (envQueryStr);
        }
      else
        {
          snprintf (envContentLen, envMaxLen, "CONTENT_LENGTH=%d", contentLen);
          putenv (envContentLen);
        }

      execl (info->path, info->path, NULL);
    }

  /* Parent */
  close (cgiOut[1]);
  close (cgiIn[0]);

  strLen = DN_BUFFER_SIZE (ioev->buff.out);

  if (!(str = (char *)malloc (strLen)))
    {
      DN_LOG (mode, MSG_E, "malloc failed: %s.\n", strerror (errno));
      abort ();
    }

  /* Write body to pipe if request is POST */
  if (DN_RequestType_POST == info->request)
    {
      size = DN_BUFFER_DATA_SIZE (ioev->buff.in);

      if (-1 == DN_ReadBuff (&ioev->buff.in, str, &size))
        {
          DN_LOG (mode, MSG_E, "DN_ReadBuff failed.\n");
          ret = -1;
          goto FINISH;
        }

      contentLen -= size;

      if (-1 == write (cgiIn[1], str, size))
        {
          DN_LOG (mode, MSG_E, "write failed: %s.\n", strerror (errno));
          ret = -1;
          goto FINISH;
        }

      while (contentLen > 0)
        {
          if (-1 == (ret = DN_Sock2Buff (ioev, fd, DN_BUFFER_SIZE (ioev->buff.in))))
            {
              DN_LOG (mode, MSG_E, "DN_Sock2Buff failed.\n");
              ret = -1;
              goto FINISH;
            }

          size = DN_BUFFER_DATA_SIZE (ioev->buff.in);

          if (-1 == DN_ReadBuff (&ioev->buff.in, str, &size))
            {
              DN_LOG (mode, MSG_E, "DN_ReadBuff failed.\n");
              ret = -1;
              goto FINISH;
            }

          contentLen -= size;

          if (-1 == write (cgiIn[1], str, size))
            {
              DN_LOG (mode, MSG_E, "write failed: %s.\n", strerror (errno));
              ret = -1;
              goto FINISH;
            }

          /* If buffer is full, but we can not find end flag of HTTP head, quit */
          if (!ret)
            {
              return 0;
            }
        }
    }

  /* Write output of CGI to socket buffer */
  while ((size = read (cgiOut[0], str, strLen)) > 0)
    {
      if (-1 == DN_WriteBuff (&ioev->buff.out, str, &size))
        {
          DN_LOG (mode, MSG_E, "DN_WriteBuff failed.\n");
          ret = -1;
          goto FINISH;
        }

      if (-1 == DN_Buff2Sock (ioev, fd, size))
        {
          DN_LOG (mode, MSG_E, "DN_Buff2Sock failed.\n");
          ret = -1;
          goto FINISH;
        }
    }

  waitpid (pid, &status, 0);

FINISH:
  if (cgiOut[0])
    {
      close (cgiOut[0]);
    }
  if (cgiIn[1])
    {
      close (cgiIn[1]);
    }

  if (str)
    {
      free (str);
      str = NULL;
    }
  if (info->path)
    {
      free (info->path);
      info->path = NULL;
    }
  if (info->args)
    {
      free (info->args);
      info->args = NULL;
    }

  return ret;
}

