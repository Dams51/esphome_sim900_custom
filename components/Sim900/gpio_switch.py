import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import gpio_switch

from . import CONF_SIM900_ID, Sim900Component

DEPENDENCIES = ["sim900"]

CONF_POWER_KEY_PIN = "power_key_pin"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_SIM900_ID): cv.use_id(Sim900Component),
    cv.Optional(CONF_POWER_KEY_PIN): gpio_switch.CONFIG_SCHEMA,
}


async def to_code(config):
    sim900_component = await cg.get_variable(config[CONF_SIM900_ID])

    if CONF_POWER_KEY_PIN in config:
        sens = await gpio_switch.new_gpio_switch(config[CONF_POWER_KEY_PIN])
        cg.add(sim900_component.set_registered_binary_sensor(sens))
