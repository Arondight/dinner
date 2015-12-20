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
 * Send response
 * ========================================================================== */
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "dn_event.h"
#include "dn_file.h"
#include "dn_cgi.h"
#include "dn_request.h"
#include "dn_response.h"
#include "dn_log.h"
#include "dn_assert.h"
#include "dn_config.h"

/* Default 200 response header for CGI.
 * No content type and HTTP end mark here*/
static const char DN_Page200ForCGI[] =
(
  "HTTP/1.0 200 OK\r\n"
  DN_SERVER_RESPONSE_STR
);

/* Default 200 response header */
static const char DN_Page200Header[] =
(
  "HTTP/1.0 200 OK\r\n"
  DN_SERVER_RESPONSE_STR
  DN_DEFAULT_CONTENT_TYPE
  DN_HTTP_HEADER_END_MARK
);

/* Default 404 response */
static const char DN_Page404[] =
(
  "HTTP/1.0 404 NOT FOUND\r\n"
  DN_SERVER_RESPONSE_STR
  DN_DEFAULT_CONTENT_TYPE
  DN_HTTP_HEADER_END_MARK
  DN_DEFAULT_HTML_START
  "<head>\r\n"
  DN_DEFAULT_HTML_CSS
  DN_DEFAULT_HTML_META
  "<title>404 NOT FOUND</title>\r\n"
  "</head>\r\n"
  "<body>\r\n"
  "  <div class=\"style\">\r\n"
  "    <p><span>4</span><span>0</span><span>4</span></p>\r\n"
  "    <p>The requested file was not found on this server.</p>\r\n"
  "  </div>\r\n"
  "</body>\r\n"
  DN_DEFAULT_HTML_END
);

/* Default 400 response */
static const char DN_Page400[] =
(
  "HTTP/1.0 400 BAD REQUEST\r\n"
  DN_SERVER_RESPONSE_STR
  DN_DEFAULT_CONTENT_TYPE
  DN_HTTP_HEADER_END_MARK
  DN_DEFAULT_HTML_START
  "<head>\r\n"
  DN_DEFAULT_HTML_CSS
  DN_DEFAULT_HTML_META
  "<title>400 BAD REQUEST</title>\r\n"
  "</head>\r\n"
  "<body>\r\n"
  "  <div class=\"style\">\r\n"
  "    <p><span>4</span><span>0</span><span>0</span></p>\r\n"
  "    <P>Your browser sent a bad request,"
  "    such as a POST without a Content-Length.</p>\r\n"
  "  </div>\r\n"
  "</body>\r\n"
  DN_DEFAULT_HTML_END
);

/* ========================================================================== *
 * Finish an HTTP connection
 * ========================================================================== */
int
DN_FinishHttp (const int epfd, DN_IOEvent_t * const ioev)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (epfd > -1, "epfd is illegal.\n", -1);
  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);

  if (-1 == DN_DelEvent (epfd, ioev))
    {
      DN_LOG (mode, MSG_E, "DN_DelEvent failed.\n");
      /* XXX: Determine what to do here.
       * If DN_DestoryDevent really should to invoke below ? */
    }

  if (-1 == DN_DestoryEvent (ioev))
    {
      DN_LOG (mode, MSG_E, "DN_DestoryEvent failed.\n");
      return -1;
    }

  return 1;
}

/* ========================================================================== *
 * Send 404 response
 * ========================================================================== */
int
DN_ResponsePage (const int fd, const DN_ResponseType_t type)
{
  DN_LogMode_t mode;
  int pos;
  int len, dataLen, contextLen;
  const char * context;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);

  if (DN_SEND200 == type)
    {
      context = DN_Page200Header;
    }
  else if (DN_SEND200CGI == type)
    {
      context  = DN_Page200ForCGI;
    }
  else if (DN_SEND400 == type)
    {
      context = DN_Page400;
    }
  else if (DN_SEND404 == type)
    {
      context = DN_Page404;
    }
  else
    {
      DN_LOG (mode, MSG_E, "unkown reseponse page.\n");
      return 0;
    }

  /* Discard all data in socket buffer, or a RST will be seed */
  if (-1 == DN_DiscardRequest (fd))
    {
      DN_LOG (mode, MSG_E, "DN_DiscardRequest failed.\n");
      /* Do nothing */
    }

  contextLen = strlen (context);

  /* Here invoke send directly without using I/O buffer of event.
   * Becase we do not need a buffer here */
  pos = 0;
  dataLen = contextLen - pos;
  while (1)
    {
      if (-1 == (len = send (fd, &context[pos], dataLen, 0)))
        {
          /* Blocking untill everything has send to socket buffer  */
          if (EWOULDBLOCK != errno && EAGAIN != errno && EINTR != errno)
            {
              return -1;
            }
        }

      pos += len;
      dataLen = contextLen - pos;

      if (dataLen < 1)
        {
          break;
        }
    }

  return 1;
}

/* ========================================================================== *
 * Send response
 * ========================================================================== */
int
DN_SendResponse (const DN_IOEventHandlerArgs_t args)
{
  DN_IOEvent_t *ev;
  DN_ResponseType_t action;
  DN_LogMode_t mode;
  int epfd, fd;
  int ret;

  DN_LOGMODE (&mode);

  ev = args.ioev;
  epfd = args.epfd;
  fd = args.fd;

  DN_ASSERT_RETURN (ev, "ev is NULL.\n", -1);
  DN_ASSERT_RETURN (epfd > -1, "epfd is illegal.\n", -1);
  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);

  action = ev->info.type;

  switch (action)
    {
    case DN_RUNCGI:
      if (-1 == DN_ExecCGI (fd, (DN_CGIInfo_t *)ev->info.arg, ev))
        {
          DN_LOG (mode, MSG_E, "DN_ExecCGI failed.\n");
          ret = -1;
          goto FINISH;
        }
      break;

    case DN_SENDFILE:
      if (-1 == DN_SendFile (fd, (char *)ev->info.arg, ev))
        {
          DN_LOG (mode, MSG_E, "DN_SendFileHeader failed.\n");
          ret = -1;
          goto FINISH;
        }
      break;

    case DN_SEND200:
      if (-1 == DN_ResponsePage (fd, DN_SEND200))
        {
          DN_LOG (mode, MSG_E, "DN_ResponsePage failed.\n");
          ret = -1;
          goto FINISH;
        }
      break;

    case DN_SEND200CGI:
      if (-1 == DN_ResponsePage (fd, DN_SEND200CGI))
        {
          DN_LOG (mode, MSG_E, "DN_ResponsePage failed.\n");
          ret = -1;
          goto FINISH;
        }
      break;

    case DN_SEND400:
      if (-1 == DN_ResponsePage (fd, DN_SEND400))
        {
          DN_LOG (mode, MSG_E, "DN_ResponsePage failed.\n");
          ret = -1;
          goto FINISH;
        }
      break;

    case DN_SEND404:
      if (-1 == DN_ResponsePage (fd, DN_SEND404))
        {
          DN_LOG (mode, MSG_E, "DN_ResponsePage failed.\n");
          ret = -1;
          goto FINISH;
        }
      break;

    default:
      DN_LOG (mode, MSG_E, "unkown response type.\n");
      ret = -1;
      goto FINISH;
    }

  ret = 1;

FINISH:
  if (-1 == DN_FinishHttp (epfd, ev))
    {
      DN_LOG (mode, MSG_E, "DN_FinishHttp failed.\n");
      return -1;
    }

  return ret;
}

