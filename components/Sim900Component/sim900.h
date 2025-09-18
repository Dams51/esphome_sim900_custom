#pragma once

#include <utility>

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/gpio/switch/gpio_switch.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/core/automation.h"
#include "pdulib.h"

namespace esphome {
namespace sim900 {

const uint16_t SIM900_READ_BUFFER_LENGTH = 1024;

enum State {
  STATE_IDLE = 0,
  STATE_INIT,
  STATE_SETUP_CMGF,
  STATE_SETUP_CLIP,
  STATE_SETUP_CSCS_IRA,
  STATE_SETUP_CSCA,
  STATE_CSCA_RESPONSE,
  STATE_SETUP_CSCS_UCS2,
  STATE_SETUP_CNMI,
  STATE_CREG,
  STATE_CREG_WAIT,
  STATE_CSQ,
  STATE_CSQ_RESPONSE,
  STATE_CPAS,
  STATE_CPAS_RESPONSE,
  STATE_SENDING_SMS_1,
  STATE_SENDING_SMS_2,
  STATE_CHECK_SMS,
  STATE_PARSE_SMS_RESPONSE,
  STATE_RECEIVE_SMS,
  STATE_RECEIVED_SMS,
  STATE_DISABLE_ECHO,
  STATE_DIALING,
  STATE_PARSE_CLIP,
  STATE_ATA_SENT,
  STATE_CHECK_CALL,
  // STATE_SETUP_USSD,
  // STATE_SEND_USSD1,
  // STATE_SEND_USSD2,
  // STATE_CHECK_USSD,
  // STATE_RECEIVED_USSD
};

class Sim900Component : public uart::UARTDevice, public api::CustomAPIDevice, public PollingComponent {
 public:
  /// Retrieve the latest sensor values. This operation takes approximately 16ms.
  void update() override;
  void loop() override;
  void dump_config() override;
#ifdef USE_BINARY_SENSOR
  void set_registered_binary_sensor(binary_sensor::BinarySensor *registered_binary_sensor) {
    registered_binary_sensor_ = registered_binary_sensor;
  }
#endif
#ifdef USE_SENSOR
  void set_rssi_sensor(sensor::Sensor *rssi_sensor) { rssi_sensor_ = rssi_sensor; }
#endif
  void set_etat_module_text_sensor(text_sensor::TextSensor *etat_module_text_sensor) { etat_module_text_sensor_ = etat_module_text_sensor; }
  void set_power_key_switch(gpio::GPIOSwitch *power_key_switch) { power_key_switch_ = power_key_switch; }

  // void add_on_sms_received_callback(std::function<void(std::string, std::string)> callback) {
  //   this->sms_received_callback_.add(std::move(callback));
  // }
  // void add_on_incoming_call_callback(std::function<void(std::string)> callback) {
  //   this->incoming_call_callback_.add(std::move(callback));
  // }
  // void add_on_call_connected_callback(std::function<void()> callback) {
  //   this->call_connected_callback_.add(std::move(callback));
  // }
  // void add_on_call_disconnected_callback(std::function<void()> callback) {
  //   this->call_disconnected_callback_.add(std::move(callback));
  // }
  // void add_on_ussd_received_callback(std::function<void(std::string)> callback) {
  //   this->ussd_received_callback_.add(std::move(callback));
  // }
  void send_sms(const std::string &recipient, const std::string &message);
  // void send_ussd(const std::string &ussd_code);
  void dial(const std::string &recipient);
  void connect();
  void disconnect();
  void toggle_power_switch();

 protected:
  void send_cmd_(const std::string &message);
  void parse_cmd_(std::string message);
  void set_registered_(bool registered);
  void set_rssi_(int rssi);
  void set_etat_module_(int state_val);
  void rise_incoming_call_event(const std::string caller);
  void rise_sms_event(const std::string sender, const std::string message);

#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *registered_binary_sensor_{nullptr};
#endif

#ifdef USE_SENSOR
  sensor::Sensor *rssi_sensor_{nullptr};
#endif

  text_sensor::TextSensor *etat_module_text_sensor_{nullptr};
  gpio::GPIOSwitch *power_key_switch_{nullptr};

  PDU pdu_object_ = PDU(512);

  std::string sender_;
  std::string message_;
  char read_buffer_[SIM900_READ_BUFFER_LENGTH];
  size_t read_pos_{0};
  uint8_t parse_index_{0};
  uint8_t watch_dog_{0};
  bool module_setup_done_{false};
  bool expect_ack_{false};
  sim900::State state_{STATE_IDLE};
  bool registered_{false};

  std::string recipient_;
  int pdu_length_{0};
  // std::string ussd_;
  bool send_pending_;
  bool dial_pending_;
  bool connect_pending_;
  bool disconnect_pending_;
  // bool send_ussd_pending_;
  uint8_t call_state_{6};

  // CallbackManager<void(std::string, std::string)> sms_received_callback_;
  // CallbackManager<void(std::string)> incoming_call_callback_;
  // CallbackManager<void()> call_connected_callback_;
  // CallbackManager<void()> call_disconnected_callback_;
  // CallbackManager<void(std::string)> ussd_received_callback_;
};

// class Sim900ReceivedMessageTrigger : public Trigger<std::string, std::string> {
//  public:
//   explicit Sim900ReceivedMessageTrigger(Sim900Component *parent) {
//     parent->add_on_sms_received_callback(
//         [this](const std::string &message, const std::string &sender) { this->trigger(message, sender); });
//   }
// };

// class Sim900IncomingCallTrigger : public Trigger<std::string> {
//  public:
//   explicit Sim900IncomingCallTrigger(Sim900Component *parent) {
//     parent->add_on_incoming_call_callback([this](const std::string &caller_id) { this->trigger(caller_id); });
//   }
// };

// class Sim900CallConnectedTrigger : public Trigger<> {
//  public:
//   explicit Sim900CallConnectedTrigger(Sim900Component *parent) {
//     parent->add_on_call_connected_callback([this]() { this->trigger(); });
//   }
// };

// class Sim900CallDisconnectedTrigger : public Trigger<> {
//  public:
//   explicit Sim900CallDisconnectedTrigger(Sim900Component *parent) {
//     parent->add_on_call_disconnected_callback([this]() { this->trigger(); });
//   }
// };
// class Sim900ReceivedUssdTrigger : public Trigger<std::string> {
//  public:
//   explicit Sim900ReceivedUssdTrigger(Sim900Component *parent) {
//     parent->add_on_ussd_received_callback([this](const std::string &ussd) { this->trigger(ussd); });
//   }
// };

template<typename... Ts> class Sim900SendSmsAction : public Action<Ts...> {
 public:
  Sim900SendSmsAction(Sim900Component *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, recipient)
  TEMPLATABLE_VALUE(std::string, message)

  void play(Ts... x) {
    auto recipient = this->recipient_.value(x...);
    auto message = this->message_.value(x...);
    this->parent_->send_sms(recipient, message);
  }

 protected:
 Sim900Component *parent_;
};

// template<typename... Ts> class Sim900SendUssdAction : public Action<Ts...> {
//  public:
//   Sim900SendUssdAction(Sim900Component *parent) : parent_(parent) {}
//   TEMPLATABLE_VALUE(std::string, ussd)

//   void play(Ts... x) {
//     auto ussd_code = this->ussd_.value(x...);
//     this->parent_->send_ussd(ussd_code);
//   }

//  protected:
//  Sim900Component *parent_;
// };

template<typename... Ts> class Sim900DialAction : public Action<Ts...> {
 public:
  Sim900DialAction(Sim900Component *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, recipient)

  void play(Ts... x) {
    auto recipient = this->recipient_.value(x...);
    this->parent_->dial(recipient);
  }

 protected:
 Sim900Component *parent_;
};
template<typename... Ts> class Sim900ConnectAction : public Action<Ts...> {
 public:
  Sim900ConnectAction(Sim900Component *parent) : parent_(parent) {}

  void play(Ts... x) { this->parent_->connect(); }

 protected:
 Sim900Component *parent_;
};

template<typename... Ts> class Sim900DisconnectAction : public Action<Ts...> {
 public:
  Sim900DisconnectAction(Sim900Component *parent) : parent_(parent) {}

  void play(Ts... x) { this->parent_->disconnect(); }

 protected:
 Sim900Component *parent_;
};

template<typename... Ts> class Sim900TogglePowerSwitchAction : public Action<Ts...> {
 public:
  Sim900TogglePowerSwitchAction(Sim900Component *parent) : parent_(parent) {}

  void play(Ts... x) { this->parent_->toggle_power_switch(); }

 protected:
 Sim900Component *parent_;
};

}  // namespace sim900
}  // namespace esphome
