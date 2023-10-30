#include "ProtocolHandler.h"
#include "CryptoHandler.h"
#include "FileHandler.h"
#include <cstring>   // For memcpy
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include "constants.h"
#include "checksum.h"

const std::string SERVER_ERROR_MSG = "server responded with error: ";

ProtocolHandler::ProtocolHandler(const std::string& server_address, int port, const std::string& name, const std::string& filePath)
        : serverAddress_(server_address), port_(port), clientName_(name), filePath_(filePath) {}

ProtocolHandler::~ProtocolHandler() {}

bool ProtocolHandler::handleConnection() {
    sockaddr_in server_addr;
    socket_ = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_ == -1) {
        std::cout << SERVER_ERROR_MSG + "failed to create a socket" << std::endl;
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    inet_pton(AF_INET, serverAddress_.c_str(), &server_addr.sin_addr);

    if (connect(socket_, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cout << SERVER_ERROR_MSG + "failed to connect to the server" << std::endl;
        return false;
    }
    return true;
}

void ProtocolHandler::sendRequest(const Request& request) {
    // Calculate the total size of the buffer
    size_t totalSize = sizeof(Request) - sizeof(request.payload) + request.payloadSize;

    // Allocate buffer
    char* buffer = new char[totalSize];

    // Copy fixed-size portion of the Request (excluding the payload pointer)
    std::memcpy(buffer, &request, sizeof(Request) - sizeof(request.payload));

    // Copy the variable-length payload
    std::memcpy(buffer + sizeof(Request) - sizeof(request.payload), request.payload, request.payloadSize);

    // Send the entire buffer
    send(socket_, buffer, totalSize, 0);

    // Clean up
    delete[] buffer;
}

Response ProtocolHandler::getResponse() {
    Response response;

    // Receive the fixed-size part of the response
    char header_buffer[1 + 2 + 4];  // version (1 byte) + code (2 bytes) + payloadSize (4 bytes)
    ssize_t bytes_received = recv(socket_, header_buffer, sizeof(header_buffer), 0);
    if (bytes_received != sizeof(header_buffer)) {
        // Handle partial receive or error
        return response;
    }

    // Deserialize the fixed-size part
    size_t offset = 0;
    response.version = char(header_buffer[offset] + '0');  // Convert to ascii value.
    offset += 1;

    response.code = ntohs(*reinterpret_cast<uint16_t*>(header_buffer + offset));
    offset += 2;

    response.payloadSize = ntohl(*reinterpret_cast<uint32_t*>(header_buffer + offset));
    offset += 4;

    // Receive the payload based on the payloadSize
    char payload_buffer[response.payloadSize];
    bytes_received = recv(socket_, payload_buffer, response.payloadSize, 0);
    if (bytes_received != response.payloadSize) {
        // Handle partial receive or error
        return response;
    }

    // Convert the payload buffer to a std::string
    response.payload = std::string(payload_buffer, response.payloadSize);

    return response;
}

std::string ProtocolHandler::compute_file_checksum(const std::string& filename) {
    CryptoHandler crypto;
    return crypto.compute_file_checksum(filename);
}

void ProtocolHandler::handleRetrySendFile(const Request& encrypted_content, int maxRetries) {
    int retry_count = 0;
    while (retry_count < maxRetries) {
        sendRequest(encrypted_content);
        Response response = getResponse();
        if (response.code == ServerResponses::FILE_RECEIVED_CRC_OK) {  // 2103: File received OK with CRC
            uint32_t receivedCRC = *reinterpret_cast<uint32_t*>(&response.payload[response.payload.size() - 4]);  // Extract last 4 bytes
            uint32_t calculatedCRC = readCrc(filePath_);

            if (receivedCRC == calculatedCRC) {
                std::cout << "CRC match!" << std::endl;
            } else {
                std::cout << "CRC not matching!" << std::endl;
            }
            break;  // Successful response, no need for more retries.
        }
    }

    // If after max_retries we did not get a successful response, handle failure (e.g., log, alert, etc.).
    if (retry_count == maxRetries) {
        std::cout << SERVER_ERROR_MSG + "reached max retries but wasn't able to successfully upload file to server" << std::endl;
        // Handle the failure case here.
    }
}

char* createFilePayloadBuffer(const std::string& fileName, const std::string& content) {
    size_t totalPayloadSize = 4 + 255 + content.size();  // 4 for content size, 255 for file name
    char* buffer = new char[totalPayloadSize];
    std::memset(buffer, 0, totalPayloadSize);

    // Convert contentSize to big-endian (network byte order)
    uint32_t contentSizeNetworkOrder = htonl(content.size());
    std::memcpy(buffer, &contentSizeNetworkOrder, 4);


    // Set file name
    std::strncpy(buffer + 4, fileName.c_str(), 255); // Assuming fileName won't exceed 255 characters

    // Set content
    std::memcpy(buffer + 4 + 255, content.data(), content.size());

    return buffer;
}

void ProtocolHandler::handleRegistration() {
    CryptoHandler crypto;
    FileHandler fileHandler;
    Response serverResponse;
    char clientId[16];

    // Step 1: Send registration request with an empty clientId:
    Request reg_request;
    memcpy(reg_request.clientId, "", 16);
    reg_request.version = PROTOCOL_VERSION;
    reg_request.code = ServerRequests::Codes::REGISTRATION;

    char* payload_buffer = new char[ServerRequests::Consts::NAME_FIELD_SIZE];
    std::memset(payload_buffer, 0, ServerRequests::Consts::NAME_FIELD_SIZE);
    std::strncpy(payload_buffer, clientName_.c_str(), ServerRequests::Consts::NAME_FIELD_SIZE - 1);
    reg_request.payloadSize = ServerRequests::Consts::NAME_FIELD_SIZE;
    reg_request.payload = payload_buffer;

    // Performing a request:
    sendRequest(reg_request);
    delete[] reg_request.payload;
    serverResponse = getResponse();
    memcpy(clientId, &serverResponse.payload[0], serverResponse.payloadSize);

    // Check if the registration was accepted by the server
    if(serverResponse.code != ServerResponses::REGISTRATION_SUCCESS) {
        // Handle registration failure
        std::cout << SERVER_ERROR_MSG + "failed to register to the server" << std::endl;
        return;
    }

    // Step 2: On successful registration, generate RSA keys
    auto [publicKey, privateKey] = crypto.generate_rsa_key_pair();
    Request pubkey_request;
    memcpy(pubkey_request.clientId, &clientId, 16);
    pubkey_request.version = PROTOCOL_VERSION;
    pubkey_request.code = ServerRequests::Codes::SEND_PUBLIC_KEY;  // Sending public key request code

    // Calculate total payload size
    size_t totalPayloadSize = ServerRequests::Consts::NAME_FIELD_SIZE + publicKey.size();

    // Allocate memory dynamically for the combined payload
    payload_buffer = new char[totalPayloadSize];

    // Initialize the buffer with zeros
    std::memset(payload_buffer, 0, totalPayloadSize);

    // Copy the client name into the buffer
    std::strncpy(payload_buffer, clientName_.c_str(), ServerRequests::Consts::NAME_FIELD_SIZE - 1);

    // Append the public key to the buffer
    std::strncpy(payload_buffer + ServerRequests::Consts::NAME_FIELD_SIZE, publicKey.c_str(), publicKey.size());

    pubkey_request.payloadSize = totalPayloadSize;
    pubkey_request.payload = payload_buffer;

    // Save the private key:
    fileHandler.savePrivateRSAKey(privateKey);

    // Performing a request, sending the public key to the server:
    sendRequest(pubkey_request);
    delete[] payload_buffer;
    serverResponse = getResponse();

    if(serverResponse.code != ServerResponses::RECEIVED_PUBLIC_KEY_SEND_AES) {
        // Handle registration failure
        std::cout << SERVER_ERROR_MSG + "didn't receive a public AES key from the server" << std::endl;
        return;
    }

    // Decrypt received AES key using the RSA private key - skip first 16 bytes of Client ID:
    std::string encrypted_aes_key = serverResponse.payload.substr(16);
    std::string aes_key = crypto.decrypt_with_rsa(encrypted_aes_key, privateKey);

    // Read the file's contents
    std::string fileContents = fileHandler.readFileContents(filePath_);

    // Step 3: Encrypt file using AES key and send to the server
    // Encrypt the file content using AES key
    std::string encrypted_content = CryptoHandler::encrypt_with_aes(fileContents, aes_key);

    // Create the payload buffer
    char* payloadBuffer = createFilePayloadBuffer(filePath_, encrypted_content);

    // Set up the request object
    Request encrypted_file_request;
    memcpy(encrypted_file_request.clientId, &clientId, 16);
    encrypted_file_request.version = PROTOCOL_VERSION;
    encrypted_file_request.code = ServerRequests::Codes::SEND_FILE;  // Sending a file request code
    encrypted_file_request.payloadSize = 4 + 255 + encrypted_content.size();  // 4 for content size, 255 for file name, rest for content
    encrypted_file_request.payload = payloadBuffer;

    // Attempting to perform the request up to 3 times:
    handleRetrySendFile(encrypted_file_request, 3);

    // Clean up
    delete[] encrypted_file_request.payload;

//    MeInfo me_info = fileHandler.readMeInfo();
//    std::string encrypted_content = crypto.encrypt_with_aes(me_info.base64Key);
//    Request encrypted_file_request;
//    memcpy(encrypted_file_request.clientId, &clientId, 16);
//    encrypted_file_request.version = PROTOCOL_VERSION;
//    encrypted_file_request.code = ServerRequests::Codes::SEND_FILE;  // Sending a file request code
//    encrypted_file_request.payloadSize = encrypted_content.size();
//    encrypted_file_request.payload = encrypted_content;
//
//    // Attempting to perform the request up to 3 times:
//    handleRetrySendFile(encrypted_file_request, 3);
}


//void ProtocolHandler::handleReconnection() {
//    CryptoHandler crypto;
//    FileHandler fileHandler;
//    Response server_response;
//
//    // Step 1: Send reconnect request
//    Request reconnect_request;
//    memcpy(reconnect_request.clientId, "YOUR_clientId", 16);
//    reconnect_request.version = '1';
//    reconnect_request.code = 1027;  // Reconnecting request code
//    reconnect_request.payloadSize = 0;  // No payload for reconnection
//    sendRequest(reconnect_request);
//
//    server_response = getResponse();
//    // Check if the reconnection was accepted by the server
//    if(server_response.code != 2105) {  // 2105: approves a reconnection request
//        // Handle reconnection failure and maybe revert to registration
//        handleRegistration();
//        return;
//    }
//
//    // Step 2: On successful reconnection, use the received AES key
//    std::string encrypted_aes_key = server_response.payload;
//    // TODO: Fetch the private RSA key from local storage or some cache
//    std::string private_key = "";  // Placeholder for now
//    std::string aes_key = crypto.decrypt_with_rsa(encrypted_aes_key, private_key);
//
//    // Step 3: Encrypt file using AES key and send to the server
//    TransferInfo transferInfo = fileHandler.readTransferInfo();
//    std::string encrypted_content = crypto.encrypt_with_aes(transferInfo.filePath);
//    Request reconnect_encrypted_file_request;
//    memcpy(reconnect_encrypted_file_request.clientId, "YOUR_clientId", 16);
//    reconnect_encrypted_file_request.version = '1';
//    reconnect_encrypted_file_request.code = 1028;  // Sending a file request code for reconnection
//    reconnect_encrypted_file_request.payloadSize = encrypted_content.size();
//    reconnect_encrypted_file_request.payload = encrypted_content;
//    sendRequest(reconnect_encrypted_file_request);
//
//    server_response = getResponse();
//    // Check the response code to handle retries or success
//    if(server_response.code != 2103) {  // 2103: File received OK with CRC
//        // Handle retries or failure based on the response code
//        handleRetrySendFile(reconnect_encrypted_file_request, 3);
//    }
//}
