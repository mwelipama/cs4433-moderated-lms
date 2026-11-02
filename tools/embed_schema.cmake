# Wrap database/schema.sql into a C++ header exposing it as a raw string literal,
# so the application carries its own schema and needs no schema file at runtime.
# Invoked by the build: cmake -DIN=<schema.sql> -DOUT=<header> -P embed_schema.cmake
file(READ "${IN}" _schema)
file(WRITE "${OUT}"
"#pragma once
// Generated from database/schema.sql. Do not edit by hand; edit the .sql and rebuild.
namespace lms {
inline const char* const EMBEDDED_SCHEMA_SQL = R\"LMSSCHEMA(
${_schema})LMSSCHEMA\";
} // namespace lms
")
