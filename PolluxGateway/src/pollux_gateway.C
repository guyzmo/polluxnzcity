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
 * Main routines
 */

//#define TIMING 50
//#define VERBOSE 
//#define API_ESCAPED_MODE // if API MODE is 2, useful for use with software flow control

#include <beaglebone.h>
#include <xbee_communicator.h>

const int panid[2] = {0x2,0xA};
const int venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};

void sigint_handler(int c) {
    Beagle::Leds::disable_leds();
    Beagle::UART::disable_uart2();
    printf("Exiting...\n");
    exit(0);
}

class XbeePollux : public XbeeCommunicator {
    void setup_signal() {
        struct sigaction sigIntHandler;

        sigIntHandler.sa_handler = sigint_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;

        sigaction(SIGINT, &sigIntHandler, NULL);
    }
    public:
        XbeePollux(char* port) : XbeeCommunicator(port) { 
            Beagle::UART::enable_uart2();
            Beagle::Leds::enable_leds();
            Beagle::Leds::set_status_led();
            this->setup_signal();
        }
        ~XbeePollux() {
            Beagle::Leds::disable_leds();
        }

        void run (XBeeFrame* frame) {
            switch (frame->api_id) {
                // XbeeCommunicator::run(frame); // print frame details
                case AT_CMD_RESP:
                    Beagle::Leds::set_rgb_led(Beagle::Leds::GREEN);
                    printf("AT command answered : '%s'\n", frame->content.at.command);
                    msleep(500);
                    Beagle::Leds::reset_rgb_led(Beagle::Leds::GREEN);
                    break;
                case RX_PACKET:
                    Beagle::Leds::set_rgb_led(Beagle::Leds::BLUE);
                    printf("Do something with : '%s'\n", frame->content.rx.payload);
                    msleep(500);
                    Beagle::Leds::reset_rgb_led(Beagle::Leds::BLUE);
                    break;
                default:
                    Beagle::Leds::set_rgb_led(Beagle::Leds::RED);
                    msleep(500);
                    Beagle::Leds::reset_rgb_led(Beagle::Leds::BLUE);
                    printf("Incoming frame that's not useful.\n");
                    Beagle::Leds::reset_rgb_led(Beagle::Leds::RED);
                    break;
            }
        }
};

int main(void) {
    XbeePollux s = XbeePollux("/dev/ttyO2");

    if (s.begin(panid,venid) >= 0) {
        for (;;) 
            s.poll();
        return 0;
    }
    return 1;
}


