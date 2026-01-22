#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace math
{

class Real48
{
public:
    // constructors
    constexpr Real48(): real48{} {};
    Real48(const float number);
    Real48(const double number);
    constexpr Real48(const Real48& o) = default;

    // conversion operators
    operator float() const;
    operator double() const noexcept;

    // assignment operators
    Real48& operator=(const Real48& b) noexcept = default;
    Real48& operator+=(const Real48& b);
    Real48& operator-=(const Real48& b);
    Real48& operator*=(const Real48& b);
    Real48& operator/=(const Real48& b);

    // arithmetic operators
    Real48 operator+() const noexcept;
    Real48 operator-() const noexcept;
    Real48 operator+(const Real48& o) const;
    Real48 operator-(const Real48& o) const;
    Real48 operator*(const Real48& o) const;
    Real48 operator/(const Real48& o) const;

    // comparison operators
    bool operator>(const Real48& o) const noexcept;
    bool operator<(const Real48& o) const noexcept;

    // classify
    enum class Class
    {
        NORMAL,
        ZERO
    };
    Class Classify() const noexcept;

    // limits
    consteval static Real48 min()
    {
        Real48 m{};
        m.real48[5] = 1;
        return m;
    }

    consteval static Real48 max()
    {
        Real48 m{};
        m.real48[0] = 0x7F;
        m.real48[1] = 0xFF;
        m.real48[2] = 0xFF;
        m.real48[3] = 0xFF;
        m.real48[4] = 0xFF;
        m.real48[5] = 0xFF;
        return t;
    }

    consteval static Real48 epsilon()
    {
        Real48 eps;
        eps.real48[5] = 90;
        return eps;
    }

private:
    unsigned char real48[6];
        
    bool s() const;

    uint8_t e() const;

    uint64_t f() const;
};

} // namespace math
