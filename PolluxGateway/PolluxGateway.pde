#include <SoftwareSerial.h>

SoftwareSerial mySerial(2,3);

void setup() {
  Serial.begin(9600);
  Serial.println("-- Start --");
  mySerial.begin(9600);
}

void loop() {
  if (mySerial.available()) Serial.write(mySerial.read());
  if (Serial.available())   mySerial.write(Serial.read());
}

