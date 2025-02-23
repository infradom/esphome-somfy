import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover
from esphome.const import CONF_ID
from esphome.const import CONF_DEVICE_CLASS #jco
from esphome.components.cover import DEVICE_CLASSES # jco

CONF_INITIAL_ROLLING_CODE  = "initial_rolling_code"

somfy_cover_ns = cg.esphome_ns.namespace("somfy_cover")
SomfyCover = somfy_cover_ns.class_("SomfyCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = cover.COVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(SomfyCover),
        cv.Optional(CONF_DEVICE_CLASS): cv.one_of(*DEVICE_CLASSES, lower=True),
        cv.Optional(CONF_INITIAL_ROLLING_CODE, default=1): cv.int_,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)
  
