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
 * I/O Event
 * ========================================================================== */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "dn_event.h"
#include "dn_io.h"
#include "dn_request.h"
#include "dn_response.h"
#include "dn_log.h"
#include "dn_assert.h"

/* ========================================================================== *
 * Set status of all events to DN_IOEventStatus_Void
 * ========================================================================== *
 * TODO: Should find a better way to determine whether a event should be
 * delete/destoryed or not. Current I assume every event's status has been init
 * with DN_IOEventStatus_Void. And to make this works well, I have to init
 * status of every events at the very beginning, this is ugly.
 * ========================================================================== */
int
DN_PreInitEvent (DN_IOEvent_t * const ioevs, const int bsize)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioevs, "ioevs is NULL.\n", -1);
  DN_ASSERT_RETURN (bsize > 0, "len is illegal.\n", -1);

  memset (ioevs, 0, bsize);

  return 1;
}

/* ========================================================================== *
 * Init an event
 * ========================================================================== */
int
DN_InitEvent (DN_IOEvent_t * const ioev, const int fd,
              DN_IOEventHandler handler)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (handler, "handler is NULL.\n", -1);
  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);

  memset (ioev, 0, sizeof (DN_IOEvent_t));

  ioev->handler = handler;
  ioev->fd = fd;
  ioev->time = time (NULL);

  if (-1 == DN_CreateBuff (&ioev->buff.in, DN_BUFFER_LEN))
    {
      DN_LOG (mode, MSG_E, "DN_CreateBuff failed.\n");
      return -1;
    }

  if (-1 == DN_CreateBuff (&ioev->buff.out, DN_BUFFER_LEN))
    {
      DN_LOG (mode, MSG_E, "DN_CreateBuff failed.\n");
      return -1;
    }

  return 1;
}

/* ========================================================================== *
 * Destory an event
 * ========================================================================== */
int
DN_DestoryEvent (DN_IOEvent_t * const ioev)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);

  if (DN_IOEventStatus_Void == ioev->status)
    {
      return 1;
    }

  if (ioev->buff.in.buff)
    {
      if (-1 == DN_DelBuff (&ioev->buff.in))
        {
          DN_LOG (mode, MSG_E, "DN_DelBuff failed.\n");
          return -1;
        }
    }

  if (ioev->buff.out.buff)
    {
      if (-1 == DN_DelBuff (&ioev->buff.out))
        {
          DN_LOG (mode, MSG_E, "DN_DelBuff faied.\n");
          return -1;
        }
    }

  if (ioev->info.arg)
    {
      free (ioev->info.arg);
      ioev->info.arg = NULL;
    }

  if (ioev->fd > 0)
    {
      close (ioev->fd);
    }

  memset (ioev, 0, sizeof (DN_IOEvent_t));

  return 1;
}

/* ========================================================================== *
 * Destory all events
 * ========================================================================== */
int
DN_DestoryAllEvent (DN_IOEvent_t * const ioevs)
{
  DN_LogMode_t mode;
  int index;
  int error;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioevs, "ioevs is NULL.\n", -1);

  error = 0;
  for (index = 0; index < DN_MAX_EVENTS; ++index)
    {
      if (-1 == DN_DestoryEvent (&ioevs[index]))
        {
          DN_LOG (mode, MSG_E, "DN_DestoryEvent failed.\n");
          error = 1;
        }
    }

  /* XXX: return -1 or 0 if failed? { */
  return error ? 0 : 1;
  /* } */
}

/* ========================================================================== *
 * Set an event
 * ========================================================================== */
int
DN_SetEvent (DN_IOEvent_t * const ioev, DN_IOEventHandler handler)
{
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (handler, "handler is NULL.\n", -1);

  ioev->handler = handler;

  return 1;
}

/* ========================================================================== *
 * Add an event
 * ========================================================================== */
int
DN_AddEvent (const int epfd, const int events, DN_IOEvent_t * const ioev)
{
  struct epoll_event ev;
  DN_LogMode_t mode;
  int epop;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (epfd > -1, "epfd is illegal.\n", -1);

  ev.data.ptr = ioev;
  ev.events = ioev->events = events;

  epop = DN_IOEventStatus_Inuse == ioev->status ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

  if (-1 == epoll_ctl (epfd, epop, ioev->fd, &ev))
    {
      DN_LOG (mode, MSG_E, "epoll_ctl failed: %s.\n", strerror (errno));
      return -1;
    }

  ioev->status = DN_IOEventStatus_Inuse;

  return 1;
}

/* ========================================================================== *
 * Delete an event
 * ========================================================================== */
int
DN_DelEvent (const int epfd, DN_IOEvent_t * const ioev)
{
  struct epoll_event ev;
  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (epfd > -1, "epfd is illegal.\n", -1);

  ev.data.ptr = ioev;

  if (DN_IOEventStatus_Inuse != ioev->status)
    {
      return 1;
    }

  if (-1 == epoll_ctl (epfd, EPOLL_CTL_DEL, ioev->fd, &ev))
    {
      DN_LOG (mode, MSG_E, "epoll_ctl failed: %s.\n", strerror (errno));
      return -1;
    }

  ioev->status = DN_IOEventStatus_Unused;

  return 1;
}

/* ========================================================================== *
 * Delete all events
 * ========================================================================== */
int
DN_DelAllEvent (const int epfd, DN_IOEvent_t * const ioevs)
{
  DN_LogMode_t mode;
  int index;
  int error;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (epfd > -1, "epfd is illegal.\n", -1);
  DN_ASSERT_RETURN (ioevs, "ioev is NULL.\n", -1);

  error = 0;
  for (index = 0; index < DN_MAX_EVENTS; ++index)
    {
      if (-1 == DN_DelEvent (epfd, &ioevs[index]))
        {
          DN_LOG (mode, MSG_E, "DN_DelEvent failed.\n");
          error = 1;
        }
    }

  /* XXX: return -1 or 0 if failed? { */
  return error ? 0 : 1;
  /* } */
}
/* ========================================================================== *
 * Accept for events
 * return 0 for limit conns
 * ========================================================================== */
int
DN_AcceptEvent (const DN_IOEventHandlerArgs_t args)
{
  DN_IOEvent_t *ioev, *ioevs;
  struct sockaddr_in addr;
  socklen_t addrLen;
  DN_LogMode_t mode;
  int epfd, fd, evfd;
  int index;

  DN_LOGMODE (&mode);

  ioevs = (DN_IOEvent_t *)args.arg;
  ioev = args.ioev;
  epfd =  args.epfd;
  fd = args.fd;

  DN_ASSERT_RETURN (ioev, "ioev is NULL.\n", -1);
  DN_ASSERT_RETURN (ioevs, "ioevs is NULL.\n", -1);
  DN_ASSERT_RETURN (epfd > -1, "epfd is illegal.\n", -1);
  DN_ASSERT_RETURN (fd > -1, "fd is illegal.\n", -1);

  addrLen = sizeof (addr);

  if (-1 == (evfd = accept (fd, (struct sockaddr *)&addr, &addrLen)))
    {
      if (EWOULDBLOCK != errno && EAGAIN != errno && EINTR != errno)
        {
          DN_LOG (mode, MSG_E, "accept failed: %s.\n", strerror (errno));
          return 0;
        }
    }

  for (index = 0; index < DN_MAX_EVENTS; ++index)
    {
      if (DN_IOEventStatus_Inuse != ioevs[index].status)
        {
          break;
        }
    }

  if (DN_MAX_EVENTS == index)
    {
      DN_LOG (mode, MSG_E, "connections limit (%d) reached.\n", DN_MAX_EVENTS);
      return 0;
    }

  if (-1 == DN_NoBlock (evfd))
    {
      DN_LOG (mode, MSG_E, "DN_NoBlock failed.\n");
      return -1;
    }

  if (-1 == DN_InitEvent (&ioevs[index], evfd,
                          (DN_IOEventHandler)DN_AcceptRequest))
    {
      DN_LOG (mode, MSG_E, "DN_InitEvent failed.\n");
      return -1;
    }

  if (-1 == DN_AddEvent (epfd, EPOLLIN, &ioevs[index]))
    {
      DN_LOG (mode, MSG_E, "DN_AddEvent failed.\n");
      return -1;
    }

  DN_LOG (mode, MSG_I, "new connection from %s:%d.\n",
                        inet_ntoa (addr.sin_addr), ntohs (addr.sin_port));

  return 1;
}

/* ========================================================================== *
 * Wait for event
 * ========================================================================== */
int
DN_WaitEvent (const int epfd, int * const fds, struct epoll_event * const epevs,
              const int evno, const int timeout)
{

  DN_LogMode_t mode;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (epfd > -1, "epfd is illegal.\n", -1);
  DN_ASSERT_RETURN (evno > -1, "evno is illegal.\n", -1);
  DN_ASSERT_RETURN (timeout > -1, "timeout will cause block forever.\n", -1);
  DN_ASSERT_RETURN (fds, "fds is NULL.\n", -1);
  DN_ASSERT_RETURN (epevs, "epevs is NULL.\n", -1);

  if (-1 == (*fds = epoll_wait (epfd, epevs, evno, timeout)))
    {
      DN_LOG (mode, MSG_E, "epoll_wait failed: %s.\n", strerror (errno));
      return -1;
    }

  return 1;
}

/* ========================================================================== *
 * Dispatch event
 * ========================================================================== */
int
DN_DispatchEvent (const int epfd, const int fds, const int maxev,
                  const struct epoll_event * const epevs,
                  DN_IOEvent_t * const ioevs)
{
  DN_IOEvent_t *ioev;
  DN_IOEventHandlerArgs_t args;
  DN_LogMode_t mode;
  int ret;
  int index;

  DN_LOGMODE (&mode);

  DN_ASSERT_RETURN (epfd > -1, "epfd is illegal.\n", -1);
  DN_ASSERT_RETURN (fds > -1, "fds is illegal.\n", -1);
  DN_ASSERT_RETURN (maxev > -1, "maxev is illegal.\n", -1);
  DN_ASSERT_RETURN (ioevs, "ioevs is NULL.\n", -1);
  DN_ASSERT_RETURN (epevs, "epevs is NULL.\n", -1);

  ret = 1;
  for (index = 0; index < fds; ++index)
    {
      ioev = (DN_IOEvent_t *)epevs[index].data.ptr;

      /* Handle I/O event */
      if (((ioev->events & EPOLLIN) && (epevs[index].events & EPOLLIN))
           || ((ioev->events & EPOLLOUT) && (epevs[index].events & EPOLLOUT)))

        {
          args.epfd = epfd;
          args.fd = ioev->fd;
          args.events = epevs[index].events;
          args.ioev = ioev;
          args.arg = (void *)ioevs;
          if (-1 == (ret = ioev->handler (args)))
            {
              /* XXX: Determine what to do here */
            }
        }
    }

  /* XXX: Determine what do return { */
  return 1 ? 1 : ret;
  /* } */
}

