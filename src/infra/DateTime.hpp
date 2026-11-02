#pragma once
#include <string>
#include <cstdint>

namespace lms {

// ISO-8601 UTC datetime helpers ("YYYY-MM-DDTHH:MM:SS"). The schema stores datetimes
// as TEXT in this form; all ordering and lateness arithmetic is done on epoch seconds
// so it is free of locale and timezone effects and is identical on every platform.
class DateTime {
public:
    static std::string nowUtc();                         // current time, ISO-8601 UTC
    static int64_t toEpoch(const std::string& iso);      // parse; throws std::invalid_argument
    static std::string fromEpoch(int64_t epochSeconds);  // format back to ISO-8601 UTC
    static bool isValid(const std::string& iso);         // true if parseable and in range

    // Days late = ceil((submitted - effectiveDeadline) / 24h), never negative.
    // A submission at or before the effective deadline is 0 days late.
    static int daysLate(int64_t submittedEpoch, int64_t effectiveDeadlineEpoch);
};

} // namespace lms
