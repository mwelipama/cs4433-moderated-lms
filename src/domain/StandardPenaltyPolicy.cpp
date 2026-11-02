#include "domain/StandardPenaltyPolicy.hpp"
#include <cmath>

namespace lms {

int StandardPenaltyPolicy::awardedMark(int rawMark, int maxMark, int daysLate) const {
    double awarded = static_cast<double>(rawMark);
    if (daysLate > 0) {
        const double penalty = 0.10 * static_cast<double>(maxMark) * static_cast<double>(daysLate);
        awarded = static_cast<double>(rawMark) - penalty;
    }
    long rounded = static_cast<long>(std::floor(awarded + 0.5)); // round half up
    if (rounded < 0) rounded = 0;
    if (rounded > maxMark) rounded = maxMark;
    return static_cast<int>(rounded);
}

} // namespace lms
