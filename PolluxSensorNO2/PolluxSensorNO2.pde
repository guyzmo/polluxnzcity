
void setup() {
    Serial.begin(9600);
}

void loop () {
    float vs, no2;
    vs = analogRead(A0);
    vs = vs*4.88/1000;
    no2 = 3300/((5-vs)*vs);
    Serial.print("NO2: ");
    Serial.print(no2);
    Serial.println(" Ohms");
}


