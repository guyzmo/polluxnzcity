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
#ifndef __XBEEWRAPPER_H__
#define __XBEEWRAPPER_H__

#include <LedWrapper.h>

//#define VERBOSE
//#define NO_ZIGBEE

//////////////////////////////////////////////////////// ZigBee XBee Library
#ifndef BUFFER_SIZE
#  define BUFFER_SIZE 32
#endif

#ifdef VERBOSE
# define xbprintf(...) {char b[42]; sprintf(b, __VA_ARGS__); PolluxXbee::send_data(b);}
# define nss_print(...) {nss.print(__VA_ARGS__);}
# define nss_println(str) {nss.println(str);}
# define dbg_printf(...) {char b[42]; sprintf(b, __VA_ARGS__); nss.println(b);}
#else
# define xbprintf(...)
# define nss_print(...)
# define nss_println(str)
# define dbg_printf(...)
#endif

#define TIMING 50

#ifdef VERBOSE
#include <SoftwareSerial.h>
SoftwareSerial nss(3,4);
#endif 

#include <XBee.h>

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
    static void send(uint8_t* payload, uint8_t len) {
        ZBTxRequest zbTx = ZBTxRequest(addr64, payload, len);

        xbee.send(zbTx);

        // flash TX indicator
        nss_print(F("### transmitting: "));
        flashLed(LED_XFR, 1, 10);

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
                    nss_println(F("success"));
                    flashLed(LED_XFR, 10, 10);
                } else {
                    // the remote XBee did not receive our packet.
                    // is it powered on?
                    nss_println(F("NACK !"));
                    flashLed(LED_ERR, 2, 100);
                }
            }
        } else if (xbee.getResponse().isError()) {
            nss_print("Error reading packet.  Error code: ");  
            nss_println(xbee.getResponse().getErrorCode());
            flashLed(LED_ERR, 1, 100);
        } else {
            // local XBee did not provide a timely TX Status Response -- 
            // should not happen
            nss_println(F("should not happen !"));
            flashLed(LED_ERR, 2, 10);
        }
    }

    static void send_data(char* payload) {
        send((uint8_t*)payload, strlen(payload));
    }

    static void send_atcmd(uint8_t* cmd, uint8_t* value, uint8_t length) {
        AtCommandRequest at_cmd_rq = AtCommandRequest(cmd, value, length);
        AtCommandResponse at_cmd_resp = AtCommandResponse();

        xbee.send(at_cmd_rq);

        // wait up to 5 seconds for the status response
        if (xbee.readPacket(5000)) {
            // got a response!

            // should be an AT command response
            if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
                xbee.getResponse().getAtCommandResponse(at_cmd_resp);

                if (at_cmd_resp.isOk()) {
                    nss_print("Command [");
                    nss_print(at_cmd_resp.getCommand()[0]);
                    nss_print(at_cmd_resp.getCommand()[1]);
                    nss_println("] was successful!");

                    if (at_cmd_resp.getValueLength() > 0) {
                        nss_print("Command value length is ");
                        nss_print(at_cmd_resp.getValueLength(), DEC);
                        nss_println("");

                        nss_print("Command value: ");

                        for (int i = 0; i < at_cmd_resp.getValueLength(); i++) {
                            nss_print(at_cmd_resp.getValue()[i], HEX);
                            nss_print(" ");
                        }

                        nss_println("");
                    }
                } 
                else {
                    nss_print("Command return error code: ");
                    nss_print(at_cmd_resp.getStatus(), HEX);
                    nss_println("");
                }
            } else {
                nss_print("Expected AT response but got ");
                nss_print(xbee.getResponse().getApiId(), HEX);
            }   
        } else {
            // at command failed
            if (xbee.getResponse().isError()) {
                nss_print("Error reading packet.  Error code: ");  
                nss_println(xbee.getResponse().getErrorCode());
            } 
            else {
                nss_print("No response from radio");  
            }
        }
    }

#endif

#ifdef NO_ZIGBEE
    static uint8_t recv_data_timeout(char* buffer, int timeout) {
        return 0;
    }
    static uint8_t recv_data_blocking(char* buffer) { 
        char buf[BUFFER_SIZE]
         Serial.read(BUFFER_SIZE);
    }
#else
    static uint8_t recv_data_timeout(char* buffer, int timeout) {
        xbee.getResponse().reset();
        if (!xbee.readPacket(timeout))
            return 0xFF;

        return recv_data(buffer);
    }
    static uint8_t recv_data_blocking(char* buffer) { 
        xbee.getResponse().reset();
        xbee.readPacketUntilAvailable();
        return recv_data(buffer);
    }

    static uint8_t recv_data(char* buffer) {

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
                        case 0: nss_println(F("hardware reset"));           flashLed(LED_XFR, 3, 10); break;
                        case 1: nss_println(F("watchdog timer reset"));     flashLed(LED_XFR, 3, 10); break;
                        case 4: nss_println(F("sync lost"));                flashLed(LED_XFR, 3, 10); break;
                        case 5: nss_println(F("coordinator realignment"));  flashLed(LED_XFR, 3, 10); break;
                        case 6: nss_println(F("coordinator start"));        flashLed(LED_XFR, 3, 10); break;
                        case ASSOCIATED: nss_println(F("associated"));      flashLed(LED_XFR, 10, 10);break;
                        case DISASSOCIATED: nss_println(F("disassociated"));flashLed(LED_ERR, 10, 20);break;
                        default:nss_println(F("unknown status"));           flashLed(LED_ERR, 5, 20); break;
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
                            case 0x00: nss_println(F("success"));break;
                            case 0x02: nss_println(F("CCA failure"));            flashLed(LED_ERR, 1, 500);break;
                            case 0x15: nss_println(F("invalid dest endpoint"));  flashLed(LED_ERR, 1, 500);break;
                            case 0x21: nss_println(F("net ack"));                flashLed(LED_ERR, 1, 500);break;
                            case 0x22: nss_println(F("not joined to a network"));flashLed(LED_ERR, 1, 500);break;
                            case 0x23: nss_println(F("self addressed"));         flashLed(LED_ERR, 1, 500);break;
                            case 0x24: nss_println(F("address not found"));      flashLed(LED_ERR, 1, 500);break;
                            case 0x25: nss_println(F("route not found"));        flashLed(LED_ERR, 1, 500);break;
                            default: nss_println(F("unknown status"));           flashLed(LED_ERR, 1, 500);break;
                        }
                    } else { 
                        nss_println(F("success"));
                        flashLed(LED_XFR, 2, 10);
                    }
                case 0x10: // TRANSMIT_REQUEST
                    nss_println(F("transmit req"));
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
#endif
};

//////////////////////////////////////////////////////// ZigBee XBee Library
#endif // __XBEEWRAPPER_H__
