//#define API_ESCAPED_MODE 1
//#define TIMING 50
//#define VERBOSE 
//#undef API_ESCAPED_MODE // if API MODE is 2, useful for use with software flow control

//#include "XbeeCommunicatorLight.h"

//const uint8_t panid[2] = {0x2,0xA};
//const uint8_t venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};


#include <fcntl.h>      // read
#include <stdio.h>      // printf
#include <unistd.h>     //open
#include <string.h>     // memset
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/epoll.h>

#define EPOLL_MAX_CONN 2
#define EPOLL_RUN_TIMEOUT -1
#define BUFF_SIZE 50

class Serial {
    char* port;

    int epfd;
    int fd;
    struct epoll_event events;

    public:
        Serial(char* port) : port(port) {
        }

        int begin() {
            int res;
            struct epoll_event ev;
            struct termios my_termios;
            
            fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
            if (fd == -1) {
                perror("Unable to open serial port.");
                return -1;
            }

            /* Thanks to Milkymist project to have found Julien Schmitt's work (GTKTerm) 
             * for figuring out the correct parameters to put into that weird struct.
             */
            tcgetattr(fd, &my_termios);
            my_termios.c_cflag = B9600;
            my_termios.c_cflag |= CS8;
            my_termios.c_cflag |= CREAD;
            my_termios.c_iflag = IGNPAR | IGNBRK;
            my_termios.c_cflag |= CLOCAL;
            my_termios.c_oflag = 0;
            my_termios.c_lflag = 0;
            my_termios.c_cc[VTIME] = 0;
            my_termios.c_cc[VMIN] = 1;
            tcsetattr(fd, TCSANOW, &my_termios);
            tcflush(fd, TCOFLUSH);
            tcflush(fd, TCIFLUSH);

            epfd = epoll_create(EPOLL_MAX_CONN);
            ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
            ev.data.fd = fd;

            res = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
            if( res < 0 ) {
                printf("Error epoll_ctl: %i\n", errno);
                return -1;
            }
            return 0;
        }

        int poll() {
            int n = epoll_wait(epfd, &events, EPOLL_MAX_CONN, EPOLL_RUN_TIMEOUT);

            if(n < 0)
                perror("Epoll failed");
            else if(n==0)
                printf("TIMEOUT\n");
            else {
                this->read();
            }
        }

        void read() {
            int size;
            char buff[BUFF_SIZE];
            memset(buff, 0, BUFF_SIZE);

            size = ::read(fd, buff, BUFF_SIZE);
            if( size < 0 )
                printf("Error Reading the device: %s\n", strerror(errno));
            else if( size > 0 ) {
                printf("Input: %x\n", *buff);
            } else
                printf("No input\n");

            if (errno == EAGAIN)
                perror("ERRNO: EAGAIN\n");
        }

        void write(char* data, int len) {
            ::write(fd, data, len);
        }
};

int main(void) {
    Serial s = Serial("/dev/ttyO2");
    if (s.begin() == 0);
        char data[] = {'+', '+', '+', 'A','T','N','D','\r'};
        s.write(data, 8);
        for (;;)
            s.poll();
    return 0;
}


