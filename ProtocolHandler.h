/**
 * Author: Erez Drutin
 * Date: 04.11.2023
 * Purpose: Serve as a header file for ProtocolHandler.cpp
 */
#ifndef DEFENSIVE_MAMAN_15_PROTOCOLHANDLER_H
#define DEFENSIVE_MAMAN_15_PROTOCOLHANDLER_H

#include <string>
#include "Logger.h"

struct Request {
    char clientId[16];
    char version;
    uint16_t code;
    uint32_t payloadSize;
    char* payload;
};

struct Response {
    char version;
    uint16_t code;
    uint32_t payloadSize;
    std::string payload;
};

class ProtocolHandler {
public:
    ProtocolHandler(std::string  server_address, int port, std::string  name, std::string  filePath);
    ~ProtocolHandler();
    bool handleConnection();
    bool handleRegistration();
    bool handleReconnection();
    void sendRequest(const Request& request);
    Response getResponse();
    static ssize_t safeReceive(int socket, void *buffer, size_t length, int flags);
    bool sendCRCStatusRequest(char *clientId, uint16_t code);
    bool handleRetrySendFile(const Request& encrypted_content, int maxRetries, char *clientId);
    Response handleRSARegistration(char* clientId, std::string& outPrivateKey);
    bool handleFileEncryptionAndSend(const std::string& encrypted_aes_key, const std::string& privateKey, const char* clientId);
    Response handleConnectionRequest(char *clientId, uint16_t requestCode);

private:
    std::string serverAddress_;
    std::string clientName_;
    std::string filePath_;
    int socket_{};
    int port_;
    Logger logger_;
};


#endif
