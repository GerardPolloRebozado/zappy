/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ChatLogs.hpp
*/
#ifndef ZAPPY_CHATLOGS_HPP
#define ZAPPY_CHATLOGS_HPP

#include <deque>
#include <string>

namespace zappy {

/**
 * @struct LogMessage
 * @brief Structure representing a single message entry in the game chat.
 */
struct LogMessage {
    std::string Log;
    std::string Type;
    std::string Team;
};

/**
 * @class ChatLogs
 * @brief Manages the history of game events and chat messages.
 * * This class acts as a buffer, maintaining a maximum of 100
 * messages.
 */
class ChatLogs {
  public:
    static constexpr size_t MAX_LOGS = 100;

    ChatLogs() = default;
    ~ChatLogs() = default;

    /**
     * @brief Adds a new message to the log history.
     * * If the log limit (MAX_LOGS) is reached, the oldest message is removed
     * before adding the new one.
     * * @param message The text content to store.
     * @param type The category of the event for filtering/coloring.
     * @param team (Optional) The name of the team if the event is related to a specific one.
     */
    void addChatLog(const std::string& message, const std::string& type,
                    const std::string& team = "") {
        if (_logMessages.size() >= MAX_LOGS) {
            _logMessages.pop_front();
        }

        _logMessages.push_back({message, type, team});
    }

    void clearChatLogs() { _logMessages.clear(); }

    /**
     * @brief Retrieves the current list of logged messages.
     * @return A constant reference to the vector of LogMessage objects.
     */
    const std::deque<LogMessage>& getLogs() const { return _logMessages; }

  private:
    std::deque<LogMessage> _logMessages;
};

} // namespace zappy

#endif // ZAPPY_CHATLOGS_HPP