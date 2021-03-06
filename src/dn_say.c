/* ========================================================================== *
 * Copyright (c) 2015-2016 秦凡东 (Qin Fandong)
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
 * Write line to stream
 * ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <sys/param.h>
#include "dn_say.h"
#include "dn_config.h"

/* See config.h { */
#ifndef DN_THREADSAFE_SAY
  #include <"DN_THREADSAFE_SAY" is need but not defined>
#endif
#ifdef DN_THREADSAFE
  #include <"DN_THREADSAFE" has a duplicate definition>
#endif
#define DN_THREADSAFE (DN_THREADSAFE_SAY)
/* } */

static DN_SayMode_t oneMode = MODE_UNKNOWN;

/* ========================================================================== *
 * Get write mode for current process
 * ========================================================================== *
 * Manual for PROC(5) has description for /proc/[pid]/fd/ below:
 *
 * This is a subdirectory containing one entry for each file which the process
 * has open, named by its file descriptor, and which is a symbolic link to
 * the actual file.
 * Thus, 0 is standard input, 1 standard output, 2 standard error, and so on.
 * ========================================================================== */
int
DN_GetSayMode (DN_SayMode_t * const mode)
{
  char path[MAXPATHLEN + 1], link[MAXPATHLEN + 1], file[MAXPATHLEN + 1];
  char pid[1 << 10];
  int index;
  int size, len;
  const int fds[] = { STDERR_FILENO, STDOUT_FILENO, STDERR_FILENO };
  const char root[] = "/proc/";
  const char null[] = "/dev/null";

  if (!mode)
    {
      fprintf (stderr, "null pointer.\n");
      return -1;
    }

  memset (path, 0, sizeof (path));

  /* /proc/[pid]/fd/ */
  memset (pid, 0, sizeof (pid));
  sprintf (pid, "%d", getpid ());
  strcpy (path, root);
  len = strlen (path);
  strncat (path, pid, MAXPATHLEN - len);
  len = strlen (path);
  strncat (path, "/", MAXPATHLEN - len);
  ++len;
  strncat (path, "fd", MAXPATHLEN - len);
  len = strlen (path);
  strncat (path, "/", MAXPATHLEN - len);
  ++len;

  memset (mode, 0, sizeof (DN_SayMode_t));
  *mode |= MODE_FILE;
  *mode |= MODE_OUT;

  for (index = 0, size = sizeof (fds) / sizeof (int);
       index < size; ++index)
    {
      memset (file, 0, sizeof (file));
      snprintf (link, MAXPATHLEN - len, "%s/%d", path, index);
      if (-1 == readlink (link, file, MAXPATHLEN))
        {
          fprintf (stderr, "readlink failed: %s\n", strerror (errno));
          break;
        }

      if (!strcmp (null, file))
        {
          /* If any file descriptor of 0, 1, 2 point to /dev/null,
           * treat this as a daemon, only invoke syslog to write line */
          *mode &= ~MODE_OUT;
          return 1;
        }
    }

  oneMode = *mode;

  return 1;
}

/* ========================================================================== *
 * Get write mode
 * ========================================================================== */
int
DN_SayMode (DN_SayMode_t * const mode)
{
  static int hasSet = 0;
  int status;

  if (!mode)
    {
      fprintf (stderr, "null pointer.\n");
      return -1;
    }

  status = 1;

  #if THREADSAFE
    {
      /* Useless, to ignore warning from syntastic plugin of vim { */
      status = oneMode && hasSet;
      /* } */
      status = getSayMode (mode);
    }
  #else
    {
      if (1 != hasSet)
        {
          hasSet = status = DN_GetSayMode (&oneMode);
        }
      *mode = oneMode;
    }
  #endif

  return status;
}

/* ========================================================================== *
 * Write line to stream
 * ========================================================================== */
int
DN_Say (const DN_SayMode_t mode, const DN_SayLevel_t level,
        const char * const str, ...)
{
  va_list ap;
  int out;
  int priority;

  if (!str)
    {
      fprintf (stderr, "null pointer.\n");
      return -1;
    }

  /* Set how to print */
  if (MSG_E == level)
    {
      out = STDERR_FILENO;
      priority = LOG_ERR;
    }
  else if (MSG_W == level)
    {
      out = STDOUT_FILENO;
      priority = LOG_WARNING;
    }
  else if (MSG_I == level)
    {
      out  = STDOUT_FILENO;
      priority = LOG_INFO;
    }
  else
    {
      return -1;
    }

  if (mode & MODE_FILE)
    {
      va_start (ap, str);
      vsyslog (priority, str, ap);
      va_end (ap);
    }

  if (mode & MODE_OUT)
    {
      va_start (ap, str);
      vdprintf (out, str, ap);
      va_end (ap);
    }

  return 1;
}

