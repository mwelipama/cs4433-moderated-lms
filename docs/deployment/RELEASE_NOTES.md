# Release Notes

## v1.1 (current)
Change request CR1: a lecturer can extend an assignment's deadline for the whole class,
not only an individual student. The effective deadline is the latest of the base deadline,
the class extension and any individual extension, applied consistently to submission
acceptance and to the late penalty, and shown on each student's assignment list. Existing
databases upgrade automatically through a safe idempotent migration.

Also in this line since v1.0: continuous integration (Ubuntu and Windows), a static-analysis
and quality-assurance pass (cppcheck clean, complexity metrics), the security and robustness
fixes (OS-CSPRNG salts, service-layer authorisation on all listing paths, impossible-date
rejection, terminal-echo restoration, commit-time rollback), and the deployment artifacts.

Tests: 38 cases / 112 assertions; 80.9% business-logic line coverage.

## v1.0
Initial release of the Moderated Learning Management System. Role-based access for
Administrator, Lecturer, Head of Department and Student; assignment submission with on-time
resubmission, first-late acceptance and the 72-hour cutoff; lecturer marking with the late
penalty; Head-of-Department moderation and controlled release; append-only mark audit trail;
Argon2id password hashing; persistent SQLite storage via a parameterised data-access layer;
CMake build with vendored SQLite and Argon2id.
