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
 * Common config
 * ========================================================================== */
#ifndef __DN_CONFIG_H__
#define __DN_CONFIG_H__

#define DN_VERSION "0.02"
#define DN_DEBUG (1)

/* Never modify this { */
#define DN_SERVER_RESPONSE_STR "Server: dinner/"DN_VERSION"\r\n"
#define DN_HTTP_HEADER_END_MARK "\r\n\r\n"
#define DN_HTTP_HEADER_END_MARK_LEN (4)
#define DN_DEFAULT_WEBROOT ("htdocs/")
#define DN_DEFAULT_PORT (8080)
#define DN_DEFAULT_MAX_EVENTS (1 << 8)
#define DN_DEFAULT_BUFFER_LEN ((1 << 14) - 1)
#define DN_DEFAULT_MAX_REQUEST_LEN (1 << 4)
#define DN_DEFAULT_WAITEVENT_TIMEOUT (1000)
/* } */

/* If this set none-zero, signal will be processed in a separate thread { */
#define DN_MULTITHREAD_DAEMON (0)
/* } */

/* There is 2 mode of say (): thread safe (1) or not (0).
 * First mode always bring right result, but has a low efficiency,
 * and a none thread safe mode will faster, but will failed if you
 * create multiple threads and reopen fd 0, 1 or 2 in a thread.
 * Default is none thread safe, it is ok, I will not reopen fd 0, 1 and 2
 * in any possible thread. { */
#define DN_THREADSAFE_SAY (0)
/* } */

#endif

