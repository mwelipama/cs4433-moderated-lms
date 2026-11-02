#include "ui/Console.hpp"

#include <iostream>
#include <limits>
#include <string>

#if defined(_WIN32)
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace lms {
namespace console {

void line() { std::cout << "------------------------------------------------------------\n"; }

void title(const std::string& text) {
    std::cout << "\n";
    line();
    std::cout << "  " << text << "\n";
    line();
}

void info(const std::string& text) { std::cout << text << "\n"; }
void error(const std::string& text) { std::cout << "! " << text << "\n"; }

void pause() {
    std::cout << "\n(press Enter to continue) ";
    std::string dummy;
    std::getline(std::cin, dummy);
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

std::string readNonEmpty(const std::string& prompt) {
    for (;;) {
        std::string value = readLine(prompt);
        if (!value.empty()) return value;
        error("a value is required.");
    }
}

std::string readPassword(const std::string& prompt) {
    std::cout << prompt;
    std::string password;
#if defined(_WIN32)
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b') {
            if (!password.empty()) { password.pop_back(); std::cout << "\b \b"; }
        } else if (ch == 3) {
            break;  // Ctrl+C
        } else {
            password.push_back(static_cast<char>(ch));
            std::cout << '*';
        }
    }
    std::cout << "\n";
#else
    termios oldAttr{};
    tcgetattr(STDIN_FILENO, &oldAttr);
    termios noEcho = oldAttr;
    noEcho.c_lflag &= ~static_cast<tcflag_t>(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &noEcho);
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
    std::cout << "\n";
#endif
    return password;
}

int readInt(const std::string& prompt) {
    for (;;) {
        const std::string value = readLine(prompt);
        try {
            size_t pos = 0;
            const int parsed = std::stoi(value, &pos);
            if (pos == value.size()) return parsed;
        } catch (const std::exception&) {
        }
        error("please enter a whole number.");
    }
}

int64_t readId(const std::string& prompt) {
    for (;;) {
        const std::string value = readLine(prompt);
        try {
            size_t pos = 0;
            const long long parsed = std::stoll(value, &pos);
            if (pos == value.size() && parsed > 0) return static_cast<int64_t>(parsed);
        } catch (const std::exception&) {
        }
        error("please enter a valid positive id.");
    }
}

bool readYesNo(const std::string& prompt) {
    for (;;) {
        const std::string value = readLine(prompt + " (y/n): ");
        if (value == "y" || value == "Y") return true;
        if (value == "n" || value == "N") return false;
        error("please answer y or n.");
    }
}

int readMenuChoice(int minChoice, int maxChoice) {
    for (;;) {
        const int choice = readInt("Choose an option: ");
        if (choice >= minChoice && choice <= maxChoice) return choice;
        error("please choose a number between " + std::to_string(minChoice) + " and " +
              std::to_string(maxChoice) + ".");
    }
}

} // namespace console
} // namespace lms
