#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

enum LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger
{
public:
    Logger()
    {
    }

    ~Logger()
    {
    }

    void log(LogLevel level, const std::string &message)
    {
        time_t now = time(0);
        tm *timeinfo = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp),
                 "%Y-%m-%d %H:%M:%S", timeinfo);

        std::ostringstream logEntry;
        logEntry << "[" << timestamp << "] "
                 << levelToString(level) << ": " << message
                 << std::endl;

        std::cout << logEntry.str();
    }

private:
    std::string levelToString(LogLevel level)
    {
        switch (level)
        {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        case CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
        }
    }
};

#endif