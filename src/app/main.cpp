#include "infra/Database.hpp"
#include "infra/SchemaInitializer.hpp"
#include "services/ServiceContainer.hpp"
#include "ui/Console.hpp"
#include "ui/ConsoleApp.hpp"

#include <exception>
#include <iostream>
#include <string>

using namespace lms;

// Entry point. Opens (and on first use creates) the SQLite database, applies the schema,
// seeds the initial administrator when the database is empty, then runs the console. The
// database file persists between runs, which is what gives persistence across restarts.
int main(int argc, char** argv) {
    const std::string dbPath = (argc > 1) ? argv[1] : "lms.db";
    try {
        Database db(dbPath);
        const bool created = SchemaInitializer::ensureSchema(db);
        ServiceContainer services(db);
        if (created)
            console::info("Initialised a new database at '" + dbPath + "'.");

        if (!services.hasAnyUser()) {
            console::title("First-run setup: create the initial administrator");
            const std::string username = console::readNonEmpty("Admin username: ");
            const std::string displayName = console::readNonEmpty("Admin display name: ");
            std::string password;
            for (;;) {
                password = console::readPassword("Admin password (min 8 chars): ");
                if (password.size() < 8) { console::error("password is too short."); continue; }
                const std::string confirm = console::readPassword("Confirm password: ");
                if (password != confirm) { console::error("passwords do not match."); continue; }
                break;
            }
            services.createInitialAdmin(username, password, displayName);
            console::info("Administrator created. You can now log in.");
        }

        ConsoleApp app(services);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
