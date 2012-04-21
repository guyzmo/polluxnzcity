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

#include <fcntl.h>      // open
#include <stdio.h>      // printf
#include <unistd.h>     //open
#include <string.h>     // memset, strerror
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/epoll.h>

#include <string>

#define EPOLL_MAX_CONN 2
#define EPOLL_RUN_TIMEOUT -1
#define BUFF_SIZE 50

namespace beagle {

/** Exception raised on Serial communication errors */
class SerialException {
    int code;
    public:
        SerialException(int i);
        void print_msg();
        int get_code();
};

/** Base class for all Serial communication
 *
 * set up the connection using begin(speed) (speed being an epoll flag, and not a full value ! use B9600, B115200...)
 * handles epoll() in poll() to check for incoming input and send output.
 * call recv() with i != 0 on timeouts
 *
 */
class Serial {
    const std::string port;

    int epfd;
    int fd;
    struct epoll_event events;
    int poll_wait;

    public:
        /** Constructor of the Serial handler
         * @param poll_wait: timeout of an epoll() call
         * @param port: string of the chardev representing the port
         */
        Serial(const std::string& port, int poll_wait);

        /** sets up the epoll() function
         * @param speed byte representing the speed, use epoll's macros : B9600, B14400 etc..
         */
        int begin(int speed);
        /** Function to call for each read on the serial port
         */
        void poll();

        /** Function called by poll() on each read
         * @param i true if timeout happened
         */
        virtual void recv(int i);

        /** read a character on serial line
         * @return read character
         */
        virtual char read();
        /** writes a string of size len on serial line
         * @param data byte array to be sent
         * @param len length of the array
         * @return number of bytes written
         */
        virtual ssize_t write(char* data, int len);
        virtual ssize_t write(uint8_t data);
};

} // namespace beagle

#endif // __BEAGLE_SERIAL_H__
