/**
 * Author: Erez Drutin
 * Date: 04.11.2023
 * Purpose: Serve as a header file for FileHandler.cpp
 */
#ifndef DEFENSIVE_MAMAN_15_FILEHANDLER_H
#define DEFENSIVE_MAMAN_15_FILEHANDLER_H

#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

struct MeInfo {
    std::string name;
    std::string uuid;
    std::string base64Key;
};

struct TransferInfo {
    std::string ipAddress;
    int port;
    std::string name;
    std::string filePath;
};

class FileHandler {
public:
    MeInfo readMeInfo();
    TransferInfo readTransferInfo();
    void writeToFile(const std::string& filename, const std::string& content);
    void saveMeInfo(const std::string& clientName, const char* clientId, const std::string& privateKey);
    void savePrivateRSAKey(const std::string &privateKey);
    std::string readFileContents(const std::string& path);
private:
    template <typename FileStream>
    FileStream openFile(const std::string &path, std::ios_base::openmode mode);
};


#endif
