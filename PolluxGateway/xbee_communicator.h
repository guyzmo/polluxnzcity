// Xbee Light Communicator Library
// Resources :
// http://www.circuitsathome.com/mcu/playing-xbee-part-4-api
// http://www.chasingtrons.com/main/2010/11/13/xbee-propeller-chip.html

#ifndef __XBEE_COMM_H__
#define __XBEE_COMM_H__

#include <unistd.h> // msleep

#include "beagle_serial.h"

// --------------------------------------------------------------- Tools

#ifdef VERBOSE
#   define debug_print(STR) printf("%s", STR);
#   define debug_printc(STR) printf("%02X", STR);
#   define debug_println(STR) printf("%s\n", STR);
#   define debug_print_hex(STR) printf("%02X", STR);
#   define debug_println_hex(STR) printf("%02X\n", STR);
#else
#   define debug_print(STR) 
#   define debug_printc(STR) 
#   define debug_println(STR) 
#   define debug_print_hex(STR) 
#   define debug_println_hex(STR) 
#endif

#define msleep(X) usleep(X*1000)
#define DEC 0
#define HEX 1
inline int min(int a, int b) { return (a < b) ? a : b; }

// --------------------------------------------------------------- Constants

/* API frame fields */
#define FRM_DLM	0x7e		//frame delimiter

/* frame types	*/
#define API_AT_CMD     0x08    //send at command
#define API_RM_CMD     0x17    //remote command request
#define API_XMIT_REQ   0x10    //transmit request	
#define API_ADDR_REQ   0x11    //explicit addressing transmit request	

#define AT_CMD_RESP    0x88    //AT Command Response
#define RM_CMD_RESP    0x97    //Remote Command Response
#define MODEM_STATUS   0x8A    //Modem Status
#define TX_STATUS      0x8B    //ZigBee Transmit Status
#define RX_PACKET      0x90    //ZigBee Receive Packet (AO=0)
#define RX_PACKET_IND  0x91    //ZigBee Explicit Rx Indicator (AO=1)
#define IO_SMPL_IND_RX 0x92    //ZigBee IO Data Sample Rx Indicator
#define SENSOR_RD_IND  0x94    //XBee Sensor Read Indicator (AO=0)
#define NODE_INDICATOR 0x95    //Node Identification Indicator (AO=0)

#define RX_PACKET_LEN  12      // 12 bytes of frame specific data between header and checksum

/* errors */

#define ERR_WRONG_FRM_DLM     -1
#define ERR_NOT_IMPLEMENTED   -2
#define ERR_UNKWNOWN_FRM_ID   -4
#define ERR_INVALID_CHECKSUM  -8

#ifndef TIMING
#   define TIMING 10
#endif

const uint8_t COORDINATOR_ADDR[8] = {0,0,0,0,0,0,0,0};
const uint8_t BROADCAST_ADDR[8] = {0,0,0,0,0,0,0xff,0xff};
const uint16_t BROADCAST_NET = 0xFFFE;

// --------------------------------------------------------------- Xbee Union Types

typedef union two_bytes {
    uint16_t i16;
    struct bytes {
        uint8_t lsb;
        uint8_t msb;
    } i8;
} uint8_16_u;

typedef struct xbee_frame {
    uint8_16_u length;
    uint8_t api_id;
    union frm_content {
        struct at {
            uint8_t frame_id;
            char command[2];
            uint8_t status;
            char values[100];
        } at;
        struct tx {
            uint8_t frame_id;
            uint8_16_u network_addr;
            uint8_t retries;
            uint8_t delivery_status;
            uint8_t discovery_status;
        } tx;
        struct rx {
            uint8_t source_addr[8];
            uint8_16_u network_addr;
            uint8_t options;
            uint8_t payload[100];
        } rx;
        struct modem_status {
            uint8_t status;
        } ms;
        uint8_t unknown[111];
    } content;
    uint16_t checksum;
} XBeeFrame;

// --------------------------------------------------------------- XBEE Lib

class XbeeCommunicator : public Serial {

    void xmit_req(uint8_t* addr64, uint16_t network, uint8_t nbytes, uint8_t* data, uint8_t frameid, uint8_t bradius, uint8_t options);
    void send_atcmd(int frameid, const char* at_command, const char* param_value);
    int rcpt_frame(XBeeFrame* frame);

    void print_data(uint8_t* data, uint16_t len, int type);
    void print_frame(XBeeFrame* frame);
        
    public:
        XbeeCommunicator(char* port);

        int begin (const int* panid, const int* vendorid);

        int read(bool no_esc=false);
        ssize_t write(uint8_t i);

        void send(const char* data);
        void recv();

        virtual void run (XBeeFrame* frame);
};

#endif // __XBEE_COMM_H__
