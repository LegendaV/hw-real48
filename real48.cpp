#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <cmath>
#include "real48.hpp"

namespace math {

static inline void store_u48_be(unsigned char out[6], uint64_t v48) {
    out[0] = (v48 >> 40) & 0xFF;
    out[1] = (v48 >> 32) & 0xFF;
    out[2] = (v48 >> 24) & 0xFF;
    out[3] = (v48 >> 16) & 0xFF;
    out[4] = (v48 >>  8) & 0xFF;
    out[5] = (v48 >>  0) & 0xFF;
}

static inline uint64_t load_u48_be(const unsigned char in[6]) {
    uint64_t v = 0;
    v |= uint64_t(in[0]) << 40;
    v |= uint64_t(in[1]) << 32;
    v |= uint64_t(in[2]) << 24;
    v |= uint64_t(in[3]) << 16;
    v |= uint64_t(in[4]) <<  8;
    v |= uint64_t(in[5]) <<  0;
    return v;
}

Real48::Real48(const float number) {
    if (number == 0.0f) {
        std::memset(real48, 0, 6);
        return;
    }
    if (!std::isfinite(number)) {
        throw std::overflow_error("float is not finite");
    }

    uint32_t bits = 0;
    std::memcpy(&bits, &number, sizeof(bits));

    const uint64_t sFloat = (bits >> 31) & 1u;
    const uint64_t eFloat = (bits >> 23) & 0xFFu;
    const uint64_t fFloat = bits & ((1u << 23) - 1u);

    if (eFloat == 0) {
        std::memset(real48, 0, 6);
        return;
    }
    if (eFloat == 0xFF) {
        throw std::overflow_error("float inf/nan");
    }

    const uint64_t e = eFloat + 2;
    if (e < 1 || e > 255) {
        throw std::overflow_error("float exponent out of range");
    }

    const uint64_t f = (fFloat << 16) & ((1ULL << 39) - 1);

    const uint64_t v48 = (sFloat << 47) | (f << 8) | (e & 0xFF);
    store_u48_be(real48, v48);
}

Real48::Real48(const double number) {
    if (number == 0.0) {
        std::memset(real48, 0, 6);
        return;
    }
    if (!std::isfinite(number)) {
        throw std::overflow_error("double is not finite");
    }

    uint64_t bits = 0;
    std::memcpy(&bits, &number, sizeof(bits));

    const uint64_t sDouble = (bits >> 63) & 1ULL;
    const uint64_t eDouble = (bits >> 52) & 0x7FFULL;
    const uint64_t fDouble = bits & ((1ULL << 52) - 1);

    if (eDouble == 0) {
        std::memset(real48, 0, 6);
        return;
    }
    if (eDouble == 0x7FF) {
        throw std::overflow_error("double inf/nan");
    }

    const uint64_t e = eDouble - 894;
    if (e < 1 || e > 255) {
        throw std::overflow_error("double exponent out of range");
    }

    const uint64_t f = (fDouble >> 13) & ((1ULL << 39) - 1);

    const uint64_t v48 = (sDouble << 47) | (f << 8) | (e & 0xFF);
    store_u48_be(real48, v48);
}

Real48::operator float() const {
    const uint64_t sign = s() ? 1ULL : 0ULL;
    const uint64_t exp  = e();
    const uint64_t frac = f();

    if (exp == 0) return 0.0f;

    const uint64_t eFloat = exp - 2;
    if (eFloat == 0 || eFloat >= 0xFF) {
        throw std::overflow_error("cannot convert to float");
    }

    const uint32_t bits =
        (uint32_t(sign) << 31) |
        (uint32_t(eFloat) << 23) |
        uint32_t((frac >> 16) & ((1ULL << 23) - 1));

    float out;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

Real48::operator double() const noexcept {
    const uint64_t sign = s() ? 1ULL : 0ULL;
    const uint64_t exp  = e();
    const uint64_t frac = f();

    if (exp == 0) return 0.0;

    const uint64_t eDouble = exp + 894;

    const uint64_t bits =
        (sign << 63) |
        (eDouble << 52) |
        ((frac & ((1ULL << 39) - 1)) << 13);

    double out;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

Real48 Real48::operator-() const noexcept {
    Real48 r{*this};
    r.real48[0] ^= 0x80;
    return r;
}

bool Real48::s() const {
    return (real48[0] & 0x80) != 0;
}

uint8_t Real48::e() const {
    return static_cast<uint8_t>(real48[5]);
}

uint64_t Real48::f() const {
    const uint64_t v48 = load_u48_be(real48);
    return (v48 >> 8) & ((1ULL << 39) - 1);
}

} // namespace math