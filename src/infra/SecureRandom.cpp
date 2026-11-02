#include "infra/SecureRandom.hpp"

#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
namespace lms {
void secureRandomBytes(unsigned char* buf, std::size_t n) {
    const NTSTATUS status =
        BCryptGenRandom(nullptr, buf, static_cast<ULONG>(n), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status != 0)
        throw std::runtime_error("BCryptGenRandom failed to produce secure random bytes");
}
} // namespace lms
#else
#include <cstdio>
namespace lms {
void secureRandomBytes(unsigned char* buf, std::size_t n) {
    std::FILE* source = std::fopen("/dev/urandom", "rb");
    if (!source) throw std::runtime_error("cannot open /dev/urandom for secure randomness");
    const std::size_t read = std::fread(buf, 1, n, source);
    std::fclose(source);
    if (read != n) throw std::runtime_error("short read from /dev/urandom");
}
} // namespace lms
#endif
