#include <string>
#include "pdulib.h"

namespace esphome {
    namespace simUtils {
        class GsmUtils {
            public:
                static std::string Decode_GSM7bit_PDU_Payload(std::string payload, int sms_text_length);
                static std::string Extended_ASCII_Char(int char_code);

            private:
                static std::string Decode_GSM_Char(int char_code);
                static std::string GSM7bits_Char(int char_code);
                static std::string GSM7bits_Extension_Char(int char_code);
        };
    }  // namespace simUtils
}  // namespace esphome
