//#define TIMING 50
//#define VERBOSE 
//#define API_ESCAPED_MODE // if API MODE is 2, useful for use with software flow control

#include "beaglebone.h"
#include "xbee_communicator.h"

const int panid[2] = {0x2,0xA};
const int venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};

class XbeePollux : public XbeeCommunicator {
    BeagleLeds leds;
    public:
        XbeePollux(char* port) : XbeeCommunicator(port) { 
            leds.set_status_led();
        }

        void run (XBeeFrame* frame) {
            //XbeeCommunicator::run(frame);
            switch (frame->api_id) {
                case RX_PACKET:
                    leds.set_rgb_led(BLUE);
                    printf("Do something with : '%s'\n", frame->content.rx.payload);
                    usleep(1000000);
                    leds.reset_rgb_led(BLUE);
                    break;
                default:
                    leds.set_rgb_led(RED);
                    usleep(1000000);
                    leds.reset_rgb_led(BLUE);
                    printf("Incoming frame that's not useful.\n");
                    leds.reset_rgb_led(RED);
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


