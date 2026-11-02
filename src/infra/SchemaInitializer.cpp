#include "infra/SchemaInitializer.hpp"
#include "infra/Database.hpp"
#include "schema_embedded.hpp"

namespace lms {

bool SchemaInitializer::ensureSchema(Database& db) {
    Statement check = db.prepare(
        "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='users'");
    check.step();
    const bool exists = check.colInt(0) > 0;
    if (exists) {
        applyMigrations(db);   // bring an older database up to date
        return false;
    }
    db.exec(EMBEDDED_SCHEMA_SQL);
    applyMigrations(db);
    return true;
}

void SchemaInitializer::applyMigrations(Database& db) {
    // Added after v1.0 for CR1 (class-wide extensions). CREATE TABLE IF NOT EXISTS makes this
    // a no-op on a fresh database (the embedded schema already has it) and a safe upgrade on
    // a database created by an earlier release.
    db.exec("CREATE TABLE IF NOT EXISTS class_extensions ("
            " assignment_id INTEGER PRIMARY KEY REFERENCES assignments(assignment_id),"
            " extended_deadline TEXT NOT NULL)");
}

} // namespace lms
