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

// --------------------------------------------------------------- Xbee Union Types

const uint8_t COORDINATOR_ADDR[8] = {0,0,0,0,0,0,0,0};
const uint8_t BROADCAST_ADDR[8] = {0,0,0,0,0,0,0xff,0xff};
const uint16_t BROADCAST_NET = 0xFFFE;

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

    /* sends transmit request to addr, network              */
    /* 256 bytes max.payload; Xbee max.payload is 100 bytes */
    void xmit_req(uint8_t* addr64, uint16_t network, uint8_t nbytes, uint8_t* data, uint8_t frameid, uint8_t bradius, uint8_t options) {
        int checksum;
        uint8_t len;

        char* byte_ptr = NULL;

        debug_print("Sending frame: '");
        debug_print_hex(FRM_DLM);
        Serial::write(FRM_DLM);                                // 1. SEND FRAME DELIMITER
        // data's length + frm type (1) + frm id (1) + addr (8) + network (4) + bradius (1) + options (1)
        len = nbytes + 16;                 
        this->write((uint8_t)(len>>8));                                // 2. SEND LENGTH
        this->write((uint8_t)(len&0xFF));

        this->write(API_XMIT_REQ);              checksum = API_XMIT_REQ;       // 3. SEND FRAME API Identifier (transmission request)
        this->write(frameid);               checksum += frameid;       // 4. SEND FRAME Unique ID
        for (int8_t i=7;i>=0;--i) {
            this->write(*(addr64+i));       checksum += *(addr64+i);   // 5. SEND 64bit ADDRESS OF REMOTE MODULE
        }
        byte_ptr = (char*)&network;
        for (int8_t i=1;i>=0;--i){
            this->write(*(byte_ptr+i));     checksum += *(byte_ptr+i); // 6. SEND PAN ID
        }
        this->write(bradius);               checksum += bradius;       // 7. BROADCAST
        this->write(options);               checksum += options;       // 8. OPTIONS

        // 9. SEND PAYLOAD
        for (uint8_t i=0;i<nbytes;++i) {
            this->write(data[i]);           checksum += data[i];
        }

        // 10. SEND CHECKSUM
        checksum = 0xff - (checksum);
        this->write(checksum);
        debug_println("'");
    }

    void send_atcmd(int frameid, const char* at_command, const char* param_value) {
        printf("send_at_cmd(%s)\n", at_command);
        uint16_t len = 0;
        int checksum;

        debug_print("Sending frame: '");
        debug_print_hex(FRM_DLM);
        Serial::write(FRM_DLM);                                         // 1. SEND FRAME DELIMITER

        // 1B for frm id + 1B for api id + size of the command (usually 2B) + size of the param (usually 0B)
        len = strlen(at_command) + strlen(param_value)+2;
        this->write((uint8_t)(len>>8));                                 // 2. SEND FRAME LENGTH
        this->write((uint8_t)(len&0xFF));
        
        this->write(API_AT_CMD);            checksum = API_AT_CMD;      // 3. SEND FRAME API Identifier (transmission request)
        this->write(frameid);               checksum += frameid;        // 4. SEND FRAME Unique ID
        
        for (uint8_t i=0;i<strlen(at_command);++i) {
            this->Serial::write(at_command[i]);     checksum += at_command[i];  // 5. SEND AT COMMAND
        }

        for (uint8_t i=0;i<strlen(param_value);++i) {
            this->Serial::write(param_value[i]);    checksum += param_value[i]; // 6. SEND PARAMETERS
        }

        checksum = 0xff - (checksum);                                   // 10. SEND CHECKSUM
        this->write(checksum);
        debug_println("'");
    }

    int rcpt_frame(XBeeFrame* frame) {
        uint8_t b;
        frame->checksum = 0;
        //1. GET FRAME DELIMITER
        debug_print("[FRM_DLM]");
        if (this->read(true) != FRM_DLM)
            return ERR_WRONG_FRM_DLM;
        
        //2. GET CONTENT LENGTH
        debug_print("[LEN]");
        frame->length.i8.msb = this->read();
        frame->length.i8.lsb = this->read();

        //3. GET COMMAND
        debug_print("[FRM_ID]");
        frame->api_id = this->read();
        switch (frame->api_id) {
            case AT_CMD_RESP:               
                frame->api_id = AT_CMD_RESP;                          frame->checksum += AT_CMD_RESP;
                frame->content.at.frame_id = this->read();            frame->checksum += frame->content.at.frame_id;
                frame->content.at.command[0] = this->read();          frame->checksum += frame->content.at.command[0];
                frame->content.at.command[1] = this->read();          frame->checksum += frame->content.at.command[1];
                frame->content.at.status = this->read();              frame->checksum += frame->content.at.status;
                for (uint8_t i=0;i<frame->length.i16-5;++i) {
                    frame->content.at.values[i] = this->read();       frame->checksum += frame->content.at.values[i];
                }
                break;

            case TX_STATUS:
                frame->api_id = TX_STATUS;                            frame->checksum += TX_STATUS;
                frame->content.tx.frame_id = this->read();            frame->checksum += frame->content.tx.frame_id;
                frame->content.tx.network_addr.i8.msb = this->read(); frame->checksum += frame->content.tx.network_addr.i8.msb;
                frame->content.tx.network_addr.i8.lsb = this->read(); frame->checksum += frame->content.tx.network_addr.i8.msb;
                frame->content.tx.retries = this->read();             frame->checksum += frame->content.tx.retries;
                frame->content.tx.delivery_status = this->read();     frame->checksum += frame->content.tx.delivery_status;
                frame->content.tx.discovery_status = this->read();    frame->checksum += frame->content.tx.discovery_status;
                break;

            case RX_PACKET:
                frame->api_id = RX_PACKET;                            frame->checksum += RX_PACKET;
                frame->content.rx.network_addr.i8.msb = this->read(); frame->checksum += frame->content.rx.network_addr.i8.msb;
                frame->content.rx.network_addr.i8.lsb = this->read(); frame->checksum += frame->content.rx.network_addr.i8.lsb;
                for (uint8_t i=0;i<8;++i) {
                    frame->content.rx.source_addr[i] = this->read();  frame->checksum += frame->content.rx.source_addr[i];
                }
                frame->content.rx.options = this->read();             frame->checksum += frame->content.rx.options;
                for (uint8_t i=0;i<frame->length.i16-RX_PACKET_LEN;++i) {
                    frame->content.rx.payload[i] = this->read();      frame->checksum += frame->content.rx.payload[i];
                }
                frame->content.rx.payload[frame->length.i16-RX_PACKET_LEN] = '\0';
                break;

            case NODE_INDICATOR:
                frame->api_id = NODE_INDICATOR;                       frame->checksum = NODE_INDICATOR;

                debug_print("[SRC_ADDR(8)]");
                for (uint8_t i=0;i<8;++i) {
                    frame->content.rx.source_addr[i] = this->read();  frame->checksum += frame->content.rx.source_addr[i];
                }

                debug_print("[NET_ADDR(2)]");
                frame->content.rx.network_addr.i8.msb = this->read(); frame->checksum += frame->content.rx.network_addr.i8.msb;
                frame->content.rx.network_addr.i8.lsb = this->read(); frame->checksum += frame->content.rx.network_addr.i8.lsb;

                debug_print("[OPTIONS]");
                frame->content.rx.options = this->read();             frame->checksum += frame->content.rx.options;

                debug_print("[NET_ADDR(2)]");
                b = this->read(); frame->checksum += b;
                b = this->read(); frame->checksum += b;

                debug_print("[SRC_ADDR(8)]");
                for (uint8_t i=0;i<8;++i) {
                    b = this->read();  frame->checksum += b;
                }

                {
                    uint8_t i=0;
                    debug_print("[PAYLOAD]");
                    b = -1;
                    while (b != 0x0) {
                        b = this->read();      frame->checksum += b;
                        frame->content.rx.payload[i] = b;
                        ++i;
                    }
                    frame->content.rx.payload[i] = '\0';
                }
                debug_print("[PRT_ADDR(2)]");
                frame->checksum += this->read(); // parent address (2 bytes)
                frame->checksum += this->read();
                debug_print("[DEV_TYPE]");
                frame->checksum += this->read(); // device type (1 byte)
                debug_print("[SRC_ACT]");
                frame->checksum += this->read(); // source action (1 byte)
                debug_print("[PROFILE(2)]");
                frame->checksum += this->read(); // profile id (2 bytes)
                frame->checksum += this->read(); 
                debug_print("[MANUFACTURER(2)]");
                frame->checksum += this->read(); // manufacturer id (2bytes)
                frame->checksum += this->read();

                break;

            case MODEM_STATUS:
                frame->api_id = MODEM_STATUS;                       frame->checksum = MODEM_STATUS;

                debug_print("[CMD_DATA]");
                frame->content.ms.status = this->read();                   frame->checksum += frame->content.ms.status;

                break;

            default:
                for (uint8_t i = 0; i < min(frame->length.i16,100); ++i) {
                    frame->content.unknown[i] = this->read(); }
                return ERR_UNKWNOWN_FRM_ID;
        }

        debug_print("[CHECKSUM(");
        debug_printc(0xFF-(frame->checksum&0xFF));
        debug_print(")]");
        //4. validate checksum
        if (0xFF-(frame->checksum&0xFF) != (uint16_t)this->read())
            return ERR_INVALID_CHECKSUM;

        return 1;
    }

    void print_data(uint8_t* data, uint16_t len, int type) {
        if (type == HEX) {
            for (uint16_t i=0;i<min(len,100);++i)
                printf("%02X", (uint8_t)data[i]);
        } else  {
            for (uint16_t i=0;i<min(len,100);++i)
                printf("%c", (uint8_t)data[i]);
        }
        printf("\n");
    }
    
    void print_frame(XBeeFrame* frame) {
        printf("-- Frame --\n");
        printf("length: "); printf("%d\n", frame->length.i16);
        printf("api_id: "); printf("%02X", frame->api_id);
        switch (frame->api_id) {
            case NODE_INDICATOR:
                printf(" ; NODE IDENTIFICATION INDICATOR\n");
                printf("src_ad: "); print_data(frame->content.rx.source_addr,8,HEX);
                printf("net_ad: "); printf("%02X\n", frame->content.rx.network_addr.i16);
                printf("recvopt: "); printf("%02X\n", frame->content.rx.options,HEX);
                printf("nodname: "); printf("%s\n", (char*)frame->content.rx.payload);
                break;
            case MODEM_STATUS:
                printf(" ; MODEM STATUS\n");
                printf("status: ");
                switch (frame->content.ms.status) {
                    case 0: printf("Hardware reset\n"); break;
                    case 1: printf("Watchdog timer reset\n"); break;
                    case 2: printf("Associated\n"); break;
                    case 3: printf("Disassociated\n");  break;
                    case 4: printf("Synchronization Lost (Beacon-enabled only)\n"); break;
                    case 5: printf("Coordinator realignment\n"); break;
                    case 6: printf("Coordinator started\n"); break;
                }
                break;
            case AT_CMD_RESP:
                printf(" ; AT COMMAND RESPONSE\n");
                printf("frm_id: "); printf("%02X\n", frame->content.at.frame_id);
                printf("at_cmd: "); printf("%s\n", frame->content.at.command);
                printf("status: "); printf("%02X\n", frame->content.at.status);
                printf("values: "); printf("%02X\n", frame->content.at.values);
                break;

            case TX_STATUS:
                printf(" ; TX STATUS\n");
                printf("frm_id: "); printf("%s\n", frame->content.tx.frame_id);
                printf("net_ad: "); printf("%02X\n", frame->content.tx.network_addr.i16);
                printf("retry : "); printf("%s\n", frame->content.tx.retries);
                printf("delivs: "); printf("%02X\n", frame->content.tx.delivery_status);
                printf("discos: "); printf("%02X\n", frame->content.tx.discovery_status);
                break;

            case RX_PACKET:
                printf(" ; RX PACKET\n");
                printf("src_ad: "); print_data(frame->content.rx.source_addr,8,HEX);
                printf("net_ad: "); printf("%02X\n", frame->content.rx.network_addr.i16);
                printf("recvopt: "); printf("%02X\n", frame->content.rx.options);
                printf("payload: "); print_data(frame->content.rx.payload,frame->length.i16,DEC);
                break;

            default:
                printf(" /!\\ UNKNOWN PACKET TYPE /!\\\n");
                printf("content: "); print_data(frame->content.unknown,frame->length.i16, HEX);
        }
        printf("-- End of Frame --\n");
    }
        
    public:
        XbeeCommunicator(char* port) : Serial(port) {}

        int begin (const int* panid, const int* vendorid) {
            printf("XbeeCommunicator.begin()\n");
            int ret = Serial::begin(B9600);
            if (ret <= 0)
                return ret;

            // discovery
            this->send_atcmd(3, "ND", "");
            //msleep(1000);
            this->send_atcmd(4, "MY", "");
                    
            return ret;
        }

        int read(bool no_esc=false) {
            try {
                int c = Serial::read();
                msleep(TIMING);
                debug_print_hex(c);
                debug_print(" ");
#ifdef API_ESCAPED_MODE
                if (no_esc == false && c == 0x7D)
                    return Serial::read()^0x20;
#endif
                return c;
            } catch (SerialException e) {
                e.print_msg();
            }
        }

        ssize_t write(uint8_t i) {
            msleep(TIMING);
            size_t s=0;
            debug_print(" ");
            switch (i) {
#ifdef API_ESCAPED_MODE
                case 0x7E:
                case 0x7D:
                case 0x11:
                case 0x13:
                    s+=Serial::write(0x7D);
                    s+=Serial::write(0x20^i);
                    return s;
#endif
                default:
                    debug_print_hex(i);
                    return s+Serial::write((uint8_t)i);
            }
        }


        void send(const char* data) {
            this->xmit_req(/* hardware addr */ (uint8_t*)COORDINATOR_ADDR,
                           /* network bcast */ (uint16_t)BROADCAST_NET, 
                           /* data's length */ strlen(data), 
                           /* data          */ (uint8_t*)data, 
                           /* frame id      */ (uint8_t)1, 
                           /* bcast radius  */ (uint8_t)8, 
                           /* options       */ (uint8_t)0);
        }

        void recv() {
            int err=0;
            char c=0;
            XBeeFrame frame;

            for (int i=0;i<111;++i)
                frame.content.unknown[i] = 0x0;

            debug_print("Recv frame: '");
            err = this->rcpt_frame(&frame);
            debug_println("'");
            switch (err) {
                case ERR_WRONG_FRM_DLM:
                    printf("---- ERROR CODE: %d ; INVALID FRAME DELIMITER ----\n", err);
                    break;
                case ERR_INVALID_CHECKSUM:
                    printf("---- ERROR CODE: %d ; INVALID CHECKSUM ----\n", err);
                    break;
                case ERR_UNKWNOWN_FRM_ID:
                case ERR_NOT_IMPLEMENTED:
                    printf("---- ERROR CODE: %d\n", err);
                    try {
                        while ((c = this->read()) != (char)-1)
                            printf("%02X",c);
                        printf("\n----\n");
                    } catch (SerialException e) {
                        printf("\n");
                        e.print_msg();
                    }
                    break;
                default:
                    run(&frame);
            }
        }

        virtual void run (XBeeFrame* frame) {
            this->print_frame(frame);
        };
};

#endif // __XBEE_COMM_H__
