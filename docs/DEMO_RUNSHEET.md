# Demonstration Run-Sheet (10–12 minutes)

Build once before recording: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`.
Start each role with `./build/lms demo.db` (delete `demo.db` first for a clean run).

1. **Intro (1 min).** State the problem: a moderated LMS where students never see a mark before the HoD releases it. Show the repository and the layered `src/` tree.
2. **First run + Admin (2 min).** Create the initial administrator. Create a department, a lecturer, a HoD (in that department), a student; a course; assign the lecturer; enrol the student. Explain role-based access.
3. **Lecturer (2 min).** Create an assignment with a near deadline. Grant the class a short extension (CR1) and explain the effective-deadline rule. Point out that a lecturer can only touch their own courses.
4. **Student (1.5 min).** List assignments (show the extended deadline). Submit work on time. Try to view the result — show it is not visible yet (moderation rule).
5. **Lecturer marks (1.5 min).** Mark the submission with feedback; show the awarded mark and that an initial audit event is written. Demonstrate a re-mark requiring a reason.
6. **HoD moderates + releases (2 min).** Show the moderation queue; adjust the mark with a reason (moderation audit event); release the assignment. Explain that only moderated submissions are released, atomically.
7. **Student sees result (0.5 min).** Now the released, moderated mark and feedback are visible.
8. **Evidence (1.5 min).** Run `ctest -C Release --output-on-failure` (38 cases / 112 assertions). Show the GitHub Actions CI run (green on Ubuntu + Windows), the cppcheck-clean QA report, and the append-only audit guarantee.
9. **Design decisions (as asked).** Patterns (Strategy/Repository/Factory), atomic transactions, Argon2id and the libsodium deviation, service-layer authorisation.
