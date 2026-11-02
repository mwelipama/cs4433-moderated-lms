#include "ui/ConsoleApp.hpp"
#include "ui/Console.hpp"
#include "domain/Errors.hpp"

#include <iostream>

namespace lms {

namespace {
std::string roleLabel(Role r) {
    switch (r) {
        case Role::Admin: return "Administrator";
        case Role::Lecturer: return "Lecturer";
        case Role::Hod: return "Head of Department";
        case Role::Student: return "Student";
    }
    return "Unknown";
}
} // namespace

std::string ConsoleApp::nameOf(int64_t userId) {
    auto u = svc_.userRepo().findById(userId);
    return u ? u->displayName() : ("user#" + std::to_string(userId));
}

void ConsoleApp::run() {
    console::title("Moderated Learning Management System");
    console::info("Sign in to continue.");
    for (;;) {
        std::cout << "\n1) Log in\n0) Exit\n";
        const int choice = console::readMenuChoice(0, 1);
        if (choice == 0) {
            console::info("Goodbye.");
            return;
        }
        auto user = login();
        if (user) dispatch(*user);
    }
}

std::unique_ptr<User> ConsoleApp::login() {
    console::title("Log in");
    const std::string username = console::readLine("Username (blank to cancel): ");
    if (username.empty()) return nullptr;
    const std::string password = console::readPassword("Password: ");
    try {
        auto user = svc_.auth().login(username, password);
        console::info("Welcome, " + user->displayName() + " (" + roleLabel(user->role()) + ").");
        return user;
    } catch (const LmsError& e) {
        console::error(e.what());
        return nullptr;
    }
}

void ConsoleApp::dispatch(User& user) {
    switch (user.role()) {
        case Role::Admin: adminMenu(user); break;
        case Role::Lecturer: lecturerMenu(user); break;
        case Role::Hod: hodMenu(user); break;
        case Role::Student: studentMenu(user); break;
    }
}

// =============================== Administrator ===============================

void ConsoleApp::adminMenu(User& user) {
    for (;;) {
        console::title("Administrator - " + user.displayName());
        std::cout << "1) Create department\n"
                     "2) List departments\n"
                     "3) Create user\n"
                     "4) List users\n"
                     "5) Activate / deactivate user\n"
                     "6) Create course\n"
                     "7) Assign lecturer to course\n"
                     "8) Enrol student in course\n"
                     "9) List courses\n"
                     "0) Log out\n";
        const int c = console::readMenuChoice(0, 9);
        if (c == 0) return;
        try {
            switch (c) {
                case 1: adminCreateDepartment(user); break;
                case 2: adminListDepartments(user); break;
                case 3: adminCreateUser(user); break;
                case 4: adminListUsers(user); break;
                case 5: adminSetActive(user); break;
                case 6: adminCreateCourse(user); break;
                case 7: adminAssignLecturer(user); break;
                case 8: adminEnrolStudent(user); break;
                case 9: adminListCourses(user); break;
            }
        } catch (const LmsError& e) {
            console::error(e.what());
        }
        console::pause();
    }
}

void ConsoleApp::adminCreateDepartment(User& user) {
    const std::string name = console::readNonEmpty("Department name: ");
    const int64_t id = svc_.userAdmin().createDepartment(user, name);
    console::info("Created department #" + std::to_string(id) + ".");
}

void ConsoleApp::adminListDepartments(User& user) {
    const auto depts = svc_.userAdmin().listDepartments(user);
    console::title("Departments");
    if (depts.empty()) { console::info("(none)"); return; }
    for (const auto& d : depts)
        std::cout << "  #" << d.id << "  " << d.name << "\n";
}

void ConsoleApp::adminCreateUser(User& user) {
    console::title("Create user");
    std::cout << "1) Administrator\n2) Lecturer\n3) Head of Department\n4) Student\n";
    const int roleChoice = console::readMenuChoice(1, 4);
    const Role role = roleChoice == 1 ? Role::Admin
                    : roleChoice == 2 ? Role::Lecturer
                    : roleChoice == 3 ? Role::Hod
                                      : Role::Student;
    const std::string username = console::readNonEmpty("Username: ");
    const std::string displayName = console::readNonEmpty("Display name: ");
    const std::string pw1 = console::readPassword("Password (min 8 chars): ");
    const std::string pw2 = console::readPassword("Confirm password: ");
    if (pw1 != pw2) { console::error("passwords do not match."); return; }

    int64_t deptId = 0;
    if (role == Role::Hod) {
        adminListDepartments(user);
        deptId = console::readId("Department id for this Head of Department: ");
    }
    const int64_t id = svc_.userAdmin().createUser(user, role, username, pw1, displayName, deptId);
    console::info("Created " + roleLabel(role) + " #" + std::to_string(id) + ".");
}

void ConsoleApp::adminListUsers(User& user) {
    const auto users = svc_.userAdmin().listUsers(user);
    console::title("Users");
    for (const auto& u : users) {
        std::cout << "  #" << u->id() << "  " << u->username() << "  [" << roleLabel(u->role())
                  << "]  " << (u->active() ? "active" : "inactive");
        if (u->hasDepartment()) std::cout << "  dept#" << u->departmentId();
        std::cout << "  - " << u->displayName() << "\n";
    }
}

void ConsoleApp::adminSetActive(User& user) {
    const int64_t id = console::readId("User id: ");
    const bool active = console::readYesNo("Set this account active?");
    svc_.userAdmin().setUserActive(user, id, active);
    console::info(std::string("User #") + std::to_string(id) + (active ? " activated." : " deactivated."));
}

void ConsoleApp::adminCreateCourse(User& user) {
    const std::string code = console::readNonEmpty("Course code: ");
    const std::string title = console::readNonEmpty("Course title: ");
    adminListDepartments(user);
    const int64_t deptId = console::readId("Department id: ");
    const int64_t id = svc_.courses().createCourse(user, code, title, deptId);
    console::info("Created course #" + std::to_string(id) + ".");
}

void ConsoleApp::adminAssignLecturer(User& user) {
    adminListCourses(user);
    const int64_t courseId = console::readId("Course id: ");
    const int64_t lecturerId = console::readId("Lecturer user id: ");
    svc_.courses().assignLecturer(user, courseId, lecturerId);
    console::info("Lecturer #" + std::to_string(lecturerId) + " assigned to course #" +
                  std::to_string(courseId) + ".");
}

void ConsoleApp::adminEnrolStudent(User& user) {
    const int64_t courseId = console::readId("Course id: ");
    const int64_t studentId = console::readId("Student user id: ");
    svc_.courses().enrolStudent(user, courseId, studentId);
    console::info("Student #" + std::to_string(studentId) + " enrolled in course #" +
                  std::to_string(courseId) + ".");
}

void ConsoleApp::adminListCourses(User& user) {
    const auto courses = svc_.courses().listCourses(user);
    console::title("Courses");
    if (courses.empty()) { console::info("(none)"); return; }
    for (const auto& c : courses)
        std::cout << "  #" << c.id << "  " << c.code << "  " << c.title
                  << "  (dept#" << c.deptId << ")\n";
}

// ================================= Lecturer ==================================

void ConsoleApp::lecturerMenu(User& user) {
    for (;;) {
        console::title("Lecturer - " + user.displayName());
        std::cout << "1) My courses\n"
                     "2) List assignments in a course\n"
                     "3) Create assignment\n"
                     "4) Grant a student an extension\n"
                     "5) View submissions to mark\n"
                     "6) Mark a submission\n"
                     "0) Log out\n";
        const int c = console::readMenuChoice(0, 6);
        if (c == 0) return;
        try {
            switch (c) {
                case 1: lecturerListCourses(user); break;
                case 2: lecturerListAssignments(user); break;
                case 3: lecturerCreateAssignment(user); break;
                case 4: lecturerGrantExtension(user); break;
                case 5: lecturerMarkQueue(user); break;
                case 6: lecturerMark(user); break;
            }
        } catch (const LmsError& e) {
            console::error(e.what());
        }
        console::pause();
    }
}

void ConsoleApp::lecturerListCourses(User& user) {
    const auto courses = svc_.lecturerCourseRepo().coursesForLecturer(user.id());
    console::title("My courses");
    if (courses.empty()) { console::info("(you are not assigned to any course)"); return; }
    for (const auto& c : courses)
        std::cout << "  #" << c.id << "  " << c.code << "  " << c.title << "\n";
}

void ConsoleApp::lecturerListAssignments(User& user) {
    const int64_t courseId = console::readId("Course id: ");
    const auto assignments = svc_.assignments().listAssignments(user, courseId);
    console::title("Assignments");
    if (assignments.empty()) { console::info("(none)"); return; }
    for (const auto& a : assignments)
        std::cout << "  #" << a.id << "  " << a.title << "  due " << a.deadline
                  << "  max " << a.maxMark << "\n";
}

void ConsoleApp::lecturerCreateAssignment(User& user) {
    const int64_t courseId = console::readId("Course id: ");
    const std::string title = console::readNonEmpty("Assignment title: ");
    const std::string deadline = console::readNonEmpty("Deadline (YYYY-MM-DDTHH:MM:SS, UTC): ");
    const int maxMark = console::readInt("Maximum mark: ");
    const int64_t id = svc_.assignments().createAssignment(user, courseId, title, deadline, maxMark);
    console::info("Created assignment #" + std::to_string(id) + ".");
}

void ConsoleApp::lecturerGrantExtension(User& user) {
    const int64_t assignmentId = console::readId("Assignment id: ");
    const int64_t studentId = console::readId("Student user id: ");
    const std::string deadline = console::readNonEmpty("Extended deadline (YYYY-MM-DDTHH:MM:SS, UTC): ");
    svc_.assignments().grantExtension(user, assignmentId, studentId, deadline);
    console::info("Extension granted.");
}

void ConsoleApp::lecturerMarkQueue(User& user) {
    const int64_t assignmentId = console::readId("Assignment id: ");
    const auto queue = svc_.marking().markQueue(user, assignmentId);
    console::title("Submissions");
    if (queue.empty()) { console::info("(nothing to mark)"); return; }
    for (const auto& s : queue)
        std::cout << "  submission #" << s.id << "  " << nameOf(s.studentId)
                  << "  [" << toString(s.state) << "]  at " << s.submittedAt << "\n";
}

void ConsoleApp::lecturerMark(User& user) {
    const int64_t submissionId = console::readId("Submission id: ");
    const int rawMark = console::readInt("Raw mark: ");
    const std::string feedback = console::readNonEmpty("Feedback: ");
    std::string reason;
    if (svc_.markRepo().findBySubmission(submissionId))
        reason = console::readNonEmpty("Reason for changing the mark: ");
    const auto result = svc_.marking().mark(user, submissionId, rawMark, feedback, reason);
    console::info("Recorded. Raw " + std::to_string(result.rawMark) + ", awarded " +
                  std::to_string(result.awardedMark) +
                  (result.daysLate > 0 ? " (" + std::to_string(result.daysLate) + " day(s) late)" : "") + ".");
}

// ============================ Head of Department =============================

void ConsoleApp::hodMenu(User& user) {
    for (;;) {
        console::title("Head of Department - " + user.displayName());
        std::cout << "1) Courses in my department\n"
                     "2) List assignments in a course\n"
                     "3) View submissions to moderate\n"
                     "4) Moderate: adjust a mark\n"
                     "5) Moderate: accept a mark unchanged\n"
                     "6) Release marks for an assignment\n"
                     "0) Log out\n";
        const int c = console::readMenuChoice(0, 6);
        if (c == 0) return;
        try {
            switch (c) {
                case 1: hodListCourses(user); break;
                case 2: hodListAssignments(user); break;
                case 3: hodModerationQueue(user); break;
                case 4: hodAdjust(user); break;
                case 5: hodAccept(user); break;
                case 6: hodRelease(user); break;
            }
        } catch (const LmsError& e) {
            console::error(e.what());
        }
        console::pause();
    }
}

void ConsoleApp::hodListCourses(User& user) {
    console::title("Courses in my department");
    bool any = false;
    for (const auto& c : svc_.courseRepo().list()) {
        if (c.deptId == user.departmentId()) {
            std::cout << "  #" << c.id << "  " << c.code << "  " << c.title << "\n";
            any = true;
        }
    }
    if (!any) console::info("(no courses in your department)");
}

void ConsoleApp::hodListAssignments(User& user) {
    const int64_t courseId = console::readId("Course id: ");
    const auto assignments = svc_.assignments().listForHod(user, courseId);
    console::title("Assignments");
    if (assignments.empty()) { console::info("(none)"); return; }
    for (const auto& a : assignments)
        std::cout << "  #" << a.id << "  " << a.title << "  due " << a.deadline
                  << "  max " << a.maxMark << "\n";
}

void ConsoleApp::hodModerationQueue(User& user) {
    const int64_t assignmentId = console::readId("Assignment id: ");
    const auto queue = svc_.moderation().moderationQueue(user, assignmentId);
    console::title("Marked submissions awaiting moderation");
    if (queue.empty()) { console::info("(nothing to moderate)"); return; }
    for (const auto& s : queue) {
        auto mark = svc_.markRepo().findBySubmission(s.id);
        std::cout << "  submission #" << s.id << "  " << nameOf(s.studentId);
        if (mark)
            std::cout << "  raw " << mark->rawMark << ", awarded " << mark->awardedMark;
        std::cout << "\n";
    }
}

void ConsoleApp::hodAdjust(User& user) {
    const int64_t submissionId = console::readId("Submission id: ");
    const int newMark = console::readInt("New awarded mark: ");
    const std::string reason = console::readNonEmpty("Reason for the change: ");
    svc_.moderation().adjust(user, submissionId, newMark, reason);
    console::info("Mark moderated and adjusted.");
}

void ConsoleApp::hodAccept(User& user) {
    const int64_t submissionId = console::readId("Submission id: ");
    svc_.moderation().accept(user, submissionId);
    console::info("Mark moderated (accepted unchanged).");
}

void ConsoleApp::hodRelease(User& user) {
    const int64_t assignmentId = console::readId("Assignment id: ");
    const int count = svc_.moderation().release(user, assignmentId);
    console::info("Released " + std::to_string(count) + " moderated submission(s) to students.");
}

// ================================== Student ==================================

void ConsoleApp::studentMenu(User& user) {
    for (;;) {
        console::title("Student - " + user.displayName());
        std::cout << "1) My courses\n"
                     "2) List assignments in a course\n"
                     "3) Submit / resubmit work\n"
                     "4) View my submission status\n"
                     "5) View my released result\n"
                     "0) Log out\n";
        const int c = console::readMenuChoice(0, 5);
        if (c == 0) return;
        try {
            switch (c) {
                case 1: studentListCourses(user); break;
                case 2: studentListAssignments(user); break;
                case 3: studentSubmit(user); break;
                case 4: studentViewSubmission(user); break;
                case 5: studentViewResult(user); break;
            }
        } catch (const LmsError& e) {
            console::error(e.what());
        }
        console::pause();
    }
}

void ConsoleApp::studentListCourses(User& user) {
    const auto courses = svc_.enrolmentRepo().coursesForStudent(user.id());
    console::title("My courses");
    if (courses.empty()) { console::info("(you are not enrolled in any course)"); return; }
    for (const auto& c : courses)
        std::cout << "  #" << c.id << "  " << c.code << "  " << c.title << "\n";
}

void ConsoleApp::studentListAssignments(User& user) {
    const int64_t courseId = console::readId("Course id: ");
    const auto assignments = svc_.assignments().listForStudent(user, courseId);
    console::title("Assignments");
    if (assignments.empty()) { console::info("(none)"); return; }
    for (const auto& a : assignments)
        std::cout << "  #" << a.id << "  " << a.title << "  due " << a.deadline
                  << "  max " << a.maxMark << "\n";
}

void ConsoleApp::studentSubmit(User& user) {
    const int64_t assignmentId = console::readId("Assignment id: ");
    const std::string content = console::readNonEmpty("Your submission text: ");
    const auto result = svc_.submissions().submit(user, assignmentId, content);
    if (result.state == SubmissionState::Late)
        console::info("Submitted LATE (" + std::to_string(result.daysLate) +
                      " day(s) late). A penalty will apply at marking.");
    else
        console::info("Submitted on time.");
}

void ConsoleApp::studentViewSubmission(User& user) {
    const int64_t assignmentId = console::readId("Assignment id: ");
    const auto sub = svc_.submissions().myGet(user, assignmentId);
    console::title("My submission");
    if (!sub) { console::info("(you have not submitted for this assignment)"); return; }
    std::cout << "  state: " << toString(sub->state) << "\n"
              << "  submitted at: " << sub->submittedAt << "\n";
    console::info("Marks become visible only once released by the Head of Department.");
}

void ConsoleApp::studentViewResult(User& user) {
    const int64_t assignmentId = console::readId("Assignment id: ");
    const auto result = svc_.submissions().myReleasedResult(user, assignmentId);
    console::title("My result");
    if (!result) {
        console::info("No released result yet for this assignment.");
        return;
    }
    std::cout << "  awarded mark: " << result->mark.awardedMark << "\n"
              << "  feedback: " << result->mark.feedback << "\n";
}

} // namespace lms
