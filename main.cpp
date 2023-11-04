#include "ProtocolHandler.h"
#include "FileHandler.h"
#include <iostream>
#include <vector>
#include "Logger.h"


/**
 * Attempts to create a client and handle either reconnection or registration processes based on the presence of
 * existing client information. If the MeInfo file exists, we will try to reconnect. If not, or if it exists but the
 * server isn't familiar with the user, we will try and register the user.
 * @param logger Reference to the Logger instance for logging.
 * @return True if the client operation was successful, false otherwise.
 */
bool handleClient(Logger& logger) {
    FileHandler fileHandler;
    TransferInfo transferInfo = fileHandler.readTransferInfo();

    try {
        MeInfo meInfo = fileHandler.readMeInfo();
        ProtocolHandler protocolHandler(transferInfo.ipAddress, transferInfo.port, meInfo.name, transferInfo.filePath);
        if (protocolHandler.handleConnection()) {
            return protocolHandler.handleReconnection();
        }
    } catch (std::runtime_error &err) {
        // If reading MeInfo fails, assume new registration is needed.
        ProtocolHandler protocolHandler(transferInfo.ipAddress, transferInfo.port, transferInfo.name, transferInfo.filePath);
        if (protocolHandler.handleConnection()) {
            return protocolHandler.handleRegistration();
        }
    }
    // Return false if neither reconnection nor registration succeeds:
    return false;
}

int main() {
    Logger logger("Main");

    try {
        bool status = handleClient(logger);
        if (status) {
            logger.info("Successfully finished client operation. Shutting down...");
        } else {
            logger.error("Failed while attempting to handle client operation. Shutting down...");
        }
    } catch (const std::exception& ex) {
        logger.error("Failed while attempting to handle client operation: " + std::string(ex.what()));
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}