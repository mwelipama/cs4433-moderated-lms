#pragma once

namespace lms {

class Database;

// Creates the database schema from the embedded DDL on first use. Idempotent: on an
// existing database it does nothing, which is what makes persistence across restarts work.
class SchemaInitializer {
public:
    static bool ensureSchema(Database& db); // true if the schema was created on this call
    static void applyMigrations(Database& db); // idempotent upgrades for older databases
};

} // namespace lms
