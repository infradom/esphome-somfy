
#include "esphome.h"
#include "SomfyRts.h"
using namespace esphome;
//#include <Arduino.h>

extern esphome::globals::RestoringGlobalsComponent<int> *somfy0code;
extern esphome::globals::RestoringGlobalsComponent<int> *somfy1code;
extern esphome::globals::RestoringGlobalsComponent<int> *somfy2code;
extern esphome::globals::RestoringGlobalsComponent<int> *somfy3code;
extern esphome::globals::RestoringGlobalsComponent<int> *somfy4code;

extern esphome::gpio::GPIOBinaryOutput *statusled;
extern esphome::gpio::GPIOBinaryOutput *tx_pin;


SomfyRts::SomfyRts(uint32_t index, bool debug) {
  ESP_LOGW("startup", "Remote created remoteID: %d", index + REMOTE_FIRST_ADDR);
  _debug = debug;
  _remoteId = index + REMOTE_FIRST_ADDR;
}

SomfyRts::SomfyRts(uint32_t index) {
  ESP_LOGW("startup", "Remote created remoteID: %d", index + REMOTE_FIRST_ADDR);
  _debug = false;
  _remoteId = index + REMOTE_FIRST_ADDR;
}


void SomfyRts::init() {
  //pinMode(REMOTE_TX_PIN, OUTPUT);
  tx_pin->turn_off();
  statusled->turn_off();
  //statusled2->turn_off();
  //digitalWrite(REMOTE_TX_PIN, LOW);

  rollingCode = _readRemoteRollingCode();
  if (rollingCode == 0 ) {
    // Make rolling code set to work.
    if (rollingCode < newRollingCode) {
      _writeRemoteRollingCode(newRollingCode);
    }
  }
  
  if (Serial) {
      ESP_LOGW("debug", "Simulated remote number : %d", _remoteId);
      ESP_LOGW("debug", "Current rolling code    : %d", rollingCode);
      //ESP_LOGW("debug", " Flash chip Id: %08X (for example: Id=001640E0 Manuf=E0, Device=4016 (swap bytes))\n", ESP.getFlashChipId());
  }
}

void SomfyRts::buildFrame(unsigned char *frame, unsigned char button) {
    unsigned int code = _readRemoteRollingCode();
    frame[0] = 0xA7;            // Encryption key. Doesn't matter much
    frame[1] = button << 4;     // Which button did  you press? The 4 LSB will be the checksum
    frame[2] = code >> 8;       // Rolling code (big endian)
    frame[3] = code;            // Rolling code
    frame[4] = _remoteId >> 16; // Remote address
    frame[5] = _remoteId >>  8; // Remote address
    frame[6] = _remoteId;       // Remote address

    if (Serial) {
        char hex[20];
        std::string s = "Frame         : ";
        for(uint8_t i = 0; i < 7; i++) {
            if(frame[i] >> 4 == 0) { // Displays leading zero in case the most significant
                s += "0";         // nibble is a 0.
            };
            sprintf(hex, "%X ", frame[i]);
            s = s + hex ; //  Serial.print(frame[i],HEX); Serial.print(" ");
        }
        ESP_LOGD("debug", s.data());
    }

    // Checksum calculation: a XOR of all the nibbles
    checksum = 0;
    for(uint8_t i = 0; i < 7; i++) {
        checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
    }
    checksum &= 0b1111; // We keep the last 4 bits only


    //Checksum integration
    frame[1] |= checksum;   //  If a XOR of all the nibbles is equal to 0, the blinds will
                            // consider the checksum ok.
    if (Serial) {
        std::string s = "With checksum : ";
        char hex[20];
        //Serial.println(""); Serial.print("With checksum : ");
        for(uint8_t i = 0; i < 7; i++) {
            if(frame[i] >> 4 == 0) {
                s += "0"; //Serial.print("0");
            };
            sprintf(hex, "%X ", frame[i]);
            s = s +  hex; 
            //    Serial.print(frame[i],HEX); Serial.print(" ");
        };
        ESP_LOGD("debug", s.data());
    }

    // Obfuscation: a XOR of all the bytes
    for(uint8_t i = 1; i < 7; i++) {
        frame[i] ^= frame[i-1];
    }

    if (Serial) {
        std::string s = "Obfuscated    : ";
        char hex[20];
        //Serial.println(""); Serial.print("Obfuscated    : ");
        for(uint8_t i = 0; i < 7; i++) {
            if(frame[i] >> 4 == 0) {
                s += "0"; //Serial.print("0");
            };
            sprintf(hex, "%X ", frame[i]);
            s = s + hex ; //   Serial.print(frame[i],HEX); Serial.print(" ");
        };
        ESP_LOGD("debug", s.data());
        //Serial.println("");
        //Serial.print("Rolling Code  : "); Serial.println(code);
    }

    //  We store the value of the rolling code in the FS
    _writeRemoteRollingCode(code + 1);
}


void SomfyRts::sendCommand(unsigned char *frame, unsigned char sync) {
    if(sync == 2) { // Only with the first frame.
        // Wake-up pulse & Silence
        //digitalWrite(REMOTE_TX_PIN, HIGH);
        tx_pin->turn_on();
        delayMicroseconds(9415);
        //digitalWrite(REMOTE_TX_PIN, LOW);
        tx_pin->turn_off();
        delayMicroseconds(89565);
    }

    // Hardware sync: two sync for the first frame, seven for the following ones.
    for (int i = 0; i < sync; i++) {
        //digitalWrite(REMOTE_TX_PIN, HIGH);
        tx_pin->turn_on();
        delayMicroseconds(4*SYMBOL);
        //digitalWrite(REMOTE_TX_PIN, LOW);
        tx_pin->turn_off();
        delayMicroseconds(4*SYMBOL);
    }

    // Software sync
    //digitalWrite(REMOTE_TX_PIN, HIGH);
    tx_pin->turn_on();
    delayMicroseconds(4550);
    //digitalWrite(REMOTE_TX_PIN, LOW);
    tx_pin->turn_off();
    delayMicroseconds(SYMBOL);


    //Data: bits are sent one by one, starting with the MSB.
    for(uint8_t i = 0; i < 56; i++) {
        if(((frame[i/8] >> (7 - (i%8))) & 1) == 1) {
            //digitalWrite(REMOTE_TX_PIN, LOW);
            tx_pin->turn_off();
            delayMicroseconds(SYMBOL);
            // PORTD ^= 1<<5;
            //digitalWrite(REMOTE_TX_PIN, HIGH);
            tx_pin->turn_on();
            delayMicroseconds(SYMBOL);
        }
        else {
            //digitalWrite(REMOTE_TX_PIN, HIGH);
            tx_pin->turn_on();
            delayMicroseconds(SYMBOL);
            // PORTD ^= 1<<5;
            //digitalWrite(REMOTE_TX_PIN, LOW);
            tx_pin->turn_off();
            delayMicroseconds(SYMBOL);
        }
    }

    //digitalWrite(REMOTE_TX_PIN, LOW);
    tx_pin->turn_off();
    delayMicroseconds(30415); // Inter-frame silence
}

void SomfyRts::sendCommandUp() {
    buildFrame(_frame, HAUT);
    sendCommand(_frame, 2);
    for(int i = 0; i<2; i++) {
      sendCommand(_frame, 7);
    }
}

void SomfyRts::sendCommandDown() {
    buildFrame(_frame, BAS);
    sendCommand(_frame, 2);
    for(int i = 0; i<2; i++) {
      sendCommand(_frame, 7);
    }
}

void SomfyRts::sendCommandStop() {
    buildFrame(_frame, STOP);
    sendCommand(_frame, 2);
    for(int i = 0; i<2; i++) {
      sendCommand(_frame, 7);
    }
}

void SomfyRts::sendCommandProg() {
    buildFrame(_frame, PROG);
    sendCommand(_frame, 2);
    for(int i = 0; i<2; i++) {
      sendCommand(_frame, 7);
    }
}

void SomfyRts::sendCommandProgGrail() {
    buildFrame(_frame, PROG);
    sendCommand(_frame, 2);
    for(int i = 0; i<35; i++) {
      sendCommand(_frame, 7);
      yield();
    }
    for(int i = 0; i<40; i++) {
      delayMicroseconds(100000);
      yield();
    }
    sendCommandProg();
}

using namespace esphome;

uint16_t SomfyRts::_readRemoteRollingCode() {
  uint16_t code = 0;
  ESP_LOGW("RF", "read - remoteId= %d", _remoteId);
  switch (_remoteId) {
    case REMOTE_FIRST_ADDR + 0: code = somfy0code->value(); break;
    case REMOTE_FIRST_ADDR + 1: code = somfy1code->value(); break;
    case REMOTE_FIRST_ADDR + 2: code = somfy2code->value(); break;
    case REMOTE_FIRST_ADDR + 3: code = somfy3code->value(); break;
    case REMOTE_FIRST_ADDR + 4: code = somfy4code->value(); break; 
    default: ESP_LOGE("RF", "error reading remoteId");
  };
  ESP_LOGW("RF", "read code = %d", code);
  return code;
}


void SomfyRts::_writeRemoteRollingCode(uint16_t code) {
  switch (_remoteId) {
    case REMOTE_FIRST_ADDR + 0: somfy0code->value() = code; break;
    case REMOTE_FIRST_ADDR + 1: somfy1code->value() = code; break;
    case REMOTE_FIRST_ADDR + 2: somfy2code->value() = code; break;
    case REMOTE_FIRST_ADDR + 3: somfy3code->value() = code; break;
    case REMOTE_FIRST_ADDR + 4: somfy4code->value() = code; break; 
  };
  ESP_LOGW("RF", "written code = %d", code);
}


uint16_t getCodeFromFile(int remoteId) {
  uint16_t code = 0;
  switch (remoteId) {
    case REMOTE_FIRST_ADDR + 0: code = somfy0code->value(); break;
    case REMOTE_FIRST_ADDR + 1: code = somfy1code->value(); break;
    case REMOTE_FIRST_ADDR + 2: code = somfy2code->value(); break;
    case REMOTE_FIRST_ADDR + 3: code = somfy3code->value(); break;
    case REMOTE_FIRST_ADDR + 4: code = somfy4code->value(); break; 
  };
  ESP_LOGW("RF", "remoteID = %d", remoteId);
  ESP_LOGW("RF", "read code = %d", code);
  return code;
}

void writeCode2file(int remoteId, uint16_t code) {
  ESP_LOGW("info", "writeCode2file %d", code);
  ESP_LOGW("info", "writeCode2file remoteId %d", remoteId);
  switch (remoteId) {
    case REMOTE_FIRST_ADDR + 0: somfy0code->value() = code; break;
    case REMOTE_FIRST_ADDR + 1: somfy1code->value() = code; break;
    case REMOTE_FIRST_ADDR + 2: somfy2code->value() = code; break;
    case REMOTE_FIRST_ADDR + 3: somfy3code->value() = code; break;
    case REMOTE_FIRST_ADDR + 4: somfy4code->value() = code; break; 
  }
}

 
std::string getInfo() {
  std::string str = "{ ";
  int code2;
  str = str + "somfy0code: " + to_string(somfy0code->value()) + ", ";
  str = str + "somfy1code: " + to_string(somfy1code->value()) + ", ";
  str = str + "somfy2code: " + to_string(somfy2code->value()) + ", ";
  str = str + "somfy3code: " + to_string(somfy3code->value()) + ", ";
  str = str + "somfy4code: " + to_string(somfy4code->value()) + " }"; 
  //somfy4code->value() = somfy4code->value()+1;
  ESP_LOGI("info", str.data());
  statusled->turn_off();
  return str;
}

