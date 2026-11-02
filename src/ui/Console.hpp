#pragma once
#include <cstdint>
#include <string>

namespace lms {

// Thin console input/output helpers used by the menus: reliable line reading, validated
// integer entry, and non-echoing password entry (so passwords never appear on screen).
namespace console {

void line();                                   // print a divider rule
void title(const std::string& text);           // print a boxed section title
void info(const std::string& text);            // normal message
void error(const std::string& text);           // error message, prefixed
void pause();                                   // "press Enter to continue"

std::string readLine(const std::string& prompt);
std::string readNonEmpty(const std::string& prompt);
std::string readPassword(const std::string& prompt);  // input is not echoed
int readInt(const std::string& prompt);
int64_t readId(const std::string& prompt);
bool readYesNo(const std::string& prompt);
int readMenuChoice(int minChoice, int maxChoice);

} // namespace console
} // namespace lms
