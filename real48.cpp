#include <cstdint>
#include <stdexcept>
#include <cstring>
#include "real48.hpp"

namespace math
{
Real48::Real48(const float number)
{
    uint64_t bits;

    std::memcpy(&bits, &number, sizeof(float));
            
    const uint64_t sFloat = bits >> 31;
    const uint64_t eFloat = bits >> 23 & ((1 << 8) - 1);
    const uint64_t fFloat = bits & ((1 << 23) - 1);

    if (eFloat == 0)
    {
        real48[5] = 0;
        return;
    }

    const uint64_t e = eFloat + 2;

    if (e < 1 || e > 255) {
        throw std::overflow_error("e < 1 || e > 255");
    }

    uint64_t result = 0;
    result |= (sFloat & 1ULL);
    result |= (fFloat & ((1ULL << 39) - 1)) << 1;
    result |= (e & 0xFFULL) << 40;

    std::memcpy(real48, &result, 6);
}

Real48::Real48(const double number)
{
    uint64_t bits;

    std::memcpy(&bits, &number, sizeof(float));

    const uint64_t sDouble = bits >> 63;
    const uint64_t eDouble = bits >> 52 & ((1 << 11) - 1);
    const uint64_t fDouble = bits & ((1ll << 52) - 1);

    if (eDouble == 0)
    {
        real48[5] = 0;
        return;
    }

    const uint64_t e = eDouble - 894;

    if (e < 1 || e > 255) {
        throw std::overflow_error("e < 1 || e > 255");
    }

    uint64_t result = 0;
    result |= (sDouble & 1ULL);
    result |= (fDouble & ((1ULL << 39) - 1)) << 1;
    result |= (e & 0xFFULL) << 40;

    std::memcpy(real48, &result, 6);
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
    return result;
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
    result |= (s_ << 63);
    result |= ((e_ + 894) << 52);
    result |= (f_ >> 13);

    double resultDouble;
    std::memcpy(&resultDouble, &result, sizeof(double));
    return result;
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

Real48 Real48::operator-() const noexcept;

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

Class Real48::Classify() const noexcept
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
