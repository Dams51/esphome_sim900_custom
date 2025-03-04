import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_SIGNAL_STRENGTH,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_DECIBEL_MILLIWATT,
)
from . import CONF_SIM900_ID, Sim900Component

DEPENDENCIES = ["Sim900Component"]

CONF_SIGNAL_RESEAU = "signal_reseau"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_SIM900_ID): cv.use_id(Sim900Component),
    cv.Optional(CONF_SIGNAL_RESEAU): sensor.sensor_schema(
        unit_of_measurement=UNIT_DECIBEL_MILLIWATT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
}


async def to_code(config):
    sim900_component = await cg.get_variable(config[CONF_SIM900_ID])

    if CONF_SIGNAL_RESEAU in config:
        sens = await sensor.new_sensor(config[CONF_SIGNAL_RESEAU])
        cg.add(sim900_component.set_rssi_sensor(sens))
