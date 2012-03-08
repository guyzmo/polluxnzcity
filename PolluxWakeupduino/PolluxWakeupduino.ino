/**
 * Pollux'NZ City Project
 * Copyright (c) 2012 CKAB, hackable:Devices
 * Copyright (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * Copyright (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
 * 
 * Copyright (c) 2009 Andrew Rapp. For the XBee example origins.
 *
 * XBee-Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XBee-Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

//#define VERBOSE
#undef VERBOSE
//#define NO_ZIGBEE
#undef NO_ZIGBEE

#define xbprintf(...) {char b[42]; sprintf(b, __VA_ARGS__); send_data(b);}

#ifdef VERBOSE
# define dbg_printf(...) {char b[42]; sprintf(b, __VA_ARGS__); send_data(b);}
#else
# define dbg_printf(...) 
#endif

#define TIMING 50

#include <EEPROM.h>
#include <XBee.h>
#include <Wire.h>

/*
This example is for Series 2 XBee
 Sends a ZB TX request with the value of analogRead(pin5) and checks the status response for success
*/

// create the XBee object
XBee xbee = XBee();


// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0, 0x0);
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

const int statusLed = 12;
const int errorLed = 13;

void flashLed(int pin, int times, int wait) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);

    if (i + 1 < times) {
      delay(wait);
    }
  }
}

#ifdef NO_ZIGBEE
inline void send_data(char* payload) {
    Serial.println(payload);
}
#else
void send_data(char* payload) {
  ZBTxRequest zbTx = ZBTxRequest(addr64, (uint8_t*)payload, strlen(payload));

  xbee.send(zbTx);

  // flash TX indicator
  flashLed(statusLed, 1, 100);

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
        flashLed(statusLed, 10, 10);
      } else {
        // the remote XBee did not receive our packet. is it powered on?
        flashLed(errorLed, 2, 100);
      }
    }
  } else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");  
    //nss.println(xbee.getResponse().getErrorCode());
    flashLed(errorLed, 1, 100);
  } else {
    // local XBee did not provide a timely TX Status Response -- should not happen
    flashLed(errorLed, 2, 100);
  }
}
#endif

#define I2C_ADDR_NOIS   0x28         // I2C Sound slave address
#define I2C_ADDR_TEMP   0x27         // I2C Temperature slave address
#define I2C_ADDR_DUST   0x26         // I2C Temperature slave address

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

    dbg_printf("send_cmd: trying device %02X", addr);

    Wire.beginTransmission(addr);
    delay(TIMING);

    dbg_printf("send_cmd: writing %d on %02X", cmd, addr);
    Wire.write(cmd);
    delay(TIMING);

    dbg_printf("send_cmd: detecting i2c tx error");

    if ((err = Wire.endTransmission()) != 0) {
        dbg_printf(" error #%d in zigbee write.", err);
        delay(TIMING);
        return err;
    } else 
        return 0;
    
}

uint8_t send_cmd(uint8_t addr, uint8_t cmd, uint8_t val) {
    int err;

    dbg_printf("send_cmd: trying device %02X", addr);

    Wire.beginTransmission(addr);
    delay(TIMING);

    dbg_printf("send_cmd: writing %02X for %02X on %02X", val, cmd, addr);
    Wire.write(cmd);
    Wire.write(val);
    delay(TIMING);

    dbg_printf("send_cmd: detecting i2c tx error");

    if ((err = Wire.endTransmission()) != 0) {
        dbg_printf(" error #%d in zigbee write.", err);
        delay(TIMING);
        return err;
    } else 
        return 0;
    
}

uint8_t recv(int addr, int n, char* buf) {
    int j=0;

    dbg_printf("recv: trying to get %d byte%s.", n, (n<1?"":"s"));

    Wire.requestFrom(addr, n);
    delay(TIMING);

    while (Wire.available() && j<=n) {
        delay(TIMING);
        buf[j] = Wire.read();
        delay(TIMING);
        dbg_printf("recv:  receiving byte #%d: %02X", j, buf[j]);
        ++j;
    }
    if (j == 0) {
        dbg_printf("recv:  error receiving bytes.");
    } else {
        buf[j] = '\0';
        dbg_printf("recv:  received %s", buf);
    }
    return j;
}

typedef struct i2c_result {
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
            xbprintf("*** Sending command %02X to device %02X", command, addr);
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
            xbprintf("*** Getting results from device %02X", addr);
            recv(0x27,1,res.v.buffer);
            res.len = res.v.i;
            xbprintf("*** LENGTH: %d", res.len);
            if (send_cmd(0x27, I2C_GET) == 0)
                recv(0x27,res.len,res.v.buffer);
        }
    }

    void send_results (char* command) {
        switch (res.type) {
            case I2C_INT:
                xbprintf("%s: %d", command, res.v.i);
                break;
            case I2C_FLT:
                xbprintf("%s: %f", command, (double)res.v.f);
                break;
            case I2C_DBL:
                xbprintf("%s: %f", command, (double)res.v.d);
                break;
            case I2C_STR:
                xbprintf("%s: %s", command, (char*)res.v.buffer);
            default:
                xbprintf("no type");
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
            for (int addr=0x20;addr<0x30;++addr) {
                res.reset();
                if (send_command(addr,I2C_CMD_INIT) == 0) {
                    devlist[devlist_idx++] = addr;
                    get_results(addr);
                    send_results((char*)"INIT");
                } else
                    xbprintf("*** No device at address %02X", addr);
            }
            store_to_eeprom();
        }
        void do_measures() {
            int i=0, addr;
            while ((addr = devlist[i++]) != 0xFF) {
                if (send_command(addr,I2C_CMD_MEAS) == 0) {
                    get_results(addr);
                    send_results((char*)"RESULT");
                }
            }
        }
};

void setup() {
    pinMode(statusLed, OUTPUT);
    pinMode(errorLed, OUTPUT);

#ifdef NO_ZIGBEE
    Serial.begin(9600);
#else
    xbee.begin(9600);
#endif
    Wire.begin();

    flashLed(statusLed, 3, 100);
    flashLed(errorLed, 3, 100);

    Pollux pollux;

    //if (!pollux.i2c_network_stored())
        pollux.init_i2c_network();
    //else
    //    pollux.reload_i2c_network();
    pollux.do_measures();
}

void loop() {   
  delay(500);
}

