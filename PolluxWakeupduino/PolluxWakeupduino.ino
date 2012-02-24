#define API_ESCAPED_MODE 1

#include "../PolluxGateway/XbeeCommunicatorLight.h"

XbeeCommunicator xbeecom(0,1);

uint8_t panid[2] = { 0x4, 0x2 };
uint8_t venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};


void setup() {
    xbeecom.begin(panid,venid);
    pinMode(13, OUTPUT);
}

void loop() {
    digitalWrite(13, HIGH);
    xbeecom.send("caca!\n");
    digitalWrite(13, LOW);
    delay(1000);
}

