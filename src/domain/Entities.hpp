#pragma once
#include "domain/Enums.hpp"
#include "infra/DateTime.hpp"
#include <cstdint>
#include <string>

namespace lms {

struct Department {
    int64_t id{0};
    std::string name;
};

struct Course {
    int64_t id{0};
    std::string code;
    std::string title;
    int64_t deptId{0};
};

struct Assignment {
    int64_t id{0};
    int64_t courseId{0};
    std::string title;
    std::string deadline;   // ISO-8601; the base deadline before any individual extension
    int maxMark{0};
};

struct Extension {
    int64_t id{0};
    int64_t assignmentId{0};
    int64_t studentId{0};
    std::string extendedDeadline;  // ISO-8601; replaces the base deadline for this student
};

struct Submission {
    int64_t id{0};
    int64_t assignmentId{0};
    int64_t studentId{0};
    std::string content;
    std::string submittedAt;       // ISO-8601
    SubmissionState state{SubmissionState::Submitted};

    int64_t submittedEpoch() const { return DateTime::toEpoch(submittedAt); }

    // Whole started days late relative to the effective deadline (0 if on time). The
    // effective deadline is resolved by the service, which knows any individual extension.
    int daysLate(const std::string& effectiveDeadlineIso) const {
        return DateTime::daysLate(submittedEpoch(), DateTime::toEpoch(effectiveDeadlineIso));
    }
};

struct Mark {
    int64_t id{0};
    int64_t submissionId{0};
    int rawMark{0};
    int awardedMark{0};
    std::string feedback;
};

struct AuditEvent {
    int64_t id{0};
    int64_t markId{0};
    AuditEventType type{AuditEventType::Initial};
    int64_t actingUserId{0};
    std::string eventTime;   // ISO-8601
    bool hasOldValue{false}; // false only for INITIAL
    int oldValue{0};
    int newValue{0};
    std::string reason;
};

} // namespace lms
