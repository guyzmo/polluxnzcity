const int ledPin =  12;      // the number of the LED pin
const int thresholdvalue=140;//The threshold to turn the led on
void setup() {
     Serial.begin(38400);
     pinMode(ledPin, OUTPUT); 
}

#define NB_MEAS 512

// sensor  dbmeter
// 680  == 345 == 90dB
// 672  == 330 == 85dB
// 620  == 415 == 80dB
// 450? == 397 == 75dB
// 420? == 390 == 70dB
// 270  == 331 == 66dB


int convert_to_dB(float val) {
    // 10.0 == 50dB
    if (val<10.0)        return 50; // <50dB
    // 15.0 == 55dB
    else if (val<15.0)   return 55; // <55dB
    // 30.0 == 60dB
    else if (val<30.0)   return 60; // <60dB
    // 70.0 == 65dB
    else if (val<70.0)   return 65; // <65dB
    // 140.0 == 70dB
    else if (val<140.0)  return 70; // <70dB
    // 230.0 == 75dB
    else if (val<230.0)  return 75; // <75dB
    // 250.0 == 80dB
    else if (val<255.0)  return 80; // <80dB
    // 255.0 == 85dB == 90 dB
    else if (val<255.0)  return 85; // <85dB
    else if (val>=255.0) return 90; // >85dB
}

const char* convert_to_dB_ascii(float val) {
    // 10.0 == 50dB
    if (val<11.0)        return "  <50dB"; // <50dB
    // 15.0 == 55dB
    else if (val<16.0)   return "50-55dB"; // <55dB
    // 30.0 == 60dB
    else if (val<31.0)   return "55-60dB"; // <60dB
    // 70.0 == 65dB
    else if (val<71.0)   return "60-65dB"; // <65dB
    // 140.0 == 70dB
    else if (val<141.0)  return "65-70dB"; // <70dB
    // 230.0 == 75dB
    else if (val<230.0)  return "70-75dB"; // <75dB
    // 250.0 == 80dB
    else if (val<251.0)  return "75-80dB"; // <80dB
    // 255.0 == 85dB == 90 dB
    else if (val<256.0)  return "80-85dB"; // <85dB
    // and over...
    else if (val>=256.0) return "  >85dB"; // >85dB
}

int max_db=0;
int i=0;

int sensor[NB_MEAS];

void loop() {
  int meterValue = analogRead(A1);
  int sensorValue = analogRead(A0);//use A0 to read the electrical signal
  float measure = 0;

  sensor[i] = sensorValue;

  if (i == NB_MEAS) {
    for (int j=0;j<NB_MEAS;++j)
        measure += sensor[j];
    measure = measure/NB_MEAS;
    Serial.print(measure);
    Serial.print(" ");
    Serial.println(convert_to_dB_ascii(measure));
    if(measure>thresholdvalue)
        digitalWrite(ledPin,HIGH);//if the value read from A0 is larger than 400,then light the LED
    else
        digitalWrite(ledPin,LOW);

    i = 0;
  } else
    ++i;

 }

