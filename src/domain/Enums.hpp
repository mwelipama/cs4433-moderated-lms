#pragma once
#include "domain/Errors.hpp"
#include <string>

namespace lms {

// The three enumerations are first-class domain types; each maps to a constrained TEXT
// column in the schema. Conversion to/from the stored text lives here so the mapping is
// defined in exactly one place.

enum class Role { Admin, Lecturer, Hod, Student };

enum class SubmissionState { Submitted, Late, Marked, Moderated, Released };

enum class AuditEventType { Initial, Change, ModerationChange };

inline std::string toString(Role r) {
    switch (r) {
        case Role::Admin: return "ADMIN";
        case Role::Lecturer: return "LECTURER";
        case Role::Hod: return "HOD";
        case Role::Student: return "STUDENT";
    }
    throw ValidationError("unknown role");
}

inline Role roleFromString(const std::string& s) {
    if (s == "ADMIN") return Role::Admin;
    if (s == "LECTURER") return Role::Lecturer;
    if (s == "HOD") return Role::Hod;
    if (s == "STUDENT") return Role::Student;
    throw ValidationError("unknown role string: '" + s + "'");
}

inline std::string toString(SubmissionState s) {
    switch (s) {
        case SubmissionState::Submitted: return "SUBMITTED";
        case SubmissionState::Late: return "LATE";
        case SubmissionState::Marked: return "MARKED";
        case SubmissionState::Moderated: return "MODERATED";
        case SubmissionState::Released: return "RELEASED";
    }
    throw ValidationError("unknown submission state");
}

inline SubmissionState submissionStateFromString(const std::string& s) {
    if (s == "SUBMITTED") return SubmissionState::Submitted;
    if (s == "LATE") return SubmissionState::Late;
    if (s == "MARKED") return SubmissionState::Marked;
    if (s == "MODERATED") return SubmissionState::Moderated;
    if (s == "RELEASED") return SubmissionState::Released;
    throw ValidationError("unknown submission state string: '" + s + "'");
}

inline std::string toString(AuditEventType t) {
    switch (t) {
        case AuditEventType::Initial: return "INITIAL";
        case AuditEventType::Change: return "CHANGE";
        case AuditEventType::ModerationChange: return "MODERATION_CHANGE";
    }
    throw ValidationError("unknown audit event type");
}

inline AuditEventType auditEventTypeFromString(const std::string& s) {
    if (s == "INITIAL") return AuditEventType::Initial;
    if (s == "CHANGE") return AuditEventType::Change;
    if (s == "MODERATION_CHANGE") return AuditEventType::ModerationChange;
    throw ValidationError("unknown audit event type string: '" + s + "'");
}

} // namespace lms
