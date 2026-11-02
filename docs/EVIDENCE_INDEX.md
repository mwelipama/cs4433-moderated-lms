# Evidence Index (by marking component)

| Component (marks) | Evidence location |
|---|---|
| Project planning (10) | `docs/project-management/`; root Phase 1 plan and schedule; risk register in the report appendix |
| Requirements (14) | `docs/requirements/` (stakeholder analysis, SRS incl. FR-22, use-case model) |
| System design (24) | `docs/system-design/` (architecture, class design, behavioural, database, traceability); `assets/diagrams/` |
| Implementation (21) | `src/` (layered); `CMakeLists.txt`; `third_party/` (vendored SQLite, Argon2id) |
| Verification & validation (12) | `tests/` (38 cases / 112 assertions); 80.9% coverage; defect log in report appendix; CI logs |
| Config management & QA (6) | Git history + pull request #1; `.github/workflows/ci.yml`; `docs/quality/` (QA report, suppressions); tags v1.0/v1.1 |
| Deployment (4) | `docs/deployment/` (INSTALL, USER_MANUAL, RELEASE_NOTES); `assets/diagrams/deployment_diagram.*` |
| Maintenance CR1 (6) | `src/.../*ClassExtension*`, `AssignmentService::extendClass`, `domain/Deadline.hpp`; `tests/test_class_extension.cpp`; PR #1 |
| Professional/legal/ethical (3) | Report section 10 (vs Lesotho Data Protection Act, 2012) |
| Final report | `docs/report/CS4433_Final_Report_Mweli_Pama_201404452.pdf` (+ .docx) |
| Demonstration | `docs/DEMO_RUNSHEET.md` |
