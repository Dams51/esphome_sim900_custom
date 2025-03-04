import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    DEVICE_CLASS_CONNECTIVITY,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from . import CONF_SIM900_ID, Sim900Component

DEPENDENCIES = ["sim900"]

CONF_ETAT_RESEAU = "etat_reseau"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_SIM900_ID): cv.use_id(Sim900Component),
    cv.Optional(CONF_ETAT_RESEAU): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_CONNECTIVITY,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
}


async def to_code(config):
    sim900_component = await cg.get_variable(config[CONF_SIM900_ID])

    if CONF_ETAT_RESEAU in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_ETAT_RESEAU])
        cg.add(sim900_component.set_registered_binary_sensor(sens))
