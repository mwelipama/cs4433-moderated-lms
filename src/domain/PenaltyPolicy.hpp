#pragma once

namespace lms {

// Strategy for the late-penalty rule. Marking depends only on this interface, so a
// different penalty rule is a new implementation rather than a change to the marking
// workflow (Open/Closed).
class PenaltyPolicy {
public:
    virtual ~PenaltyPolicy() = default;
    // Awarded mark after applying the penalty for `daysLate` whole days late.
    virtual int awardedMark(int rawMark, int maxMark, int daysLate) const = 0;
};

} // namespace lms
