#define API_ESCAPED_MODE

#include "XbeeCommunicatorLight.h"

XbeeCommunicator xbeecom(2,3);
const uint8_t panid[2] = {0x4,0x2};
const uint8_t venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};

void setup() {
    Serial.begin(9600);
    Serial.println("-- Start --");

    xbeecom.begin(panid,venid);

    Serial.println("-- Sending --");
    xbeecom.send("caca!\n");
    if (xbeecom.available() > 7) {
        Serial.println(xbeecom.recv());
        //while (xbeecom.available()) {
        //   Serial.print(xbeecom.read(),HEX); Serial.print(" ");
        //
        //Serial.println();
    }}

void loop() {
    if (xbeecom.available() > 2) {
        Serial.println(xbeecom.recv());
        //while (xbeecom.available()) {
        //   Serial.print(xbeecom.read(),HEX); Serial.print(" ");
        //
        //erial.println();
    } else {
        Serial.print(xbeecom.available()); Serial.println("|");
    }
    delay(1000);
}

