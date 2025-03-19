#include "sim900.h"
#include "simUtils.h"
#include "esphome/core/log.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

namespace esphome {
namespace sim900 {

static const char *const TAG = "sim900";

const char ASCII_CR = 0x0D;
const char ASCII_LF = 0x0A;
const char ASCII_CTRL_Z = 0x1A;

void Sim900Component::update() {
  if (this->watch_dog_++ == 2) {
    ESP_LOGD(TAG, "WatchDog - %d", this->state_);
    this->state_ = STATE_INIT;
    this->module_setup_done_ = false;
    this->expect_ack_ = false;
    this->write_byte(ASCII_CTRL_Z);
  }

  if (this->expect_ack_)
    return;

  if (state_ == STATE_INIT) {
    if (this->registered_ && this->send_pending_) {
      ESP_LOGD(TAG, "Envoi SMS - Taille PDU = %i", this->pdu_length_);
      std::string cmd = "AT+CMGS=" + std::to_string(this->pdu_length_);
      ESP_LOGV(TAG, "S: %s - %d", cmd.c_str(), this->state_);
      this->watch_dog_ = 0;
      this->write_str(cmd.c_str());
      this->write_byte(ASCII_CR);
      this->state_ = STATE_SENDING_SMS_1;
      return; // '>' expected, not ACK
    } else if (this->registered_ && this->dial_pending_) {
      this->send_cmd_("ATD" + this->recipient_ + ';');
      this->state_ = STATE_DIALING;
    } else if (this->registered_ && this->connect_pending_) {
      this->connect_pending_ = false;
      ESP_LOGI(TAG, "Connecting...");
      this->send_cmd_("ATA");
      this->state_ = STATE_ATA_SENT;
    // } else if (this->registered_ && this->send_ussd_pending_) {
    //   this->send_cmd_("AT+CSCS=\"GSM\"");
    //   this->state_ = STATE_SEND_USSD1;
    } else if (this->registered_ && this->disconnect_pending_) {
      this->disconnect_pending_ = false;
      ESP_LOGI(TAG, "Disconnecting...");
      this->send_cmd_("ATH");
    } else if (this->registered_ && this->call_state_ != 6) {
      this->send_cmd_("AT+CLCC");
      this->state_ = STATE_CHECK_CALL;
      return;
    } else {
      if (this->module_setup_done_) {
        this->send_cmd_("AT+CREG?");
        this->state_ = STATE_CREG_WAIT;
        return;
      } else {
        this->send_cmd_("AT");
        this->state_ = STATE_SETUP_CMGF;
      }
    }
    this->expect_ack_ = true;
  } else if (state_ == STATE_RECEIVED_SMS) {
    // Serial Buffer should have flushed.
    // Send cmd to delete received sms
    char delete_cmd[20];
    sprintf(delete_cmd, "AT+CMGD=%d", this->parse_index_);
    this->send_cmd_(delete_cmd);
    this->state_ = STATE_CHECK_SMS;
    this->expect_ack_ = true;
  }
}

void Sim900Component::send_cmd_(const std::string &message) {
  ESP_LOGV(TAG, "S: %s - %d", message.c_str(), this->state_);
  this->watch_dog_ = 0;
  this->write_str(message.c_str());
  this->write_byte(ASCII_CR);
  this->write_byte(ASCII_LF);
}

void Sim900Component::parse_cmd_(std::string message) {
  if (message.empty())
    return;

  ESP_LOGV(TAG, "R: %s - %d", message.c_str(), this->state_);

  if (this->state_ != STATE_RECEIVE_SMS) {
    if (message == "RING" || message.compare(0, 6, "+CLIP:") == 0) {
      // Incoming call...
      this->state_ = STATE_PARSE_CLIP;
      this->expect_ack_ = false;
    } else if (message == "NO CARRIER") {
      if (this->call_state_ != 6) {
        this->call_state_ = 6;
        // this->call_disconnected_callback_.call();
      }
      return; // Next message
    } else if (message.compare(0, 6, "+CMTI:") == 0) {
      ESP_LOGI(TAG, "TODO : Handle +CMTI");
      // this->state_ = STATE_CHECK_SMS;
    } else if (message == "ERROR") {
      ESP_LOGE(TAG, "Command error");
    // } else if (message.compare(0, 6, "+CUSD:") == 0) {
    //   // Incoming USSD MESSAGE
    //   this->state_ = STATE_CHECK_USSD;
    }
  }

  bool ok = message == "OK";
  if (this->expect_ack_) {
    this->expect_ack_ = false;
    if (!ok) {
      if (this->state_ == STATE_SETUP_CMGF && message == "AT") {
        // Expected ack but AT echo received
        this->state_ = STATE_DISABLE_ECHO;
        this->expect_ack_ = true;
      } else {
        ESP_LOGW(TAG, "Not ack. %d %s", this->state_, message.c_str());
        this->state_ = STATE_IDLE;  // Let it timeout
        ESP_LOGI(TAG, "IDLE State, waiting for timeout - %d", this->state_);
        return;
      }
    }
  } else if (ok && (this->state_ != STATE_PARSE_SMS_RESPONSE && this->state_ != STATE_CHECK_CALL &&
                    this->state_ != STATE_RECEIVE_SMS && this->state_ != STATE_DIALING)) {
    ESP_LOGW(TAG, "Received unexpected OK. Ignoring");
    return;
  }

  switch (this->state_) {
    case STATE_INIT:
      // Fall thru STATE_CHECK_SMS same
      send_cmd_("AT+CMGL=0,1");
      this->state_ = STATE_PARSE_SMS_RESPONSE;
      this->parse_index_ = 0;
      break;

  // SETUP
    case STATE_DISABLE_ECHO: // Step 0
      send_cmd_("ATE0");
      this->state_ = STATE_SETUP_CMGF;
      this->expect_ack_ = true;
      break;
    case STATE_SETUP_CMGF: // Step 1
      send_cmd_("AT+CMGF=0");
      this->state_ = STATE_SETUP_CLIP;
      this->expect_ack_ = true;
      break;
    case STATE_SETUP_CLIP: // Step 2
      send_cmd_("AT+CLIP=1");
      this->state_ = STATE_SETUP_CSCS_IRA;
      this->expect_ack_ = true;
      break;
    case STATE_SETUP_CSCS_IRA: // Step 3
      send_cmd_("AT+CSCS=\"IRA\"");
      this->state_ = STATE_SETUP_CSCA;
      this->expect_ack_ = true;
      break;
    case STATE_SETUP_CSCA: // Step 4.1
      send_cmd_("AT+CSCA?");
      this->state_ = STATE_CSCA_RESPONSE;
      break;
    case STATE_CSCA_RESPONSE: // Step 4.2
      // SMS Service Center Address : needed for SMS sending in PDU mode
      if (message.compare(0, 6, "+CSCA:") == 0) {
        std::string SCA = message.substr(8, 12);
        ESP_LOGD(TAG, "SMS Service Center Address = %s", SCA.c_str());
        this->pdu_object_.setSCAnumber(SCA.c_str());
      }
      this->expect_ack_ = true;
      this->state_ = STATE_SETUP_CSCS_UCS2;
      break;
    case STATE_SETUP_CSCS_UCS2: // Step 5
      send_cmd_("AT+CSCS=\"UCS2\"");
      this->state_ = STATE_SETUP_CNMI;
      this->expect_ack_ = true;
      break;
    case STATE_SETUP_CNMI: // Step 6
      send_cmd_("AT+CNMI=2,1");
      this->state_ = STATE_CREG;
      this->module_setup_done_ = true;
      this->expect_ack_ = true;
      break;

  // UPDATE Status
    case STATE_CREG: // Step 1.1
      send_cmd_("AT+CREG?");
      this->state_ = STATE_CREG_WAIT;
      break;
    case STATE_CREG_WAIT: { // Step 1.2
      // Response: "+CREG: 0,1" -- the one there means registered ok
      //           "+CREG: -,-" means not registered ok
      bool registered = message.compare(0, 6, "+CREG:") == 0 && (message[9] == '1' || message[9] == '5');
      if (registered) {
        if (!this->registered_) {
          ESP_LOGD(TAG, "Registered OK");
        }
        this->state_ = STATE_CSQ;
        this->expect_ack_ = true;
      } else {
        ESP_LOGW(TAG, "Registration Fail");
        if (message[7] == '0') {  // Network registration is disable, enable it
          send_cmd_("AT+CREG=1");
          this->expect_ack_ = true;
          this->state_ = STATE_SETUP_CMGF;
        } else {
          // Keep waiting registration
          this->state_ = STATE_INIT;
        }
      }
      set_registered_(registered);
      break;
    }
    case STATE_CSQ: // Step 2.1
      send_cmd_("AT+CSQ");
      this->state_ = STATE_CSQ_RESPONSE;
      break;
    case STATE_CSQ_RESPONSE: // Step 2.2
      if (message.compare(0, 5, "+CSQ:") == 0) {
        size_t comma = message.find(',', 6);
        if (comma != 6) {
          int rssi = parse_number<int>(message.substr(6, comma - 6)).value_or(0);
          set_rssi_(rssi);
        }
      }
      this->expect_ack_ = true;
      this->state_ = STATE_CPAS;
      break;
    case STATE_CPAS: // Step 3.1
      send_cmd_("AT+CPAS");
      this->state_ = STATE_CPAS_RESPONSE;
      break;
    case STATE_CPAS_RESPONSE: // Step 3.2
      if (message.compare(0, 6, "+CPAS:") == 0) {
        int val = message[7] - '0';
        set_etat_module_(val);
      }
      this->expect_ack_ = true;
      this->state_ = STATE_CHECK_SMS;
      break;

    case STATE_CHECK_SMS:
      send_cmd_("AT+CMGL=0,1");
      this->state_ = STATE_PARSE_SMS_RESPONSE;
      this->parse_index_ = 0;
      break;
    case STATE_PARSE_SMS_RESPONSE:
      if (message.compare(0, 6, "+CMGL:") == 0 && this->parse_index_ == 0) {
        std::stringstream ss(message.substr(7)); // Without "+CMGL: "
        std::vector<std::string> tokens;
        std::string token;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        if (tokens.size() != 4) {
          ESP_LOGD(TAG, "Invalid message %d %s", this->state_, message.c_str());
          return;
        }

        this->parse_index_ = parse_number<uint8_t>(tokens[0]).value_or(0);
        this->sender_.clear();
        this->message_.clear();
        this->state_ = STATE_RECEIVE_SMS;
      }
      // Otherwise we receive another OK
      if (ok) {
        send_cmd_("AT+CLCC");
        this->state_ = STATE_CHECK_CALL;
      }
      break;
    case STATE_CHECK_CALL:
      if (message.compare(0, 6, "+CLCC:") == 0 && this->parse_index_ == 0) {
        this->expect_ack_ = true;
        size_t start = 7;
        size_t end = message.find(',', start);
        uint8_t item = 0;
        while (end != start) {
          item++;
          // item 1 call index for +CHLD
          // item 2 dir 0 Mobile originated; 1 Mobile terminated
          if (item == 3) {  // stat
            uint8_t current_call_state = parse_number<uint8_t>(message.substr(start, end - start)).value_or(6);
            if (current_call_state != this->call_state_) {
              ESP_LOGD(TAG, "Call state is now: %d", current_call_state);
              // if (current_call_state == 0)
              //   this->call_connected_callback_.call();
            }
            this->call_state_ = current_call_state;
            break;
          }
          // item 4 = ""
          // item 5 = Received timestamp
          start = end + 1;
          end = message.find(',', start);
        }

        if (item < 2) {
          ESP_LOGD(TAG, "Invalid message %d %s", this->state_, message.c_str());
          return;
        }
      } else if (ok) {
        if (this->call_state_ != 6) {
          // no call in progress
          this->call_state_ = 6;  // Disconnect
          // this->call_disconnected_callback_.call();
        }
      }
      this->state_ = STATE_INIT;
      break;

  // SIM Stuff
    case STATE_RECEIVE_SMS:
      if (ok || message.compare(0, 6, "+CMGL:") == 0) {
        // PDU Already decoded
        ESP_LOGD(TAG, "Received SMS from: %s", this->sender_.c_str());
        ESP_LOGD(TAG, "%s", this->message_.c_str());
        rise_sms_event(this->sender_, this->message_);
        // this->sms_received_callback_.call(this->message_, this->sender_);
        this->state_ = STATE_RECEIVED_SMS;
      } else {
        // --------------------  PDU  --------------------
        std::string pdu = message;

        int decode_pos = 0;
        int smsc_length = std::stoul(pdu.substr(decode_pos, 2), nullptr, 16);
        decode_pos += 2;
        // ESP_LOGV(TAG, "PDU - SMSC = %i", smsc_length);

        // Skip le num du SMSC
        decode_pos += 2 * smsc_length;

        int pdu_header = std::stoul(pdu.substr(decode_pos, 2), nullptr, 16);
        decode_pos += 2;
        // ESP_LOGV(TAG, "PDU - Header = %i", pdu_header);

        int sender_length = std::stoul(pdu.substr(decode_pos, 2), nullptr, 16);
        decode_pos += 2;
        // ESP_LOGV(TAG, "PDU - sender length = %i", sender_length);

        decode_pos += 2;

        char sender[sender_length + 1];
        for (int i = 0; i < sender_length; i = i+2 ){
          if (i == sender_length - 1) {
            decode_pos++;
            sender[i+1] = 0;
          }
          else
          {
            sender[i+1] = pdu[decode_pos++];
          }
          sender[i] = pdu[decode_pos++];
        }
        std::string data_sender = sender;
        this->sender_ = "+" + data_sender;
        // ESP_LOGV(TAG, "PDU - sender = %s", data_sender.c_str());

        // TP-PID
        decode_pos += 2;

        // TP-DCS = codage (change selon s'il y a des accents (00 = normal / 7 bits, 08 = accent / 16 bits))
        int data_code = std::stoul(pdu.substr(decode_pos, 2), nullptr, 16);
        decode_pos += 2;
        // ESP_LOGV(TAG, "PDU - encodage = %i", data_code);

        decode_pos += 14;

        // TP-UDL = length of data
        int data_length = std::stoul(pdu.substr(decode_pos, 2), nullptr, 16);
        decode_pos += 2;
        // ESP_LOGV(TAG, "PDU - data length = %i", data_length);

        std::string payload = pdu.substr(decode_pos);
        std::string output_sms_string = "";
        ESP_LOGV(TAG, "PDU - payload = %s", payload.c_str());
        if (data_code == 8) {
          ESP_LOGV(TAG, "PDU - 16 bits message");
          
          int payload_pos = 0;
          for (int i = 0; i < data_length; i = i+2 ){
            int char_code = std::stoul(payload.substr(payload_pos, 4), nullptr, 16);
            payload_pos += 4;
            // ESP_LOGV(TAG, "PDU - char_code = %i", char_code);

            if (char_code < 128)
            {
              char ascii = char_code;
              output_sms_string += ascii;
              // ESP_LOGV(TAG, "PDU - char = %s", ascii);
            }
            else
            {
              // Extended ASCII
              // ESP_LOGV(TAG, "PDU - char_code = %i", char_code);
              output_sms_string.append(simUtils::GsmUtils::Extended_ASCII_Char(char_code));
            }
          }
        }
        else
        {
          ESP_LOGV(TAG, "PDU - 7 bits message");
          output_sms_string = simUtils::GsmUtils::Decode_GSM7bit_PDU_Payload(payload, data_length);
        }
        this->message_ = output_sms_string;
      }
      break;
    case STATE_RECEIVED_SMS:
    // case STATE_RECEIVED_USSD:
      // Let the buffer flush. Next poll will request to delete the parsed index message.
      break;
    case STATE_SENDING_SMS_1:
      if (message == "> ") {
        const char * PDU = this->pdu_object_.getSMS();
        std::string str_PDU = PDU;
        str_PDU = str_PDU.substr(0, str_PDU.length() - 1);
        ESP_LOGD(TAG, "Envoi SMS - PDU = '%s'", str_PDU.c_str());
        ESP_LOGV(TAG, "S: %s - %d", str_PDU.c_str(), this->state_);
        this->watch_dog_ = 0;
        this->write_str(str_PDU.c_str());
        this->write_byte(ASCII_CTRL_Z);
        this->state_ = STATE_SENDING_SMS_2;
      } else {
        set_registered_(false);
        this->state_ = STATE_INIT;
        this->send_cmd_("AT+CMEE=2");
        this->write_byte(ASCII_CTRL_Z);
      }
      break;
    case STATE_SENDING_SMS_2:
      if (message.compare(0, 6, "+CMGS:") == 0) {
        ESP_LOGD(TAG, "SMS Sent OK: %s", message.c_str());
        this->send_pending_ = false;
        this->state_ = STATE_CHECK_SMS;
        this->expect_ack_ = true;
      }
      break;
    case STATE_DIALING:
      if (ok) {
        ESP_LOGI(TAG, "Dialing: '%s'", this->recipient_.c_str());
        this->dial_pending_ = false;
      } else {
        this->set_registered_(false);
        this->send_cmd_("AT+CMEE=2");
        this->write_byte(ASCII_CTRL_Z);
      }
      this->state_ = STATE_INIT;
      break;
    case STATE_PARSE_CLIP:
      if (message.compare(0, 6, "+CLIP:") == 0) {
        std::string caller_id;
        size_t start = 7;
        size_t end = message.find(',', start);
        uint8_t item = 0;
        while (end != start) {
          item++;
          if (item == 1) {  // Slot Index
            // Add 1 and remove 2 from substring to get rid of "quotes"
            caller_id = message.substr(start + 1, end - start - 2);
            break;
          }
          // item 4 = ""
          // item 5 = Received timestamp
          start = end + 1;
          end = message.find(',', start);
        }
        if (this->call_state_ != 4) {
          this->call_state_ = 4;
          ESP_LOGI(TAG, "Incoming call from %s", caller_id.c_str());
          rise_incoming_call_event(caller_id);
          // incoming_call_callback_.call(caller_id);
        }
        this->state_ = STATE_INIT;
      }
      break;
    case STATE_ATA_SENT:
      ESP_LOGI(TAG, "Call connected");
      if (this->call_state_ != 0) {
        this->call_state_ = 0;
        // this->call_connected_callback_.call();
      }
      this->state_ = STATE_INIT;
      break;

  // USSD
    // case STATE_SETUP_USSD:
    //   // send_cmd_("AT+CUSD=1");
    //   // this->state_ = STATE_CREG;
    //   // this->expect_ack_ = true;
    //   break;
    // case STATE_SEND_USSD1:
    //   // this->send_cmd_("AT+CUSD=1, \"" + this->ussd_ + "\"");
    //   // this->state_ = STATE_SEND_USSD2;
    //   // this->expect_ack_ = true;
    //   break;
    // case STATE_SEND_USSD2:
    //   // ESP_LOGD(TAG, "SendUssd2: '%s'", message.c_str());
    //   // if (message == "OK") {
    //   //   // Dialing
    //   //   ESP_LOGD(TAG, "Dialing ussd code: '%s' done.", this->ussd_.c_str());
    //   //   this->state_ = STATE_CHECK_USSD;
    //   //   this->send_ussd_pending_ = false;
    //   // } else {
    //   //   this->set_registered_(false);
    //   //   this->state_ = STATE_INIT;
    //   //   this->send_cmd_("AT+CMEE=2");
    //   //   this->write_byte(ASCII_CTRL_Z);
    //   // }
    //   break;
    // case STATE_CHECK_USSD:
    //   // ESP_LOGD(TAG, "Check ussd code: '%s'", message.c_str());
    //   // if (message.compare(0, 6, "+CUSD:") == 0) {
    //   //   this->state_ = STATE_RECEIVED_USSD;
    //   //   this->ussd_ = "";
    //   //   size_t start = 10;
    //   //   size_t end = message.find_last_of(',');
    //   //   if (end > start) {
    //   //     this->ussd_ = message.substr(start + 1, end - start - 2);
    //   //     this->ussd_received_callback_.call(this->ussd_);
    //   //   }
    //   // }
    //   // // Otherwise we receive another OK, we do nothing just wait polling to continuously check for SMS
    //   // if (message == "OK")
    //   //   this->state_ = STATE_INIT;
    //   break;
    default:
      ESP_LOGW(TAG, "Unhandled: %s - %d", message.c_str(), this->state_);
      break;
  }
}  // namespace sim900

void Sim900Component::loop() {
  // Read message
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);

    if (this->read_pos_ == SIM900_READ_BUFFER_LENGTH)
      this->read_pos_ = 0;

    ESP_LOGVV(TAG, "Buffer pos: %u %d", this->read_pos_, byte);  // NOLINT

    if (byte == ASCII_CR)
      continue;
    if (byte >= 0x7F)
      byte = '?';  // need to be valid utf8 string for log functions.
    this->read_buffer_[this->read_pos_] = byte;

    if (this->state_ == STATE_SENDING_SMS_1 && this->read_pos_ == 1 && byte == ' ' && this->read_buffer_[0] == '>')
      this->read_buffer_[++this->read_pos_] = ASCII_LF;

    if (this->read_buffer_[this->read_pos_] == ASCII_LF) {
      this->read_buffer_[this->read_pos_] = 0;
      this->read_pos_ = 0;
      this->parse_cmd_(this->read_buffer_);
    } else {
      this->read_pos_++;
    }
  }
  if (state_ == STATE_INIT && this->registered_ &&
      (this->call_state_ != 6  // A call is in progress
       || this->send_pending_ || this->dial_pending_ || this->connect_pending_ || this->disconnect_pending_)) {
    this->update();
  }
}

void Sim900Component::send_sms(const std::string &recipient, const std::string &message) {
  if (this->send_pending_){
    ESP_LOGW(TAG, "send_sms(), Impossible d'envoyer le SMS, un autre envoi est déjà en attente");
  } else {
    ESP_LOGI(TAG, "send_sms(), SMS en atente d'envoi = Tel : '%s', SMS : '%s'", recipient.c_str(), message.c_str());
    this->pdu_length_ = 0;
    this->pdu_length_ = this->pdu_object_.encodePDU(recipient.c_str(), message.c_str());
    if (this->pdu_length_ > 0) {
      this->send_pending_ = true;
    } else {
      switch(this->pdu_length_) {
        case this->pdu_object_.UCS2_TOO_LONG:
        case this->pdu_object_.GSM7_TOO_LONG:
            ESP_LOGE(TAG, "Envoi SMS - Message too long to send as a single message, change to multipart");
            break;
          case this->pdu_object_.WORK_BUFFER_TOO_SMALL:
            ESP_LOGE(TAG, "Envoi SMS - Work buffer too small, change PDU constructor");
            break;
          case this->pdu_object_.ADDRESS_FORMAT:
            ESP_LOGE(TAG, "Envoi SMS - SCA or Target address illegal characters or too long");
            break;
          case this->pdu_object_.MULTIPART_NUMBERS:
            ESP_LOGE(TAG, "Envoi SMS - Multipart numbers illogical");
            break;
          case this->pdu_object_.ALPHABET_8BIT_NOT_SUPPORTED:
            ESP_LOGE(TAG, "Envoi SMS - 8 bit alphabert not supported");
            break;
      }
    }
  }
}

// void Sim900Component::send_ussd(const std::string &ussd_code) {
//   ESP_LOGD(TAG, "Sending USSD code: %s", ussd_code.c_str());
//   this->ussd_ = ussd_code;
//   this->send_ussd_pending_ = true;
//   this->update();
// }
void Sim900Component::dump_config() {
  ESP_LOGCONFIG(TAG, "SIM900:");
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "Registered", this->registered_binary_sensor_);
#endif
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "Rssi", this->rssi_sensor_);
#endif
  LOG_TEXT_SENSOR("  ", "Etat du module", this->etat_module_text_sensor_);
}
void Sim900Component::dial(const std::string &recipient) {
  this->recipient_ = recipient;
  this->dial_pending_ = true;
}
void Sim900Component::connect() { this->connect_pending_ = true; }
void Sim900Component::disconnect() { this->disconnect_pending_ = true; }

void Sim900Component::set_registered_(bool registered) {
  this->registered_ = registered;
#ifdef USE_BINARY_SENSOR
  if (this->registered_binary_sensor_ != nullptr)
    this->registered_binary_sensor_->publish_state(registered);
#endif
}
void Sim900Component::set_rssi_(int rssi) {
  int signal=0;
  switch (rssi) {
    case 0:
      signal = 115;
      break;
    case 1:
      signal = 111;
      break;
    case 31:
      signal = 52;
      break;
    case 99:
      signal = 0;
      break;
    default:
      signal = map(rssi, 2, 30, 110, 54);
  }
  signal = signal * -1;
  ESP_LOGD(TAG, "RSSI: signal = %i dBm", signal);
#ifdef USE_SENSOR
  if (this->rssi_sensor_ != nullptr) {
    this->rssi_sensor_->publish_state(signal);
  } else {
    ESP_LOGD(TAG, "RSSI: %d", signal);
  }
#else
  ESP_LOGD(TAG, "RSSI: %d", signal);
#endif
}
void Sim900Component::set_etat_module_(int state_val) {
    std::string modem_status = "";
    switch (state_val) {
      case 0:
        modem_status = "Ready";
        break;
      case 2:
        modem_status = "Unknown";
        break;
      case 3:
        modem_status = "Ringing";
        break;
      case 4:
        modem_status = "Call in progress";
        break;
      default:
        modem_status = "Unknown";
    }
  if (this->etat_module_text_sensor_ != nullptr) {
    this->etat_module_text_sensor_->publish_state(modem_status);
  } else {
    ESP_LOGD(TAG, "Etat du module: %s", modem_status.c_str());
  }
}

void Sim900Component::rise_incoming_call_event(const std::string caller) 
{
  std::map<std::string, std::string> event_data;
  std::string lbl_caller = "caller";
  event_data.insert(std::make_pair(lbl_caller, caller));
  fire_homeassistant_event("esphome.incoming_call_event", event_data);
  ESP_LOGI(TAG, "rise_incoming_call_event : caller = %s", caller.c_str());
}

void Sim900Component::rise_sms_event(const std::string sender, const std::string message) 
{
  std::map<std::string, std::string> event_data;
  std::string lbl_sender = "sender";
  std::string lbl_message = "message";
  event_data.insert(std::make_pair(lbl_sender, sender));
  event_data.insert(std::make_pair(lbl_message, message));
  fire_homeassistant_event("esphome.received_sms_event", event_data);
  ESP_LOGI(TAG, "rise_sms_event : sender = %s, message = '%s'", sender.c_str(), message.c_str());
}

}  // namespace sim900
}  // namespace esphome
