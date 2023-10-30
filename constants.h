//
// Created by Erez on 27/10/2023.
//

#ifndef DEFENSIVE_MAMAN_15_CONSTANTS_H
#define DEFENSIVE_MAMAN_15_CONSTANTS_H
#include "string"

const char CLIENTS_BASE_PATH[] = "/Users/erez/Desktop/defensive_prog_lab/c++/defensive_maman_15";
const char PROTOCOL_VERSION = '3';

namespace ServerRequests {
    namespace Codes {
        constexpr uint16_t REGISTRATION = 1025;
        constexpr uint16_t SEND_PUBLIC_KEY = 1026;
        constexpr uint16_t RECONNECT = 1027;
        constexpr uint16_t SEND_FILE = 1028;
        constexpr uint16_t CRC_CORRECT = 1029;
        constexpr uint16_t CRC_INCORRECT_RESEND = 1030;
        constexpr uint16_t CRC_INCORRECT_DONE = 1031;
    }
    namespace Consts {
        constexpr uint16_t NAME_FIELD_SIZE = 255;
    }
}

namespace ServerResponses {
    constexpr uint16_t REGISTRATION_SUCCESS = 2100;
    constexpr uint16_t REGISTRATION_FAILED = 2101;
    constexpr uint16_t RECEIVED_PUBLIC_KEY_SEND_AES = 2102;
    constexpr uint16_t FILE_RECEIVED_CRC_OK = 2103;
    constexpr uint16_t CONFIRM_RECEIPT = 2104;
    constexpr uint16_t APPROVE_RECONNECT_SEND_AES = 2105;
    constexpr uint16_t RECONNECT_REJECTED = 2106;
}

#endif //DEFENSIVE_MAMAN_15_CONSTANTS_H
