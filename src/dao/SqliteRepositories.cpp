#include "dao/SqliteRepositories.hpp"
#include "domain/UserFactory.hpp"

namespace lms {

namespace {

// Column order shared by every user SELECT below, so hydration stays in one place.
const char* const kUserCols =
    "user_id, username, password_hash, display_name, role, active, department_id";

std::unique_ptr<User> hydrateUser(Statement& st) {
    const int64_t id = st.colInt64(0);
    const std::string username = st.colText(1);
    const std::string hash = st.colText(2);
    const std::string display = st.colText(3);
    const Role role = roleFromString(st.colText(4));
    const bool active = st.colInt(5) != 0;
    const bool hasDept = !st.colIsNull(6);
    const int64_t deptId = hasDept ? st.colInt64(6) : 0;
    return UserFactory::create(role, id, username, hash, display, active, hasDept, deptId);
}

Course hydrateCourse(Statement& st) {
    Course c;
    c.id = st.colInt64(0);
    c.code = st.colText(1);
    c.title = st.colText(2);
    c.deptId = st.colInt64(3);
    return c;
}

Submission hydrateSubmission(Statement& st) {
    Submission s;
    s.id = st.colInt64(0);
    s.assignmentId = st.colInt64(1);
    s.studentId = st.colInt64(2);
    s.content = st.colText(3);
    s.submittedAt = st.colText(4);
    s.state = submissionStateFromString(st.colText(5));
    return s;
}

} // namespace

// ----------------------------- Users -----------------------------

int64_t SqliteUserRepository::create(const NewUser& u) {
    Statement st = db_.prepare(
        "INSERT INTO users(username, password_hash, display_name, role, active, department_id) "
        "VALUES(?,?,?,?,?,?)");
    st.bind(1, u.username).bind(2, u.passwordHash).bind(3, u.displayName)
      .bind(4, toString(u.role)).bind(5, u.active ? 1 : 0);
    if (u.hasDepartment) st.bind(6, u.departmentId); else st.bindNull(6);
    st.step();
    return db_.lastInsertId();
}

std::unique_ptr<User> SqliteUserRepository::findById(int64_t id) {
    Statement st = db_.prepare(std::string("SELECT ") + kUserCols + " FROM users WHERE user_id=?");
    st.bind(1, id);
    if (!st.step()) return nullptr;
    return hydrateUser(st);
}

std::unique_ptr<User> SqliteUserRepository::findByUsername(const std::string& username) {
    Statement st = db_.prepare(std::string("SELECT ") + kUserCols + " FROM users WHERE username=?");
    st.bind(1, username);
    if (!st.step()) return nullptr;
    return hydrateUser(st);
}

bool SqliteUserRepository::usernameExists(const std::string& username) {
    Statement st = db_.prepare("SELECT 1 FROM users WHERE username=?");
    st.bind(1, username);
    return st.step();
}

std::vector<std::unique_ptr<User>> SqliteUserRepository::list() {
    Statement st = db_.prepare(std::string("SELECT ") + kUserCols + " FROM users ORDER BY role, username");
    std::vector<std::unique_ptr<User>> out;
    while (st.step()) out.push_back(hydrateUser(st));
    return out;
}

void SqliteUserRepository::setActive(int64_t id, bool active) {
    Statement st = db_.prepare("UPDATE users SET active=? WHERE user_id=?");
    st.bind(1, active ? 1 : 0).bind(2, id);
    st.step();
}

// -------------------------- Departments ---------------------------

int64_t SqliteDepartmentRepository::create(const std::string& name) {
    Statement st = db_.prepare("INSERT INTO departments(name) VALUES(?)");
    st.bind(1, name);
    st.step();
    return db_.lastInsertId();
}

std::optional<Department> SqliteDepartmentRepository::findById(int64_t id) {
    Statement st = db_.prepare("SELECT dept_id, name FROM departments WHERE dept_id=?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return Department{st.colInt64(0), st.colText(1)};
}

std::optional<Department> SqliteDepartmentRepository::findByName(const std::string& name) {
    Statement st = db_.prepare("SELECT dept_id, name FROM departments WHERE name=?");
    st.bind(1, name);
    if (!st.step()) return std::nullopt;
    return Department{st.colInt64(0), st.colText(1)};
}

std::vector<Department> SqliteDepartmentRepository::list() {
    Statement st = db_.prepare("SELECT dept_id, name FROM departments ORDER BY name");
    std::vector<Department> out;
    while (st.step()) out.push_back(Department{st.colInt64(0), st.colText(1)});
    return out;
}

// ----------------------------- Courses ----------------------------

int64_t SqliteCourseRepository::create(const std::string& code, const std::string& title, int64_t deptId) {
    Statement st = db_.prepare("INSERT INTO courses(code, title, dept_id) VALUES(?,?,?)");
    st.bind(1, code).bind(2, title).bind(3, deptId);
    st.step();
    return db_.lastInsertId();
}

std::optional<Course> SqliteCourseRepository::findById(int64_t id) {
    Statement st = db_.prepare("SELECT course_id, code, title, dept_id FROM courses WHERE course_id=?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return hydrateCourse(st);
}

std::optional<Course> SqliteCourseRepository::findByCode(const std::string& code) {
    Statement st = db_.prepare("SELECT course_id, code, title, dept_id FROM courses WHERE code=?");
    st.bind(1, code);
    if (!st.step()) return std::nullopt;
    return hydrateCourse(st);
}

std::vector<Course> SqliteCourseRepository::list() {
    Statement st = db_.prepare("SELECT course_id, code, title, dept_id FROM courses ORDER BY code");
    std::vector<Course> out;
    while (st.step()) out.push_back(hydrateCourse(st));
    return out;
}

// ----------------------- Lecturer <-> Course ----------------------

void SqliteLecturerCourseRepository::assign(int64_t courseId, int64_t lecturerId) {
    Statement st = db_.prepare(
        "INSERT OR IGNORE INTO lecturer_courses(course_id, lecturer_id) VALUES(?,?)");
    st.bind(1, courseId).bind(2, lecturerId);
    st.step();
}

bool SqliteLecturerCourseRepository::isAssigned(int64_t courseId, int64_t lecturerId) {
    Statement st = db_.prepare(
        "SELECT 1 FROM lecturer_courses WHERE course_id=? AND lecturer_id=?");
    st.bind(1, courseId).bind(2, lecturerId);
    return st.step();
}

std::vector<Course> SqliteLecturerCourseRepository::coursesForLecturer(int64_t lecturerId) {
    Statement st = db_.prepare(
        "SELECT c.course_id, c.code, c.title, c.dept_id FROM courses c "
        "JOIN lecturer_courses lc ON lc.course_id = c.course_id "
        "WHERE lc.lecturer_id=? ORDER BY c.code");
    st.bind(1, lecturerId);
    std::vector<Course> out;
    while (st.step()) out.push_back(hydrateCourse(st));
    return out;
}

// -------------------------- Enrolments ----------------------------

void SqliteEnrolmentRepository::enrol(int64_t courseId, int64_t studentId) {
    Statement st = db_.prepare(
        "INSERT OR IGNORE INTO enrolments(course_id, student_id) VALUES(?,?)");
    st.bind(1, courseId).bind(2, studentId);
    st.step();
}

bool SqliteEnrolmentRepository::isEnrolled(int64_t courseId, int64_t studentId) {
    Statement st = db_.prepare("SELECT 1 FROM enrolments WHERE course_id=? AND student_id=?");
    st.bind(1, courseId).bind(2, studentId);
    return st.step();
}

std::vector<Course> SqliteEnrolmentRepository::coursesForStudent(int64_t studentId) {
    Statement st = db_.prepare(
        "SELECT c.course_id, c.code, c.title, c.dept_id FROM courses c "
        "JOIN enrolments e ON e.course_id = c.course_id "
        "WHERE e.student_id=? ORDER BY c.code");
    st.bind(1, studentId);
    std::vector<Course> out;
    while (st.step()) out.push_back(hydrateCourse(st));
    return out;
}

std::vector<std::unique_ptr<User>> SqliteEnrolmentRepository::studentsForCourse(int64_t courseId) {
    Statement st = db_.prepare(
        std::string("SELECT ") +
        "u.user_id, u.username, u.password_hash, u.display_name, u.role, u.active, u.department_id "
        "FROM users u JOIN enrolments e ON e.student_id = u.user_id "
        "WHERE e.course_id=? ORDER BY u.display_name");
    st.bind(1, courseId);
    std::vector<std::unique_ptr<User>> out;
    while (st.step()) out.push_back(hydrateUser(st));
    return out;
}

// -------------------------- Assignments ---------------------------

int64_t SqliteAssignmentRepository::create(int64_t courseId, const std::string& title,
                                           const std::string& deadline, int maxMark) {
    Statement st = db_.prepare(
        "INSERT INTO assignments(course_id, title, deadline, max_mark) VALUES(?,?,?,?)");
    st.bind(1, courseId).bind(2, title).bind(3, deadline).bind(4, maxMark);
    st.step();
    return db_.lastInsertId();
}

std::optional<Assignment> SqliteAssignmentRepository::findById(int64_t id) {
    Statement st = db_.prepare(
        "SELECT assignment_id, course_id, title, deadline, max_mark FROM assignments WHERE assignment_id=?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    Assignment a;
    a.id = st.colInt64(0); a.courseId = st.colInt64(1); a.title = st.colText(2);
    a.deadline = st.colText(3); a.maxMark = st.colInt(4);
    return a;
}

std::vector<Assignment> SqliteAssignmentRepository::listByCourse(int64_t courseId) {
    Statement st = db_.prepare(
        "SELECT assignment_id, course_id, title, deadline, max_mark FROM assignments "
        "WHERE course_id=? ORDER BY deadline");
    st.bind(1, courseId);
    std::vector<Assignment> out;
    while (st.step()) {
        Assignment a;
        a.id = st.colInt64(0); a.courseId = st.colInt64(1); a.title = st.colText(2);
        a.deadline = st.colText(3); a.maxMark = st.colInt(4);
        out.push_back(a);
    }
    return out;
}

// --------------------------- Extensions ---------------------------

void SqliteExtensionRepository::set(int64_t assignmentId, int64_t studentId,
                                    const std::string& extendedDeadline) {
    Statement st = db_.prepare(
        "INSERT INTO extensions(assignment_id, student_id, extended_deadline) VALUES(?,?,?) "
        "ON CONFLICT(assignment_id, student_id) DO UPDATE SET extended_deadline=excluded.extended_deadline");
    st.bind(1, assignmentId).bind(2, studentId).bind(3, extendedDeadline);
    st.step();
}

std::optional<Extension> SqliteExtensionRepository::find(int64_t assignmentId, int64_t studentId) {
    Statement st = db_.prepare(
        "SELECT extension_id, assignment_id, student_id, extended_deadline FROM extensions "
        "WHERE assignment_id=? AND student_id=?");
    st.bind(1, assignmentId).bind(2, studentId);
    if (!st.step()) return std::nullopt;
    Extension e;
    e.id = st.colInt64(0); e.assignmentId = st.colInt64(1);
    e.studentId = st.colInt64(2); e.extendedDeadline = st.colText(3);
    return e;
}

// -------------------------- Submissions ---------------------------

std::optional<Submission> SqliteSubmissionRepository::find(int64_t assignmentId, int64_t studentId) {
    Statement st = db_.prepare(
        "SELECT submission_id, assignment_id, student_id, content, submitted_at, state "
        "FROM submissions WHERE assignment_id=? AND student_id=?");
    st.bind(1, assignmentId).bind(2, studentId);
    if (!st.step()) return std::nullopt;
    return hydrateSubmission(st);
}

std::optional<Submission> SqliteSubmissionRepository::findById(int64_t id) {
    Statement st = db_.prepare(
        "SELECT submission_id, assignment_id, student_id, content, submitted_at, state "
        "FROM submissions WHERE submission_id=?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return hydrateSubmission(st);
}

int64_t SqliteSubmissionRepository::insert(const Submission& s) {
    Statement st = db_.prepare(
        "INSERT INTO submissions(assignment_id, student_id, content, submitted_at, state) "
        "VALUES(?,?,?,?,?)");
    st.bind(1, s.assignmentId).bind(2, s.studentId).bind(3, s.content)
      .bind(4, s.submittedAt).bind(5, toString(s.state));
    st.step();
    return db_.lastInsertId();
}

void SqliteSubmissionRepository::update(const Submission& s) {
    Statement st = db_.prepare(
        "UPDATE submissions SET content=?, submitted_at=?, state=? WHERE submission_id=?");
    st.bind(1, s.content).bind(2, s.submittedAt).bind(3, toString(s.state)).bind(4, s.id);
    st.step();
}

void SqliteSubmissionRepository::setState(int64_t id, SubmissionState state) {
    Statement st = db_.prepare("UPDATE submissions SET state=? WHERE submission_id=?");
    st.bind(1, toString(state)).bind(2, id);
    st.step();
}

std::vector<Submission> SqliteSubmissionRepository::listByAssignment(int64_t assignmentId) {
    Statement st = db_.prepare(
        "SELECT submission_id, assignment_id, student_id, content, submitted_at, state "
        "FROM submissions WHERE assignment_id=? ORDER BY submission_id");
    st.bind(1, assignmentId);
    std::vector<Submission> out;
    while (st.step()) out.push_back(hydrateSubmission(st));
    return out;
}

// ----------------------------- Marks ------------------------------

int64_t SqliteMarkRepository::insert(int64_t submissionId, int rawMark, int awardedMark,
                                     const std::string& feedback) {
    Statement st = db_.prepare(
        "INSERT INTO marks(submission_id, raw_mark, awarded_mark, feedback) VALUES(?,?,?,?)");
    st.bind(1, submissionId).bind(2, rawMark).bind(3, awardedMark).bind(4, feedback);
    st.step();
    return db_.lastInsertId();
}

void SqliteMarkRepository::update(int64_t markId, int rawMark, int awardedMark,
                                  const std::string& feedback) {
    Statement st = db_.prepare(
        "UPDATE marks SET raw_mark=?, awarded_mark=?, feedback=? WHERE mark_id=?");
    st.bind(1, rawMark).bind(2, awardedMark).bind(3, feedback).bind(4, markId);
    st.step();
}

std::optional<Mark> SqliteMarkRepository::findBySubmission(int64_t submissionId) {
    Statement st = db_.prepare(
        "SELECT mark_id, submission_id, raw_mark, awarded_mark, feedback FROM marks WHERE submission_id=?");
    st.bind(1, submissionId);
    if (!st.step()) return std::nullopt;
    Mark m;
    m.id = st.colInt64(0); m.submissionId = st.colInt64(1);
    m.rawMark = st.colInt(2); m.awardedMark = st.colInt(3); m.feedback = st.colText(4);
    return m;
}

std::optional<Mark> SqliteMarkRepository::findById(int64_t id) {
    Statement st = db_.prepare(
        "SELECT mark_id, submission_id, raw_mark, awarded_mark, feedback FROM marks WHERE mark_id=?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    Mark m;
    m.id = st.colInt64(0); m.submissionId = st.colInt64(1);
    m.rawMark = st.colInt(2); m.awardedMark = st.colInt(3); m.feedback = st.colText(4);
    return m;
}

// --------------------------- Audit trail --------------------------

int64_t SqliteAuditRepository::append(const AuditEvent& e) {
    Statement st = db_.prepare(
        "INSERT INTO audit_events(mark_id, event_type, acting_user_id, event_time, "
        "old_value, new_value, reason) VALUES(?,?,?,?,?,?,?)");
    st.bind(1, e.markId).bind(2, toString(e.type)).bind(3, e.actingUserId).bind(4, e.eventTime);
    if (e.hasOldValue) st.bind(5, e.oldValue); else st.bindNull(5);
    st.bind(6, e.newValue).bind(7, e.reason);
    st.step();
    return db_.lastInsertId();
}

std::vector<AuditEvent> SqliteAuditRepository::listByMark(int64_t markId) {
    Statement st = db_.prepare(
        "SELECT audit_id, mark_id, event_type, acting_user_id, event_time, old_value, new_value, reason "
        "FROM audit_events WHERE mark_id=? ORDER BY audit_id");
    st.bind(1, markId);
    std::vector<AuditEvent> out;
    while (st.step()) {
        AuditEvent e;
        e.id = st.colInt64(0); e.markId = st.colInt64(1);
        e.type = auditEventTypeFromString(st.colText(2));
        e.actingUserId = st.colInt64(3); e.eventTime = st.colText(4);
        e.hasOldValue = !st.colIsNull(5);
        e.oldValue = e.hasOldValue ? st.colInt(5) : 0;
        e.newValue = st.colInt(6); e.reason = st.colText(7);
        out.push_back(e);
    }
    return out;
}

// ----------------------- Class-wide extensions --------------------

void SqliteClassExtensionRepository::set(int64_t assignmentId, const std::string& extendedDeadline) {
    Statement st = db_.prepare(
        "INSERT INTO class_extensions(assignment_id, extended_deadline) VALUES(?,?) "
        "ON CONFLICT(assignment_id) DO UPDATE SET extended_deadline=excluded.extended_deadline");
    st.bind(1, assignmentId).bind(2, extendedDeadline);
    st.step();
}

std::optional<std::string> SqliteClassExtensionRepository::find(int64_t assignmentId) {
    Statement st = db_.prepare(
        "SELECT extended_deadline FROM class_extensions WHERE assignment_id=?");
    st.bind(1, assignmentId);
    if (!st.step()) return std::nullopt;
    return st.colText(0);
}

} // namespace lms
