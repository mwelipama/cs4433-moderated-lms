#pragma once
#include "domain/Errors.hpp"
#include <string>

namespace lms {

// Whitespace set matches the database CHECK on audit reasons, so application-level and
// storage-level "non-empty" agree exactly.
inline std::string trimWhitespace(const std::string& s) {
    const char* ws = " \t\n\r\v\f";
    const auto begin = s.find_first_not_of(ws);
    if (begin == std::string::npos) return "";
    const auto end = s.find_last_not_of(ws);
    return s.substr(begin, end - begin + 1);
}

inline bool isBlank(const std::string& s) { return trimWhitespace(s).empty(); }

inline void requireNonBlank(const std::string& value, const std::string& field) {
    if (isBlank(value)) throw ValidationError(field + " must not be empty");
}

inline void requireRange(int value, int lo, int hi, const std::string& field) {
    if (value < lo || value > hi)
        throw ValidationError(field + " must be between " + std::to_string(lo) +
                              " and " + std::to_string(hi));
}

} // namespace lms
