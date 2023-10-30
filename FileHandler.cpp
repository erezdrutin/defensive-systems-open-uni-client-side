#include "FileHandler.h"
#include "constants.h"

template <typename FileStream>
FileStream FileHandler::openFile(const std::string &path, std::ios_base::openmode mode) {
    FileStream file(path, mode);
    if (!file) {
        throw std::runtime_error("Unable to open " + path);
    }
    return file;
}

MeInfo FileHandler::readMeInfo() {
    const std::string path = "/Users/erez/Desktop/defensive_prog_lab/c++/defensive_maman_15/me.info";
    std::ifstream file = openFile<std::ifstream>(path, std::ios::in);

    MeInfo info;
    std::getline(file, info.name);
    std::getline(file, info.uuid);
    std::getline(file, info.base64Key);
    file.close();  // Close the file after reading
    return info;
}


TransferInfo FileHandler::readTransferInfo() {
    const std::string path = "/Users/erez/Desktop/defensive_prog_lab/c++/defensive_maman_15/transfer.info";
    std::ifstream file = openFile<std::ifstream>(path, std::ios::in);

    TransferInfo info;
    std::string ip_port;
    std::getline(file, ip_port);
    std::getline(file, info.name);
    std::getline(file, info.filePath);
    file.close();  // Close the file after reading

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

void FileHandler::writeToFile(const std::string &filename, const std::string &content) {
    std::ofstream file = openFile<std::ofstream>(filename, std::ios::out);
    file << content;
    file.close();
}


void FileHandler::savePrivateRSAKey(const std::string &privateKey) {
    std::string filePath = std::string(CLIENTS_BASE_PATH) + "/id_rsa";
    writeToFile(filePath, privateKey);
}

std::string FileHandler::readFileContents(const std::string& path) {
    std::ifstream fileStream = openFile<std::ifstream>(path, std::ios::in | std::ios::binary);
    std::ostringstream contents;
    contents << fileStream.rdbuf();
    fileStream.close();
    return contents.str();
}