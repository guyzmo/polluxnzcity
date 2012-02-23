// Xbee Light Communicator Library
// Resources :
// http://www.circuitsathome.com/mcu/playing-xbee-part-4-api
// http://www.chasingtrons.com/main/2010/11/13/xbee-propeller-chip.html

#include <SoftwareSerial.h>

#define SPEED 9600

/* API frame fields */
#define FRM_DLM	0x7e		//frame delimiter

/* frame types	*/
#define XMIT_REQ     0x10    //transmit request	
#define API_ID       0x08    //send at command

#define AT_CMD_RESP  0x88
#define MODEM_STATUS 0x8A
#define TX_STATUS    0x8B
#define RX_PACKET    0x90

#define RX_PACKET_LEN 12    // 12 bytes of frame specific data between header and checksum

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
        uint8_t unknown[111];
    } content;
    uint8_t checksum;
} XBeeFrame;

class XbeeCommunicator : public SoftwareSerial {
    bool _reset;

#ifdef API_ESCAPED_MODE
    size_t write(uint8_t i) {
        int s=0;
        switch (i) {
            case 0x7E:
            case 0x7D:
            case 0x11:
            case 0x13:
                s+=SoftwareSerial::write(0x7D);
                s+=SoftwareSerial::write(0x20^i);
                return s;
            default:
                return SoftwareSerial::write(i);
        }
    }
#endif

    /* sends transmit request to addr, network              */
    /* 256 bytes max.payload; Xbee max.payload is 100 bytes */
    void xmit_req(uint8_t* addr64, uint16_t network, uint8_t nbytes, uint8_t* data, uint8_t frameid, uint8_t bradius, uint8_t options) {
        uint8_t tmpword = 0;
        int checksum;

        char* byte_ptr = NULL;
        
        SoftwareSerial::write(FRM_DLM);                                // 1. SEND FRAME DELIMITER
        
        tmpword = nbytes+4;            // payload plus frame type, fram ID, addr, network, bcast radius, options
        byte_ptr = (char*) &tmpword;    // little-endian assumed
        for (int i=1;i>-1;--i)
            this->write(*(byte_ptr)+i);                                // 2. SEND FRAME LENGTH
        
        this->write(XMIT_REQ);              checksum = XMIT_REQ;       // 3. SEND FRAME API Identifier (transmission request)
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
    }

    void send_atcmd(int frameid, const char* at_command, const char* param_value) {
        uint8_t tmpword = 0;
        int checksum;

        char* byte_ptr = NULL;
        
        // 1. SEND FRAME DELIMITER
        SoftwareSerial::write(FRM_DLM);

        // 2. SEND FRAME LENGTH
        int nbytes = strlen(at_command) + strlen(param_value);
        tmpword = nbytes+14;            // payload plus frame type, fram ID, addr, network, bcast radius, options
        byte_ptr = (char*) &tmpword;    // little-endian assumed
        for (int i=1;i>-1;--i)
            this->write(*(byte_ptr)+i);

        // 3. SEND FRAME API Identifier (transmission request)
        this->write(API_ID);                checksum = API_ID;

        // 4. SEND FRAME Unique ID
        this->write(frameid);               checksum += frameid;

        // 5. SEND AT COMMAND
        for (uint8_t i=0;i<strlen(at_command);++i) {
            this->write(at_command[i]);     checksum += at_command[i];
        }

        // 6. SEND PARAMETERS
        for (uint8_t i=0;i<strlen(param_value);++i) {
            this->write(param_value[i]);    checksum += param_value[i];
        }

        // 10. SEND CHECKSUM
        checksum = 0xff - (checksum);
        this->write(checksum);
    }

    void print_data(uint8_t* data, uint16_t len, int type) {
        for (uint16_t i=0;i<min(len,100);++i)
            Serial.print((char)data[i], type);
        Serial.println();
    }

    void print_frame(XBeeFrame* frame) {
        Serial.println("-- Frame --");
        Serial.print("length: "); Serial.println(frame->length.i16, DEC);
        Serial.print("api_id: "); Serial.println(frame->api_id, HEX);
        switch (frame->api_id) {
            case AT_CMD_RESP:
                Serial.println(" ; AT COMMAND RESPONSE");
                Serial.print("frm_id: "); Serial.println(frame->content.at.frame_id,DEC);
                Serial.print("at_cmd: "); Serial.println(frame->content.at.command);
                Serial.print("status: "); Serial.println(frame->content.at.status, HEX);
                Serial.print("values: "); Serial.println(frame->content.at.values);
            case TX_STATUS:
                Serial.println(" ; TX STATUS");
                Serial.print("frm_id: "); Serial.println(frame->content.tx.frame_id,DEC);
                Serial.print("net_ad: "); Serial.println(frame->content.tx.network_addr.i16,HEX);
                Serial.print("retry : "); Serial.println(frame->content.tx.retries);
                Serial.print("delivs: "); Serial.println(frame->content.tx.delivery_status,HEX);
                Serial.print("discos: "); Serial.println(frame->content.tx.discovery_status,HEX);
            case RX_PACKET:
                Serial.print(" ; RX PACKET");
                Serial.print("src_ad: "); print_data(frame->content.rx.source_addr,8,HEX);
                Serial.print("net_ad: "); Serial.println(frame->content.rx.network_addr.i16,HEX);
                Serial.print("recvopt: "); Serial.println(frame->content.rx.options,HEX);
                Serial.print("payload: "); print_data(frame->content.rx.payload,frame->length.i16,DEC);
            default:
                Serial.println(" /!\\ UNKNOWN PACKET TYPE /!\\");
                Serial.print("content: "); print_data(frame->content.unknown,frame->length.i16, HEX);
        }
        Serial.println("-- End of Frame --");
    }
        
    bool rcpt_frame(XBeeFrame* frame) {
        //1. GET FRAME DELIMITER
        if (SoftwareSerial::read() != FRM_DLM) return false;
        
        //2. GET CONTENT LENGTH
        frame->length.i8.msb = this->read();
        frame->length.i8.lsb = this->read();

        //3. GET COMMAND
        switch (this->read()) {
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
                frame->content.rx.network_addr.i8.lsb = this->read(); frame->checksum += frame->content.rx.network_addr.i8.msb;
                for (uint8_t i=0;i<8;++i) {
                    frame->content.rx.source_addr[i] = this->read();  frame->checksum += frame->content.rx.source_addr[i];
                }
                frame->content.rx.options = this->read();             frame->checksum += frame->content.rx.options;
                for (uint8_t i=0;i<frame->length.i16-RX_PACKET_LEN;++i) {
                    frame->content.rx.payload[i] = this->read();      frame->checksum += frame->content.rx.payload[i];
                }
                frame->content.rx.payload[frame->length.i16-RX_PACKET_LEN] = '\0';
                break;

            case MODEM_STATUS:
                return false;

            default:
                for (uint8_t i = 0; i < min(frame->length.i16,100); ++i)
                    frame->content.unknown[i] = this->read();
                return false;
        }

        if (0xFF-frame->checksum != this->read())
            return true;

        //4. validate checksum
        return false;
    }

    public:
        XbeeCommunicator(int tx, int rx) : SoftwareSerial(tx,rx), _reset(false) { }
    int read() {
        char c = SoftwareSerial::read();
        if (c == 0x7D)
            return SoftwareSerial::read()^0x20;
        return c;
    }

        void begin (const uint8_t* panid, const uint8_t* vendorid) {
            SoftwareSerial::begin(SPEED);
            this->set_up_panid(panid);
            this->set_up_vendorid(vendorid);
        }

        void send(const char* data) {
            this->xmit_req((uint8_t*)BROADCAST_ADDR, (uint16_t)BROADCAST_NET, strlen(data), (uint8_t*)data, (uint8_t)1, (uint8_t)0, (uint8_t)0);
        }

        char* recv() {
            XBeeFrame frame;
            for (int i=0;i<111;++i)
                frame.content.unknown[i] = 'A';
            this->rcpt_frame(&frame);
            this->print_frame(&frame);
            return (char*)"";
        }

        void set_up_panid(const uint8_t* panid) {
            this->send_atcmd(42, "ID", (char*)panid);
            if (this->available() > 7) {
                //Serial.print(xbeecom.recv());
                while (this->available()) {
                    Serial.print(this->read(),HEX);
                    Serial.print(" ");
                }
                Serial.println();
            }
                    
        }

        void set_up_vendorid(const uint8_t* vid) {
            this->send_atcmd(42, "DH", (char*)vid);
            if (this->available() > 7) {
                //Serial.print(xbeecom.recv());
                while (this->available()) {
                    Serial.print(this->read(),HEX);
                    Serial.print(" ");
                }
                Serial.println();
            }
        }

};
