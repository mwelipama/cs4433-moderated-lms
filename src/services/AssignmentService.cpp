#include "services/AssignmentService.hpp"
#include "services/Validation.hpp"
#include "domain/Errors.hpp"
#include "infra/DateTime.hpp"

namespace lms {

void AssignmentService::requireLecturerForCourse(const User& actor, int64_t courseId) const {
    if (actor.role() != Role::Lecturer)
        throw AuthorizationError("only a lecturer may perform this action");
    if (!lecturerCourses_.isAssigned(courseId, actor.id()))
        throw AuthorizationError("you are not assigned to this course");
}

int64_t AssignmentService::createAssignment(const User& actor, int64_t courseId,
                                            const std::string& title,
                                            const std::string& deadlineIso, int maxMark) {
    if (!courses_.findById(courseId))
        throw NotFoundError("no course with id " + std::to_string(courseId));
    requireLecturerForCourse(actor, courseId);
    requireNonBlank(title, "assignment title");
    if (!DateTime::isValid(deadlineIso))
        throw ValidationError("deadline is not a valid ISO-8601 datetime (YYYY-MM-DDTHH:MM:SS)");
    if (maxMark <= 0)
        throw ValidationError("maximum mark must be greater than zero");
    return assignments_.create(courseId, trimWhitespace(title), deadlineIso, maxMark);
}

void AssignmentService::grantExtension(const User& actor, int64_t assignmentId, int64_t studentId,
                                       const std::string& extendedDeadlineIso) {
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundError("no assignment with id " + std::to_string(assignmentId));
    requireLecturerForCourse(actor, assignment->courseId);
    if (!enrolments_.isEnrolled(assignment->courseId, studentId))
        throw ValidationError("that student is not enrolled in this assignment's course");
    if (!DateTime::isValid(extendedDeadlineIso))
        throw ValidationError("extended deadline is not a valid ISO-8601 datetime");
    extensions_.set(assignmentId, studentId, extendedDeadlineIso);
}

std::vector<Assignment> AssignmentService::listAssignments(const User& actor, int64_t courseId) {
    requireLecturerForCourse(actor, courseId);
    return assignments_.listByCourse(courseId);
}

} // namespace lms
