#pragma once
#include "infra/DateTime.hpp"
#include <optional>
#include <string>

namespace lms {

// The effective deadline for a student is the most generous (latest) of the base deadline,
// a class-wide extension (if any) and the student's individual extension (if any). Because
// an extension only ever moves a deadline later, taking the maximum preserves the original
// rule (an individual extension overrides the base deadline) and lets a class extension and
// an individual extension coexist without ever making a student who has one but not the
// other worse off. Submission acceptance and the late penalty both use this one function, so
// they can never diverge.
inline std::string effectiveDeadline(const std::string& base,
                                     const std::optional<std::string>& classExtension,
                                     const std::optional<std::string>& individualExtension) {
    std::string bestIso = base;
    int64_t best = DateTime::toEpoch(base);
    auto consider = [&](const std::optional<std::string>& candidate) {
        if (candidate) {
            const int64_t e = DateTime::toEpoch(*candidate);
            if (e > best) { best = e; bestIso = *candidate; }
        }
    };
    consider(classExtension);
    consider(individualExtension);
    return bestIso;
}

} // namespace lms
