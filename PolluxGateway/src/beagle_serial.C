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

#include <beagle_serial.h>

// Serial Exception class

SerialException::SerialException(int i) : code(i) {}

void SerialException::print_msg() {
    printf("Serial Error #%d: %s\n", code, strerror(code));
}
int SerialException::get_code() {
    return code;
}

// Serial class

Serial::Serial(const std::string& port) : port(port) {}

int Serial::begin(int speed) {
    debug_print("Serial.begin()\n");
    int res;
    struct epoll_event ev;
    struct termios my_termios;

    fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
        perror("Unable to open serial port.");
        return -1;
    }

    /* Thanks to Milkymist project to have found Julien Schmitt's work (GTKTerm) 
     * for figuring out the correct parameters to put into that weird struct.
     */
    tcgetattr(fd, &my_termios);
    my_termios.c_cflag = speed;
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
    return 1;
}

int Serial::poll() {
    //printf("poll()\n");

    int n = epoll_wait(epfd, &events, EPOLL_MAX_CONN, EPOLL_RUN_TIMEOUT);

    if(n < 0)
        perror("Epoll failed");
    else if(n==0)
        printf("TIMEOUT\n");
    else {
        recv();
    }
}

void Serial::recv() {
    try {
        printf("Input: ");
        char c=-1;
        while ((c = this->read()) != -1)
            printf("%x",c);
        printf("\n");
    } catch (SerialException e) {
        printf("\n");
        e.print_msg();
    }
}

char Serial::read() {
    int size;
    char c;

    size = ::read(fd, &c, 1);
    if( size > 0 )
        return c;
    else if( size == 0 || errno == 11) {
        //printf("No input\n");
        return -1;
    }
    throw(SerialException(errno));
}

ssize_t Serial::write(char* data, int len) {
    //printf("Serial.write(%s)\n", data);
    ::write(fd, data, len);
}

ssize_t Serial::write(uint8_t data) {
    //printf("\nSerial.write(%x)\n", data);
    ::write(fd, &data, sizeof(data));
}
