#include "infra/Database.hpp"

namespace lms {

Database::Database(const std::string& path) {
    const int rc = sqlite3_open_v2(
        path.c_str(), &db_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        const std::string msg = db_ ? sqlite3_errmsg(db_) : "out of memory";
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        throw DbError("cannot open database '" + path + "': " + msg);
    }
    sqlite3_busy_timeout(db_, 5000);
    exec("PRAGMA foreign_keys = ON");
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::exec(const std::string& sql) {
    char* err = nullptr;
    const int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        const std::string msg = err ? err : sqlite3_errmsg(db_);
        if (err) sqlite3_free(err);
        throw DbError("exec failed: " + msg);
    }
}

Statement Database::prepare(const std::string& sql) {
    return Statement(db_, sql);
}

int64_t Database::lastInsertId() const {
    return sqlite3_last_insert_rowid(db_);
}

int Database::changes() const {
    return sqlite3_changes(db_);
}

// ---- Statement ----

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db) {
    const int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr);
    if (rc != SQLITE_OK)
        throw DbError("prepare failed: " + std::string(sqlite3_errmsg(db_)) +
                      " [" + sql + "]");
}

Statement::~Statement() {
    if (stmt_) sqlite3_finalize(stmt_);
}

Statement::Statement(Statement&& other) noexcept : db_(other.db_), stmt_(other.stmt_) {
    other.stmt_ = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        if (stmt_) sqlite3_finalize(stmt_);
        db_ = other.db_;
        stmt_ = other.stmt_;
        other.stmt_ = nullptr;
    }
    return *this;
}

void Statement::check(int rc, const char* what) const {
    if (rc != SQLITE_OK)
        throw DbError(std::string(what) + ": " + sqlite3_errmsg(db_));
}

Statement& Statement::bind(int index, int value) {
    check(sqlite3_bind_int(stmt_, index, value), "bind int");
    return *this;
}

Statement& Statement::bind(int index, int64_t value) {
    check(sqlite3_bind_int64(stmt_, index, value), "bind int64");
    return *this;
}

Statement& Statement::bind(int index, const std::string& value) {
    check(sqlite3_bind_text(stmt_, index, value.c_str(),
                            static_cast<int>(value.size()), SQLITE_TRANSIENT),
          "bind text");
    return *this;
}

Statement& Statement::bindNull(int index) {
    check(sqlite3_bind_null(stmt_, index), "bind null");
    return *this;
}

bool Statement::step() {
    const int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw DbError("step failed: " + std::string(sqlite3_errmsg(db_)));
}

void Statement::reset() {
    sqlite3_reset(stmt_);
    sqlite3_clear_bindings(stmt_);
}

int Statement::colInt(int index) const { return sqlite3_column_int(stmt_, index); }
int64_t Statement::colInt64(int index) const { return sqlite3_column_int64(stmt_, index); }

std::string Statement::colText(int index) const {
    const unsigned char* text = sqlite3_column_text(stmt_, index);
    if (!text) return std::string();
    return std::string(reinterpret_cast<const char*>(text));
}

bool Statement::colIsNull(int index) const {
    return sqlite3_column_type(stmt_, index) == SQLITE_NULL;
}

} // namespace lms
