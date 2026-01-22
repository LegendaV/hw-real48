#include <cstdint>
#include <stdexcept>
#include <cstring>
#include "real48.hpp"

namespace math
{
Real48::Real48(const float number)
{
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

    const uint64_t f = (uint64_t(fFloat) << 16) & ((1ULL << 39) - 1);

    const uint64_t v48 = (sFloat << 47) | (f << 8) | (e & 0xFFULL);

    real48[0] = (v48 >> 40) & 0xFF;
    real48[1] = (v48 >> 32) & 0xFF;
    real48[2] = (v48 >> 24) & 0xFF;
    real48[3] = (v48 >> 16) & 0xFF;
    real48[4] = (v48 >>  8) & 0xFF;
    real48[5] = (v48 >>  0) & 0xFF;
}

Real48::Real48(const double number)
{
    uint64_t bits = 0;
    std::memcpy(&bits, &number, sizeof(bits));

    const uint64_t sDouble = (bits >> 63) & 1ULL;
    const uint64_t eDouble = (bits >> 52) & 0x7FFULL;
    const uint64_t fDouble = bits & ((1ULL << 52) - 1ULL);

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

    const uint64_t f = (fDouble >> 13) & ((1ULL << 39) - 1ULL);

    const uint64_t v48 = (sDouble << 47) | (f << 8) | (e & 0xFFULL);

    real48[0] = (v48 >> 40) & 0xFF;
    real48[1] = (v48 >> 32) & 0xFF;
    real48[2] = (v48 >> 24) & 0xFF;
    real48[3] = (v48 >> 16) & 0xFF;
    real48[4] = (v48 >>  8) & 0xFF;
    real48[5] = (v48 >>  0) & 0xFF;
}

// conversion operators
Real48::operator float() const
{
    auto s_ = s();
    auto e_ = e();
    auto f_ = f();

    if (e_ == 0)
    {
        return 0;
    }

    if (e_ <= 1)
    {
        throw std::runtime_error("e_ <= 1");
    }

    uint64_t result = 0;
    result |= (s_ << 31);
    result |= ((e_ - 2) << 23);
    result |= (f_ >> 16);

    float resultFloat;
    std::memcpy(&resultFloat, &result, sizeof(float));
    return resultFloat;
}

Real48::operator double() const noexcept
{
    auto s_ = s();
    auto e_ = e();
    auto f_ = f();

    if (e_ == 0)
    {
        return 0;
    }

    uint64_t result = 0;
    result |= (uint64_t(s_) << 63);
    result |= ((uint64_t(e_) + 894) << 52);
    result |= (f_ << 13);

    double resultDouble;
    std::memcpy(&resultDouble, &result, sizeof(double));
    return resultDouble;
}

// assignment operators
Real48& Real48::operator+=(const Real48& b)
{
    auto doubleResult = static_cast<double>(*this) + static_cast<double>(b);
    *this = Real48(doubleResult);
    return *this;
}

Real48& Real48::operator-=(const Real48& b)
{
    auto doubleResult = static_cast<double>(*this) - static_cast<double>(b);
    *this = Real48(doubleResult);
    return *this;
}

Real48& Real48::operator*=(const Real48& b)
{
    auto doubleResult = static_cast<double>(*this) * static_cast<double>(b);
    *this = Real48(doubleResult);
    return *this;
}

Real48& Real48::operator/=(const Real48& b)
{
    auto doubleResult = static_cast<double>(*this) / static_cast<double>(b);
    *this = Real48(doubleResult);
    return *this;
}

// arithmetic operators
Real48 Real48::operator+() const noexcept
{
    return *this;
}

Real48 Real48::operator-() const noexcept
{
    Real48 result{*this};
    result.real48[0] ^= 0x80;
    return result;
}

Real48 Real48::operator+(const Real48& o) const
{
    auto doubleResult = static_cast<double>(*this) + static_cast<double>(o);
    return Real48(doubleResult);
}

Real48 Real48::operator-(const Real48& o) const
{
    auto doubleResult = static_cast<double>(*this) - static_cast<double>(o);
    return Real48(doubleResult);
}

Real48 Real48::operator*(const Real48& o) const
{
    auto doubleResult = static_cast<double>(*this) * static_cast<double>(o);
    return Real48(doubleResult);
}

Real48 Real48::operator/(const Real48& o) const
{
    auto doubleResult = static_cast<double>(*this) / static_cast<double>(o);
    return Real48(doubleResult);
}

// comparison operators
bool Real48::operator>(const Real48& o) const noexcept
{
    return (static_cast<double>(*this) > static_cast<double>(o));
}

bool Real48::operator<(const Real48& o) const noexcept
{
    return (static_cast<double>(*this) < static_cast<double>(o));
}

Real48::Class Real48::Classify() const noexcept
{
    if (e() == 0)
    {
        return Class::ZERO;
    }
    return Class::NORMAL;
}

bool Real48::s() const
    {
        return (real48[0] >> 7) & 1;
    }

    uint8_t Real48::e() const
    {
        return static_cast<uint8_t>(real48[5]);
    }

    uint64_t Real48::f() const
    {
        uint64_t f = 0;
        for (int i = 0; i < 6; ++i)
            f = (f << 8) | real48[i];

        return (f >> 8) & ((1ULL << 39) - 1);
    }
} // namespace math
