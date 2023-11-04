/**
 * Author: Erez Drutin
 * Date: 04.11.2023
 * Purpose: Serve as a header file for Logger.cpp
 */
#ifndef DEFENSIVE_MAMAN_15_LOGGER_H
#define DEFENSIVE_MAMAN_15_LOGGER_H

#include <iostream>
#include <string>
#include <ctime>
#include <sstream>

class Logger {
public:
    enum class Level {
        INFO,
        WARNING,
        ERROR
    };

private:
    std::string name;
    Level level;

    static std::string getCurrentTime() ;
    static std::string levelToString(Level level) ;
    void log(Level logLevel, const std::string& message) const;

public:
    explicit Logger(std::string  name, Level level = Level::INFO);
    void info(const std::string& message) const;
    void warning(const std::string& message) const;
    void error(const std::string& message) const;
    void serverError(const std::string& message) const;
};

#endif
