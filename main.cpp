#include "ProtocolHandler.h"
#include "CryptoHandler.h"
#include "FileHandler.h"
#include <iostream>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "AESWrapper.h"
#include "CryptoHandler.h"


int main() {
//    CryptoHandler crypto;
//
//    std::string key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
//    std::string message = "Hello, World!";
//
//    std::string encrypted_message = CryptoHandler::encrypt_with_aes(message, key);
//    std::cout << "Encrypted Message: " << encrypted_message << std::endl;

    try {
        // Initialize the FileHandler
        FileHandler fileHandler;
        TransferInfo transferInfo = fileHandler.readTransferInfo();
//        try {
//            MeInfo meInfo = fileHandler.readMeInfo();
//            ProtocolHandler protocolHandler(transferInfo.ipAddress, transferInfo.port, meInfo.name);
//            if (protocolHandler.handleConnection())
//                protocolHandler.handleReconnection();
//        } catch (std::runtime_error &err) {
//            ProtocolHandler protocolHandler(transferInfo.ipAddress, transferInfo.port, transferInfo.name);
//            if (protocolHandler.handleConnection())
//                protocolHandler.handleRegistration();
//        }


        ProtocolHandler protocolHandler(transferInfo.ipAddress, transferInfo.port, transferInfo.name,
                                        "/Users/erez/Desktop/defensive_prog_lab/c++/defensive_maman_15/cool.txt");
        if (protocolHandler.handleConnection())
            protocolHandler.handleRegistration();

        // Read the me.info and transfer.info

        // Initialize the ProtocolHandler with the server details

        // Connect to the server

        // Here, we should decide whether to handle registration or reconnection.
        // This decision can be based on some criteria, such as whether the client has previously registered.
        // For this demonstration, we'll just use the registration flow:

        // If reconnection is needed:
        // protocolHandler.handleReconnection();

        std::cout << "Operation completed successfully!" << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
