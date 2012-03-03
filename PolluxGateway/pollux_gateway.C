//#define API_ESCAPED_MODE 1
//#define TIMING 50
//#define VERBOSE 
//#undef API_ESCAPED_MODE // if API MODE is 2, useful for use with software flow control

//#include "XbeeCommunicatorLight.h"

//#define VERBOSE 1
#include "xbee_communicator.h"

const int panid[2] = {0x2,0xA};
const int venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};

class XbeePollux : public XbeeCommunicator {
    public:
        XbeePollux(char* port) : XbeeCommunicator(port) { }

        void run (XBeeFrame* frame) {
            //XbeeCommunicator::run(frame);
            switch (frame->api_id) {
                case RX_PACKET:
                    printf("Do something with : '%s'\n", frame->content.rx.payload);
                    break;
                default:
                    printf("Incoming frame that's not useful.\n");
                    break;
            }
        }
};

int main(void) {
    XbeePollux s = XbeePollux("/dev/ttyO2");
    //Serial s = Serial("/dev/ttyO2");

    //if (s.begin(B9600) == 0) {
    if (s.begin(panid,venid) >= 0) {
        //char data[] = { 0x7e, 0x00, 0x04, 0x08, 0x52, 0x4D, 0x59, 0xFF };
        //((Serial) s).write(data, 8);
        for (;;) 
            s.poll();
        return 0;
    }
    return 1;
}


