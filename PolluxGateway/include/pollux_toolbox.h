#ifndef __POLLUX_TOOLBOX_H__
#define __POLLUX_TOOLBOX_H__

#include <unistd.h> // msleep

#ifdef VERBOSE
#   define debug_print(STR) printf("%s", STR);
#   define debug_printc(STR) printf("%02X", STR);
#   define debug_println(STR) printf("%s\n", STR);
#   define debug_print_hex(STR) printf("%02X", STR);
#   define debug_println_hex(STR) printf("%02X\n", STR);
#else
#   define debug_print(STR) 
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
