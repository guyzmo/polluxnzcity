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
#include <sys/epoll.h>

#define EPOLL_MAX_CONN 2
#define EPOLL_RUN_TIMEOUT -1

class Serial {
    public:

        Serial (char* port) {
            int epfd = epoll_create(EPOLL_MAX_CONN);
            int fd = open("/dev/ttyO2", O_RDWR | O_NONBLOCK);

            if( fd > 0 ) {
                const int BUFF_SIZE = 50;
                char buff[BUFF_SIZE];
                memset(buff, 0, BUFF_SIZE);

                struct epoll_event ev;
                struct epoll_event events;
                ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
                ev.data.fd = fd;

                int res = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
                if( res < 0 )
                    printf("Error epoll_ctl: %i\n", errno);
            }
        }

        int poll() {
            int n = epoll_wait(epfd, &events, EPOLL_MAX_CONN, EPOLL_RUN_TIMEOUT);

            printf("Epoll unblocked\n");
            if(n < 0)
                perror("Epoll failed\n");
            else if(n==0)
                printf("TIMEOUT\n");
            else
            {
                int size = read(fd, buff, BUFF_SIZE-1);
                if( size < 0 )
                    printf("Error Reading the device: %s\n", strerror(errno));
                    return -1;
                else if( size > 0 ) {
                    printf("Input found!\n");
                    buff[size] = '\0';
                    printf("%x\n", buff);
                    return 1;
                } else {
                    printf("No input\n");
                    return 0;
                }

                if( errno == EAGAIN ) {
                    printf("ERRNO: EAGAIN\n");
                    return -2;
                }
                return -3;
            }
        }

        virtual int read() {
            
            
        };

        virtual int write(int val) {

        }
};


int main() {
    //XbeeCommunicator xbeecom("/dev/ttyO2");
    //xbeecom.begin(panid,venid);
    
    Serial xbeecom = Serial("/dev/ttyO2");

    while (xbeecom.poll() > 0) {
        printf("%s", xbeecom.read());
    }
};
