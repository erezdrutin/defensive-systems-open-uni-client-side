/**
 * Author: Erez Drutin
 * Date: 04.11.2023
 * Purpose: Handle all file related operations in the client-side code.
 */
#include "FileHandler.h"
#include "constants.h"
#include "Base64Wrapper.h"

/**
 * Opens a file stream for a given path and mode.
 * @tparam FileStream The type of the file stream (e.g., ifstream, ofstream).
 * @param path The path to the file.
 * @param mode The opening mode (e.g., ios::in, ios::out).
 * @throws std::runtime_error If the file cannot be opened.
 * @return The opened file stream.
 */
template <typename FileStream>
FileStream FileHandler::openFile(const std::string &path, std::ios_base::openmode mode) {
    FileStream file(path, mode);
    if (!file) {
        throw std::runtime_error("Unable to open " + path);
    }
    return file;
}

/**
 * Reads and returns information about the client from a predefined file.
 * @return A MeInfo structure containing the client's name, UUID, and base64-encoded key.
 */
MeInfo FileHandler::readMeInfo() {
    std::string path = std::string(CLIENTS_BASE_PATH) + std::string(ME_INFO_FILE_NAME);
    auto file = openFile<std::ifstream>(path, std::ios::in);

    MeInfo info;
    std::getline(file, info.name);
    std::getline(file, info.uuid);
    std::getline(file, info.base64Key);
    file.close();  // Close the file after reading
    return info;
}

/**
 * Reads and returns transfer information from a predefined file.
 * @throws std::runtime_error If the name is too long or if the format for IP and port is invalid.
 * @return A TransferInfo structure containing the IP address, port, name, and file path for transfer.
 */
TransferInfo FileHandler::readTransferInfo() {
    std::string path = std::string(CLIENTS_BASE_PATH) + std::string(TRANSFER_INFO_FILE_NAME);
    auto file = openFile<std::ifstream>(path, std::ios::in);

    TransferInfo info;
    std::string ip_port;
    std::getline(file, ip_port);
    std::getline(file, info.name);
    std::getline(file, info.filePath);
    file.close();  // Close the file after reading

    if (info.name.length() > 254) {
        throw std::runtime_error("Invalid name, name length must be <= 254 chars.");
    }

    // Split the ip_port string to extract IP and port
    size_t pos = ip_port.find(':');
    if (pos != std::string::npos) {
        info.ipAddress = ip_port.substr(0, pos);
        info.port = std::stoi(ip_port.substr(pos + 1));
    } else {
        throw std::runtime_error("Invalid format for IP and port in " + path);
    }

    return info;
}

/**
 * Writes content to a file with the specified filename.
 * @param filename The name of the file to write to.
 * @param content The content to write to the file.
 */
void FileHandler::writeToFile(const std::string &filename, const std::string &content) {
    auto file = openFile<std::ofstream>(filename, std::ios::out);
    file << content;
    file.close();
}

/**
 * Saves the client's information to a predefined file, utilizing the custom format specified in Maman15:
 * Row 1: Consists of Client Name.
 * Row 2: Consists of Client ID.
 * Row 3: Consists of RSA encrypted key in Base64.
 * @param clientName The client's name.
 * @param clientId The client's unique identifier.
 * @param privateKey The client's private key.
 */
void FileHandler::saveMeInfo(const std::string& clientName, const char* clientId, const std::string& privateKey) {
    std::string path = std::string(CLIENTS_BASE_PATH) + std::string(ME_INFO_FILE_NAME);
    auto file = openFile<std::ofstream>(path, std::ios::out | std::ios::trunc);

    // Write clientName
    file << clientName << '\n';

    // Write ClientID in ASCII representation
    for (int i = 0; i < 16; ++i) {
        file << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(clientId[i]) & 0xFF);
    }
    file << '\n';

    // Write private key in base64 format
    std::string base64PrivateKey = Base64Wrapper::encode(privateKey);
    // Remove newline characters from the base64 encoded string
    base64PrivateKey.erase(std::remove(base64PrivateKey.begin(), base64PrivateKey.end(), '\n'), base64PrivateKey.end());
    file << base64PrivateKey;

    file.close();
}

/**
 * Saves the private RSA key to a predefined private key file path.
 * @param privateKey The RSA private key to save.
 */
void FileHandler::savePrivateRSAKey(const std::string &privateKey) {
    std::string filePath = std::string(CLIENTS_BASE_PATH) + std::string(PRIVATE_KEY_FILE);
    writeToFile(filePath, privateKey);
}

/**
 * Reads the entire contents of a file into a string and returns it.
 * @param path The path to the file to read.
 * @return A string containing the contents of the file.
 */
std::string FileHandler::readFileContents(const std::string& path) {
    auto fileStream = openFile<std::ifstream>(path, std::ios::in | std::ios::binary);
    std::ostringstream contents;
    contents << fileStream.rdbuf();
    fileStream.close();
    return contents.str();
}