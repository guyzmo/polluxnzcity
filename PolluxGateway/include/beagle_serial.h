/* https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
 * http://stackoverflow.com/questions/27247/could-you-recommend-some-guides-about-epoll-on-linux
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
