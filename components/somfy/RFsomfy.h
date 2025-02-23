//#include "esphome.h"
//using namespace esphome;
#include <SomfyRts.h>
//#include <Ticker.h>
//#include <FS.h>
//#include <LittleFS.h>

// cmd 11 - program mode
// cmd 16 - program mode for grail  - JCO
// cmd 21 - delete rolling code file
// cmd 41 - List files
// cmd 51 - Test filesystem.
// cmd 61 - Format filesystem and test.
// cmd 71 - Show actual rolling code
// cmd 81 - Get all rolling code
// cmd 85 - Write new rolling codes

#define REMOTE_COUNT 5   // <- Number of somfy blinds.

int      xcode[REMOTE_COUNT];
uint16_t iCode[REMOTE_COUNT];

char const * string2char(std::string command) {
  if(command.length()!=0){
      char *p = const_cast<char*>(command.c_str());
      return p;
  } 
  return "";
}


// get code from all files
void getCodeFromAllFiles() {
  uint16_t code = 0;
  ESP_LOGW("somfy", "* Get all rolling codes from files");
  for (int i=0; i<REMOTE_COUNT; i++) {
    code = getCodeFromFile(REMOTE_FIRST_ADDR + i);
    ESP_LOGI("somfy", "Remoteid %d", REMOTE_FIRST_ADDR + i);
    ESP_LOGI("file", "Code: %d from file %d", code, i);
    xcode[i] = code;
  }
}


SomfyRts rtsDevices[REMOTE_COUNT] = { 
    SomfyRts(0),
    SomfyRts(1),
    SomfyRts(2),
    SomfyRts(3),
    SomfyRts(4)
}; 

class RFsomfy : public Component, public Cover {
 private:
  int index;
  //Ticker ticker;

 //protected:
 //  uint8_t width{8};
  
 public:
  int remoteId = -1;    
  unsigned char frame[7];
  std::string device_class;

  void set_code(const uint16_t code) { 
    // this->width = width;
    iCode[remoteId-REMOTE_FIRST_ADDR] = code;
    writeCode2file(remoteId, code);
    xcode[remoteId-REMOTE_FIRST_ADDR] = code;
    ESP_LOGW("somfy", "set_def_code for remoteId %d", remoteId);
    ESP_LOGW("somfy", "set_def_code to %d", code);
    }

  void set_def_code(const uint16_t code) { 
    // this->width = width;
    iCode[remoteId-REMOTE_FIRST_ADDR] = code;
    //writeCode2file(remoteId, code);
    xcode[remoteId-REMOTE_FIRST_ADDR] = code;
    ESP_LOGW("somfy", "set_def_code for remoteId %d", remoteId);
    ESP_LOGW("somfy", "set_def_code to %d", code);
    }


  RFsomfy(int rmx, std::string dev_class) : Cover() { //register
    index = rmx;
    remoteId = REMOTE_FIRST_ADDR + index;
    device_class = dev_class;
    ESP_LOGW("somfy", "Cover index %d", index);
  }

  void setup() override {
    // This will be called by App.setup()
    ESP_LOGW("RFsomfy", "Starting Device");
    //Serial.begin(115200);
    //Serial.println("Initialize remote devices");
    //pinMode(STATUS_LED_PIN, OUTPUT);
    //digitalWrite(STATUS_LED_PIN, HIGH);
    statusled->set_state(HIGH);
    ESP_LOGD("RFsomfy","Somfy ESPHome Cover v0.12");
    ESP_LOGD("RFsomfy","Initialize remote devices");
    for (int i=0; i<REMOTE_COUNT; i++) {
      rtsDevices[i].init();
      //Serial.println("i: " + String(i));
      ESP_LOGD("somfy", "Initialize remote %d", REMOTE_FIRST_ADDR + i);
      ESP_LOGD("somfy","file path:");
     //string fp = file_path(i);
      //Serial.print("Init remote: ");
      //Serial.println(fp);
      xcode[i] = 1; //i;
    } 

    //digitalWrite(STATUS_LED_PIN, LOW);
    statusled->set_state(LOW);
    //  testFs();
    getCodeFromAllFiles();
  }



  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(false);
    traits.set_supports_position(true);
    traits.set_supports_tilt(true); // to send other commands
    return traits;
  }
  
  
  void control(const CoverCall &call) override {

    // This will be called every time the user requests a state change.
    
    //digitalWrite(STATUS_LED_PIN, HIGH);
    statusled->set_state(HIGH);
    delay(50);
    
    ESP_LOGW("RFsomfy", "Using remote %d", REMOTE_FIRST_ADDR + index);
    ESP_LOGW("RFsomfy", "Remoteid %d", remoteId);
    ESP_LOGW("RFsomfy", "index %d", index);
    
    //Serial.print("remoteId: ");
    //Serial.println(remoteId);
    
    
    if (call.get_position().has_value()) {
      float pos = *call.get_position();
      // Write pos (range 0-1) to cover
      // ...
      int ppos = pos * 100;
      ESP_LOGD("RFsomfy", "get_position is: %d", ppos);

      if (ppos == 0) {
        ESP_LOGD("RFsomfy","POS 0");
        if (device_class == "shade") rtsDevices[index].sendCommandDown();
        else rtsDevices[index].sendCommandUp();
        pos = 0.01;
      }

      if (ppos == 100) {
        ESP_LOGD("RFsomfy","POS 100");
        if (device_class == "shade") rtsDevices[index].sendCommandUp();
        else rtsDevices[index].sendCommandDown();
        pos = 0.99;
      }

      // Publish new state
      this->position = pos;
      this->publish_state();
    }
    if (call.get_stop()) {
      // User requested cover stop
      ESP_LOGD("RFsomfy","get_stop");
      //Serial.println("* Command STOP - ");
      //Serial.println(remoteId);
      rtsDevices[index].sendCommandStop();
    }
    
    if (call.get_tilt().has_value()) {
      auto tpos = *call.get_tilt();
      int xpos = tpos * 100;
      ESP_LOGI("tilt", "Command tilt xpos: %d", xpos);

      
      if (xpos == 11) {
        ESP_LOGW("tilt","program mode");
        //digitalWrite(STATUS_LED_PIN, HIGH);
        statusled->set_state(HIGH);
        rtsDevices[index].sendCommandProg();
        delay(1000);
      }

      if (xpos == 16) {
        ESP_LOGW("tilt","program mode - grail");
        //digitalWrite(STATUS_LED_PIN, HIGH);
        statusled->set_state(HIGH);
        rtsDevices[index].sendCommandProgGrail();
        delay(1000);
      }

      if (xpos == 71) {
       // get roling code from file
       uint16_t code = 0;
       code = getCodeFromFile(remoteId);
       ESP_LOGI("file", "read Code %d for remote : %d", code, remoteId);
       xcode[remoteId] = code;
      }

      if (xpos == 81) {
       // get all roling code from file
       getCodeFromAllFiles();
      }

      if (xpos == 85) {
       // Write new roling codes
        ESP_LOGW("file", "going to write codes");
        for (int i=0; i<REMOTE_COUNT; i++) {
          writeCode2file(REMOTE_FIRST_ADDR + i, iCode[i]);
        }
      }
    }
    
    //digitalWrite(STATUS_LED_PIN, LOW);
    statusled->set_state(LOW);
    delay(50);
  } 
}; 

class RFsomfyUp: public Component, public Switch {
 private:
  int index;
 public:
  int remoteId = -1;

  void setup() override {
    // This will be called by App.setup()
  }
  void write_state(bool state) override {
    // This will be called every time the user requests a state change.
    //Serial.println("* Command UP");
    ESP_LOGD("somfy", "command up");
    rtsDevices[index].sendCommandUp();
    // Acknowledge new state by publishing it
    publish_state(state);
  }
  RFsomfyUp(int rmx) : Switch() { //register
    index = rmx;
    remoteId = index;
    ESP_LOGD("somfy", "Switch %d", index);
  }
};

class RFsomfyDown: public Component, public Switch {
 private:
  int index;
 public:
  int remoteId = -1;
  void setup() override {
    // This will be called by App.setup()
  }
  void write_state(bool state) override {
    // This will be called every time the user requests a state change.
    //Serial.println("* Command Down");
    ESP_LOGD("somfy", "command down");
    rtsDevices[index].sendCommandDown();
    // Acknowledge new state by publishing it
    publish_state(state);
  }
  RFsomfyDown(int rmx) : Switch() { //register
    index = rmx;
    remoteId = index;
    ESP_LOGD("somfy", "Switch %d", index);
  }
};

class RFsomfyMy: public Component, public Switch {
 private:
  int index;
 public:
  int remoteId = -1;
  void setup() override {
    // This will be called by App.setup()
  }
  void write_state(bool state) override {
    // This will be called every time the user requests a state change.
    //Serial.println("* Command STOP");
    ESP_LOGD("somfy", "command stop");
    rtsDevices[index].sendCommandStop();
    // Acknowledge new state by publishing it
    publish_state(state);
  }
  RFsomfyMy(int rmx) : Switch() { //register
    index = rmx;
    remoteId = index;
    ESP_LOGD("somfy", "Switch %d", index);
  }
};


class RFsomfyProg: public Component, public Switch {
 private:
  int index;
 public:
  int remoteId = -1;
  void setup() override {
    // This will be called by App.setup()
  }
  void write_state(bool state) override {
    // This will be called every time the user requests a state change.
    //Serial.println("* Command PROG");
    ESP_LOGD("somfy", "command prog");
    if (remoteId > 2) rtsDevices[index].sendCommandProgGrail();
    else rtsDevices[index].sendCommandProg();
  }
  RFsomfyProg(int rmx) : Switch() { //register
    index = rmx;
    remoteId = index;
    ESP_LOGD("somfy", "Switch %d", index);
  }
};

class RFsomfyInfo : public PollingComponent, public TextSensor {
 public:
  // constructor
  RFsomfyInfo() : PollingComponent(60000) {}

  void setup() override {
    // This will be called by App.setup()
    ESP_LOGW("setup", "setting up RFsomfyInfo");
  }

  void update() override {
    std::string tmp;
    bool bl_code {false};
    tmp = getInfo();
    publish_state(string2char(tmp));

    for (int i=0; i<REMOTE_COUNT; i++) {
      if (iCode[i] != 0) {
        bl_code = true;
        ESP_LOGD("icode", "%u : icode: %d", i, iCode[i]);
      }
    }
    if (bl_code) {
      ESP_LOGW("set_code", "Atention! After set code, and it works, remove it from your YAML");
    }
  }

};
