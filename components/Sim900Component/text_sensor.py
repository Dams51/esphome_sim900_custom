import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import CONF_SIM900_ID, Sim900Component

DEPENDENCIES = ["Sim900Component"]

CONF_ETAT_MODULE   = "etat_module"
CONF_ETAT_APPEL  = "etat_appel"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_SIM900_ID): cv.use_id(Sim900Component),
    cv.Optional(CONF_ETAT_MODULE): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_ETAT_APPEL): text_sensor.text_sensor_schema(),
}


async def to_code(config):
    sim900_component = await cg.get_variable(config[CONF_SIM900_ID])

    if CONF_ETAT_MODULE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_ETAT_MODULE])
        cg.add(sim900_component.set_etat_module_text_sensor(sens))

    if CONF_ETAT_APPEL in config:
        sens = await text_sensor.new_text_sensor(config[CONF_ETAT_APPEL])
        cg.add(sim900_component.set_call_state_text_sensor(sens))
