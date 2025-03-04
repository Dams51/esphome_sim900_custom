# import esphome.codegen as cg
# import esphome.config_validation as cv
# from esphome.components.gpio import switch as gpio_switch
# from esphome.components import switch
# from esphome.const import CONF_PIN

# from . import CONF_SIM900_ID, Sim900Component

# DEPENDENCIES = ["Sim900Component"]

# CONF_POWER_KEY_PIN = "power_key_pin"

# CONFIG_SCHEMA = {
#     cv.GenerateID(CONF_SIM900_ID): cv.use_id(Sim900Component),
#     cv.Optional(CONF_POWER_KEY_PIN): gpio_switch.CONFIG_SCHEMA,
# }


# async def to_code(config):
#     sim900_component = await cg.get_variable(config[CONF_SIM900_ID])

#     if CONF_POWER_KEY_PIN in config:
#         sens = await switch.new_switch(config[CONF_POWER_KEY_PIN])
#         pin = await cg.gpio_pin_expression(config[CONF_POWER_KEY_PIN][CONF_PIN])
#         cg.add(sens.set_pin(pin))
#         cg.add(sim900_component.set_power_key_switch(sens))
