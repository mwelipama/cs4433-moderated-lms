#include "infra/SchemaInitializer.hpp"
#include "infra/Database.hpp"
#include "schema_embedded.hpp"

namespace lms {

bool SchemaInitializer::ensureSchema(Database& db) {
    Statement check = db.prepare(
        "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='users'");
    check.step();
    const bool exists = check.colInt(0) > 0;
    if (exists) return false;
    db.exec(EMBEDDED_SCHEMA_SQL);
    return true;
}

} // namespace lms
