#pragma once
#include <cstddef>

namespace lms {

// Fills `buf` with `n` cryptographically secure random bytes drawn from the operating
// system's CSPRNG (BCryptGenRandom on Windows, /dev/urandom on POSIX). This is used for
// password salts, where std::random_device is not a safe choice because the C++ standard
// does not require it to be non-deterministic on every platform. Throws on failure.
void secureRandomBytes(unsigned char* buf, std::size_t n);

} // namespace lms
