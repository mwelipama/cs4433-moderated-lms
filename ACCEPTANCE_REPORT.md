# Final Acceptance Report — CS4433 Moderated LMS

Repository: https://github.com/mwelipama/cs4433-moderated-lms · branch `main`. Release points: **v1.0 = ec7fb46**, **v1.1 = f074586** (tags created locally; see the limitations note on pushing them).
Build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` (GCC 11/13, CMake 3.16+; CI on Ubuntu and Windows).
Tests: `ctest -C Release` → 38 cases / 112 assertions, all pass. Coverage: 80.9% business-logic lines. Static analysis: cppcheck clean (one justified suppression). Metrics: ~3,900 LOC, average cyclomatic complexity 2.3.

## Status by marking component
| Component | Status | Notes |
|---|---|---|
| Project planning (10) | COMPLETE AND EVIDENCED | Phase 1 plan, schedule, risk register |
| Requirements (14) | COMPLETE AND EVIDENCED | SRS (21 FR + FR-22, 10 NFR), stakeholder analysis, use cases |
| System design (24) | COMPLETE AND EVIDENCED | Architecture, class, behavioural, database, traceability, deployment diagram; CR1 reflected in SRS/UC/DB/traceability |
| Implementation (21) | COMPLETE AND EVIDENCED | All Must requirements; patterns; parameterised SQL; atomic transactions; Argon2id; append-only audit |
| Verification & validation (12) | COMPLETE AND EVIDENCED | Strategy, 38/112 tests, 80.9% coverage, defect log, CI green |
| Config management & QA (6) | COMPLETE AND EVIDENCED | Git + PR #1 workflow, CI (Ubuntu+Windows), cppcheck clean, metrics, self-review, tags |
| Deployment (4) | COMPLETE AND EVIDENCED | Deployment diagram, install guide, role manuals, release notes |
| Maintenance CR1 (6) | COMPLETE AND EVIDENCED | Class-wide extension implemented as v1.1 with six regression tests, merged via PR #1 |
| Professional/legal/ethical (3) | COMPLETE AND EVIDENCED | Report section 10 against Lesotho Data Protection Act, 2012 |

## Deviations and honest limitations
- **NFR-03 (libsodium):** the implementation uses a vendored Argon2id reference implementation behind `IPasswordHasher`, not system libsodium, for build portability. Same algorithm and parameters; disclosed, not claimed as full compliance.
- **Class diagram image:** the new `class_extensions` table and `extendClass` operation are documented in the database design, SRS, use-case model and traceability; the rendered class-diagram PNG was not regenerated for the single added table.
- **Viva/demonstration:** the demonstration must be performed by the author; a run-sheet is provided (`docs/DEMO_RUNSHEET.md`).
- **Release tags:** v1.0 and v1.1 are defined at the commits above; this build sandbox's proxy blocks tag pushes, so the two tags must be pushed once from the author's machine (`git fetch origin && git tag v1.0 ec7fb46 && git tag v1.1 f074586 && git push origin v1.0 v1.1`).
- Marks are awarded by the lecturer; no grade is claimed here.
