#include "infra/DateTime.hpp"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <stdexcept>

namespace lms {

// Days since 1970-01-01 for a civil date (Howard Hinnant's algorithm). Portable and
// timezone-free, which is why we do not rely on timegm/_mkgmtime.
static int64_t daysFromCivil(int y, unsigned m, unsigned d) {
    y -= (m <= 2);
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153u * (m + (m > 2 ? -3u : 9u)) + 2u) / 5u + d - 1u;
    const unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
    return static_cast<int64_t>(era) * 146097 + static_cast<int>(doe) - 719468;
}

static void civilFromDays(int64_t z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460u + doe / 36524u - doe / 146096u) / 365u;
    const int yy = static_cast<int>(yoe) + static_cast<int>(era) * 400;
    const unsigned doy = doe - (365u * yoe + yoe / 4u - yoe / 100u);
    const unsigned mp = (5u * doy + 2u) / 153u;
    d = doy - (153u * mp + 2u) / 5u + 1u;
    m = mp + (mp < 10u ? 3u : -9u);
    y = yy + (m <= 2u);
}

std::string DateTime::nowUtc() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    return fromEpoch(static_cast<int64_t>(t));
}

static bool validCivilDate(int y, int m, int d) {
    if (m < 1 || m > 12 || d < 1) return false;
    static const int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int dim = daysInMonth[m - 1];
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) dim = 29;
    return d <= dim;
}

int64_t DateTime::toEpoch(const std::string& iso) {
    int Y = 0, Mo = 0, D = 0, h = 0, mi = 0, s = 0;
    const int n = std::sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%d", &Y, &Mo, &D, &h, &mi, &s);
    if (n < 3)
        throw std::invalid_argument("invalid ISO-8601 datetime: '" + iso + "'");
    if (h < 0 || h > 23 || mi < 0 || mi > 59 || s < 0 || s > 60 || !validCivilDate(Y, Mo, D))
        throw std::invalid_argument("datetime field out of range: '" + iso + "'");
    const int64_t days = daysFromCivil(Y, static_cast<unsigned>(Mo), static_cast<unsigned>(D));
    return days * 86400 + h * 3600 + mi * 60 + s;
}

std::string DateTime::fromEpoch(int64_t epochSeconds) {
    int64_t days = epochSeconds / 86400;
    int64_t rem = epochSeconds % 86400;
    if (rem < 0) { rem += 86400; --days; }
    int Y; unsigned Mo, D;
    civilFromDays(days, Y, Mo, D);
    const int h = static_cast<int>(rem / 3600);
    const int mi = static_cast<int>((rem % 3600) / 60);
    const int s = static_cast<int>(rem % 60);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02u-%02uT%02d:%02d:%02d", Y, Mo, D, h, mi, s);
    return std::string(buf);
}

bool DateTime::isValid(const std::string& iso) {
    try { toEpoch(iso); return true; }
    catch (const std::exception&) { return false; }
}

int DateTime::daysLate(int64_t submittedEpoch, int64_t effectiveDeadlineEpoch) {
    if (submittedEpoch <= effectiveDeadlineEpoch) return 0;
    const int64_t elapsed = submittedEpoch - effectiveDeadlineEpoch;
    return static_cast<int>((elapsed + 86399) / 86400); // ceil to whole started days
}

} // namespace lms
