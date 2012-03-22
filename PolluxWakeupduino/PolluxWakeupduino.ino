/**
 * Pollux'NZ City Project
 * Copyright (c) 2012 CKAB, hackable:Devices
 * Copyright (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * Copyright (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
 * 
 * Copyright (c) 2009 Andrew Rapp. For the XBee example origins.
 *
 * Pollux'NZ City is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pollux'NZ City is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project. If not, see <http://www.gnu.org/licenses/>.
 */

//#define VERBOSE
//#undef VERBOSE
//#define NO_ZIGBEE
//#undef NO_ZIGBEE

#define xbprintf(...) {char b[42]; sprintf(b, __VA_ARGS__); PolluxXbee::send_data(b);}

#ifdef VERBOSE
# define dbg_printf(...) {char b[42]; sprintf(b, __VA_ARGS__); PolluxXbee::send_data(b);}
#else
# define dbg_printf(...) 
#endif

#define nss_print(...) {nss.print(__VA_ARGS__);}
#define nss_println(str) {nss.println(str);}
# define dbg_printf(...) {char b[42]; sprintf(b, __VA_ARGS__); nss.println(b);}

#define TIMING 50

#ifndef BUFFER_SIZE
#  define BUFFER_SIZE 32
#endif

#include <SoftwareSerial.h>

#include <EEPROM.h>
#include <XBee.h>
#include <Wire.h>

#define LED_STA 11 // GREEN
#define LED_ERR 10 // RED
#define LED_XFR 9  // BLUE

#define I2C_BEGIN 0x23
#define I2C_END 0x29

SoftwareSerial nss(2,3);

/*
This example is for Series 2 XBee
 Sends a ZB TX request with the value of analogRead(pin5) and checks the status response for success
*/

void flashLed(int pin, int times, int wait) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);
    if (pin == LED_STA)
      analogWrite(pin, 8);

    if (i + 1 < times) {
      delay(wait);
    }
  }
}

//////////////////////////////////////////////////////// ZigBee XBee Library
// create the XBee object
XBee xbee = XBee();

// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0, 0x0);

XBeeResponse response = XBeeResponse();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

class PolluxXbee {
    public:
#ifdef NO_ZIGBEE
    static inline void send_data(char* payload) {
        Serial.println(payload);
    }
#else
    static void send_data(char* payload) {
        ZBTxRequest zbTx = ZBTxRequest(addr64, (uint8_t*)payload, strlen(payload));

        xbee.send(zbTx);

        // flash TX indicator
        nss_println(F("transmitting..."));
        flashLed(LED_XFR, 1, 100);

        // after sending a tx request, we expect a status response
        // wait up to half second for the status response
        if (xbee.readPacket(500)) {
            // got a response!

            // should be a znet tx status            	
            if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
                xbee.getResponse().getZBTxStatusResponse(txStatus);

                // get the delivery status, the fifth byte
                if (txStatus.getDeliveryStatus() == SUCCESS) {
                    // success.  time to celebrate
                    nss_println(F("transmit success"));
                    flashLed(LED_XFR, 10, 10);
                } else {
                    // the remote XBee did not receive our packet.
                    // is it powered on?
                    nss_println(F("transmit, NACK !"));
                    flashLed(LED_ERR, 2, 100);
                }
            }
        } else if (xbee.getResponse().isError()) {
            nss.print("Error reading packet.  Error code: ");  
            nss.println(xbee.getResponse().getErrorCode());
            flashLed(LED_ERR, 1, 100);
        } else {
            // local XBee did not provide a timely TX Status Response -- 
            // should not happen
            nss_println(F("should not happen !"));
            flashLed(LED_ERR, 2, 10);
        }
    }
#endif

#ifdef NO_ZIGBEE
    static inline char* recv_data() {
        Serial.read(32);
    }
#else
    static uint8_t recv_data(char* buffer) { 

        xbee.getResponse().reset();
        xbee.readPacketUntilAvailable();

        // is there something ?
        if (xbee.getResponse().isAvailable()) {
            switch (xbee.getResponse().getApiId()) {
                // is it a zb rx packet ?
                case ZB_RX_RESPONSE:
                    nss_print(F("ZB RX RESPONSE: "));
                    // now fill our zb rx class
                    xbee.getResponse().getZBRxResponse(rx);

                    switch (rx.getOption()) {
                        case ZB_PACKET_ACKNOWLEDGED:
                            // the sender got an ACK
                            flashLed(LED_XFR, 10, 10);
                            nss_println(F("ACK"));
                            break;
                        default:
                            // we got it (obviously) but sender didn't
                            // get an ACK
                            flashLed(LED_ERR, 1, 2000);
                            nss_println(F("missing ACK"));
                    }
                    for (uint8_t i=0;i<rx.getDataLength();++i)
                        buffer[i] = (char)rx.getData()[i];
                    nss_print(F("getting data: "));
                    nss_println(rx.getDataLength());
                    return rx.getDataLength();
                break;

                // the local XBee sends this response on certain events,
                // like association/dissociation
                case MODEM_STATUS_RESPONSE:
                    xbee.getResponse().getModemStatusResponse(msr);
                    nss_print(F("MODEM STATUS: "));

                    switch (msr.getStatus()) {
                        case 0: // Hardware Reset
                        case 1: // Watchdog timer reset
                        case 4: // Sync lost
                        case 5: // Coordinator realignment
                            nss_println(F("minor status, ignored"));
                            break; // ignore all status above
                        case 6: // Coordinator started
                            nss_println(F("coordinator start"));
                            flashLed(LED_XFR, 10, 10);
                        case ASSOCIATED: //2
                            // yay this is great.  flash led
                            nss_println(F("associated"));
                            flashLed(LED_XFR, 10, 10);
                            break;
                        case DISASSOCIATED: //3
                            // this is awful.. flash led to show our discontent
                            nss_println(F("disassociated"));
                            flashLed(LED_ERR, 10, 20);
                            break;
                        default:
                            // another status
                            nss_println(F("unknown status"));
                            flashLed(LED_XFR, 5, 10);
                    }
                    break;
                case 0x88: // AT_COMMAND_RESPONSE
                    nss_println(F("AT CMD RESPONSE"));
                    break;
                case 0x97: // REMOTE_COMMAND_RESPONSE
                    nss_println(F("REMOTE CMD RESPONSE"));
                    break;
                case 0x8B: // TRANSMIT_STATUS
                    nss_print(F("TRANSMIT STATUS: "));
                    if (!txStatus.isSuccess()) {
                        switch (txStatus.getDeliveryStatus()) {
                            case 0x00: // success
                                nss_println(F("success"));
                                break;
                            case 0x02: // CCA failure
                                nss_println(F("CCA failure"));
                                flashLed(LED_ERR, 1, 500);
                                break;
                            case 0x15: // invalid dest endpoint
                                nss_println(F("invalid dest endpoint"));
                                flashLed(LED_ERR, 2, 500);
                                break;
                            case 0x21: // net ack
                                nss_println(F("net ack"));
                                break;
                            case 0x22: // not joined to a network
                                nss_println(F("not joined to a network"));
                                flashLed(LED_ERR, 3, 500);
                                break;
                            case 0x23: // self-addressed
                                nss_println(F("self addressed"));
                                flashLed(LED_ERR, 4, 500);
                                break;
                            case 0x24: // address not found
                                nss_println(F("address not found"));
                                flashLed(LED_ERR, 5, 500);
                                break;
                            case 0x25: // route not found
                                nss_println(F("route not found"));
                                flashLed(LED_ERR, 6, 500);
                                break;
                            default:
                                nss_println(F("unknown status"));
                                flashLed(LED_ERR, 20, 20);
                                break;
                        }
                    } else
                        nss_println(F("success"));

                    
                case 0x10: // TRANSMIT_REQUEST
                    nss_println(F("transmit req"));
                    flashLed(LED_XFR, 2, 2000);
                    break;
                default:
                    nss_println(F("unknown API ID"));
                    // not something we were expecting
                    flashLed(LED_ERR, 2, 2000);    
            }
        } else if (xbee.getResponse().isError()) {
            nss_print(F("ERROR: "));
            nss_println(xbee.getResponse().getErrorCode());
            flashLed(LED_ERR, 5, 500);    
        }
        return 0;
    }
};

#endif

//////////////////////////////////////////////////////// ZigBee XBee Library


//////////////////////////////////////////////////////// I2C Library
#define I2C_CMD_INIT 1
#define I2C_LEN  2
#define I2C_TYPE 4
#define I2C_CMD_MEAS 8
#define I2C_GET 16

#define I2C_INT 1
#define I2C_FLT 2
#define I2C_DBL 4
#define I2C_CHR 8
#define I2C_STR 16

uint8_t send_cmd(uint8_t addr, uint8_t cmd) {
    int err;

    dbg_printf("*** send_cmd: trying device %02X", addr);

    Wire.beginTransmission(addr);
    delay(TIMING);

    dbg_printf("*** send_cmd: writing %d on %02X", cmd, addr);
    Wire.write(cmd);
    delay(TIMING);

    dbg_printf("*** send_cmd: detecting i2c tx error");

    if ((err = Wire.endTransmission()) != 0) {
        dbg_printf("*** error #%d in i2c write.", err);
        delay(TIMING);
        return err;
    } else 
        return 0;
    
}

uint8_t send_cmd(uint8_t addr, uint8_t cmd, uint8_t val) {
    int err;

    dbg_printf("*** send_cmd: trying device %02X", addr);

    Wire.beginTransmission(addr);
    delay(TIMING);

    dbg_printf("*** send_cmd: writing %02X for %02X on %02X", val, cmd, addr);
    Wire.write(cmd);
    Wire.write(val);
    delay(TIMING);

    dbg_printf("*** send_cmd: detecting i2c tx error");

    if ((err = Wire.endTransmission()) != 0) {
        dbg_printf("***  error #%d in i2c write.", err);
        delay(TIMING);
        return err;
    } else 
        return 0;
    
}

uint8_t recv(int addr, int n, char* buf) {
    int j=0;

    dbg_printf("*** recv: trying to get %d byte%s.", n, (n<1?"":"s"));

    Wire.requestFrom(addr, n);
    delay(TIMING);

    while (Wire.available() && j<=n) {
        delay(TIMING);
        buf[j] = Wire.read();
        delay(TIMING);
        dbg_printf("*** recv: rcving byte #%d: %02X", j, buf[j]);
        ++j;
    }
    if (j == 0) {
        dbg_printf("*** recv: error rcving bytes.");
    } else {
        buf[j] = '\0';
        dbg_printf("*** recv: received %s", buf);
    }
    return j;
}

typedef struct i2c_result {
    int command;
    int type;
    int len;
    union multi_val {
        char buffer[32];
        double d;
        float f;
        int i;
        char c;
    } v;
    void reset() { for (int i=0;i<32;++i) v.buffer[i] = 0; }
    i2c_result() { reset(); }
} i2c_result;

class Pollux {
    i2c_result res;
    uint8_t devlist[30];
    uint8_t devlist_idx;

    int send_command(uint8_t addr, uint8_t command) {
        if (send_cmd(addr, command) == 0) {
            xbprintf("*** Tx cmd %02X to device %02X", command, addr);
            recv(addr,1,res.v.buffer);
            if (send_cmd(0x27, I2C_TYPE) == 0) {
                recv(0x27,1,res.v.buffer);
                res.type = res.v.c;
                xbprintf("*** TYPE: %d", res.type);
            }
            return 0;
        }
        return -1;
    }

    void get_results(uint8_t addr) {
        if (send_cmd(addr, I2C_LEN) == 0) {
            xbprintf("*** Rx results from device %02X", addr);
            recv(0x27,1,res.v.buffer);
            res.len = res.v.i;
            xbprintf("*** LENGTH: %d", res.len);
            if (send_cmd(0x27, I2C_GET) == 0)
                recv(0x27,res.len,res.v.buffer);
        }
    }

    void send_results (uint8_t command, uint8_t addr) {
        uint8_t payload[BUFFER_SIZE];
        switch (res.type) {
            case I2C_INT:
                payload[0] = command;
                payload[1] = addr;
                payload[2] = (uint8_t)1;
                payload[3] = res.type;
                payload[4] = res.v.i;
                PolluxXbee::send_data((char*)payload);
                break;
            case I2C_FLT:
                payload[0] = command;
                payload[1] = addr;
                payload[2] = (uint8_t)sizeof(float);
                payload[3] = res.type;
                for (uint8_t i=0; i<sizeof(float);++i)
                    payload[i+4] = res.v.buffer[i];
                PolluxXbee::send_data((char*)payload);
                break;
            case I2C_DBL:
                payload[0] = command;
                payload[1] = addr;
                payload[2] = (uint8_t)sizeof(double);
                payload[3] = res.type;
                for (uint8_t i=0; i<sizeof(double);++i)
                    payload[i+4] = res.v.buffer[i];
                PolluxXbee::send_data((char*)payload);
                break;
            case I2C_STR:
                payload[0] = command;
                payload[1] = addr;
                payload[2] = (uint8_t)strlen(res.v.buffer);
                payload[3] = res.type;
                for (uint8_t i=0; i<=strlen(res.v.buffer);++i)
                    payload[i+4] = res.v.buffer[i];
                PolluxXbee::send_data((char*)payload);
                break;
            default:
                break;
        }
    }

    void store_to_eeprom() {
        EEPROM.write(0,0xFF);
        for (int i=0;i<=30;++i)
            EEPROM.write(i+1, devlist[i]);
    }
        
    public:
        Pollux() {
            for (int i=0;i<=30;++i)
                devlist[i] = 0xFF;
            devlist_idx=0;
        }

        bool i2c_network_stored() {
            if (EEPROM.read(0) == 0xFF)
                return true;
            return false;
        }

        void reload_i2c_network() {
            for (int i=0;i<=30;++i)
                devlist[i] = EEPROM.read(i+1);
        }

        void init_i2c_network() {
            analogWrite(LED_STA, 8);
            for (int addr=I2C_BEGIN;addr<I2C_END;++addr) {
                res.reset();
                if (send_command(addr,I2C_CMD_INIT) == 0) {
                    devlist[devlist_idx++] = addr;
                    get_results(addr);
                    send_results(I2C_CMD_INIT, addr);
                } else
                    xbprintf("*** No device at address %02X", addr);
            }
            store_to_eeprom();
            analogWrite(LED_STA, 0);
            flashLed(LED_STA, 1, 100);
        }
        void do_measures() {
            int i=0, addr;
            analogWrite(LED_STA, 8);
            while ((addr = devlist[i++]) != 0xFF) {
            /* XXX
                if (send_command(addr,I2C_GET_NB_MEAS) == 0) {
                    get_results(addr);
                    int nb = res.v.buffer[0];
                    for (int j=0; j<nb;++j) {
            */
                        if (send_command(addr,I2C_CMD_MEAS) == 0) {
                            get_results(addr);
                            send_results(I2C_CMD_MEAS, addr);
                        }
            /* XXX
                }
            */

            }
            analogWrite(LED_STA, 0);
        }
};

void setup() {
    pinMode(LED_STA, OUTPUT);
    pinMode(LED_ERR, OUTPUT);
    pinMode(LED_XFR, OUTPUT);

    nss.begin(9600);

#ifdef NO_ZIGBEE
    Serial.begin(9600);
#else
    xbee.begin(9600);
#endif
    Wire.begin();

    flashLed(LED_STA, 3, 100);

    Pollux pollux;

    // 1. Send WUD_WAKEUP to gateway
    // 2. wait for command
    // 3. if I2C_INIT :
    pollux.init_i2c_network();
    // 4. if I2C_MEAS :
    pollux.do_measures();
}

void loop() {   
    char buf[42];
    int n = PolluxXbee::recv_data(buf);
    nss_print("received ");
    nss_print(n);
    nss_print(" bytes : ");
    nss_println(buf[0]);
    nss_println(buf[1]);
    nss_println(buf[2]);
    nss_println(buf[3]);
    if (n != 0)
        flashLed(LED_XFR, 500, n);
    flashLed(LED_STA, 2, 100);
}




/*

//////////////////////////// POLLUX SENSOR BOARD ///////////////////////////

////////////////////////////////////////////////////////////////////////////

#include <Wire.h>

#define I2C_SLAVE_ADDR_TEMP   0x27         // I2C Temperature slave address
#define I2C_SLAVE_ADDR_DUST   0x26         // I2C Temperature slave address

#define DELAY_TEMP_READ   200              // wait 140 ms before ask temp value
#define DELAY_NEW_SAMPLE 1000              // time before new sample (1s)

float tempValue;
byte* floatPtr;

void setup(){
  Wire.begin();                            // start I2C Bus as Master
  Serial.begin(38400);                     // start serial to send dust output
  tempValue = 0;
}

void loop(){
  tempValue = 0;
  
  // **** GET DUST ***************************************
  
  Wire.beginTransmission(I2C_SLAVE_ADDR_DUST);  // transmit to slave device
  Wire.write(0xFF);                             // sends usuless data 
  Wire.endTransmission();                       // stop transmitting

  delay(DELAY_TEMP_READ);

  Wire.requestFrom(I2C_SLAVE_ADDR_DUST, 4);     // request 4 byte from slave (float = 4 bytes)

  if (Wire.available()) {
    floatPtr = (byte*) &tempValue;
        (*floatPtr) = Wire.read(); ++floatPtr;
        (*floatPtr) = Wire.read(); ++floatPtr;
        (*floatPtr) = Wire.read(); ++floatPtr;
        (*floatPtr) = Wire.read();
        Serial.print("Dust = ");      // print out slave byte to Serial monitor
        Serial.print(tempValue,DEC);  // print out slave byte to Serial monitor
        Serial.println(" mg");         // print out slave byte to Serial monitor
  } else {
    Serial.println("NaN");  
  }
  
  delay(DELAY_TEMP_READ);
  
  // **** GET TEMP ***************************************
  
  Wire.beginTransmission(I2C_SLAVE_ADDR_TEMP);  // transmit to slave device
  Wire.write(0xAA);                             // sends usuless data 
  Wire.endTransmission();                       // stop transmitting
  
  delay(DELAY_TEMP_READ);

  Wire.requestFrom(I2C_SLAVE_ADDR_TEMP, 4);     // request 4 byte from slave (float = 4 bytes)

  if (Wire.available()) {
    floatPtr = (byte*) &tempValue;
        (*floatPtr) = Wire.read(); ++floatPtr;
        (*floatPtr) = Wire.read(); ++floatPtr;
        (*floatPtr) = Wire.read(); ++floatPtr;
        (*floatPtr) = Wire.read();
        Serial.print("Temp = ");      // print out slave byte to Serial monitor
        Serial.print(tempValue,DEC);  // print out slave byte to Serial monitor
        Serial.println(" C");         // print out slave byte to Serial monitor
  } else {
    Serial.println("NaN");  
  }
  
  delay(DELAY_TEMP_READ);
  
  // **** USE FAN ***************************************
  
  Wire.beginTransmission(I2C_SLAVE_ADDR_TEMP);  // transmit to slave device
  Wire.write(0xBB);                             // sends usuless data 
  Wire.endTransmission();                       // stop transmitting
  
  delay(DELAY_TEMP_READ);
  delay(DELAY_TEMP_READ);
  
  Wire.beginTransmission(I2C_SLAVE_ADDR_TEMP);  // transmit to slave device
  Wire.write(0xCC);                             // sends usuless data 
  Wire.endTransmission();                       // stop transmitting

  delay(DELAY_TEMP_READ);
  
  // **** GET SPL ****************************************

  Wire.requestFrom(I2C_SLAVE_ADDR_TEMP, 1);     // request 4 byte from slave (float = 4 bytes)

  if (Wire.available()) {
    int soundValue = Wire.read();
	Serial.print("Sound = ");              // print out slave byte to Serial monitor
	Serial.print(soundValue,DEC);          // print out slave byte to Serial monitor
	Serial.println(" dB");                 // print out slave byte to Serial monitor
  } else {
    Serial.println("NaN");  
  }

  delay(DELAY_NEW_SAMPLE);
}


//////////////////////////// POLLUX SENSOR DUST ////////////////////////////

float dustValue;
byte* floatPtr;

void setup(){
  Wire.begin();                            // start I2C Bus as Master
  Serial.begin(38400);                     // start serial to send dust output
  dustValue = 0;
}

void loop(){
  dustValue = 0;
  
  Wire.beginTransmission(I2C_SLAVE_ADDR);  // transmit to slave device
  Wire.write(0xFF);                        // sends usuless data 
  Wire.endTransmission();                  // stop transmitting

  delay(DELAY_DUST_READ);

  Wire.requestFrom(I2C_SLAVE_ADDR, 4);     // request 4 byte from slave (float = 4 bytes)

  if (Wire.available()) {
    floatPtr = (byte*) &dustValue;
	(*floatPtr) = Wire.read(); ++floatPtr;
	(*floatPtr) = Wire.read(); ++floatPtr;
	(*floatPtr) = Wire.read(); ++floatPtr;
	(*floatPtr) = Wire.read();
	Serial.println(dustValue,DEC);     // print out slave byte to Serial monitor
  } else {
    Serial.println("NaN");  
  }

  delay(DELAY_NEW_SAMPLE);
}

////////////////////////////////////////////////////////////////////////////

*/
