import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import (
    CONF_ID,
    CONF_MESSAGE,
    CONF_TRIGGER_ID,
)
from esphome.components import uart

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@Dam51"]
MULTI_CONF = True

sim900_ns = cg.esphome_ns.namespace("sim900")
Sim900Component = sim900_ns.class_("Sim900Component", cg.Component)

# Sim900ReceivedMessageTrigger = sim900_ns.class_(
#     "Sim900ReceivedMessageTrigger",
#     automation.Trigger.template(cg.std_string, cg.std_string),
# )
# Sim900IncomingCallTrigger = sim900_ns.class_(
#     "Sim900IncomingCallTrigger",
#     automation.Trigger.template(cg.std_string),
# )
# Sim900CallConnectedTrigger = sim900_ns.class_(
#     "Sim900CallConnectedTrigger",
#     automation.Trigger.template(),
# )
# Sim900CallDisconnectedTrigger = sim900_ns.class_(
#     "Sim900CallDisconnectedTrigger",
#     automation.Trigger.template(),
# )

# Sim900ReceivedUssdTrigger = sim900_ns.class_(
#     "Sim900ReceivedUssdTrigger",
#     automation.Trigger.template(cg.std_string),
# )

# Actions
Sim900SendSmsAction = sim900_ns.class_("Sim900SendSmsAction", automation.Action)
# Sim900SendUssdAction = sim900_ns.class_("Sim900SendUssdAction", automation.Action)
Sim900DialAction = sim900_ns.class_("Sim900DialAction", automation.Action)
Sim900ConnectAction = sim900_ns.class_("Sim900ConnectAction", automation.Action)
Sim900DisconnectAction = sim900_ns.class_("Sim900DisconnectAction", automation.Action)

CONF_SIM900_ID = "sim900_id"
# CONF_ON_SMS_RECEIVED = "on_sms_received"
# CONF_ON_USSD_RECEIVED = "on_ussd_received"
# CONF_ON_INCOMING_CALL = "on_incoming_call"
# CONF_ON_CALL_CONNECTED = "on_call_connected"
# CONF_ON_CALL_DISCONNECTED = "on_call_disconnected"
CONF_RECIPIENT = "recipient"
CONF_USSD = "ussd"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Sim900Component),
            # cv.Optional(CONF_ON_SMS_RECEIVED): automation.validate_automation(
            #     {
            #         cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
            #             Sim900ReceivedMessageTrigger
            #         ),
            #     }
            # ),
            # cv.Optional(CONF_ON_INCOMING_CALL): automation.validate_automation(
            #     {
            #         cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
            #             Sim900IncomingCallTrigger
            #         ),
            #     }
            # ),
            # cv.Optional(CONF_ON_CALL_CONNECTED): automation.validate_automation(
            #     {
            #         cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
            #             Sim900CallConnectedTrigger
            #         ),
            #     }
            # ),
            # cv.Optional(CONF_ON_CALL_DISCONNECTED): automation.validate_automation(
            #     {
            #         cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
            #             Sim900CallDisconnectedTrigger
            #         ),
            #     }
            # ),
            # cv.Optional(CONF_ON_USSD_RECEIVED): automation.validate_automation(
            #     {
            #         cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
            #             Sim900ReceivedUssdTrigger
            #         ),
            #     }
            # ),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)
FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "sim900", require_tx=True, require_rx=True
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # for conf in config.get(CONF_ON_SMS_RECEIVED, []):
    #     trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
    #     await automation.build_automation(
    #         trigger, [(cg.std_string, "message"), (cg.std_string, "sender")], conf
    #     )
    # for conf in config.get(CONF_ON_INCOMING_CALL, []):
    #     trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
    #     await automation.build_automation(trigger, [(cg.std_string, "caller_id")], conf)
    # for conf in config.get(CONF_ON_CALL_CONNECTED, []):
    #     trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
    #     await automation.build_automation(trigger, [], conf)
    # for conf in config.get(CONF_ON_CALL_DISCONNECTED, []):
    #     trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
    #     await automation.build_automation(trigger, [], conf)

    # for conf in config.get(CONF_ON_USSD_RECEIVED, []):
    #     trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
    #     await automation.build_automation(trigger, [(cg.std_string, "ussd")], conf)


SIM900_SEND_SMS_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(Sim900Component),
        cv.Required(CONF_RECIPIENT): cv.templatable(cv.string_strict),
        cv.Required(CONF_MESSAGE): cv.templatable(cv.string),
    }
)


@automation.register_action(
    "sim900.send_sms", Sim900SendSmsAction, SIM900_SEND_SMS_SCHEMA
)
async def sim900_send_sms_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_RECIPIENT], args, cg.std_string)
    cg.add(var.set_recipient(template_))
    template_ = await cg.templatable(config[CONF_MESSAGE], args, cg.std_string)
    cg.add(var.set_message(template_))
    return var


SIM900_DIAL_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(Sim900Component),
        cv.Required(CONF_RECIPIENT): cv.templatable(cv.string_strict),
    }
)


@automation.register_action("sim900.dial", Sim900DialAction, SIM900_DIAL_SCHEMA)
async def sim900_dial_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_RECIPIENT], args, cg.std_string)
    cg.add(var.set_recipient(template_))
    return var


@automation.register_action(
    "sim900.connect",
    Sim900ConnectAction,
    cv.Schema({cv.GenerateID(): cv.use_id(Sim900Component)}),
)
async def sim900_connect_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    return var


# SIM900_SEND_USSD_SCHEMA = cv.Schema(
#     {
#         cv.GenerateID(): cv.use_id(Sim900Component),
#         cv.Required(CONF_USSD): cv.templatable(cv.string_strict),
#     }
# )


# @automation.register_action(
#     "sim900.send_ussd", Sim900SendUssdAction, SIM900_SEND_USSD_SCHEMA
# )
# async def sim900_send_ussd_to_code(config, action_id, template_arg, args):
#     paren = await cg.get_variable(config[CONF_ID])
#     var = cg.new_Pvariable(action_id, template_arg, paren)
#     template_ = await cg.templatable(config[CONF_USSD], args, cg.std_string)
#     cg.add(var.set_ussd(template_))
#     return var


@automation.register_action(
    "sim900.disconnect",
    Sim900DisconnectAction,
    cv.Schema({cv.GenerateID(): cv.use_id(Sim900Component)}),
)
async def sim900_disconnect_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    return var
