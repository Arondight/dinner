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
#ifndef __DN_RESPONSE_H__
#define __DN_RESPONSE_H__

/* Default HTML start */
#define DN_DEFAULT_HTML_START \
  "<!DOCTYPE html>\r\n" \
  "<html>\r\n"  \

/* Default HTML end */
#define DN_DEFAULT_HTML_END \
  "</html>\r\n"

/* Default HTML meta */
#define DN_DEFAULT_HTML_META \
  "<meta charset=utf-8 />\r\n"

/* Default content type */
#define DN_DEFAULT_CONTENT_TYPE \
  "Content-Type: text/html\r\n"

/* Default CSS */
#define DN_DEFAULT_HTML_CSS  \
  "<style type=\"text/css\">\r\n" \
  "body {\r\n"  \
  "  background-color: #ECECEC;\r\n"  \
  "  font-family: 'Open Sans', sans-serif;\r\n" \
  "  font-size: 14px;\r\n"  \
  "  color: #3c3c3c;\r\n" \
  "}\r\n" \
  "\r\n"  \
  ".style p:first-child {\r\n"  \
  "  text-align: center;\r\n"   \
  "  font-family: cursive;\r\n" \
  "  font-size: 150px;\r\n" \
  "  font-weight: bold;\r\n"  \
  "  line-height: 100px;\r\n" \
  "  letter-spacing: 5px;\r\n"  \
  "  color: #fff;\r\n"  \
  "}\r\n" \
  "\r\n"  \
  ".style p:first-child span {\r\n" \
  "  cursor: pointer;\r\n"  \
  "  text-shadow: 0px 0px 2px #686868,\r\n" \
  "    0px 1px 1px #ddd,\r\n" \
  "    0px 2px 1px #d6d6d6,\r\n"  \
  "    0px 3px 1px #ccc,\r\n" \
  "    0px 4px 1px #c5c5c5,\r\n"  \
  "    0px 5px 1px #c1c1c1,\r\n"  \
  "    0px 6px 1px #bbb,\r\n" \
  "    0px 7px 1px #777,\r\n" \
  "    0px 8px 3px rgba(100, 100, 100, 0.4),\r\n" \
  "    0px 9px 5px rgba(100, 100, 100, 0.1),\r\n" \
  "    0px 10px 7px rgba(100, 100, 100, 0.15),\r\n" \
  "    0px 11px 9px rgba(100, 100, 100, 0.2),\r\n"  \
  "    0px 12px 11px rgba(100, 100, 100, 0.25),\r\n"  \
  "    0px 13px 15px rgba(100, 100, 100, 0.3);\r\n" \
  "  -webkit-transition: all .1s linear;\r\n" \
  "  transition: all .1s linear;\r\n" \
  "}\r\n" \
  "\r\n"  \
  ".style p:first-child span:hover {\r\n" \
  "  text-shadow: 0px 0px 2px #686868,\r\n" \
  "    0px 1px 1px #fff,\r\n" \
  "    0px 2px 1px #fff,\r\n" \
  "    0px 3px 1px #fff,\r\n" \
  "    0px 4px 1px #fff,\r\n" \
  "    0px 5px 1px #fff,\r\n" \
  "    0px 6px 1px #fff,\r\n" \
  "    0px 7px 1px #777,\r\n" \
  "    0px 8px 3px #fff,\r\n" \
  "    0px 9px 5px #fff,\r\n" \
  "    0px 10px 7px #fff,\r\n"  \
  "    0px 11px 9px #fff,\r\n"  \
  "    0px 12px 11px #fff,\r\n" \
  "    0px 13px 15px #fff;\r\n" \
  "  -webkit-transition: all .1s linear;\r\n" \
  "  transition: all .1s linear;\r\n" \
  "}\r\n" \
  "\r\n"  \
  ".style p:not(:first-child) {\r\n"  \
  "  text-align: center;\r\n" \
  "  color: #666;\r\n"  \
  "  font-family: cursive;\r\n" \
  "  font-size: 20px;\r\n"  \
  "  text-shadow: 0 1px 0 #fff;\r\n"  \
  "  letter-spacing: 1px;\r\n"  \
  "  line-height: 2em;\r\n" \
  "  margin-top: -50px;\r\n"  \
  "}\r\n" \
  "</style>\r\n"

typedef enum DN_ResponseType
{
  DN_RUNCGI = 1,
  DN_SENDFILE,
  DN_SEND200,
  DN_SEND200CGI,
  DN_SEND400,
  DN_SEND404,
} DN_ResponseType_t;

typedef struct DN_ResponseInfo
{
  DN_ResponseType_t type;
  void *arg;   /* For extra data */
} DN_ResponseInfo_t;

/* Send 404 response */
int DN_ResponsePage (const int fd, const DN_ResponseType_t type);
/* Send response */
int DN_SendResponse (const int epfd, const int fd, const int events,
                     void * const ioev, void * const arg);

#endif

