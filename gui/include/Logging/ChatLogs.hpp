/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ChatLogs.hpp
*/
#ifndef ZAPPY_CHATLOGS_HPP
#define ZAPPY_CHATLOGS_HPP

#include <string>
#include <vector>

namespace zappy {

struct LogMessage {
    std::string Log;
    std::string Type;
};

class ChatLogs {
  public:
    static constexpr size_t MAX_LOGS = 100;

    ChatLogs() = default;
    ~ChatLogs() = default;

    void addChatLog(const std::string& message, const std::string& type) {
        if (_logMessages.size() >= MAX_LOGS) {
            _logMessages.erase(_logMessages.begin());
        }

        _logMessages.push_back({message, type});
    }

    void clearChatLogs() { _logMessages.clear(); }

    const std::vector<LogMessage>& getLogs() const { return _logMessages; }

  private:
    std::vector<LogMessage> _logMessages;
};

} // namespace zappy

#endif // ZAPPY_CHATLOGS_HPP