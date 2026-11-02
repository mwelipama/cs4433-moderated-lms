#include "doctest.h"
#include "TestEnv.hpp"

#include <cstdio>
#include <string>

using namespace lms;

TEST_CASE("Data written in one session is present after reopening the database file") {
    const std::string path = "lms_test_persistence.sqlite";
    std::remove(path.c_str());

    int64_t studentId = 0;
    int64_t assignmentId = 0;
    {
        TestEnv env(path);                       // opens/creates the file-backed database
        auto ids = env.seedWorld();
        studentId = ids.studentId;
        assignmentId = ids.assignmentId;
        auto student = env.userById(studentId);
        env.clock.set("2026-01-20T00:00:00");
        env.submissionSvc.submit(*student, assignmentId, "persisted work");
    }  // env destroyed -> connection closed

    {
        Database db(path);                       // fresh connection to the same file
        SqliteSubmissionRepository subs(db);
        SqliteUserRepository users(db);
        auto sub = subs.find(assignmentId, studentId);
        REQUIRE(sub.has_value());
        CHECK(sub->content == "persisted work");
        CHECK(users.findByUsername("stud") != nullptr);
    }

    std::remove(path.c_str());
}
