//
// Created by Erez on 27/10/2023.
//

#ifndef DEFENSIVE_MAMAN_15_PROTOCOLHANDLER_H
#define DEFENSIVE_MAMAN_15_PROTOCOLHANDLER_H

#include <string>
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
    ProtocolHandler(const std::string& server_address, int port, const std::string& name, const std::string& filePath);
    ~ProtocolHandler();
    bool handleConnection();
    void handleRegistration();
    void sendRequest(const Request& request);
    Response getResponse();
    void handleReconnection();
    void handleRetrySendFile(const Request& encrypted_content, int maxRetries);
    std::string compute_file_checksum(const std::string& filename);

private:
    std::string serverAddress_;
    std::string clientName_;
    std::string filePath_;
    int socket_;
    int port_;
    // ... any other necessary member variables (like a socket object) ...
};


#endif //DEFENSIVE_MAMAN_15_PROTOCOLHANDLER_H
