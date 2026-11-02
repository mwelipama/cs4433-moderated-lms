#include "doctest.h"
#include "domain/StandardPenaltyPolicy.hpp"
#include "infra/DateTime.hpp"

using namespace lms;

TEST_CASE("Standard penalty policy applies 10% of max per started day") {
    StandardPenaltyPolicy p;
    const int M = 100;
    CHECK(p.awardedMark(80, M, 0) == 80);   // on time: no penalty
    CHECK(p.awardedMark(80, M, 1) == 70);   // 1 day: -10
    CHECK(p.awardedMark(80, M, 2) == 60);   // 2 days: -20
    CHECK(p.awardedMark(80, M, 3) == 50);   // 3 days: -30
}

TEST_CASE("Penalty is clamped to the [0, max] range") {
    StandardPenaltyPolicy p;
    CHECK(p.awardedMark(5, 100, 3) == 0);    // 5 - 30 -> clamp to 0
    CHECK(p.awardedMark(100, 100, 0) == 100);
    CHECK(p.awardedMark(120, 100, 0) == 100); // above max -> clamp to max
}

TEST_CASE("Awarded mark rounds half up") {
    StandardPenaltyPolicy p;
    // max 25, 1 day late -> penalty 2.5; raw 20 -> 17.5 -> rounds up to 18
    CHECK(p.awardedMark(20, 25, 1) == 18);
}

TEST_CASE("Days late is the ceiling of elapsed whole days, never negative") {
    const int64_t d0 = DateTime::toEpoch("2026-02-01T00:00:00");
    CHECK(DateTime::daysLate(d0, d0) == 0);                  // exactly on time
    CHECK(DateTime::daysLate(d0 - 60, d0) == 0);             // before the deadline
    CHECK(DateTime::daysLate(d0 + 1, d0) == 1);              // 1 second late -> 1 day
    CHECK(DateTime::daysLate(d0 + 86400, d0) == 1);          // exactly 24h -> 1 day
    CHECK(DateTime::daysLate(d0 + 86401, d0) == 2);          // just over 24h -> 2 days
    CHECK(DateTime::daysLate(d0 + 3 * 86400, d0) == 3);      // 72h -> 3 days
}

TEST_CASE("Datetime round-trips through epoch") {
    const std::string iso = "2026-03-15T13:45:09";
    CHECK(DateTime::fromEpoch(DateTime::toEpoch(iso)) == iso);
    CHECK(DateTime::isValid(iso));
    CHECK_FALSE(DateTime::isValid("not-a-date"));
    CHECK_FALSE(DateTime::isValid("2026-13-01T00:00:00"));
}
