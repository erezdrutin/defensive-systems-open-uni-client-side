/**
 * Author: Erez Drutin
 * Date: 04.11.2023
 * Purpose: Handle all logging related operations in the client-side code.
 */
#include "Logger.h"
#include <iomanip>
#include <utility>

/**
 * Gets the current system time formatted as a string.
 * @return A string representing the current time in "YYYY-MM-DD HH:MM:SS" format.
 */
std::string Logger::getCurrentTime() {
    auto now = std::time(nullptr);
    std::tm tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/**
 * Converts a logging level enum to its corresponding string representation.
 * @param level The logging level to convert.
 * @return A string representing the logging level.
 */
std::string Logger::levelToString(Level level) {
    switch(level) {
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
    }
}

/**
 * Logs a message to the appropriate output stream based on the log level.
 * @param logLevel The severity level of the log message.
 * @param message The message to log.
 */
void Logger::log(Level logLevel, const std::string& message) const {
    // Use std::cerr for ERROR level, std::cout for others
    auto& stream = logLevel == Level::ERROR ? std::cerr : std::cout;
    stream << getCurrentTime() << " - " << name << " - "
           << levelToString(logLevel) << " - " << message << std::endl;
}

Logger::Logger(std::string name, Level level)
        : name(std::move(name)), level(level) {}

/**
* Logs an informational message.
* @param message The message to log at the INFO level.
*/
void Logger::info(const std::string& message) const {
    log(Level::INFO, message);
}

/**
 * Logs a warning message.
 * @param message The message to log at the WARNING level.
 */
void Logger::warning(const std::string& message) const {
    log(Level::WARNING, message);
}

/**
 * Logs an error message.
 * @param message The message to log at the ERROR level.
 */
void Logger::error(const std::string& message) const {
    log(Level::ERROR, message);
}

/**
 * Logs an error message received from a server response.
 * @param message The server error message to log.
 */
void Logger::serverError(const std::string& message) const {
    std::string errMessage = "server responded with error: " + message;
    log(Level::ERROR, errMessage);
}