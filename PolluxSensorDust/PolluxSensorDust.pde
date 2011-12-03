
void setup() {
    Serial.begin(9600);
    pinMode(9,OUTPUT);
    digitalWrite(9,LOW);
}

void loop () {
    float dust, Vo;
    digitalWrite(9,HIGH);
    delayMicroseconds(320); // pulse for 0.32ms
    digitalWrite(9,LOW);
    Vo = analogRead(A5);
    Vo = Vo*4.88;
    dust = (Vo-900)/500;
    Serial.print("Vo: ");
    Serial.print(Vo);
    Serial.print(" mV ; dust: ");
    Serial.print(dust);
    Serial.println(" mg/m3");
}


