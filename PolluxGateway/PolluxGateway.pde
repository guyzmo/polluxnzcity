#include <SoftwareSerial.h>

class XbeeCommunicator : public SoftwareSerial {
    boolean check_return() {
        while (!this->available());
        c = this->read();
        if (c != 'O') return false;
        while (!this->available());
        c = this->read();
        if (c != 'K') return false;
        while (!this->available());
        c = this->read();
        if (c != 0xD) return false;
        return true;
    }

    boolean enter_command_mode_context() {
        char c;
        this->print("+++");
        return this->check_return();
    }

    boolean leave_command_mode_context() {
        char c;
        this->print("+++");
        return this->check_return();
    }

    public:
        XbeeCommunicator(int tx, int rx) : SoftwareSerial(tx,rx) { }

        void begin (int speed) {
            SoftwareSerial::begin(speed);
            this->set_up_panid(42);
            this->set_up_vendorid("0013A200");
        }

        void set_up_panid(char* panid) {
            this->enter_command_mode_context();
            this->print("ATID");
            this->print(panid);
            this->leave_command_mode_context();
        }

        void set_up_vendorid(char* vid) {
            this->enter_command_mode_context();
            this->print("ATDH");
            this->print(vid);
            this->leave_command_mode_context();
        }



} xbeecom(2,3);

void setup() {
    xbeecom.begin(9600);
    Serial.begin(9600);
    Serial.println("-- Start --");
}

void loop() {
}

