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
 * Beaglebone hardware abstraction library
 */

#ifndef __BEAGLEBONE_H__
#define __BEAGLEBONE_H__

#include <fcntl.h>      // read
#include <unistd.h>     // open
#include <stdio.h>

namespace beagle {

struct UART {
    static void enable_uart2() {
        int fd;
        fd = open("/sys/kernel/debug/omap_mux/spi0_d0",O_WRONLY);write(fd, "1", 1);close(fd);
        fd = open("/sys/kernel/debug/omap_mux/spi0_sclk",O_WRONLY);write(fd, "21", 2);close(fd);
    }
    static void disable_uart2() {
        int fd;
        fd = open("/sys/kernel/debug/omap_mux/spi0_d0",O_WRONLY);write(fd, "0", 1);close(fd);
        fd = open("/sys/kernel/debug/omap_mux/spi0_sclk",O_WRONLY);write(fd, "20", 2);close(fd);
    }
};

struct Leds {
        static const int RED = 0;
        static const int GREEN = 1;
        static const int BLUE = 2;

        static void enable_leds () {
            int fd;
            fd = open("/sys/kernel/debug/omap_mux/lcd_data0",O_WRONLY);write(fd, "7", 1);close(fd);
            fd = open("/sys/kernel/debug/omap_mux/lcd_data2",O_WRONLY);write(fd, "7", 1);close(fd);
            fd = open("/sys/kernel/debug/omap_mux/lcd_data4",O_WRONLY);write(fd, "7", 1);close(fd);
            fd = open("/sys/kernel/debug/omap_mux/lcd_data6",O_WRONLY);write(fd, "7", 1);close(fd);

            if ((fd = open("/sys/class/gpio/gpio70/direction",O_RDONLY)) == -1) {
                fd = open("/sys/class/gpio/export",O_WRONLY);write(fd, "70", 2);
            }
            close(fd);
            if ((fd = open("/sys/class/gpio/gpio72/direction",O_RDONLY)) == -1) {
                fd = open("/sys/class/gpio/export",O_WRONLY);write(fd, "72", 2);
            }
            close(fd);
            if ((fd = open("/sys/class/gpio/gpio74/direction",O_RDONLY)) == -1) {
                fd = open("/sys/class/gpio/export",O_WRONLY);write(fd, "74", 2);
            }
            close(fd);
            if ((fd = open("/sys/class/gpio/gpio76/direction",O_RDONLY)) == -1) {
                fd = open("/sys/class/gpio/export",O_WRONLY);write(fd, "76", 2);
            }
            close(fd);
            
            fd = open("/sys/class/gpio/gpio70/direction",O_WRONLY);write(fd, "out", 4);close(fd);
            fd = open("/sys/class/gpio/gpio72/direction",O_WRONLY);write(fd, "out", 4);close(fd);
            fd = open("/sys/class/gpio/gpio74/direction",O_WRONLY);write(fd, "out", 4);close(fd);
            fd = open("/sys/class/gpio/gpio76/direction",O_WRONLY);write(fd, "out", 4);close(fd);
            printf("leds enabled\n");
        }
        static void disable_leds () {
            int fd;
            reset_status_led();
            reset_rgb_led(RED);
            reset_rgb_led(BLUE);
            reset_rgb_led(GREEN);
            printf("leds turned off\n");
            fd = open("/sys/class/gpio/unexport",O_WRONLY);write(fd, "70", 2);close(fd);
            fd = open("/sys/class/gpio/unexport",O_WRONLY);write(fd, "72", 2);close(fd);
            fd = open("/sys/class/gpio/unexport",O_WRONLY);write(fd, "74", 2);close(fd);
            fd = open("/sys/class/gpio/unexport",O_WRONLY);write(fd, "76", 2);close(fd);
            printf("leds disabled\n");
        }

        static void set_status_led() {
            int fd = open("/sys/class/gpio/gpio70/value",O_WRONLY);write(fd, "1", 1);close(fd);
        }
        static void reset_status_led() {
            int fd = open("/sys/class/gpio/gpio70/value",O_WRONLY);write(fd, "0", 1);close(fd);
        }
        static void set_rgb_led(int color) {
            int fd;
            switch (color) {
                case RED:
                    fd = open("/sys/class/gpio/gpio72/value",O_WRONLY);write(fd, "1", 1);close(fd);
                    break;
                case BLUE:
                    fd = open("/sys/class/gpio/gpio74/value",O_WRONLY);write(fd, "1", 1);close(fd);
                    break;
                case GREEN:
                    fd = open("/sys/class/gpio/gpio76/value",O_WRONLY);write(fd, "1", 1);close(fd);
                    break;
            }
        }
        static void reset_rgb_led(int color) {
            int fd;
            switch (color) {
                case RED:
                    fd = open("/sys/class/gpio/gpio72/value",O_WRONLY);write(fd, "0", 1);close(fd);
                    break;
                case BLUE:
                    fd = open("/sys/class/gpio/gpio74/value",O_WRONLY);write(fd, "0", 1);close(fd);
                    break;
                case GREEN:
                    fd = open("/sys/class/gpio/gpio76/value",O_WRONLY);write(fd, "0", 1);close(fd);
                    break;
            }
        }

};

} // namespace Beagle

#endif // __BEAGLEBONE_H__
