/**
 * Pollux'NZ City Project
 * Copyright (c) 2012 CKAB, hackable:Devices
 * Copyright (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * Copyright (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
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
#ifndef __POLLUX_COMMUNICATOR_H__
#define __POLLUX_COMMUNICATOR_H__

//#define VERBOSE
//#define NO_ZIGBEE

#include <LedWrapper.h>

#ifndef BUFFER_SIZE
#  define BUFFER_SIZE 32
#endif

class Pollux {
    i2c_result res;
    bool _halt;
    uint8_t devlist[30];
    uint8_t devlist_idx;

    int send_command(uint8_t addr, uint8_t command) {
        if (send_cmd(addr, command) == 0) {
            xbprintf("*** Tx cmd %02X to device %02X", command, addr);
            recv(addr,1,res.v.buffer);
            if (send_cmd(addr, I2C_TYPE) == 0) {
                recv(addr,1,res.v.buffer);
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
            recv(addr,1,res.v.buffer);
            res.len = res.v.i;
            xbprintf("*** LENGTH: %d", res.len);
            if (send_cmd(addr, I2C_GET) == 0)
                recv(addr,res.len,res.v.buffer);
        }
    }

    void send_results (uint8_t command, uint8_t addr, uint8_t reg) {
        uint8_t payload[BUFFER_SIZE];
        switch (res.type) {
            case I2C_INT:
                payload[0] = command;
                payload[1] = addr;
                payload[2] = reg;
                payload[3] = (uint8_t)2;
                payload[4] = res.type;
                payload[5] = res.v.buffer[0];
                payload[6] = res.v.buffer[1];
                PolluxXbee::send(payload, 7);
                break;
            case I2C_FLT:
                payload[0] = command;
                payload[1] = addr;
                payload[2] = reg;
                payload[3] = (uint8_t)sizeof(float);
                payload[4] = res.type;
                for (uint8_t i=0; i<sizeof(float);++i)
                    payload[i+5] = res.v.buffer[i];
                PolluxXbee::send(payload, 9);
                break;
            case I2C_DBL:
                payload[0] = command;
                payload[1] = addr;
                payload[2] = reg;
                payload[3] = (uint8_t)sizeof(double);
                payload[4] = res.type;
                for (uint8_t i=0; i<sizeof(double);++i)
                    payload[i+5] = res.v.buffer[i];
                PolluxXbee::send(payload, 13);
                break;
            case I2C_STR:
                payload[0] = command;
                payload[1] = addr;
                payload[2] = reg;
                payload[3] = (uint8_t)strlen(res.v.buffer);
                payload[4] = res.type;
                for (uint8_t i=0; i<=strlen(res.v.buffer);++i)
                    payload[i+5] = res.v.buffer[i];
                PolluxXbee::send(payload, 5+strlen(res.v.buffer));
                break;
            default:
                break;
        }
    }
        
    public:
        Pollux() {
            /* wait for and drop MODEM STATUS frames:
             * expected: 'hardware reset' and 'associated frames'
             */
            nss_println("**** WAKE MODE ****\n");
            this->_halt=false;

            flashLed(LED_STA, 3, 100);

            char buf1[42];
            int t = millis();
            while (millis() - t < 5000)
                PolluxXbee::recv_data_timeout(buf1, 500);
        }

    /** tell the beaglebone I'm awake 
     * i.e. send a command frame having only the command 'CMD_INIT' flag set
     */
    void send_wake_up() {
        nss_println("### SENDING WAKEUP");
        char buf[] = {CMD_INIT,0,0,0,0,0};
        PolluxXbee::send((uint8_t*)buf,1);
    }

    /** for a given measure request it from the sensor */
    void do_measure(uint8_t i2c_addr, uint8_t reg) {
        nss_println("### DO MEASURE");
        if (send_command(i2c_addr,CMD_MEAS) == 0) {
            get_results(i2c_addr);
            send_results(CMD_MEAS, i2c_addr, reg);
        }
    }

    uint8_t wait_for_command() {
        nss_println("### wait for command");
        char buf[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        int n = PolluxXbee::recv_data_timeout(buf, 5000);
        nss_print("### received ");
        nss_print(n);
        nss_print("### bytes : ");

        uint8_t cmd = buf[0];
        nss_print("### command: ");dbg_printf("%02X", buf[0]);
        uint8_t addr = buf[1];
        nss_print("### address: ");dbg_printf("%02X", buf[1]);
        uint8_t nb = buf[2];
        nss_print("### nbmeas: ");dbg_printf("%d", buf[2]);
        //uint8_t* params[29];
        //for (uint8_t i=0;i<nb;++i)
        //  params[i] = buf[3+i];
    
        switch (cmd) {
            case CMD_MEAS:
                for (uint8_t i=0;i<nb;++i)
                    do_measure(addr,i); //,params[i]);
                return 1;
            case CMD_HALT:
                this->_halt = true;
                return 0;
            default:
                nss_println("### unknown command");
        }
    }

    void halt() {
        nss_println("**** SLEEP MODE ****\n");
        // commit halting
        if (this->_halt) {
            nss_println("sending halt command");
            char buf[] = {CMD_HALT,0,0,0,0,0};
            PolluxXbee::send((uint8_t*)buf,1);
        }
        digitalWrite(LED_STA,LOW);
        // store watchdog sleep time
        // tell arduino to shuts itself down
    }
};

#endif // __POLLUX_COMMUNICATOR_H__
