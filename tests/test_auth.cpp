#include "doctest.h"
#include "TestEnv.hpp"
#include "domain/Errors.hpp"
#include "infra/Argon2idPasswordHasher.hpp"

using namespace lms;

TEST_CASE("Argon2id hashing produces a verifiable PHC string") {
    Argon2idPasswordHasher hasher(1, 256, 1);
    const std::string phc = hasher.hash("s3cret-password");
    CHECK(phc.rfind("$argon2id$", 0) == 0);           // PHC identifier for argon2id
    CHECK(hasher.verify(phc, "s3cret-password"));
    CHECK_FALSE(hasher.verify(phc, "wrong-password"));
}

TEST_CASE("Login succeeds for valid credentials and fails otherwise") {
    TestEnv env;
    env.seedWorld();
    auto user = env.auth.login("lect", "password1");
    CHECK(user->role() == Role::Lecturer);
    CHECK_THROWS_AS(env.auth.login("lect", "bad"), AuthenticationError);
    CHECK_THROWS_AS(env.auth.login("nobody", "password1"), AuthenticationError);
}

TEST_CASE("An inactive account cannot log in") {
    TestEnv env;
    auto ids = env.seedWorld();
    env.userAdmin.setUserActive(*env.admin, ids.studentId, false);
    CHECK_THROWS_AS(env.auth.login("stud", "password1"), AuthenticationError);
}

TEST_CASE("Only an administrator may create users") {
    TestEnv env;
    auto ids = env.seedWorld();
    auto lecturer = env.userById(ids.lecturerId);
    CHECK_THROWS_AS(
        env.userAdmin.createUser(*lecturer, Role::Student, "x", "password1", "X"),
        AuthorizationError);
}
