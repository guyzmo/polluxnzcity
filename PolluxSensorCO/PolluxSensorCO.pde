
#define PULSE 5
#define RL1 6
#define RL2 7

void setup() {
    Serial.begin(9600);
    pinMode(PULSE,OUTPUT);
    pinMode(RL1,OUTPUT);
    pinMode(RL2,OUTPUT);
    digitalWrite(PULSE,HIGH);
    digitalWrite(RL1,HIGH);
    digitalWrite(RL2,HIGH);
}

void loop () {
    float V1,V2;
    digitalWrite(RL1,LOW);
    delay(2);
    V1 = analogRead(A3);
    delay(2);
    digitalWrite(RL1,HIGH);
    digitalWrite(PULSE,LOW);
    delay(9);
    digitalWrite(RL2,LOW);
    delay(5);
    digitalWrite(RL2,HIGH);
    digitalWrite(PULSE,HIGH);
    V2 = analogRead(A3);
    V1 = V1*4.88;
    V2 = V2*4.88;
    Serial.print("V1 = ");
    Serial.print(V1);
    Serial.print(" mV ; V2 = ");
    Serial.print(V2);
    Serial.println(" mV");
    delay(900);
    

}


