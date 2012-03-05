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
 * resources that helped:
 *  https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
 *  http://stackoverflow.com/questions/27247/could-you-recommend-some-guides-about-epoll-on-linux
 *  https://github.com/milkymist/milkymist/blob/master/tools/flterm.c
 *
 * Serial communication library
 */

#ifndef __BEAGLE_SERIAL_H__
#define __BEAGLE_SERIAL_H__

#include <fcntl.h>      // read
#include <stdio.h>      // printf
#include <unistd.h>     //open
#include <string.h>     // memset
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/epoll.h>

#include <pollux_toolbox.h>

#define EPOLL_MAX_CONN 2
#define EPOLL_RUN_TIMEOUT -1
#define BUFF_SIZE 50

class SerialException {
    int code;
    public:
        SerialException(int i);
        void print_msg();
        int get_code();
};

class Serial {
    const char* port;

    int epfd;
    int fd;
    struct epoll_event events;

    public:
        Serial(const char* port);

        int begin(int speed);
        int poll();

        virtual void recv();

        virtual char read();
        virtual ssize_t write(char* data, int len);
        virtual ssize_t write(uint8_t data);
};

#endif // __BEAGLE_SERIAL_H__
