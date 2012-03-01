//#define API_ESCAPED_MODE 1
#define TIMING 50
#define VERBOSE 
#undef API_ESCAPED_MODE // if API MODE is 2, useful for use with software flow control

#include "XbeeCommunicatorLight.h"

XbeeCommunicator xbeecom(2,3);
const uint8_t panid[2] = {0x2,0xA};
const uint8_t venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};

void setup() {
    Serial.begin(9600);
    Serial.println();
    Serial.println();
    Serial.println("-- Start --");

    xbeecom.begin(panid,venid);
    // 1. find all nodes on the network

/*
    Serial.println("-- Sending --");
    xbeecom.send("caca!\n");
    Serial.print(xbeecom.available()); Serial.print("|   ");
    if (xbeecom.available() > 1) {
#if defined VERBOSE && VERBOSE == 2
        Serial.print("Recv frame: ");
        while (xbeecom.available()) {
           Serial.print(xbeecom.read(),HEX);
           Serial.print(" ");
        }
#else
        Serial.println(xbeecom.recv());
#endif
            Serial.println();
    }
*/
}

void loop() {
    //xbeecom.send("caca!\n");
    if (xbeecom.available() > 1) {
        Serial.println();
        Serial.print(xbeecom.available()); Serial.print("|   ");
        Serial.println(xbeecom.recv());
    } else 
        Serial.print('.');
        
    delay(1000);
}

