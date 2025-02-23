#include "esphome/core/log.h"
#include "somfy_cover.h"

namespace esphome {
namespace somfy_cover {

static const char *TAG = "somfy_cover.cover";

void SomfyCover::setup() {

}

void SomfyCover::loop() {

}

void SomfyCover::dump_config() {
    ESP_LOGCONFIG(TAG, "Somfy cover");
}

cover::CoverTraits SomfyCover::get_traits() {
    auto traits = cover::CoverTraits();
    traits.set_is_assumed_state(false);
    traits.set_supports_position(false);
    traits.set_supports_tilt(false);

    return traits;
}

void SomfyCover::control(const cover::CoverCall &call) {
    
}

}  // namespace somfy_cover
}  // namespace esphome
