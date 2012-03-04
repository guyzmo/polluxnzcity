
#define RED 0
#define GREEN 1
#define BLUE 2

class BeagleLeds {
    void enable_leds () {
        int fd;
        fd = open("/sys/kernel/debug/omap_mux/lcd_data0",O_WRONLY);write(fd, "7", 1);close(fd);
        fd = open("/sys/kernel/debug/omap_mux/lcd_data2",O_WRONLY);write(fd, "7", 1);close(fd);
        fd = open("/sys/kernel/debug/omap_mux/lcd_data4",O_WRONLY);write(fd, "7", 1);close(fd);
        fd = open("/sys/kernel/debug/omap_mux/lcd_data6",O_WRONLY);write(fd, "7", 1);close(fd);

        if ((fd = open("/sys/class/gpio/gpio70/direction",O_RDONLY)) == -1) {
            fd = open("/sys/class/gpio/export",O_WRONLY);write(fd, "70", 1);
        }
        close(fd);
        if ((fd = open("/sys/class/gpio/gpio72/direction",O_RDONLY)) == -1) {
            fd = open("/sys/class/gpio/export",O_WRONLY);write(fd, "72", 1);
        }
        close(fd);
        if ((fd = open("/sys/class/gpio/gpio74/direction",O_RDONLY)) == -1) {
            fd = open("/sys/class/gpio/export",O_WRONLY);write(fd, "74", 1);
        }
        close(fd);
        if ((fd = open("/sys/class/gpio/gpio76/direction",O_RDONLY)) == -1) {
            fd = open("/sys/class/gpio/export",O_WRONLY);write(fd, "76", 1);
        }
        close(fd);
        
        fd = open("/sys/class/gpio/gpio70/direction",O_WRONLY);write(fd, "out", 4);close(fd);
        fd = open("/sys/class/gpio/gpio72/direction",O_WRONLY);write(fd, "out", 4);close(fd);
        fd = open("/sys/class/gpio/gpio74/direction",O_WRONLY);write(fd, "out", 4);close(fd);
        fd = open("/sys/class/gpio/gpio76/direction",O_WRONLY);write(fd, "out", 4);close(fd);
    }
    void disable_leds () {
        int fd;
        fd = open("/sys/class/gpio/unexport",O_WRONLY);write(fd, "70", 1);close(fd);
        fd = open("/sys/class/gpio/unexport",O_WRONLY);write(fd, "72", 1);close(fd);
        fd = open("/sys/class/gpio/unexport",O_WRONLY);write(fd, "74", 1);close(fd);
        fd = open("/sys/class/gpio/unexport",O_WRONLY);write(fd, "76", 1);close(fd);
    }
    public:
        BeagleLeds() {
            enable_leds();
        }
        ~BeagleLeds() {
            disable_leds();
        }
        void set_status_led() {
            int fd = open("/sys/class/gpio/gpio70/value",O_WRONLY);write(fd, "1", 2);close(fd);
        }
        void reset_status_led() {
            int fd = open("/sys/class/gpio/gpio70/value",O_WRONLY);write(fd, "0", 2);close(fd);
        }
        void set_rgb_led(int color) {
            int fd;
            switch (color) {
                case RED:
                    fd = open("/sys/class/gpio/gpio72/value",O_WRONLY);write(fd, "1", 2);close(fd);
                    break;
                case GREEN:
                    fd = open("/sys/class/gpio/gpio74/value",O_WRONLY);write(fd, "1", 2);close(fd);
                    break;
                case BLUE:
                    fd = open("/sys/class/gpio/gpio76/value",O_WRONLY);write(fd, "1", 2);close(fd);
                    break;
            }
        }
        void reset_rgb_led(int color) {
            int fd;
            switch (color) {
                case RED:
                    fd = open("/sys/class/gpio/gpio72/value",O_WRONLY);write(fd, "0", 2);close(fd);
                    break;
                case GREEN:
                    fd = open("/sys/class/gpio/gpio74/value",O_WRONLY);write(fd, "0", 2);close(fd);
                    break;
                case BLUE:
                    fd = open("/sys/class/gpio/gpio76/value",O_WRONLY);write(fd, "0", 2);close(fd);
                    break;
            }
        }

};


