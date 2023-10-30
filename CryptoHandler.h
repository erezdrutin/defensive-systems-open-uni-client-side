//
// Created by Erez on 27/10/2023.
//

#ifndef DEFENSIVE_MAMAN_15_CRYPTOHANDLER_H
#define DEFENSIVE_MAMAN_15_CRYPTOHANDLER_H

#include <string>
#include <utility> // for std::pair

class CryptoHandler {
public:
    CryptoHandler();
    ~CryptoHandler();

    std::pair<std::string, std::string> generate_rsa_key_pair();
    static std::string encrypt_with_aes(const std::string& plaintext, const std::string& aes_key);
    std::string decrypt_with_aes(const std::string& ciphertext);
    std::string encrypt_with_rsa(const std::string& plaintext, const std::string& public_key);
    std::string decrypt_with_rsa(const std::string& ciphertext, const std::string& private_key);
    std::string encode_base64(const std::string& plaintext);
    std::string decode_base64(const std::string& encoded_text);
    std::string compute_file_checksum(const std::string& filename); // Added the checksum function
};


#endif //DEFENSIVE_MAMAN_15_CRYPTOHANDLER_H
