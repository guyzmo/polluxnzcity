/*
                  +-\/-+
        (RESET)  1|    |8  VCC (2.7-5.5V)
      Temp Read  2|    |7  I2C   SCK -> Uno A5
          DEBUG  3|    |6  (PB4) NA
            GND  4|    |5  I2C   SDA -> Uno A4
                  +----+ 
*/


#define I2C_SLAVE_ADDR 0x27 // I2C slave address
#define DESC "Temperature sensor"

// #include <TinyWire.h>

#include "Statistic/Statistic.h"      // Arduino Statistic lib

#define TEMP_READ        3  // Temperature read on pin 2 (ADC3)
#define DELAY_ADC       10  // time before re-read ADC 10 us

//////////////////////////////////////////////////////////////////////////////////////////
//// TinyWire.h

extern "C" {
#include "usiTwiSlave.h"
}

#define I2C_CMD_INIT 1
#define I2C_CMD_MEAS 8

#define I2C_LEN  2
#define I2C_TYPE 4
#define I2C_GET 16

#define I2C_INT 1
#define I2C_FLT 2
#define I2C_DBL 4
#define I2C_CHR 8
#define I2C_STR 16

#define BUFFER_SIZE 32
#ifndef DESC
# define DESC "Unknown Sensor."
#endif

///// buffer.h
class Buffer {
    /* thanks to Akash Nath, akashnath{at}rediffmail{dot}com for this snippet,
       taken from http://datastructures.itgo.com/staque/queues/circular        */
    uint8_t queue[BUFFER_SIZE];
    uint8_t front;
    uint8_t rear;

    public:
        Buffer() : front(0), rear(-1) {
            for (int i=0;i<BUFFER_SIZE;++i) queue[i] = 0;
        }

        int push(uint8_t data) {
            if(rear == BUFFER_SIZE-1)
                rear = 0;
            else
                rear = rear + 1;
            if( front == rear && queue[front] != 0)
                return -1;
            else
                queue[rear] = data;
            return rear;
        }
        int push(char* str) {
            if (strlen(str) > BUFFER_SIZE)
                return -1;
            for (uint8_t i=0;i<strlen(str);++i)
                if (size() < BUFFER_SIZE)
                    push(str[i]);
                else
                    return -1;
            return size();
        }
        uint8_t pop() {
            int popped_front = 0;
            if (front == rear+1)
                return -1;
            else
            {
                popped_front = queue[front];
                queue[front] = 0;
                if (front == BUFFER_SIZE-1)
                    front = 0;
                else
                    front = front + 1;
            }
            return popped_front;
        }
        int size() {
            if ((rear-front+1) < 0 || (rear-front+1) > BUFFER_SIZE)
                return 0;
            return rear-front+1;
        }
        void clear() {
            front = 0;
            rear = -1;
            for (int i=0;i<BUFFER_SIZE;++i) queue[i] = 0;
        }
};
////

class TinyWire {
    static Buffer input_buffer;
    static Buffer output_buffer;
    static uint8_t last;

    static uint8_t type;

    static void (*_request_callback)(void);

    static void _on_receive_handler(uint8_t reg, uint8_t value);
    static uint8_t _on_request_handler(uint8_t reg);


    public:
        void begin(uint8_t i2c_slave_address) {
            usiTwiSlaveInit(i2c_slave_address, 
                            _on_request_handler, 
                            _on_receive_handler);
            this->type = 0xFF;
        }
        int write(uint8_t data) {
            if (output_buffer.size() == BUFFER_SIZE)
                return -1;
            output_buffer.push(data);
            return output_buffer.size();
        }
        int write(char* str) {
            return output_buffer.push(str);
        }
        uint8_t available() {
            return input_buffer.size();
        }
        void set_type(uint8_t t) {
            type = t;
        }
        uint8_t read() {
            return input_buffer.pop();
        }
        static void set_request_callback(void (*func)(void));
} Wire;

//////////////////////////////////////////////////////////////////////////////////////////
//// TinyWire.cpp

Buffer TinyWire::input_buffer;
Buffer TinyWire::output_buffer;
uint8_t TinyWire::last = 0xFF;
uint8_t TinyWire::type = 0xFF;
void (*TinyWire::_request_callback)(void);

void TinyWire::set_request_callback(void (*func)(void)) {
    _request_callback = func;
}

void TinyWire::_on_receive_handler(uint8_t reg, uint8_t value) {
    switch (reg) {
        default:
            input_buffer.push(reg);
            input_buffer.push(value);
    }
}

uint8_t TinyWire::_on_request_handler(uint8_t reg) {
    switch (reg) {
        case I2C_CMD_INIT:

            digitalWrite(PB4, HIGH);
            delay(500);
            digitalWrite(PB4, LOW);
            output_buffer.clear();
            output_buffer.push((char*)DESC);
            last = I2C_GET;
            type = I2C_STR;
            return 0;
        case I2C_CMD_MEAS:
            output_buffer.clear();

            digitalWrite(PB4, HIGH);
            delay(500);
            digitalWrite(PB4, LOW);
            _request_callback();
            return 0;

        case I2C_TYPE:
            return type;
        case I2C_LEN:
            return output_buffer.size();
        case I2C_GET:
            return output_buffer.pop();
        default:
            if (output_buffer.size() > 0)
                if (last == I2C_GET)
                    return output_buffer.pop();
                else
                    return 0x00;
            else
                last = 0xFF;
            return 0x00;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////

void sensor_meas() {
    Statistic temp;
    float     result;
    
    digitalWrite(PB4, HIGH);
    delay(500);
    digitalWrite(PB4, LOW);

    temp.clear();
    result  = 0;

    for(uint8_t i=0;i<50;i++) {          // make 50 temperature sample
        temp.add(analogRead(TEMP_READ)); // read and store temperature sensor output
        delayMicroseconds(DELAY_ADC);    // wait 10 us to stabilise ADC
    }

    result = ((temp.average()*0.0049)-0.5)*100 ; // convert the result into a human readable output

    if(result < 0) result = 0;           // avoid eratic datas
    temp.clear();                        // clear statistics to avoid leack and data stacking

    Wire.set_type(I2C_FLT);
    Wire.write((uint8_t)((((int)result)>>8)&0xFF));  // MSB
    Wire.write((uint8_t)(((int)result)&0xFF));       // LSB
}

void setup() {
    pinMode(PB4,OUTPUT);
    Wire.begin(I2C_SLAVE_ADDR); // slave address
    Wire.set_request_callback(&sensor_meas);
}

void loop() {
 /* nop */
}

