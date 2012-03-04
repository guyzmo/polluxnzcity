//#define TIMING 50
//#define VERBOSE 
//#define API_ESCAPED_MODE // if API MODE is 2, useful for use with software flow control

#include "beaglebone.h"
#include "xbee_communicator.h"

const int panid[2] = {0x2,0xA};
const int venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};

void sigint_handler(int c) {
    BeagleLeds::disable_leds();
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
            BeagleLeds::enable_leds();
            BeagleLeds::set_status_led();
            this->setup_signal();
        }
        ~XbeePollux() {
            BeagleLeds::disable_leds();
        }

        void run (XBeeFrame* frame) {
            switch (frame->api_id) {
                case RX_PACKET:
                    // XbeeCommunicator::run(frame); // print frame details
                    BeagleLeds::set_rgb_led(BeagleLeds::BLUE);
                    printf("Do something with : '%s'\n", frame->content.rx.payload);
                    msleep(500);
                    BeagleLeds::reset_rgb_led(BeagleLeds::BLUE);
                    break;
                default:
                    BeagleLeds::set_rgb_led(BeagleLeds::RED);
                    msleep(500);
                    BeagleLeds::reset_rgb_led(BeagleLeds::BLUE);
                    printf("Incoming frame that's not useful.\n");
                    BeagleLeds::reset_rgb_led(BeagleLeds::RED);
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


