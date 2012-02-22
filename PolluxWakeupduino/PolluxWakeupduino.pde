void setup() {
    Serial.begin(9600);
    pinMode(13,OUTPUT);
}

void loop() {
    Serial.print('F');
    delay(500);
    if (Serial.available()) {
        char c = Serial.read();
        if (c > 32 && c < 127) {
            digitalWrite(13,HIGH);
            delay(500);
            digitalWrite(13,LOW);
        } else
            delay(500);
    }
}

