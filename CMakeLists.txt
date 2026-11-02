cmake_minimum_required(VERSION 3.16)
project(moderated_lms VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# --- vendored SQLite (compiled as C, no external dependency) ---
add_library(sqlite3 STATIC third_party/sqlite/sqlite3.c)
target_include_directories(sqlite3 PUBLIC third_party/sqlite)
target_compile_definitions(sqlite3 PRIVATE
  SQLITE_THREADSAFE=1
  SQLITE_DEFAULT_FOREIGN_KEYS=1
  SQLITE_OMIT_LOAD_EXTENSION
  SQLITE_DQS=0)
if(NOT WIN32)
  find_package(Threads REQUIRED)
  target_link_libraries(sqlite3 PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
endif()

# --- vendored Argon2id reference implementation (compiled as C) ---
add_library(argon2 STATIC
  third_party/argon2/argon2.c
  third_party/argon2/core.c
  third_party/argon2/encoding.c
  third_party/argon2/ref.c
  third_party/argon2/thread.c
  third_party/argon2/blake2/blake2b.c)
target_include_directories(argon2 PUBLIC third_party/argon2)
target_compile_definitions(argon2 PRIVATE ARGON2_NO_THREADS)

# --- embed the database schema into a generated header ---
set(SCHEMA_SRC ${CMAKE_SOURCE_DIR}/database/schema.sql)
set(GEN_DIR ${CMAKE_BINARY_DIR}/generated)
set(SCHEMA_HDR ${GEN_DIR}/schema_embedded.hpp)
add_custom_command(
  OUTPUT ${SCHEMA_HDR}
  COMMAND ${CMAKE_COMMAND} -DIN=${SCHEMA_SRC} -DOUT=${SCHEMA_HDR} -P ${CMAKE_SOURCE_DIR}/tools/embed_schema.cmake
  DEPENDS ${SCHEMA_SRC} ${CMAKE_SOURCE_DIR}/tools/embed_schema.cmake
  COMMENT "Embedding database/schema.sql")
add_custom_target(generate_schema DEPENDS ${SCHEMA_HDR})

# --- core library: all business logic, no entry point ---
file(GLOB_RECURSE LMS_CORE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM LMS_CORE_SOURCES ${CMAKE_SOURCE_DIR}/src/app/main.cpp)
add_library(lms_core STATIC ${LMS_CORE_SOURCES} ${SCHEMA_HDR})
add_dependencies(lms_core generate_schema)
target_include_directories(lms_core PUBLIC src ${GEN_DIR})
target_link_libraries(lms_core PUBLIC sqlite3 argon2)
if(MSVC)
  target_compile_options(lms_core PRIVATE /W4 /EHsc)
else()
  target_compile_options(lms_core PRIVATE -Wall -Wextra)
endif()

# --- console application ---
add_executable(lms src/app/main.cpp)
target_link_libraries(lms PRIVATE lms_core)

# --- automated tests ---
enable_testing()
file(GLOB_RECURSE LMS_TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
add_executable(lms_tests ${LMS_TEST_SOURCES})
target_include_directories(lms_tests PRIVATE third_party/doctest)
target_link_libraries(lms_tests PRIVATE lms_core)
add_test(NAME lms_unit_integration COMMAND lms_tests)
