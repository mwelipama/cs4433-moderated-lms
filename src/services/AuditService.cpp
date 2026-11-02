#include "services/AuditService.hpp"

namespace lms {

void AuditService::recordInitial(int64_t markId, int64_t actingUserId, int newValue) {
    AuditEvent e;
    e.markId = markId;
    e.type = AuditEventType::Initial;
    e.actingUserId = actingUserId;
    e.eventTime = clock_.nowIso();
    e.hasOldValue = false;              // INITIAL has no previous value
    e.newValue = newValue;
    e.reason = "Initial mark entry";    // fixed system reason (FR-17)
    repo_.append(e);
}

void AuditService::recordChange(int64_t markId, int64_t actingUserId, int oldValue, int newValue,
                                const std::string& reason) {
    AuditEvent e;
    e.markId = markId;
    e.type = AuditEventType::Change;
    e.actingUserId = actingUserId;
    e.eventTime = clock_.nowIso();
    e.hasOldValue = true;
    e.oldValue = oldValue;
    e.newValue = newValue;
    e.reason = reason;
    repo_.append(e);
}

void AuditService::recordModerationChange(int64_t markId, int64_t actingUserId, int oldValue,
                                          int newValue, const std::string& reason) {
    AuditEvent e;
    e.markId = markId;
    e.type = AuditEventType::ModerationChange;
    e.actingUserId = actingUserId;
    e.eventTime = clock_.nowIso();
    e.hasOldValue = true;
    e.oldValue = oldValue;
    e.newValue = newValue;
    e.reason = reason;
    repo_.append(e);
}

} // namespace lms
