/**
 * Author: Erez Drutin
 * Date: 04.11.2023
 * Purpose: Serve as a header file for CryptoHandler.cpp
 */
#ifndef DEFENSIVE_MAMAN_15_CRYPTOHANDLER_H
#define DEFENSIVE_MAMAN_15_CRYPTOHANDLER_H

#include <string>
#include <utility>

class CryptoHandler {
public:
    CryptoHandler();
    ~CryptoHandler();

    static std::pair<std::string, std::string> generate_rsa_key_pair();
    static std::string encrypt_with_aes(const std::string& plaintext, const std::string& aes_key);
    static std::string decrypt_with_rsa(const std::string& ciphertext, const std::string& private_key);
};


#endif
