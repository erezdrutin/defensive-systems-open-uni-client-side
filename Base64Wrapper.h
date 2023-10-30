//
// Created by Erez on 27/10/2023.
//

#ifndef DEFENSIVE_MAMAN_15_BASE64WRAPPER_H
#define DEFENSIVE_MAMAN_15_BASE64WRAPPER_H

#include <string>
#include "cryptopp/base64.h"


class Base64Wrapper
{
public:
    static std::string encode(const std::string& str);
    static std::string decode(const std::string& str);
};



#endif //DEFENSIVE_MAMAN_15_BASE64WRAPPER_H
