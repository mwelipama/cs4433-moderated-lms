#pragma once
#include "domain/PenaltyPolicy.hpp"

namespace lms {

// The agreed rule: A = clamp(R - 0.10 * M * d, 0, M), rounded to the nearest whole mark
// (half up). With d = 0 the awarded mark equals the raw mark.
class StandardPenaltyPolicy : public PenaltyPolicy {
public:
    int awardedMark(int rawMark, int maxMark, int daysLate) const override;
};

} // namespace lms
