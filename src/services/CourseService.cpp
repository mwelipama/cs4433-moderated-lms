#include "services/CourseService.hpp"
#include "services/Validation.hpp"
#include "domain/Errors.hpp"

namespace lms {

void CourseService::requireAdmin(const User& actor) const {
    if (actor.role() != Role::Admin)
        throw AuthorizationError("only an administrator may perform this action");
}

int64_t CourseService::createCourse(const User& actor, const std::string& code,
                                    const std::string& title, int64_t departmentId) {
    requireAdmin(actor);
    requireNonBlank(code, "course code");
    requireNonBlank(title, "course title");
    if (!departments_.findById(departmentId))
        throw NotFoundError("no department with id " + std::to_string(departmentId));
    const std::string c = trimWhitespace(code);
    if (courses_.findByCode(c))
        throw ConflictError("a course with code '" + c + "' already exists");
    return courses_.create(c, trimWhitespace(title), departmentId);
}

void CourseService::assignLecturer(const User& actor, int64_t courseId, int64_t lecturerId) {
    requireAdmin(actor);
    if (!courses_.findById(courseId))
        throw NotFoundError("no course with id " + std::to_string(courseId));
    auto lecturer = users_.findById(lecturerId);
    if (!lecturer) throw NotFoundError("no user with id " + std::to_string(lecturerId));
    if (lecturer->role() != Role::Lecturer)
        throw ValidationError("user " + std::to_string(lecturerId) + " is not a lecturer");
    lecturerCourses_.assign(courseId, lecturerId);
}

void CourseService::enrolStudent(const User& actor, int64_t courseId, int64_t studentId) {
    requireAdmin(actor);
    if (!courses_.findById(courseId))
        throw NotFoundError("no course with id " + std::to_string(courseId));
    auto student = users_.findById(studentId);
    if (!student) throw NotFoundError("no user with id " + std::to_string(studentId));
    if (student->role() != Role::Student)
        throw ValidationError("user " + std::to_string(studentId) + " is not a student");
    enrolments_.enrol(courseId, studentId);
}

std::vector<Course> CourseService::listCourses(const User& actor) {
    requireAdmin(actor);
    return courses_.list();
}

} // namespace lms
