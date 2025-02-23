
#include "esphome/core/log.h"
#include "somfy_switch.h"

namespace esphome {
namespace somfy_switch {

static const char *TAG = "somfy_switch.switch";

void SomfySwitch::setup() {

}

void SomfySwitch::write_state(bool state) {

}

void SomfySwitch::dump_config(){
    ESP_LOGCONFIG(TAG, "Somfy custom switch");
}

} //namespace somfy_switch
} //namespace esphome
