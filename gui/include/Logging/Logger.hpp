/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** Logger.hpp
*/

#ifndef ZAPPY_LOGGER_HPP
#define ZAPPY_LOGGER_HPP

#include "Logging/LogConfig.hpp"
#include <iostream>
#include <string>

namespace zappy {

enum class log_level_e {
    Error,
    Debug,
    Info
};

inline void logPrint(log_level_e level, const std::string& msg)
{
    switch (level) {
        case log_level_e::Error:
            std::cerr << "[ERROR] " << msg << std::endl;
            break;
        case log_level_e::Debug:
            std::cout << "[DEBUG] " << msg << std::endl;
            break;
        case log_level_e::Info:
            std::cout << "[INFO] " << msg << std::endl;
            break;
    }
}

} // namespace zappy

#if ZAPPY_LOG_LEVEL >= ERROR
#define ZAPPY_LOG_E(msg) zappy::logPrint(zappy::log_level_e::Error, (msg))
#else
#define ZAPPY_LOG_E(msg) ((void)0)
#endif

#if ZAPPY_LOG_LEVEL >= DEBUG
#define ZAPPY_LOG_D(msg) zappy::logPrint(zappy::log_level_e::Debug, (msg))
#else
#define ZAPPY_LOG_D(msg) ((void)0)
#endif

#if ZAPPY_LOG_LEVEL >= INFO
#define ZAPPY_LOG_I(msg) zappy::logPrint(zappy::log_level_e::Info, (msg))
#else
#define ZAPPY_LOG_I(msg) ((void)0)
#endif

#endif // ZAPPY_LOGGER_HPP
