#include <sstream>
#include "CryptoHandler.h"
#include "AESWrapper.h"
#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "checksum.h"

CryptoHandler::CryptoHandler() {}

CryptoHandler::~CryptoHandler() {}

std::pair<std::string, std::string> CryptoHandler::generate_rsa_key_pair() {
    RSAPrivateWrapper rsaPrivate;
    return {rsaPrivate.getPublicKey(), rsaPrivate.getPrivateKey()};
}

std::string CryptoHandler::encrypt_with_aes(const std::string& plaintext, const std::string& aes_key) {
    if (aes_key.length() != 16) {  // AES-128 key length is 16 bytes
        throw std::invalid_argument("Key length must be 16 bytes.");
    }

    AESWrapper aesWrapper(reinterpret_cast<const unsigned char*>(aes_key.c_str()), 16);
    std::string encrypted = aesWrapper.encrypt(plaintext.c_str(), plaintext.length());

    // Convert encrypted message to hexadecimal format
    std::ostringstream oss;
    for (unsigned char c : encrypted) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }

    return oss.str();
}


std::string CryptoHandler::decrypt_with_aes(const std::string& ciphertext) {
    AESWrapper aes;
    return aes.decrypt(ciphertext.c_str(), ciphertext.length());
}

std::string CryptoHandler::encrypt_with_rsa(const std::string& plaintext, const std::string& public_key) {
    RSAPublicWrapper rsaPublic(public_key);
    return rsaPublic.encrypt(plaintext.c_str(), plaintext.length());
}

std::string CryptoHandler::decrypt_with_rsa(const std::string& ciphertext, const std::string& private_key) {
    RSAPrivateWrapper rsaPrivate(private_key);
    return rsaPrivate.decrypt(ciphertext);
}

std::string CryptoHandler::encode_base64(const std::string& plaintext) {
    return Base64Wrapper::encode(plaintext);
}

std::string CryptoHandler::decode_base64(const std::string& encoded_text) {
    return Base64Wrapper::decode(encoded_text);
}

std::string CryptoHandler::compute_file_checksum(const std::string& filename) {
    return readfile(filename);  // Using the readfile function directly from cksum_new.cpp
}
