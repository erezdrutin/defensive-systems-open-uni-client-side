//
// Created by Erez on 27/10/2023.
//

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
    void savePrivateRSAKey(const std::string &privateKey);
    std::string readFileContents(const std::string& path);
private:
    template <typename FileStream>
    FileStream openFile(const std::string &path, std::ios_base::openmode mode);
};



#endif //DEFENSIVE_MAMAN_15_FILEHANDLER_H
