#ifndef somfyrts_h
#define somfyrts_h
#include "esphome.h"

#define SYMBOL 640
#define HAUT 0x2
#define STOP 0x1
#define BAS 0x4
#define PROG 0x8


#define LOW false
#define HIGH true
#define Serial true
using namespace esphome;

#define REMOTE_FIRST_ADDR 10000 //0x169511   // <- Change remote name and remote code here!

class SomfyRts { // : public gpio::GPIOBinaryOutput {

  private:
    bool _debug;
    uint32_t _remoteId;
    uint16_t newRollingCode = 3000; //7421;       //<-- Change it!
    uint16_t rollingCode = 0;
    unsigned char _frame[7];
    char checksum;
    uint16_t _readRemoteRollingCode();
    void _writeRemoteRollingCode(uint16_t code);
    //String _getConfigFilename();

  public:
    /*
    void setup() override {
    // This will be called by App.setup()
      //pinMode(REMOTE_TX_PIN, OUTPUT);
      set_pin( );
    };
    void write_state(bool state) override {
      //digitalWrite(REMOTE_TX_PIN, state);
      turn_off();
    }; */
    SomfyRts(uint32_t index, bool debug);
    SomfyRts(uint32_t index);
    void init();
    void sendCommandUp();
    void sendCommandDown();
    void sendCommandStop();
    void sendCommandProg();
    void sendCommandProgGrail();
    void buildFrame(unsigned char *frame, unsigned char button);
    void sendCommand(unsigned char *frame, unsigned char sync);
};


void writeCode2file(int remoteId, uint16_t code);
uint16_t getCodeFromFile(int remoteId); 
std::string getInfo();


#endif
