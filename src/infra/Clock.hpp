#pragma once
#include "infra/DateTime.hpp"
#include <string>
#include <cstdint>

namespace lms {

// Abstraction over "now" so that time-dependent logic (deadlines, lateness) is
// deterministic under test. Production uses SystemClock; tests inject a fixed clock.
class Clock {
public:
    virtual ~Clock() = default;
    virtual std::string nowIso() const = 0;
    virtual int64_t nowEpoch() const = 0;
};

class SystemClock : public Clock {
public:
    std::string nowIso() const override { return DateTime::nowUtc(); }
    int64_t nowEpoch() const override { return DateTime::toEpoch(DateTime::nowUtc()); }
};

} // namespace lms
