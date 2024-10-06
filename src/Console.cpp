#include "Console.h"
#include <conio.h>
#include <string>
#include <iostream>
#include <sstream>


Console& Console::getInstance()
{
    static Console instance;
    return instance;
}

Console::Console()
{
    registerCommand("quit", "quit the program", std::bind(&Console::quit, this));
    registerCommand("help", "display console commands", std::bind(&Console::help, this));
}

// console handler
void Console::handler(void)
{
    while (!quitRequest)
    {
        std::cout << "\n>" << std::flush;

        std::string command;
        std::cin >> command;

        if (commands.find(command) != commands.end())
        {
            // command exists - execute it
            commands[command].second();
        }
        else
        {
            //command not found
            std::cout << "\ninvalid command: " << command.c_str();
        }

        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');
    }
}


// log message in console window
void Console::log(LogLevel level, std::string message, bool dontRepeat)
{
    struct CheckTxt
    {
        uint8_t checkSum;
        uint8_t checkXor;
    };

    static CheckTxt prevCheck{0, 0};
    if(dontRepeat)
    {
        CheckTxt currentCheck{0, 0};
        for(auto ch : message)
        {
            currentCheck.checkSum += static_cast<uint8_t>(ch);
            currentCheck.checkXor ^= static_cast<uint8_t>(ch);
        }
        if((currentCheck.checkSum != prevCheck.checkSum) || (currentCheck.checkXor != prevCheck.checkXor))
        {
            //this is a new message
            prevCheck.checkSum = currentCheck.checkSum;
            prevCheck.checkXor = currentCheck.checkXor;
        }
        else
        {
            //this message is repeated
            return;
        }
    }
    else
    {
        //any message that can be repeated
        prevCheck = CheckTxt{0, 0};
    }

    int iCurrentLevel = static_cast<int>(currentLevel);
    int iLevel = static_cast<int>(level);
    if ((iLevel <= iCurrentLevel) && (iCurrentLevel > 0))
    {
        std::cout << "\r" << levelText.find(level)->second.c_str() << ": " << message.c_str() << std::endl;
    }
}

//register console command
void Console::registerCommand(std::string command, std::string description, std::function<void(void)> action)
{
    commands[command] = std::pair < std::string, std::function<void(void)>>(description, action);
}

// display the list of console commands
void Console::help(void)
{
    for (auto const& [command, resource] : commands)
    {
        std::cout << command.c_str() << " - " << resource.first.c_str() << std::endl;
    }
}
