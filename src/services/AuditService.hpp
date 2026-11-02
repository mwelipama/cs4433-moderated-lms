#pragma once
#include "dao/Repositories.hpp"
#include "infra/Clock.hpp"
#include <string>

namespace lms {

// Builds and appends audit events, keeping the INITIAL/CHANGE/MODERATION_CHANGE rules and
// the fixed system reason in one place. Callers invoke it inside their own transaction so
// the audit event commits together with the mark it describes.
class AuditService {
public:
    AuditService(IAuditRepository& repo, Clock& clock) : repo_(repo), clock_(clock) {}

    void recordInitial(int64_t markId, int64_t actingUserId, int newValue);
    void recordChange(int64_t markId, int64_t actingUserId, int oldValue, int newValue,
                      const std::string& reason);
    void recordModerationChange(int64_t markId, int64_t actingUserId, int oldValue, int newValue,
                                const std::string& reason);

private:
    IAuditRepository& repo_;
    Clock& clock_;
};

} // namespace lms
