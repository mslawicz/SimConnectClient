#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <functional>

#define ONLY_ONCE   true

enum class LogLevel
{
    None,
    Always,
    Error,
    Warning,
    Info,
    Debug,
    NoOfLevels
};


class Console
{
public:
    Console(Console const&) = delete;
    Console& operator=(Console const&) = delete;
    static Console& getInstance();
    void log(LogLevel level, std::string message, bool dontRepeat = false);
    void handler(void);
    bool isQuitRequest(void) const { return quitRequest; }
    void registerCommand(std::string command, std::string description, std::function<void(void)> action);
    void quit(void) { quitRequest = true; }
    void help(void);
private:
    Console();
    LogLevel currentLevel{ LogLevel::Debug };
    const std::unordered_map<LogLevel, std::string> levelText
    {
        {LogLevel::None, ""},
        {LogLevel::Always, ":"},
        {LogLevel::Error, "error"},
        {LogLevel::Warning, "warning"},
        {LogLevel::Info, "info"},
        {LogLevel::Debug, "debug"}
    };
    bool quitRequest{ false };
    std::map < std::string, std::pair<std::string, std::function<void(void)>>> commands;
};
