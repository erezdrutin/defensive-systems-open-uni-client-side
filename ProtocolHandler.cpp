/**
 * Author: Erez Drutin
 * Date: 04.11.2023
 * Purpose: Handle all protocol related operations in the client-side code.
 */
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
#include <system_error>
#include <utility>


ProtocolHandler::ProtocolHandler(std::string  server_address, int port, std::string  name, std::string  filePath)
        : serverAddress_(std::move(server_address)), port_(port), clientName_(std::move(name)), filePath_(std::move(filePath)), logger_("ProtocolHandler") {}

ProtocolHandler::~ProtocolHandler() = default;

/**
 * Establishes a connection to the server.
 * @return True if the connection is successful, false otherwise.
 */
bool ProtocolHandler::handleConnection() {
    sockaddr_in server_addr{};
    socket_ = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_ == -1) {
        logger_.serverError("failed to create a socket");
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    inet_pton(AF_INET, serverAddress_.c_str(), &server_addr.sin_addr);

    if (connect(socket_, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        logger_.serverError("failed to connect to the server");
        return false;
    }
    return true;
}

/**
 * Sends a request object to the server. If we were unable to send the message - throwing / logging the error.
 * @param request The request object to send.
 */
void ProtocolHandler::sendRequest(const Request& request) {
    // Calculate the total size of the buffer
    size_t totalSize = sizeof(Request) - sizeof(request.payload) + request.payloadSize;

    // Allocate buffer
    char* buffer = new char[totalSize];

    // Copy fixed-size portion of the Request (excluding the payload pointer)
    std::memcpy(buffer, &request, sizeof(Request) - sizeof(request.payload));

    // Copy the variable-length payload
    std::memcpy(buffer + sizeof(Request) - sizeof(request.payload), request.payload, request.payloadSize);

    // Sending the request to the server:
    try {
        if (send(socket_, buffer, totalSize, 0) == -1) {
            // Throwing a custom error:
            throw std::system_error(errno, std::system_category(), "Failed to send message to server");
        }
    } catch (const std::exception& e) {
        logger_.error((std::ostringstream() << "Exception caught in sendRequest: " << e.what()).str());
    }

    // Clean up
    delete[] buffer;
}

/**
 * Receives data safely from a socket into a buffer and returns how many bytes were received.
 * @param socket The socket descriptor.
 * @param buffer The buffer to receive the data into.
 * @param length The number of bytes to receive.
 * @param flags The flags to pass to recv.
 * @return The number of bytes received, or -1 if an error occurred.
 */
ssize_t ProtocolHandler::safeReceive(int socket, void *buffer, size_t length, int flags) {
    ssize_t bytes_received;
    try {
        bytes_received = recv(socket, buffer, length, flags);
        if (bytes_received == -1) {
            // Handle error
            throw std::system_error(errno, std::system_category(), "recv failed");
        }
    } catch (const std::exception& e) {
        // Handle exception
        std::cerr << "Exception caught in safeReceive: " << e.what() << '\n';
        bytes_received = -1; // Set to -1 to indicate an error to the caller
    }
    return bytes_received;
}

/**
 * Receives and deserializes a response from the server into a Response object.
 * @return The deserialized response object.
 */
Response ProtocolHandler::getResponse() {
    Response response;

    // Receive the fixed-size part of the response
    char header_buffer[1 + 2 + 4];  // version (1 byte) + code (2 bytes) + payloadSize (4 bytes)
    ssize_t bytes_received = safeReceive(socket_, header_buffer, sizeof(header_buffer), 0);
    if (bytes_received != sizeof(header_buffer)) {
        logger_.serverError((std::ostringstream() << "Received partial headers data, expected " << sizeof(header_buffer) << "bytes, got " << bytes_received).str());
        return response;
    }

    // Deserialize the fixed-size part
    size_t offset = 0;
    response.version = char(header_buffer[offset] + '0');  // Convert to ascii value.
    offset += 1;

    response.code = ntohs(*reinterpret_cast<uint16_t*>(header_buffer + offset));
    offset += 2;

    response.payloadSize = ntohl(*reinterpret_cast<uint32_t*>(header_buffer + offset));

    // Receive the payload based on the payloadSize
    char payload_buffer[response.payloadSize];
    bytes_received = safeReceive(socket_, payload_buffer, response.payloadSize, 0);
    if (bytes_received != response.payloadSize) {
        logger_.serverError((std::ostringstream() << "Received partial payload, expected " << response.payloadSize << "bytes, got " << bytes_received).str());
        return response;
    }

    // Convert the payload buffer to a std::string
    response.payload = std::string(payload_buffer, response.payloadSize);
    return response;
}


/**
 * Sends a CRC status request to the server with the received code.
 * @param clientId The client's identifier.
 * @param code The request code.
 * @return True if the server confirms the message, false otherwise.
 */
bool ProtocolHandler::sendCRCStatusRequest(char *clientId, uint16_t code) {
    Request file_request{};
    memcpy(file_request.clientId, clientId, 16);
    file_request.version = PROTOCOL_VERSION;
    file_request.code = code;

    char* payload_buffer = new char[filePath_.length() + 1];
    std::strcpy(payload_buffer, filePath_.c_str());
    file_request.payloadSize = filePath_.length() + 1;
    file_request.payload = payload_buffer;

    // Send the request and get the response
    sendRequest(file_request);
    delete[] file_request.payload;
    Response file_response = getResponse();
    return file_response.code == ServerResponses::CONFIRM_MSG;
}

/**
 * Handles the process of sending a file with retries and CRC checks. Based on the different server response codes,
 * logging and returning a boolean value that indicates whether the operation was successful or not.
 * @param encrypted_content The request containing encrypted content to send.
 * @param maxRetries The maximum number of retries allowed.
 * @param clientId The client's identifier.
 * @return True if the file was successfully sent and verified, false otherwise.
 */
bool ProtocolHandler::handleRetrySendFile(const Request& encrypted_content, int maxRetries, char *clientId) {
    int retry_count = 0;
    bool status = false;
    while (retry_count < maxRetries) {
        sendRequest(encrypted_content);
        Response response = getResponse();
        if (response.code == ServerResponses::FILE_RECEIVED_CRC_OK) {
            // Extract last 4 bytes that represent the CRC:
            uint32_t receivedCRC = *reinterpret_cast<uint32_t*>(&response.payload[response.payload.size() - 4]);
            uint32_t calculatedCRC = readCrc(filePath_);

            if (receivedCRC == calculatedCRC) {
                logger_.info("CRC Match, Responding with CRC Correct status to server");
                if (sendCRCStatusRequest(clientId, ServerRequests::Codes::CRC_CORRECT)) {
                    logger_.info("Successfully finished Client's file sending flow");
                    status = true;
                } else {
                    logger_.serverError("Failed to finish Client's file sending flow - server didn't accept message.");
                }
                break;
            } else {
                logger_.error("CRC not matching, Responding with CRC Incorrect status to server...");
                sendCRCStatusRequest(clientId, ServerRequests::Codes::CRC_INCORRECT_RESEND);
            }
        }
        retry_count++;
    }

    // If after max_retries we didn't get a successful response, send a failure request with CRC_INCORRECT_DONE code:
    if (retry_count == maxRetries) {
        logger_.serverError("reached max retries but wasn't able to successfully upload file to server");
        sendCRCStatusRequest(clientId, ServerRequests::Codes::CRC_INCORRECT_DONE);
    }
    return status;
}

/**
 * Creates a payload buffer for file content that we can utilize when attempting to send a request to the server.
 * @param fileName The name of the file.
 * @param content The content of the file.
 * @return A pointer to the dynamically allocated payload buffer.
 */
char* createFilePayloadBuffer(const std::string& fileName, const std::string& content) {
    size_t totalPayloadSize = 4 + 255 + content.size();  // 4 for content size, 255 for file name
    char* buffer = new char[totalPayloadSize + 1];
    std::memset(buffer, 0, totalPayloadSize);

    // Convert contentSize to big-endian (network byte order)
    uint32_t contentSizeNetworkOrder = htonl(content.size());
    std::memcpy(buffer, &contentSizeNetworkOrder, 4);

    // Set file name, assuming fileName won't exceed 255 chars due to previous validations:
    std::strncpy(buffer + 4, fileName.c_str(), 255);

    // Set content
    strcpy(buffer + 4 + 255, content.data());

    return buffer;
}

/**
 * Handles a connection request with the server.
 * @param clientId The client's identifier.
 * @param requestCode The request code to send.
 * @return The response from the server.
 */
Response ProtocolHandler::handleConnectionRequest(char *clientId, uint16_t requestCode) {
    Request request {};
    std::vector<char> payload_buffer(ServerRequests::Consts::NAME_FIELD_SIZE, 0);
    // Initializing a default "empty" clientId:
    std::memset(request.clientId, 0, sizeof(request.clientId));
    // Defining the request attributes:
    request.version = PROTOCOL_VERSION;
    request.code = requestCode;
    std::strncpy(payload_buffer.data(), clientName_.c_str(), ServerRequests::Consts::NAME_FIELD_SIZE - 1);
    request.payloadSize = ServerRequests::Consts::NAME_FIELD_SIZE;
    request.payload = payload_buffer.data();

    // Performing a request:
    sendRequest(request);
    Response serverResponse = getResponse();
    // Store the received ClientId and return the response:
    memcpy(clientId, serverResponse.payload.c_str(), std::min<size_t>(16, serverResponse.payload.size()));
    return serverResponse;
}

/**
 * Handles the RSA registration process with the server.
 * @param clientId The client's identifier.
 * @param outPrivateKey The private key generated will be stored here.
 * @return The response from the server.
 */
Response ProtocolHandler::handleRSARegistration(char* clientId, std::string& outPrivateKey) {
    CryptoHandler crypto;
    FileHandler fileHandler;
    Response serverResponse;
    char* payload_buffer;

    // Generate RSA keys
    auto [publicKey, privateKey] = CryptoHandler::generate_rsa_key_pair();

    Request pubkey_request{};
    memcpy(pubkey_request.clientId, clientId, 16);
    pubkey_request.version = PROTOCOL_VERSION;
    pubkey_request.code = ServerRequests::Codes::SEND_PUBLIC_KEY;

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

    // Save the private key + me.info:
    fileHandler.savePrivateRSAKey(privateKey);
    fileHandler.saveMeInfo(clientName_, pubkey_request.clientId, privateKey);

    // Performing a request, sending the public key to the server:
    sendRequest(pubkey_request);
    delete[] payload_buffer;
    serverResponse = getResponse();

    // Save the private key to the output reference
    outPrivateKey = privateKey;
    return serverResponse;
}

/**
 * Handles the encryption and sending of a file to the server.
 * @param encrypted_aes_key The AES key received from the server.
 * @param privateKey The RSA private key for decryption.
 * @param clientId The client's identifier.
 * @return True if the file is successfully encrypted and sent, false otherwise.
 */
bool ProtocolHandler::handleFileEncryptionAndSend(const std::string& encrypted_aes_key, const std::string& privateKey, const char* clientId) {
    CryptoHandler crypto;
    FileHandler fileHandler;

    // Decrypt received AES key using the RSA private key - skip first 16 bytes of Client ID:
    std::string aes_key = CryptoHandler::decrypt_with_rsa(encrypted_aes_key, privateKey);

    // Read the file's contents
    std::string fileContents = fileHandler.readFileContents(filePath_);

    // Encrypt the file content using AES key
    std::string encrypted_content = CryptoHandler::encrypt_with_aes(fileContents, aes_key);

    // Create the payload buffer
    char* payloadBuffer = createFilePayloadBuffer(filePath_, encrypted_content);

    // Set up the request object
    Request encrypted_file_request{};
    memcpy(encrypted_file_request.clientId, clientId, 16);
    encrypted_file_request.version = PROTOCOL_VERSION;
    encrypted_file_request.code = ServerRequests::Codes::SEND_FILE;  // Sending a file request code
    encrypted_file_request.payloadSize = 4 + 255 + encrypted_content.size();  // 4 for content size, 255 for file name, rest for content
    encrypted_file_request.payload = payloadBuffer;

    // Attempting to perform the request up to 3 times:
    bool status = ProtocolHandler::handleRetrySendFile(encrypted_file_request, 3, (char *)clientId);

    // Clean up
    delete[] encrypted_file_request.payload;
    return status;
}

/**
 * Handles the registration process with the server.
 * @return True if registration is successful, false otherwise.
 */
bool ProtocolHandler::handleRegistration() {
    char clientId[16];
    logger_.info((std::ostringstream() << "Starting registration flow for client " << clientName_ << "...").str());

    // Step 1: Send registration request with an empty clientId + check if response is valid:
    logger_.info((std::ostringstream() << "Attempting to register " << clientName_ << " to server").str());
    Response serverResponse = ProtocolHandler::handleConnectionRequest(clientId, ServerRequests::Codes::REGISTRATION);
    if(serverResponse.code != ServerResponses::REGISTRATION_SUCCESS) {
        logger_.serverError("Failed to register to the server");
        return false;
    }
    logger_.info((std::ostringstream() << "successfully registered " << clientName_ << " to server").str());

    // Step 2: Handle RSA registration
    logger_.info("Attempting to generate RSA pair and send public key to server");
    std::string privateKey;
    serverResponse = handleRSARegistration(clientId, privateKey);

    if (serverResponse.code != ServerResponses::RECEIVED_PUBLIC_KEY_SEND_AES) {
        logger_.serverError("Received an invalid status from the server during RSA generation step");
        return false;  // Exit if there was an error during RSA registration
    }
    logger_.info("Successfully generated RSA pair and received valid status and AES key from server");

    // Step 3: Encrypt file using AES key and send to the server
    logger_.info("Attempting to encrypt file using AES key and send to server");
    std::string encrypted_aes_key = serverResponse.payload.substr(16);
    return handleFileEncryptionAndSend(encrypted_aes_key, privateKey, clientId);
}

/**
 * Handles the reconnection process with the server. If the client doesn't exist yet, this method will fallback into
 * registration.
 * @return True if reconnection is successful, false otherwise.
 */
bool ProtocolHandler::handleReconnection() {
    char clientId[16];
    logger_.info((std::ostringstream() << "Starting reconnection flow for client " << clientName_ << "...").str());

    // Step 1: Send registration request with an empty clientId + check if response is valid:
    Response serverResponse = ProtocolHandler::handleConnectionRequest(clientId, ServerRequests::Codes::RECONNECT);
    if (serverResponse.code == ServerResponses::RECONNECT_REJECTED) {
        // Try and to perform a registration request.
        return ProtocolHandler::handleRegistration();
    }
    else if(serverResponse.code != ServerResponses::APPROVE_RECONNECT_SEND_AES) {
        logger_.serverError((std::ostringstream() << "failed to reconnect to the server - " << serverResponse.payload.c_str()).str());
        return false;
    }
    // Step 2: On successful reconnection, handle RSA registration
    std::string privateKey;
    serverResponse = handleRSARegistration(clientId, privateKey);

    if (serverResponse.code != ServerResponses::RECEIVED_PUBLIC_KEY_SEND_AES) {
        logger_.serverError("Received an invalid status from the server during RSA generation step");
        return false;  // Exit if there was an error during RSA registration
    }
    logger_.info("Successfully generated RSA pair and received valid status and AES key from server");

    // Step 3: Encrypt file using AES key and send to the server
    logger_.info("Attempting to encrypt file using AES key and send to server");
    std::string encrypted_aes_key = serverResponse.payload.substr(16);
    return handleFileEncryptionAndSend(encrypted_aes_key, privateKey, clientId);
}
