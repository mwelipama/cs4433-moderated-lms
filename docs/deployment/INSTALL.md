# Installation and Build Guide

The Moderated LMS runs on a single computer, entirely offline. It has no server,
database service or network dependency to install — SQLite and the Argon2id password
hasher are compiled into the program from vendored sources.

## Prerequisites
- CMake 3.16 or newer
- A C++17 compiler: GCC 9+, Clang 10+, or MSVC (Visual Studio 2019+)
- Git (to clone the repository)

No other libraries are required.

## Build
```
git clone https://github.com/mwelipama/cs4433-moderated-lms
cd cs4433-moderated-lms
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
This produces the application `build/lms` (and the test runner `build/lms_tests`).

## First run and administrator setup
```
./build/lms            # creates ./lms.db in the current directory
```
On the first run against a new database the program applies its schema and prompts
you to create the initial administrator account (username, display name and a
password of at least eight characters). Log in as that administrator to create
departments, users, courses and enrolments.

To keep the database somewhere specific, pass a path:
```
./build/lms /path/to/school-lms.db
```

## Backup
The entire system state is the single SQLite database file (`lms.db` by default).
Backing up the system is copying that one file while the program is not running.
Keep regular copies; there is nothing else to back up.

## Platforms
Because every dependency is vendored, the same source builds and runs on Windows,
Linux and macOS. Continuous integration builds and tests the project on Ubuntu and
Windows on every change.

## Tests
```
cd build
ctest -C Release --output-on-failure
```
