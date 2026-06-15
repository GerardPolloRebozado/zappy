/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** LogConfig.hpp
*/

/**
 * @file LogConfig.hpp
 * @brief Compile-time log level constants for the GUI logger.
 *
 * Log verbosity is controlled at build time via the CMake variable
 * @c ZAPPY_LOG_LEVEL. See the GUI README for usage.
 */

#ifndef ZAPPY_LOG_CONFIG_HPP
#define ZAPPY_LOG_CONFIG_HPP

/** @brief Minimum level: only @ref log_error messages are emitted. */
#define ERROR 1
/** @brief Includes @ref log_error and @ref log_debug messages. */
#define DEBUG 2
/** @brief Includes all log macros (@ref log_error, @ref log_debug, @ref log_info). */
#define INFO 3

#ifndef ZAPPY_LOG_LEVEL
#define ZAPPY_LOG_LEVEL INFO
#endif

#endif // ZAPPY_LOG_CONFIG_HPP
