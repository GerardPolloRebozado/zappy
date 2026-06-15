/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** Logger.hpp
*/

/**
 * @file Logger.hpp
 * @brief Compile-time filtered logging macros for the GUI.
 *
 * Provides @ref log_error, @ref log_debug and @ref log_info macros that print
 * tagged messages to the terminal. Macros below the configured @c ZAPPY_LOG_LEVEL
 * are stripped at compile time and produce no code.
 */

#ifndef ZAPPY_LOGGER_HPP
#define ZAPPY_LOGGER_HPP

#include "Logging/LogConfig.hpp"
#include <iostream>
#include <string>

namespace zappy {

inline void log_error(const std::string& msg) { std::cerr << "[ERROR] " << msg << std::endl; }

inline void log_debug(const std::string& msg) { std::cout << "[DEBUG] " << msg << std::endl; }

inline void log_info(const std::string& msg) { std::cout << "[INFO] " << msg << std::endl; }

} // namespace zappy

#if ZAPPY_LOG_LEVEL >= ERROR
#define log_error(msg) zappy::log_error((msg))
#else
#define log_error(msg) ((void)0)
#endif

#if ZAPPY_LOG_LEVEL >= DEBUG
#define log_debug(msg) zappy::log_debug((msg))
#else
#define log_debug(msg) ((void)0)
#endif

#if ZAPPY_LOG_LEVEL >= INFO
#define log_info(msg) zappy::log_info((msg))
#else
#define log_info(msg) ((void)0)
#endif

#endif // ZAPPY_LOGGER_HPP
