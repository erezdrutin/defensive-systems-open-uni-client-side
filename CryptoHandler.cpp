/**
 * Author: Erez Drutin
 * Date: 04.11.2023
 * Purpose: Handle all "crypto" related stuff in the client-side utilizing the provided wrappers and checksum code.
 */
#include <sstream>
#include "CryptoHandler.h"
#include "AESWrapper.h"
#include "RSAWrapper.h"
#include "checksum.h"

CryptoHandler::CryptoHandler() = default;
CryptoHandler::~CryptoHandler() = default;

/**
 * Generates a pair of RSA keys (public and private).
 * @return A pair containing the public key and private key as strings.
 */
std::pair<std::string, std::string> CryptoHandler::generate_rsa_key_pair() {
    RSAPrivateWrapper rsaPrivate;
    return {rsaPrivate.getPublicKey(), rsaPrivate.getPrivateKey()};
}

/**
 * Encrypts a plaintext string using AES encryption with a specified key.
 * @param plaintext The input string to encrypt.
 * @param aes_key The AES key as a string, which must be exactly 16 bytes long.
 * @throws std::invalid_argument If the key length is not 16 bytes.
 * @return The encrypted string in hexadecimal format.
 */
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

/**
 * Decrypts a ciphertext string using RSA encryption with a specified private key.
 * @param ciphertext The encrypted string to decrypt.
 * @param private_key The RSA private key as a string.
 * @return The decrypted string.
 */
std::string CryptoHandler::decrypt_with_rsa(const std::string& ciphertext, const std::string& private_key) {
    RSAPrivateWrapper rsaPrivate(private_key);
    return rsaPrivate.decrypt(ciphertext);
}
