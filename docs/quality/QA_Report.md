# Quality Assurance Report

## Coding standard
- C++17 throughout; one class/responsibility per file; layered dependencies (presentation -> services -> DAO/domain -> infrastructure).
- RAII for every resource (database connection, prepared statement, terminal echo state).
- All SQL is parameterised and confined to the DAO layer.
- Services depend on interfaces (repositories, penalty policy, clock, hasher) for testability.
- Errors are reported through a typed exception hierarchy; the console never lets an exception escape a menu action.
- Naming: PascalCase types, camelCase members with a trailing underscore, UPPER_SNAKE for SQL keywords.

## Static analysis
Tool: cppcheck 2.13 (`--enable=warning,style,performance,portability --std=c++17`).

- Initial run: 0 errors, 0 warnings; ~30 style/performance suggestions (const-reference parameters, one non-explicit constructor, raw filter loops), and **no correctness, memory-safety or undefined-behaviour findings**.
- Addressed: made the single-argument `Argon2idPasswordHasher` constructor `explicit`; changed hydration helpers and console handlers to take `const` references; used `const` range-for where the element is only read.
- Suppressed with justification (see `.cppcheck-suppressions`): `useStlAlgorithm` — the short filter loops are kept explicit for readability.
- Final run: **0 findings.**

## Code metrics (lizard, application code only, excludes vendored dependencies)
- Source size: ~3,900 lines (source + tests); ~2,580 logical lines of code across 239 functions.
- Average cyclomatic complexity: **2.3**; no function exceeds the complexity-15 threshold (0 warnings).

## Self code-review checklist
- [x] Every Must requirement has code and at least one test.
- [x] All SQL parameterised; no string concatenation of user input.
- [x] Marking, moderation and release are atomic (single transaction; commit inside the try so a failed commit also rolls back).
- [x] `submissions.state` is the only source of mark visibility; students cannot read unreleased marks.
- [x] Audit trail append-only (database triggers) and tested.
- [x] Passwords hashed with Argon2id; salts from the OS CSPRNG; nothing stored in plain text.
- [x] Role and ownership checks in the service layer, including the read-only listing paths.
- [x] Invalid input (bad marks, empty feedback, impossible dates) rejected without crashing.
- [x] Clean build with -Wall -Wextra and no warnings on GCC 11 and 13; CI green on Ubuntu and Windows.
