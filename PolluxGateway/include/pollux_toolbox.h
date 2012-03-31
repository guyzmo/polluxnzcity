/*
 * Pollux'NZ City source code
 *
 * (c) 2012 CKAB / hackable:Devices
 * (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Toolbox header
 */

#ifndef __POLLUX_TOOLBOX_H__
#define __POLLUX_TOOLBOX_H__

#include <unistd.h> // msleep

#ifdef VERBOSE
#   define debug_print(STR) printf("%s", STR);
#   define debug_printf(...) printf(__VA_ARGS__);
#   define debug_printc(STR) printf("%02X", STR);
#   define debug_println(STR) printf("%s\n", STR);
#   define debug_print_hex(STR) printf("%02X", STR);
#   define debug_println_hex(STR) printf("%02X\n", STR);
#else
#   define debug_print(STR) 
#   define debug_printf(...)
#   define debug_printc(STR) 
#   define debug_println(STR) 
#   define debug_print_hex(STR) 
#   define debug_println_hex(STR) 
#endif

#define msleep(X) usleep(X*1000)
#define DEC 0
#define HEX 1
inline int min(int a, int b) { return (a < b) ? a : b; }

#endif
