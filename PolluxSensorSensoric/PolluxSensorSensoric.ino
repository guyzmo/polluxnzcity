#include "Statistic.h"

#define CO_PIN     0
#define NO2_PIN    1
#define ADC_DELAY 1
#define AOP_AMP   23.6
#define CO_COEF   132.41525
#define NO2_COEF  13.241525
Statistic myStats;
int i ;

void setup(void) {
  Serial.begin(9600);
  Serial.println("Demo Statistics lib Average");
  myStats.clear(); //explicitly start clean
  i = 0;
}

void loop(void) {
  for(i=0;i<5;i++) {
    myStats.add(analogRead(CO_PIN));
    delayMicroseconds(ADC_DELAY);
  }

  Serial.print("CO  : ");
  float result = (myStats.average()*0.0045-(0.04*AOP_AMP))*CO_COEF ;
  //if(result<0) result = 0;
  Serial.print(result, 4);

  Serial.print("ppm (dev : ");

  Serial.print(myStats.pop_stdev()*0.45, 4);
  Serial.println(" mV)");

  myStats.clear();
    for(i=0;i<5;i++) {
    myStats.add(analogRead(NO2_PIN));
    delayMicroseconds(ADC_DELAY);
  }
  
  Serial.print("NO2 : ");
  result = (myStats.average()*0.0045-(0.04*AOP_AMP))*NO2_COEF ;
  //if(result<0) result = 0;
  Serial.print(result, 4);

  Serial.print("ppm (dev : ");

  Serial.print(myStats.pop_stdev()*0.45, 4);
  Serial.println(" mV)");

  myStats.clear();
  
  Serial.println("---------------------------------");
  delay(1000);
}

// (0.040*AOP_AMP) = 0,944
// (0.200*AOP_AMP) = 4,720
// (4,720-0,944)   = 3,776
// 500 / 3,776 = 132.41525
// 50  / 3,776 = 13.241525
