#pragma once
#include <stdexcept>
#include <string>

namespace lms {

// Domain-level error hierarchy. Services throw these; the console layer catches them and
// turns them into readable messages, so business rules never print or exit by themselves.
class LmsError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ValidationError : public LmsError {       // malformed or out-of-range input
public:
    using LmsError::LmsError;
};

class AuthenticationError : public LmsError {   // wrong credentials / inactive account
public:
    using LmsError::LmsError;
};

class AuthorizationError : public LmsError {    // authenticated but not permitted
public:
    using LmsError::LmsError;
};

class NotFoundError : public LmsError {         // referenced entity does not exist
public:
    using LmsError::LmsError;
};

class ConflictError : public LmsError {         // a business rule forbids the action
public:
    using LmsError::LmsError;
};

} // namespace lms
