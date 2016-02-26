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
 * I/O Event
 * ========================================================================== */
#ifndef __DN_EVENT_H__
#define __DN_EVENT_H__

#include <time.h>
#include <sys/epoll.h>
#include "dn_buffer.h"
#include "dn_response.h"
#include "dn_config.h"

#ifndef DN_DEFAULT_MAX_EVENTS
  #include <"DN_DEFAULT_MAX_EVENTS" is needed but not defined>
#endif
#ifdef DN_MAX_EVENTS
  #include <"DN_MAX_EVENTS" has a duplicate definition>
#endif
#define DN_MAX_EVENTS (DN_DEFAULT_MAX_EVENTS)
#define DN_ACCEPT_EVENT (DN_MAX_EVENTS)

#ifndef DN_DEFAULT_BUFFER_LEN
  #include <"DN_DEFAULT_BUFFER_LEN" is needed but not defined>
#endif
#ifdef DN_BUFFER_LEN
  #include <"DN_BUFFER_LEN" has a duplicate definition>
#endif
#define DN_BUFFER_LEN (DN_DEFAULT_BUFFER_LEN)

typedef enum DN_IOEventStatus
{
  DN_IOEventStatus_Void = 0,  /* Event has not init yet */
  DN_IOEventStatus_Unused,    /* Event has init but deleted */
  DN_IOEventStatus_Inuse,     /* Event is in use */
} DN_IOEventStatus_t;

typedef struct DN_IOEvent DN_IOEvent_t;

typedef struct DN_IOEventHandlerArgs
{
  int epfd;
  int fd;
  int events;
  DN_IOEvent_t *ioev;
  void *arg;
} DN_IOEventHandlerArgs_t;

typedef int (*DN_IOEventHandler) (const DN_IOEventHandlerArgs_t args);

typedef struct DN_IOBuff
{
  DN_Buff_t in;
  DN_Buff_t out;
} DN_IOBuff_t;

struct DN_IOEvent
{
  DN_IOBuff_t buff;
  DN_IOEventHandler handler;
  DN_IOEventStatus_t status;
  DN_ResponseInfo_t info;
  time_t time;
  int fd;
  int events;
};

/* Fill zero */
int DN_PreInitEvent (DN_IOEvent_t * const ioevs, const int bsize);
/* Init an event */
int DN_InitEvent (DN_IOEvent_t * const ioev, const int fd,
                  DN_IOEventHandler handler);
/* Destory an event */
int DN_DestoryAllEvent (DN_IOEvent_t * const ioevs);
/* Destory all events */
int DN_DestoryEvent (DN_IOEvent_t * const ioev);
/* Set an event */
int DN_SetEvent (DN_IOEvent_t * const ioev, DN_IOEventHandler handler);
/* Add an event */
int DN_AddEvent (const int epfd, const int events, DN_IOEvent_t * const ioev);
/* Ddelete an event */
int DN_DelEvent (const int epfd, DN_IOEvent_t * const ioev);
/* Ddelete all events */
int DN_DelAllEvent (const int epfd, DN_IOEvent_t * const ioev);
/* Accept an event */
int DN_AcceptEvent (const DN_IOEventHandlerArgs_t args);
/* Wait for coming events */
int DN_WaitEvent (const int epfd, int * const fds,
                  struct epoll_event * const epevs,
                  const int evno, const int timeout);
/* Dispatch events */
int DN_DispatchEvent (const int epfd, const int fds, const int maxev,
                      const struct epoll_event * const epevs,
                      DN_IOEvent_t * const ioevs);

#endif

