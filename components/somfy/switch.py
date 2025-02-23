import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

somfy_switch_ns = cg.esphome_ns.namespace("somfy_switch")
SomfySwitch = somfy_switch_ns.class_("SomfySwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = switch.switch_schema(SomfySwitch).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
  
