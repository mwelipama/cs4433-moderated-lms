# Gate 0 — Baseline and Assignment Reconciliation

**Project:** CS4433 Moderated Learning Management System
**Author:** Mweli Pama (201404452)
**Repository:** https://github.com/mwelipama/cs4433-moderated-lms
**Purpose:** Establish the factual baseline before any build work — reconcile the
official assignment brief, the existing Phase 1 deliverables, and the lecturer's
latest instructions; produce a completion matrix; and surface every conflict for
an explicit decision rather than resolving it by guessing.

Status vocabulary: **DESIGNED → IMPLEMENTED → TESTED → VERIFIED**, or **OPEN**
(not started). A phase is only ever reported at the status its real evidence supports.

---

## 1. Sources inspected

| Source | Notes |
|---|---|
| `CS4433 Final Project -Individual Assignment.pdf` | Official brief — the master requirements source. Read in full (4 pages, ~2,200 words). |
| `CS4433_Phase1_Project_Management_Plan_...pdf` / `.docx` | Phase 1 report (~3,080 words, 7 sections). Read in full. |
| `CS4433_Phase1_Project_Schedule_...xlsx` + Appendix 1/2 | Schedule master + Gantt exports. |
| Lecturer's latest instruction (relayed by the author) | Final report must be **4–5 pages**. |
| Git repository | Initialised; Phase 1 committed (`4d8abe5`) and pushed to `main`. |

---

## 2. Authoritative mark scheme (from the official PDF)

| Phase | Components (marks) | Total |
|---|---|---:|
| 1 — Project Planning & Management | Feasibility 4 · WBS+schedule 3 · Risk 3 | **10** |
| 2 — Requirements Engineering | Stakeholder analysis + conflict resolution 3 · SRS 5 · Use case model 6 | **14** |
| 3 — System Design | Architecture 5 · Class diagram 7 · Behavioural models 7 · Database 5 | **24** |
| 4 — Implementation | Functionality 9 · Code quality & design-fidelity 7 · Persistence & security 5 | **21** |
| 5 — Verification & Validation | Test strategy 3 · Automated unit+integration 5 · System+acceptance 4 | **12** |
| 6 — Configuration Mgmt & QA | CM 3 · QA 3 | **6** |
| 7 — Deployment | Deployment diagram, install guide, user manuals, release notes, tag v1.0 | **4** |
| 8 — Maintenance (CR1) | Classification+impact 2 · Implementation 2 · Regression 2 | **6** |
| Professional, Legal & Ethical | Analysis | **3** |
| **TOTAL** | | **100** |

This is the governing scheme. See Conflict #2 — it differs from the summary table in the working brief.

---

## 3. Completion matrix

| # | Required deliverable (brief) | Existing evidence | Status | Remaining work | Final evidence location (planned) |
|---|---|---|---|---|---|
| P1 | Feasibility, WBS, Gantt+critical path, risk register (>=6) | Full Phase 1 report: 5-dimension feasibility + PROCEED; 10-item WBS; Gantt (Fig 1) + dependencies + milestones + critical path 1->2->3->4->5->7->8; 8-risk register (Tech/Project/People) with L,I,exposure,mitigation,owner; gate governance | **VERIFIED** | Keep risk register updated at each gate (brief requires it); add a brief "plan vs actual execution" reconciliation at the end | `docs/project-management/` (reports already in repo root) |
| P2 | Stakeholder analysis + conflict resolution; SRS (>=12 FR, >=6 measurable NFR, unique IDs, MoSCoW, verifiable); Use case diagram (4 actors, include/extend/generalisation) + 3 fully-dressed use cases incl. "Submit assignment" | None | **OPEN** | Build all of it | `docs/requirements/` + `assets/diagrams/` |
| P3 | Architecture + rationale (1 rejected alternative) + component diagram; design-level class diagram (visibility/types/multiplicity/inheritance) with >=2 design patterns + SOLID; state machine (submission); activity diagram w/ swimlanes; sequence diagrams (submit incl. late+resubmit, moderate+release); ERD >=3NF + data dictionary + SQL schema + class-to-table mapping | None | **OPEN** | Build all of it. No implementation may start until this passes (brief: "may not begin implementation until design has passed Gate 2") | `docs/system-design/` + `assets/diagrams/` + `database/` |
| P4 | C++17 app: 4 roles RBAC; abstract User + polymorphism; the >=2 patterns; late-penalty/extension/resubmission/moderation rules; SQLite via DAO + parameterised queries; salted password hashes; mark-change audit log; exceptions+validation; CMake; documented coding standard | None (no `src/`, `CMakeLists.txt`, or `.cpp`) | **OPEN** | Build the application | `src/`, `include/`, `database/`, `CMakeLists.txt` |
| P5 | Test strategy (V-Model levels, entry/exit, EP+BVA); automated unit+integration tests (GoogleTest/Catch2) via CI, >=70% business-logic line coverage (report actual); system+acceptance tests from use cases; defect log | None | **OPEN** | Build and **actually run**; record real results & coverage | `tests/`, `docs/testing/` |
| P6 | Git branching strategy + meaningful commits + tagged releases; CI (e.g. GitHub Actions) building+testing on every push; static analyser (cppcheck/clang-tidy) findings addressed; code metrics; self code-review record | Partial: Git repo exists with a meaningful initial commit; `.gitignore` present | **OPEN** (CM started) | Branching strategy doc, CI workflow, static analysis, metrics, review record | `.github/workflows/`, `docs/project-management/` |
| P7 | Deployment diagram; installation guide; per-role user manuals; release notes; tag **v1.0** | None | **OPEN** | Build; tag v1.0 only once the app really builds | `docs/deployment/`, `assets/diagrams/`, GitHub release |
| P8 | CR1 (class-wide extensions) classification+impact; implement as **v1.1**; regression tests | None | **OPEN** | After v1.0 exists | `docs/maintenance/`, GitHub release |
| PLE | Professional/legal/ethical analysis (Lesotho Data Protection Act 2012; privacy; security; fairness of late penalties; accessibility; accountability) | None | **OPEN** | Write with verified APA 7 citations | `docs/professional-practice/` |
| X | Requirements Traceability Matrix: FR -> use case -> design (class/diagram/table) -> source file -> test case (brief sec.6, mandatory) | None | **OPEN** | Build and keep in sync across all gates | `docs/` + report appendix |
| R | Final report + references + appendices | None | **OPEN** | Produce the 4–5 page report (condensed 14-section structure) linking to GitHub-hosted artefacts; consistent references; appendices (traceability matrix, full test results, user manuals, updated risk register) | `docs/final-report/` |

**Headline:** Phase 1 (10 marks) is done and banked. The remaining **90 marks** are OPEN.

---

## 4. Conflicts identified and owner decisions (settled)

> These were surfaced for decision; the owner's decisions are now recorded and final.

**Conflict #1 — Final report length (SETTLED by owner).**
The official brief (sec.7) specifies a single report of 6,000–8,000 words in 14 prescribed sections. The lecturer's **latest instruction is 4–5 pages**, and the owner has ruled it **supersedes** the 6,000–8,000-word requirement for the final report. *Caveat:* retain the lecturer's 4–5 page instruction **in writing** if available, as evidence it supersedes the brief. The report therefore follows the **concise-report + GitHub-evidence model**: a tight 4–5 page report that keeps the required 14-section structure in condensed form and links to the full SRS, design, test and evidence artefacts hosted in the repo. The full artefacts are still produced — the P2–P8 marks depend on the artefacts, not the page count.

**Conflict #2 — Mark weightings (resolve: official PDF governs).**
The summary table in the working brief shows V&V **15**, "Deployment and Maintenance" combined **6**, Ethics **4**. The official PDF shows V&V **12**, Deployment **4** + Maintenance **6** (separate), Ethics **3**. Both sum to 100 but allocate effort differently. *Adopting the official PDF scheme* (sec.2 above) unless you say otherwise.

**Conflict #3 — Submission package.**
Official PDF: one PDF report + Git link. Latest guidance: GitHub as the primary evidence store with a concise report. Reconciled under Decision #1; GitHub will hold all detailed evidence either way.

**Conflict #4 — Console vs simple desktop GUI (APPROVED: console).**
The brief allows "console or simple desktop GUI". The owner has **approved a C++17 console application with SQLite — no Qt GUI**. Priorities: correctness, role enforcement, business rules, persistence, auditability, testing, and a demonstrable workflow for the mandatory viva.

**Conflict #5 — Deadline (SETTLED).**
The final submission deadline is **2 November 2026**, per the latest lecturer update supplied by the owner (the brief is dated 7 Nov 2026 with "7 weeks allowed"; the 2 Nov date governs unless a later extension is received).

---

## 5. Stakeholder conflicts to resolve in Gate 1 (listed, not resolved here)

These are intrinsic to the case study and are Gate 1 (Requirements) work:
- **SC1 — Mark visibility:** Principal ("no mark visible until the HoD has checked it") vs Lecturer ("students get marks as soon as I have marked"). Governs the moderation/release workflow.
- **SC2 — Late work:** HoD ("-10% of the available mark per day late, up to 3 days, then zero") vs Student rep ("late work should not simply be rejected"). Reconcilable as a graduated penalty, not rejection.
- **SC3 — Resubmission & feedback:** Student rep ("fix a mistake and resubmit before the deadline"; "written feedback, not just a number") — largely additive requirements.
- **SC4 — Non-functional (IT officer):** offline operation; protected passwords; responsive "even with all 800 students". Drive measurable NFRs.

---

## 6. Governing constraints (apply to every subsequent gate)

- **Viva is mandatory** (15–20 min): every artefact and line of code must be genuinely explainable by the author. Nothing is built that the author cannot defend.
- **Fixed toolchain:** C++17, CMake, SQLite, Git + hosted repo, **GoogleTest or Catch2**, **cppcheck or clang-tidy**.
- **Hard technical rules:** offline / single machine or LAN; ~800 students; **salted password hashes** (no plaintext); **mark-change audit log** (who/when/old/new/why); **parameterised queries** through a **DAO layer only**; abstract `User` class with inheritance + polymorphism; **>=2 design patterns**; **ERD >=3NF**; exceptions + input validation.
- **Gate 2 lock:** implementation may not begin until the design passes Gate 2.
- **Consistency/traceability** is explicitly marked: code must follow the design, DB must match the class diagram, every FR must be tested.
- **Out of scope** (do not build): online payment, video conferencing, automated grading, plagiarism detection, web/mobile interfaces.

---

## 7. Proposed execution order and the one blocker

Order follows the brief's Waterfall gates: **G1 Requirements -> G2 Design (lock) -> G3 Implementation -> G4 V&V -> G5 CM/QA -> G6 Deployment (v1.0) -> G7 Maintenance (v1.1) -> G8 Report + Ethics + traceability**. CM/QA groundwork (CI skeleton) can begin in parallel once there is code to build.

**Blocking decisions:** none outstanding — report length (4–5 pages), application type (C++17 console + SQLite) and deadline (2 Nov 2026) are all settled. Gate 1 can proceed on the official brief once this baseline is accepted.

---

## 8. Baseline repository state (as of Gate 0)

- Branch `main`; initial commit `4d8abe5` (Phase 1 deliverables + README + .gitignore).
- No `src/`, `include/`, `tests/`, `database/`, `CMakeLists.txt`, CI workflow, or design docs yet.
- `.gitignore` already excludes build output, `*.exe`/`*.o`, and all `*.db`/`*.sqlite` so no binaries or learner databases will be committed.

**End of Gate 0 baseline.**
