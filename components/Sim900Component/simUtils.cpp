#include "simUtils.h"
#include "esphome/core/log.h"
// #include <cstring>


namespace esphome {
    namespace simUtils {

        static const char *const TAG = "GsmUtils";

        static std::string GsmUtils::Decode_GSM7bit_PDU_Payload(std::string payload, int sms_text_length) {
            std::string output = "";
            int payload_pos = 0;
            int buffer_sms_length = payload.length() / 2;
            // ESP_LOGV(TAG, "buffer_sms_length = %i", buffer_sms_length);
        
            int buffer_1 = 0;
            int buffer_2 = 0;
            int char_val = 0;
            bool next_char_is_extension = false;
            int output_text_length = 0;
        
            if (buffer_sms_length > 0) {
                buffer_1 = std::stoul(payload.substr(payload_pos, 2), nullptr, 16);
                payload_pos += 2;
            
                if (buffer_sms_length > 1) {
                    buffer_2 = std::stoul(payload.substr(payload_pos, 2), nullptr, 16);
                    payload_pos += 2;
                }
            
                char_val = BITMASK_7BITS & buffer_1;
                output.append(this->Decode_GSM_Char(char_val));
                output_text_length++;
            }
            int carry_on_bits = 1;
            int i = 1;
        
            for (; i < buffer_sms_length; ++i) {
                char_val = BITMASK_7BITS &	((buffer_2 << carry_on_bits) | (buffer_1 >> (8 - carry_on_bits)));
                output.append(this->Decode_GSM_Char(char_val));
                output_text_length++;
            
                if (output_text_length == sms_text_length) break;
            
                carry_on_bits++;
            
                if (carry_on_bits == 8) {
                    carry_on_bits = 1;
                    char_val = buffer_2 & BITMASK_7BITS;
                    output.append(this->Decode_GSM_Char(char_val));
                    output_text_length++;
                    if (output_text_length == sms_text_length) break;
                }
            
                buffer_1 = buffer_2;
            
                if (i + 1 < buffer_sms_length) {
                    buffer_2 = std::stoul(payload.substr(payload_pos, 2), nullptr, 16);
                    payload_pos += 2;
                }
            }
            
            if (output_text_length < sms_text_length) { // Add last remainder.
                char_val = buffer_1 >> (8 - carry_on_bits);
                output.append(this->Decode_GSM_Char(char_val));
                output_text_length++;
            }
        
        
            return output;
        }

        static std::string GsmUtils::Decode_GSM_Char(int char_code) {
            static bool next_char_is_extension = false;
            if (char_code == 27)
            {
                next_char_is_extension = true;
            }
            else
            {
                if (next_char_is_extension)
                {
                    next_char_is_extension = false;
                    return this->GSM7bits_Extension_Char(char_code);
                }
                else
                {
                    return this->GSM7bits_Char(char_code);
                }
            }
            return "";
        }
        
        static std::string GsmUtils::GSM7bits_Char(int char_code) {
            // Standard GSM7bits
            switch(char_code) {
                case 0:
                    return "@";
                    break;
                case 1:
                    return "£";
                    break;
                case 2:
                    return "$";
                    break;
                case 3:
                    return "¥";
                    break;
                case 4:
                    return "è";
                    break;
                case 5:
                    return "é";
                    break;
                case 6:
                    return "ù";
                    break;
                case 7:
                    return "ì";
                    break;
                case 8:
                    return "ò";
                    break;
                case 9:
                    return "Ç";
                    break;
                case 10:
                    return "\n";
                    break;
                case 11:
                    return "Ø";
                    break;
                case 12:
                    return "ø";
                    break;
                case 13:
                    return "\r";
                    break;
                case 14:
                    return "Å";
                    break;
                case 15:
                    return "å";
                    break;
                case 16:
                    return "Δ";
                    break;
                case 17:
                    return "_";
                    break;
                case 18:
                    return "Φ";
                    break;
                case 19:
                    return "Γ";
                    break;
                case 20:
                    return "Λ";
                    break;
                case 21:
                    return "Ω";
                    break;
                case 22:
                    return "Π";
                    break;
                case 23:
                    return "Ψ";
                    break;
                case 24:
                    return "Σ";
                    break;
                case 25:
                    return "Θ";
                    break;
                case 26:
                    return "Ξ";
                    break;
                case 27:
                    return ""; // ESC = Extension GSM 7bit
                    break;
                case 28:
                    return "Æ";
                    break;
                case 29:
                    return "æ";
                    break;
                case 30:
                    return "ß";
                    break;
                case 31:
                    return "É";
                    break;
                case 32:
                    return " ";
                    break;
                case 33:
                    return "!";
                    break;
                case 34:
                    return "\"";
                    break;
                case 35:
                    return "#";
                    break;
                case 36:
                    return "¤";
                    break;
                case 37:
                    return "%";
                    break;
                case 38:
                    return "&";
                    break;
                case 39:
                    return "'";
                    break;
                case 40:
                    return "(";
                    break;
                case 41 :
                    return ")";
                    break;
                case 42 :
                    return "*";
                    break;
                case 43 :
                    return "+";
                    break;
                case 44 :
                    return ",";
                    break;
                case 45 :
                    return "-";
                    break;
                case 46 :
                    return ".";
                    break;
                case 47 :
                    return "/";
                    break;
                case 48 :
                    return "0";
                    break;
                case 49 :
                    return "1";
                    break;
                case 50 :
                    return "2";
                    break;
                case 51 :
                    return "3";
                    break;
                case 52 :
                    return "4";
                    break;
                case 53 :
                    return "5";
                    break;
                case 54 :
                    return "6";
                    break;
                case 55 :
                    return "7";
                    break;
                case 56 :
                    return "8";
                    break;
                case 57 :
                    return "9";
                    break;
                case 58 :
                    return ":";
                    break;
                case 59 :
                    return ";";
                    break;
                case 60 :
                    return "<";
                    break;
                case 61 :
                    return "=";
                    break;
                case 62 :
                    return ">";
                    break;
                case 63 :
                    return "?";
                    break;
                case 64 :
                    return "¡";
                    break;
                case 65 :
                    return "A";
                    break;
                case 66 :
                    return "B";
                    break;
                case 67 :
                    return "C";
                    break;
                case 68 :
                    return "D";
                    break;
                case 69 :
                    return "E";
                    break;
                case 70 :
                    return "F";
                    break;
                case 71 :
                    return "G";
                    break;
                case 72 :
                    return "H";
                    break;
                case 73 :
                    return "I";
                    break;
                case 74 :
                    return "J";
                    break;
                case 75 :
                    return "K";
                    break;
                case 76 :
                    return "L";
                    break;
                case 77 :
                    return "M";
                    break;
                case 78 :
                    return "N";
                    break;
                case 79 :
                    return "O";
                    break;
                case 80 :
                    return "P";
                    break;
                case 81 :
                    return "Q";
                    break;
                case 82 :
                    return "R";
                    break;
                case 83 :
                    return "S";
                    break;
                case 84 :
                    return "T";
                    break;
                case 85 :
                    return "U";
                    break;
                case 86 :
                    return "V";
                    break;
                case 87 :
                    return "W";
                    break;
                case 88 :
                    return "X";
                    break;
                case 89 :
                    return "Y";
                    break;
                case 90 :
                    return "Z";
                    break;
                case 91 :
                    return "Ä";
                    break;
                case 92 :
                    return "Ö";
                    break;
                case 93 :
                    return "Ñ";
                    break;
                case 94 :
                    return "Ü";
                    break;
                case 95 :
                    return "§";
                    break;
                case 96 :
                    return "¿";
                    break;
                case 97 :
                    return "a";
                    break;
                case 98 :
                    return "b";
                    break;
                case 99 :
                    return "c";
                    break;
                case 100:
                    return "d";
                    break;
                case 101:
                    return "e";
                    break;
                case 102:
                    return "f";
                    break;
                case 103:
                    return "g";
                    break;
                case 104:
                    return "h";
                    break;
                case 105:
                    return "i";
                    break;
                case 106:
                    return "j";
                    break;
                case 107:
                    return "k";
                    break;
                case 108:
                    return "l";
                    break;
                case 109:
                    return "m";
                    break;
                case 110:
                    return "n";
                    break;
                case 111:
                    return "o";
                    break;
                case 112:
                    return "p";
                    break;
                case 113:
                    return "q";
                    break;
                case 114:
                    return "r";
                    break;
                case 115:
                    return "s";
                    break;
                case 116:
                    return "t";
                    break;
                case 117:
                    return "u";
                    break;
                case 118:
                    return "v";
                    break;
                case 119:
                    return "w";
                    break;
                case 120:
                    return "x";
                    break;
                case 121:
                    return "y";
                    break;
                case 122:
                    return "z";
                    break;
                case 123:
                    return "ä";
                    break;
                case 124:
                    return "ö";
                    break;
                case 125:
                    return "ñ";
                    break;
                case 126:
                    return "ü";
                    break;
                case 127:
                    return "à";
                    break;
                default:
                    return "?";
            }
            return "?";
        }
        
        static std::string GsmUtils::GSM7bits_Extension_Char(int char_code) {
            // Basic GSM7 Character Set Extension
            switch(char_code) {
                // case 10:
                    //   return "FF";
                    //   break;
                // case 13:
                    //   return "CR2";
                    //   break;
                case 20:
                    return "^";
                    break;
                // case 27:
                    //   return "SS2";
                    //   break;
                case 40:
                    return "{";
                    break;
                case 41:
                    return "}";
                    break;
                case 47:
                    return "\\";
                    break;
                case 60:
                    return "[";
                    break;
                case 61:
                    return "~";
                    break;
                case 62:
                    return "]";
                    break;
                case 64:
                    return "|";
                    break;
                case 101:
                    return "€";
                    break;
                default:
                    return "?";
            }
            return "?";
        }
        
        static std::string GsmUtils::Extended_ASCII_Char(int char_code) {
            // ASCII Extended from char(160) to char(255)
            switch(char_code) {
                case 160:
                    return " "; // "&nbsp;";
                    break;
                case 161:
                    return "¡";
                    break;
                case 162:
                    return "¢";
                    break;
                case 163:
                    return "£";
                    break;
                case 164:
                    return "¤";
                    break;
                case 165:
                    return "¥";
                    break;
                case 166:
                    return "¦";
                    break;
                case 167:
                    return "§";
                    break;
                case 168:
                    return "¨";
                    break;
                case 169:
                    return "©";
                    break;
                case 170:
                    return "ª";
                    break;
                case 171:
                    return "«";
                    break;
                case 172:
                    return "¬";
                    break;
                // case 173:
                    //   return "SHY";
                    //   break;
                case 174:
                    return "®";
                    break;
                case 175:
                    return "¯";
                    break;
                case 176:
                    return "°";
                    break;
                case 177:
                    return "±";
                    break;
                case 178:
                    return "²";
                    break;
                case 179:
                    return "³";
                    break;
                case 180:
                    return "´";
                    break;
                case 181:
                    return "µ";
                    break;
                case 182:
                    return "¶";
                    break;
                case 183:
                    return "·";
                    break;
                case 184:
                    return "¸";
                    break;
                case 185:
                    return "¹";
                    break;
                case 186:
                    return "º";
                    break;
                case 187:
                    return "»";
                    break;
                case 188:
                    return "¼";
                    break;
                case 189:
                    return "½";
                    break;
                case 190:
                    return "¾";
                    break;
                case 191:
                    return "¿";
                    break;
                case 192:
                    return "À";
                    break;
                case 193:
                    return "Á";
                    break;
                case 194:
                    return "Â";
                    break;
                case 195:
                    return "Ã";
                    break;
                case 196:
                    return "Ä";
                    break;
                case 197:
                    return "Å";
                    break;
                case 198:
                    return "Æ";
                    break;
                case 199:
                    return "Ç";
                    break;
                case 200:
                    return "È";
                    break;
                case 201:
                    return "É";
                    break;
                case 202:
                    return "Ê";
                    break;
                case 203:
                    return "Ë";
                    break;
                case 204:
                    return "Ì";
                    break;
                case 205:
                    return "Í";
                    break;
                case 206:
                    return "Î";
                    break;
                case 207:
                    return "Ï";
                    break;
                case 208:
                    return "Ð";
                    break;
                case 209:
                    return "Ñ";
                    break;
                case 210:
                    return "Ò";
                    break;
                case 211:
                    return "Ó";
                    break;
                case 212:
                    return "Ô";
                    break;
                case 213:
                    return "Õ";
                    break;
                case 214:
                    return "Ö";
                    break;
                case 215:
                    return "×";
                    break;
                case 216:
                    return "Ø";
                    break;
                case 217:
                    return "Ù";
                    break;
                case 218:
                    return "Ú";
                    break;
                case 219:
                    return "Û";
                    break;
                case 220:
                    return "Ü";
                    break;
                case 221:
                    return "Ý";
                    break;
                case 222:
                    return "Þ";
                    break;
                case 223:
                    return "ß";
                    break;
                case 224:
                    return "à";
                    break;
                case 225:
                    return "á";
                    break;
                case 226:
                    return "â";
                    break;
                case 227:
                    return "ã";
                    break;
                case 228:
                    return "ä";
                    break;
                case 229:
                    return "å";
                    break;
                case 230:
                    return "æ";
                    break;
                case 231:
                    return "ç";
                    break;
                case 232:
                    return "è";
                    break;
                case 233:
                    return "é";
                    break;
                case 234:
                    return "ê";
                    break;
                case 235:
                    return "ë";
                    break;
                case 236:
                    return "ì";
                    break;
                case 237:
                    return "í";
                    break;
                case 238:
                    return "î";
                    break;
                case 239:
                    return "ï";
                    break;
                case 240:
                    return "ð";
                    break;
                case 241:
                    return "ñ";
                    break;
                case 242:
                    return "ò";
                    break;
                case 243:
                    return "ó";
                    break;
                case 244:
                    return "ô";
                    break;
                case 245:
                    return "õ";
                    break;
                case 246:
                    return "ö";
                    break;
                case 247:
                    return "÷";
                    break;
                case 248:
                    return "ø";
                    break;
                case 249:
                    return "ù";
                    break;
                case 250:
                    return "ú";
                    break;
                case 251:
                    return "û";
                    break;
                case 252:
                    return "ü";
                    break;
                case 253:
                    return "ý";
                    break;
                case 254:
                    return "þ";
                    break;
                case 255:
                    return "ÿ";
                    break;

                case 956:
                    return "µ";
                    break;
                case 8364:
                    return "€";
                    break;
            default:
                return "?";
            }
            ESP_LOGW(TAG, "Extended_ASCII_Char - char_code %i non trouvé", char_code);
            return "?";
        }

    }  // namespace simUtils
}  // namespace esphome
