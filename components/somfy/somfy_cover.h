#pragma once

#include "esphome/core/component.h"
#include "esphome/components/cover/cover.h"

namespace esphome {
namespace somfy_cover {

class SomfyCover : public cover::Cover, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  cover::CoverTraits get_traits() override;
  
 protected:
  void control(const cover::CoverCall &call) override;
};

}  // namespace somfy_cover
}  // namespace esphome
