#pragma once
#include <sqlite3.h>

#include <cstdint>
#include <stdexcept>
#include <string>

namespace lms {

// Thrown for any database-level failure (open, prepare, step, constraint violation).
class DbError : public std::runtime_error {
public:
    explicit DbError(const std::string& message) : std::runtime_error(message) {}
};

// RAII wrapper over a prepared statement. All data values enter SQL only through bind*,
// never through string concatenation, which is how the parameterised-query guarantee
// (NFR-04) is enforced structurally rather than by convention.
class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    Statement& bind(int index, int value);
    Statement& bind(int index, int64_t value);
    Statement& bind(int index, const std::string& value);
    Statement& bindNull(int index);

    bool step();   // true when a row is available, false when the statement is done
    void reset();

    int colInt(int index) const;
    int64_t colInt64(int index) const;
    std::string colText(int index) const;
    bool colIsNull(int index) const;

private:
    void check(int rc, const char* what) const;
    sqlite3* db_{nullptr};
    sqlite3_stmt* stmt_{nullptr};
};

// RAII wrapper over a SQLite connection. Owns the handle, enforces foreign keys, and
// provides an all-or-nothing transaction helper used by the services to make every
// mark/state/audit change atomic.
class Database {
public:
    explicit Database(const std::string& path);
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void exec(const std::string& sql);     // execute one or more statements without parameters
    Statement prepare(const std::string& sql);
    int64_t lastInsertId() const;
    int changes() const;

    // Runs fn inside BEGIN IMMEDIATE ... COMMIT. Any exception triggers ROLLBACK and is
    // rethrown, so a partially applied unit of work can never be committed. Not reentrant:
    // a transaction must not be started while one is already open on this connection.
    template <typename Fn>
    void transaction(Fn&& fn) {
        exec("BEGIN IMMEDIATE");
        try {
            fn();
            exec("COMMIT");
        } catch (...) {
            try { exec("ROLLBACK"); } catch (...) { /* preserve original exception */ }
            throw;
        }
    }

    sqlite3* handle() const { return db_; }

private:
    sqlite3* db_{nullptr};
};

} // namespace lms
