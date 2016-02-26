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
 * dinner - A lightweight web server
 * ========================================================================== *
 * It named dinner because first version
 * was written during (and after) my dinner time.
 * ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "dn_event.h"
#include "dn_io.h"
#include "dn_log.h"
#include "dn_daemonize.h"
#include "dn_assert.h"
#include "dn_config.h"

static DN_LogMode_t mode;
static sigset_t mask;
static int epfd;

/* ========================================================================== *
 * Show usage
 * ========================================================================== */
int
usage (void)
{
  int line;
  const char * const content[] =
    {
      "This is dinner, version "DN_VERSION" .",
      "",
      "Usage:",
      "  --port, -p\tset port",
      "  --workdir, -r\tset work directory",
      "  --daemon, -d\twork as daemon",
      "  --help, -h\tshow this help",
      NULL  /* Last item must be NULL */
    };

  for (line = 0; content[line]; ++line)
    {
      puts (content[line]);
    }

  return 1;
}

/* ========================================================================== *
 * Bind a signal to a handler
 * ========================================================================== */
int
actionBinding (const int signo, void (* const handler) (int))
{
  struct sigaction action;

  DN_ASSERT_RETURN (signo > -1, "signo is illegal.\n", -1);

  action.sa_handler = handler;    /* NULL is ok here */
  sigemptyset (&action.sa_mask);
  sigaddset (&action.sa_mask, signo);
  action.sa_flags = 0;
  if (sigaction (signo, &action, NULL) < 0)
    {
      DN_LOG (mode, MSG_E, "sigaction failed: %s\n", strerror (errno));
      return -1;
    }

  return 1;
}

/* ========================================================================== *
 * Handle SIGHUP
 * ========================================================================== *
 * This to handle SIGHUP, for reload config, will be implemented one day
 * ========================================================================== */
void
sighupHandler (const int signo)
{
  DN_ASSERT_ABORT (SIGHUP == signo, "signo is illegal.\n");
}

/* ========================================================================== *
 * Handle SIGTERM
 * ========================================================================== */
void
sigtermHandler (const int signo, DN_IOEvent_t * const ioevs)
{
  int error;

  DN_ASSERT_ABORT (SIGTERM == signo || SIGINT == signo,
                   "signo is illegal.\n");

  DN_LOG (mode, MSG_I, "stopping dinner.\n");
  error = 0;

  DN_LOG (mode, MSG_I, "delete all events.\n");
  if (-1 == DN_DelAllEvent (epfd, ioevs))
    {
      DN_LOG (mode, MSG_E, "DN_DelAllEvent failed.\n");
      error = 1;
    }

  DN_LOG (mode, MSG_I, "destory all events.\n");
  if (-1 == DN_DestoryAllEvent (ioevs))
    {
      DN_LOG (mode, MSG_E, "DN_DestoryAllEvent failed.\n");
      error = 1;
    }

  DN_LOG (mode, MSG_I, "dinner is stoped %s errors.\n",
                        error ? "with" : "without");

  exit (error ? 1 : 0);
}


/* ========================================================================== *
 * A thread to handle signals
 * ========================================================================== */
void *
sighandler (void *arg)
{
  int sig;

  DN_ASSERT_ABORT (arg, "arg is NULL.\n");

  while (1)
    {
      if (sigwait (&mask, &sig))
        {
          DN_LOG (mode, MSG_E, "sigwait failed: %s\n", strerror (errno));
          exit (1);
        }

      switch (sig)
        {
        case SIGHUP:
          sighupHandler (SIGHUP);
          break;

        case SIGINT:
          /* Fall through */
        case SIGTERM:
          sigtermHandler (SIGTERM, (DN_IOEvent_t *)arg);
          break;

        default:
          ;   /* Ignore other signals */
        }
    }

  return NULL;
}

/* ========================================================================== *
 * If a file is a directory
 * ========================================================================== */
int
isdir (const char * const path, int * const result)
{
  struct stat status;

  DN_ASSERT_RETURN (path, "path is NULL.\n", -1);
  DN_ASSERT_RETURN (result, "result is NULL.\n", -1);

  if (-1 == stat (path, &status))
    {
      /* Here is not an error, we get -1 if path is not exist { */
      return 0;
      /* } */
    }

  *result = (S_IFDIR == (status.st_mode & S_IFMT)) ? 1 : 0;

  return 1;
}

/* ========================================================================== *
 * Change work directory
 * ========================================================================== */
int
chroot (const char * const path)
{
  if (-1 == chdir (path))
    {
      DN_LOG (mode, MSG_E, "chdir failed: %s.\n", strerror (errno));
      return -1;
    }

  return 1;
}

/* ========================================================================== *
 * Startup
 * ========================================================================== */
int
startup (int * const fd, uint16_t * const port)
{
  struct sockaddr_in addr;
  int sockfd;
  int on;
  const socklen_t addrLen = sizeof (addr);

  DN_ASSERT_RETURN (fd, "fd is NULL.\n", -1);
  DN_ASSERT_RETURN (port, "port is NULL.\n", -1);

  *fd = -1;

  if (-1 == (sockfd = socket (AF_INET, SOCK_STREAM, 0)))
    {
      DN_LOG (mode, MSG_E, "socket failed: %s.\n", strerror (errno));
      return -1;
    }

  on = 1;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) != 0)
    {
      DN_LOG (mode, MSG_E, "setsockopt failed: %s.\n", strerror (errno));
      return -1;
    }

  memset (&addr, 0, addrLen);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  addr.sin_port = port ? htons(*port) : 0;

  if (-1 == bind (sockfd, (struct sockaddr *)&addr, addrLen))
    {
      DN_LOG (mode, MSG_E, "bind failed: %s.\n", strerror (errno));
      return -1;
    }

  if (!addr.sin_port)
    {
      if (-1 == getsockname (sockfd, (struct sockaddr *)&addr,
                             (socklen_t *)&addrLen))
        {
          DN_LOG (mode, MSG_E, "getsockname failed: %s.\n", strerror (errno));
          return -1;
        }
    }

  if (-1 == DN_NoBlock (sockfd))
    {
      DN_LOG (mode, MSG_E, "DN_NoBlock failed.\n");
      /* XXX: Determine what should to do here { *
      return -1;
       * } */
    }

  if (-1 == listen (sockfd, 5))
    {
      DN_LOG (mode, MSG_E, "listen failed: %s.\n", strerror (errno));
      return -1;
    }

  *fd = sockfd;
  *port = ntohs (addr.sin_port);

  return 1;
}

/* ========================================================================== *
 * Main loop for events
 * ========================================================================== */
int
eventloop (const int listenfd)
{
  struct epoll_event epevs[DN_MAX_EVENTS];
  /* Last item is events[DN_ACCEPT_EVENT], and this for listenfd { */
  DN_IOEvent_t ioevs[DN_MAX_EVENTS + 1];
  /* } */
  pthread_t tid;
  int fds;
  int index, len;
  /* XXX: Should we handle SIGKILL here ? { */
  const int sigs[] = { SIGHUP, SIGTERM, SIGINT, };
  /* } */

  DN_ASSERT_RETURN (listenfd > -1, "illegel listenfd.\n", -1);

  if (-1 == DN_PreInitEvent (ioevs, sizeof (ioevs)))
    {
      DN_LOG (mode, MSG_E, "DN_PreInitEvent failed.\n");
      /* Do nothing */
    }

  /* Create a thread to handle signals */
  len = sizeof (sigs) / sizeof (int);
  for (index = 0; index < len; ++index)
    {
      if (-1 == actionBinding (sigs[index], SIG_DFL))
        {
          DN_LOG (mode, MSG_E, "actionBinding failed\n");
          exit (1);
        }
    }

  sigfillset (&mask);
  if (pthread_sigmask (SIG_BLOCK, &mask, NULL))
    {
      DN_LOG (mode, MSG_E, "pthread_sigmask failed: %s\n", strerror (errno));
      exit (1);
    }

  if (pthread_create (&tid, NULL, sighandler, ioevs))
    {
      DN_LOG (mode, MSG_E, "pthread_create failed: %s\n", strerror (errno));
      exit (1);
    }

  /* Set event for accept */
  if (-1 == (epfd = epoll_create1 (0)))
    {
      DN_LOG (mode, MSG_E, "epoll_create1 failed: %s.\n", strerror (errno));
      return -1;
    }

  if (-1 == DN_InitEvent (&ioevs[DN_ACCEPT_EVENT], listenfd,
                          (DN_IOEventHandler)DN_AcceptEvent))
    {
      DN_LOG (mode, MSG_E, "DN_InitEvent failed.\n");
      return -1;
    }

  if (-1 == DN_AddEvent (epfd, EPOLLIN, &ioevs[DN_ACCEPT_EVENT]))
    {
      DN_LOG (mode, MSG_E, "DN_AddEvent failed.\n");
      return -1;
    }

  /* Begin event loop */
  while (1)
    {
      if (-1 == DN_WaitEvent (epfd, &fds, epevs, DN_MAX_EVENTS,
                              DN_DEFAULT_WAITEVENT_TIMEOUT))
        {
          DN_LOG (mode, MSG_E, "DN_WaitEvent failed: %s.\n");
          continue;
        }

      if (-1 == DN_DispatchEvent (epfd, fds, DN_MAX_EVENTS, epevs, ioevs))
        {
          DN_LOG (mode, MSG_E, "DN_DispatchEvent failed.\n");
          continue;
        }
    }

  return 1;
}

/* ========================================================================== *
 * MAIN
 * ========================================================================== */
int
main (const int argc, const char * const * argv)
{
  char path[MAXPATHLEN + 1];
  char **cmdarg;
  int specPath;
  int runAsDaemon;
  int listenfd;
  int opt;
  int dir;
  int len;
  uint16_t port;
  const char optstr[] = "p:r:dh";
  const struct option opts[] =
    {
      { "port", required_argument, 0, 'p' },
      { "workdir", required_argument, 0, 'r' },
      { "daemon", no_argument, 0, 'd' },
      { "help", no_argument, 0, 'h' },
      { 0, 0, 0, 0 }  /* Last line */
    };

  DN_LOGMODE (&mode);

  /* Get current path */
  if (!getcwd (path, MAXPATHLEN - strlen (DN_DEFAULT_WEBROOT) - 1))
    {
      DN_LOG (mode, MSG_E, "getcwd failed: %s.\n", strerror (errno));
      return -1;
    }
  len = strlen (path);
  strncat (path, "/", MAXPATHLEN);

  cmdarg = (char **)argv;
  port = DN_DEFAULT_PORT;
  specPath = 0;
  runAsDaemon = 0;

  /* TODO: Get options */
  while (-1 != (opt = getopt_long (argc, cmdarg, optstr, opts, NULL)))
    {
      switch (opt)
        {
        case 'p':
          port = atoi (optarg);
          break;

        case 'r':
          specPath = 1;
          strncat (path, optarg, MAXPATHLEN);
          break;

        case 'd':
          runAsDaemon = 1;
          break;

        case 'h':
          if (-1 == usage ())
            {
              DN_LOG (mode, MSG_E, "usage failed.\n");
              exit (1);
            }
          exit (0);
        }
    }

  /* Daemonize if necessary */
  if (runAsDaemon)
    {
      DN_LOG (mode, MSG_I, "daemonizing dinner.\n");
      if (-1 == DN_Daemonize (NULL))
        {
          DN_LOG (mode, MSG_E, "DN_Daemonize failed.\n");
          exit (1);
        }
    }

  DN_TRUELOGMODE (&mode);

  /* Change base directory */
  if (!specPath)
    {
      strncat (path, DN_DEFAULT_WEBROOT, MAXPATHLEN);
    }

  DN_LOG (mode, MSG_I, "trying work directory: \"%s\".\n", path);

  if (-1 == isdir (path, &dir))
    {
      DN_LOG (mode, MSG_E, "isdir failed.\n");
      return -1;
    }

  if (!dir)
    {
      DN_LOG (mode, MSG_W, "path is unreachable \"%s\".\n", path);
      path[len] = 0;
    }

  if (-1 == chroot (path))
    {
      DN_LOG (mode, MSG_E, "chroot failed.\n");
      return -1;
    }

  DN_LOG (mode, MSG_I, "change work directory: \"%s\".\n", path);

  if (startup (&listenfd, &port) < 1)
    {
      DN_LOG (mode, MSG_E, "startup failed.\n");
      exit (1);
    }

  DN_LOG (mode, MSG_I, "dinner start on port %d with fd %d.\n", port, listenfd);

  if (-1 == eventloop (listenfd))
    {
      DN_LOG (mode, MSG_E, "eventloop failed.\n");
      exit (1);
    }

  return 0;
}

