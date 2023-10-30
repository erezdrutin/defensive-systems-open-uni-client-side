//
// Created by Erez on 27/10/2023.
//

#include "AESWrapper.h"
#include "cryptopp/modes.h"
#include "cryptopp/aes.h"
#include "cryptopp/filters.h"
#include "cryptopp/osrng.h"
#include <stdexcept>
#include <immintrin.h>	// _rdrand32_step


unsigned char* AESWrapper::GenerateKey(unsigned char* buffer, unsigned int length)
{
    for (size_t i = 0; i < length; i += sizeof(unsigned int))
        _rdrand32_step(reinterpret_cast<unsigned int*>(&buffer[i]));
    return buffer;
}

AESWrapper::AESWrapper()
{
    GenerateKey(_key, DEFAULT_KEYLENGTH);
}

AESWrapper::AESWrapper(const unsigned char* key, unsigned int length)
{
    if (length != DEFAULT_KEYLENGTH)
        throw std::length_error("key length must be 16 bytes");
    CryptoPP::memcpy_s(_key, DEFAULT_KEYLENGTH, key, length);
}

AESWrapper::~AESWrapper()
{
}

const unsigned char* AESWrapper::getKey() const
{
    return _key;
}

std::string AESWrapper::encrypt(const char* plain, unsigned int length)
{
    // Generate a random IV
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock(iv, sizeof(iv));

    CryptoPP::AES::Encryption aesEncryption(_key, DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

    std::string cipher;
    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipher));
    stfEncryptor.Put(reinterpret_cast<const CryptoPP::byte*>(plain), length);
    stfEncryptor.MessageEnd();

    // Prefix the IV to the cipher text before returning
    return std::string(reinterpret_cast<char*>(iv), CryptoPP::AES::BLOCKSIZE) + cipher;
}


std::string AESWrapper::decrypt(const char* cipher, unsigned int length)
{
    if (length < CryptoPP::AES::BLOCKSIZE)
        throw std::length_error("cipher length too short to contain an IV");

    // Extract the IV from the cipher text
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
    memcpy(iv, cipher, CryptoPP::AES::BLOCKSIZE);

    CryptoPP::AES::Decryption aesDecryption(_key, DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

    std::string decrypted;
    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decrypted));
    stfDecryptor.Put(reinterpret_cast<const CryptoPP::byte*>(cipher + CryptoPP::AES::BLOCKSIZE), length - CryptoPP::AES::BLOCKSIZE);
    stfDecryptor.MessageEnd();

    return decrypted;
}
