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
 * Do assert
 * ========================================================================== */
#ifndef __DN_ASSERT_H__
#define __DN_ASSERT_H__

#define DN_ASSERT_ABORT(e,s)  \
 ((e) ? (void)0 : ((DN_LOG (mode, MSG_E, (s))), abort ()))

#define DN_ASSERT_LOG(e,s)  \
 ((e) ? (void)0 : ((DN_LOG (mode, MSG_E, (s)))))

/* TODO: Once I try to write this assert as a expression.
 * But I can not deal with return statement in a ternary operand.
 * I do not kown if this really can write as a expression. */
#define DN_ASSERT_RETURN(e,s,r) \
 if (!(e)) \
  { \
    DN_LOG (mode, MSG_E, (s));  \
    return (r); \
  }

#endif

