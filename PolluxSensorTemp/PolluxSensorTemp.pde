
void setup() {
    Serial.begin(9600);
}

void loop () {
    int temp;
    temp = analogRead(A1);
    temp = (temp-102)/2;
    Serial.print("Temperature: ");
    Serial.println(temp);
}


