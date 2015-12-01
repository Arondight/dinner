/* ========================================================================== *
 * Copyright (c) 2015 秦凡东 (Qin Fandong)
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
 * Daemonize self
 * ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/resource.h>

enum {
  DN_DAEMONIZE_STATUS_OK = 1,
  DN_DAEMONIZE_STATUS_ERROR = 1 << 1,
};

/* ========================================================================== *
 * Set mask to 0, let a new file may has any privilege
 * ========================================================================== */
int
DN_DaemonSetMask (void)
{
  umask (0);
  return DN_DAEMONIZE_STATUS_OK;
}

/* ========================================================================== *
 * Change work directory to root
 * ========================================================================== */
int
DN_DaemonChroot (void)
{
  const char rootdir[] = "/";

  if (chdir (rootdir))
    {
      perror ("chdir failed:");
      return DN_DAEMONIZE_STATUS_ERROR;
    }

  return DN_DAEMONIZE_STATUS_OK;
}

/* ========================================================================== *
 * Create a new session
 * ========================================================================== */
int
DN_DaemonNewSession (void)
{
  pid_t pid;

  if ((pid = fork ()) < 0)
    {
      perror ("fork falied:");
      exit (1);
    }

  if (pid > 0)
    {
      exit (0);
    }

  if (-1 == setsid ())
    {
      perror ("setsid failed:");
      return DN_DAEMONIZE_STATUS_ERROR;
    }

  return DN_DAEMONIZE_STATUS_OK;
}

/* ========================================================================== *
 * Disable controlling terminal
 * ========================================================================== */
int
DN_DaemonDiscardCtrlTerminal (void)
{
  struct sigaction action;
  pid_t pid;

  action.sa_handler = SIG_IGN;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;

  if (sigaction (SIGHUP, &action, NULL) < 0)
    {
      perror ("sigaction failed:");
      return DN_DAEMONIZE_STATUS_ERROR;
    }

  if ((pid = fork ()) < 0)
    {
      perror ("fork failed:");
      exit (1);
    }

  if (pid > 0)
    {
      exit (0);
    }

  return DN_DAEMONIZE_STATUS_OK;
}

/* ========================================================================== *
 * Close all file descriptor
 * ========================================================================== */
int
DN_DaemonCloseFD (void)
{
  struct rlimit limit;

  if (getrlimit (RLIMIT_NOFILE, &limit) < 0)
    {
      perror ("getrlimit falied:");
      return DN_DAEMONIZE_STATUS_ERROR;
    }

  int fd = 0;
  while (fd < (int)(RLIM_INFINITY == limit.rlim_max ? 1024 : limit.rlim_max))
    {
      close (fd);
      ++fd;
    }

  return DN_DAEMONIZE_STATUS_OK;
}

/* ========================================================================== *
 * Reopen stdin, stdout and stderr to /dev/null
 * Invoke this after DN_DaemonCloseFD
 * ========================================================================== */
int
DN_DaemonReopenFD (void)
{
  const char * const nullFile = "/dev/null";
  int in, out, err;

  in = open (nullFile, O_RDWR);
  out = dup (in);
  err = dup (in);

  if (STDIN_FILENO != in || STDOUT_FILENO != out || STDERR_FILENO != err)
    {
      fprintf (stderr, "open or dup failed: %s", strerror (errno));
      return DN_DAEMONIZE_STATUS_ERROR;
    }

  return DN_DAEMONIZE_STATUS_OK;
}

/* ========================================================================== *
 * Init log
 * ========================================================================== */
int
DN_DaemonInitLog (const char * const ident)
{
  openlog (ident, LOG_CONS, LOG_DAEMON);

  return DN_DAEMONIZE_STATUS_OK;
}

/* ========================================================================== *
 * Make caller to a daemon
 * ========================================================================== */
int
DN_Daemonize (const char * const ident)
{
  int status = 0;

  status |= DN_DaemonSetMask ();
  status |= DN_DaemonNewSession ();
  status |= DN_DaemonDiscardCtrlTerminal ();
  status |= DN_DaemonChroot ();
  status |= DN_DaemonCloseFD ();
  status |= DN_DaemonReopenFD ();
  status |= DN_DaemonInitLog (ident); /* NULL is okay here, no need to check */

  return DN_DAEMONIZE_STATUS_ERROR & status ? -1 : 1;
}

