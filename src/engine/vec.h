#ifndef GAME_VEC_H
#define GAME_VEC_H

#include <raylib.h>
#include <cmath>
#include <string>

#include "RandomGen.h"
#include "util.h"

struct Vec2ui {
    std::uint32_t x, y;

    [[nodiscard]] constexpr static Vec2ui zero() {
        return {0, 0};
    }

    constexpr bool operator==(const Vec2ui& other) const {
        return x == other.x && y == other.y;
    }

    constexpr Vec2ui operator+(const Vec2ui& other) const {
        return {
            x + other.x,
            y + other.y
        };
    }

    constexpr Vec2ui operator-(const Vec2ui& other) const {
        return {
            x - other.x,
            y - other.y
        };
    }

    [[nodiscard]] constexpr std::uint32_t distanceSquared(const Vec2ui& other) const {
        const std::int32_t xi = static_cast<std::int32_t>(other.x) - static_cast<std::int32_t>(x);
        const std::int32_t yi = static_cast<std::int32_t>(other.x) - static_cast<std::int32_t>(x);

        return xi*xi + yi*yi;
    }

    constexpr Vec2ui operator*(const float other) const {
        return {
            static_cast<std::uint32_t>(static_cast<float>(x) * other),
            static_cast<std::uint32_t>(static_cast<float>(y) * other)
        };
    }

    [[nodiscard]] std::string toString() const {
        return GameUtil::fmt("Vec2ui(x=%zu, y=%zu)", x, y);
    }

    constexpr Vec2ui operator%(const Vec2ui& other) const {
        return {x % other.x, y % other.y};
    }
};

struct Vec2 {
    float x, y;

    [[nodiscard]] static constexpr Vec2 zero() {
        return {0, 0};
    }

    [[nodiscard]] static constexpr Vec2 halves() {
        return {0.5f, 0.5f};
    }

    static constexpr Vec2 fromInts(const std::uint32_t x, const std::uint32_t y) {
        return { static_cast<float>(x), static_cast<float>(y) };
    }

    static constexpr Vec2 fromAngle(const float angle, const float magnitude) {
        const float x = std::cos(angle);
        const float y = std::sin(angle);

        // no normalization since the mag is already 1
        return Vec2{x, y} * magnitude;
    }

    static constexpr Vec2 randomDirection(const float magnitude) {
        const double angle = RandomGen::randomNormalized() * PI * 2;
        return fromAngle(static_cast<float>(angle), magnitude);
    }

    constexpr Vec2(const Vector2 other) {
        x = other.x;
        y = other.y;
    }

    constexpr Vec2(const float x, const float y) {
        this->x = x;
        this->y = y;
    }

    [[nodiscard]] constexpr int xInt() const {
        return static_cast<int>(x);
    }

    [[nodiscard]] constexpr int yInt() const {
        return static_cast<int>(y);
    }

    constexpr Vec2 operator+(const Vec2& b) const {
        return {x + b.x, y + b.y};
    }

    constexpr Vec2 operator+(const float b) const {
        return {x + b, y + b};
    }

    constexpr Vec2 operator-(const Vec2& b) const {
        return {x - b.x, y - b.y};
    }

    constexpr Vec2 operator*(const float b) const {
        return {x * b, y * b};
    }

    constexpr Vec2 operator*(const double b) const {
        return {x * static_cast<float>(b), y * static_cast<float>(b)};
    }

    constexpr Vec2 operator*(const int b) const {
        return {x * static_cast<float>(b), y * static_cast<float>(b)};
    }

    constexpr Vec2 operator*(const Vec2& b) const {
        return {x * b.x, y * b.y};
    }

    constexpr Vec2 operator/(const Vec2& b) const {
        return {x / b.x, y / b.y};
    }


    constexpr Vec2 operator+=(const Vec2& b) {
        x += b.x;
        y += b.y;
        return *this;
    }

    constexpr Vec2 operator%(const float other) const {
        return {std::fmod(x, other), std::fmod(y, other)};
    }

    [[nodiscard]] constexpr Vec2 clamp(const Vec2 min, const Vec2 max) const {
        return Vec2{
            GameUtil::clamp(x, min.x, max.x),
            GameUtil::clamp(y, min.y, max.y)
        };
    }

    [[nodiscard]] constexpr float toAngle() const {
        return std::atan2(y, x);
    }

    [[nodiscard]] constexpr float dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }

    [[nodiscard]] constexpr float magnitude() const {
       return std::sqrt(dot(*this)) ;
    }

    [[nodiscard]] constexpr float magnitudeSquared() const {
        return dot(*this);
    }

    [[nodiscard]] constexpr bool isApprox(const Vec2 other) const {
        return GameUtil::isApprox(x, other.x)
            && GameUtil::isApprox(y, other.y);
    }

    [[nodiscard]] constexpr bool isApproxZero() const {
        return isApprox(zero());
    }

    [[nodiscard]] constexpr Vec2 normalizeOrZero() const {
        const float mag = magnitude();
        if (mag == 0) return {0, 0};
        return {x/mag, y/mag};
    }

    [[nodiscard]] constexpr Vec2 moveTowards(const Vec2 target, const float delta) const {
        return {
            GameUtil::moveTowards(x, target.x, delta),
            GameUtil::moveTowards(y, target.y, delta),
        };
    }

    [[nodiscard]] constexpr Vec2 moveTowardsMagnitude(const float target, const float delta) const {
        // const float angle = toAngle();
        // const float mag = magnitude();
        //
        // return fromAngle(angle, GameUtil::moveTowards(mag, target, delta));
        return resize(GameUtil::lerp(magnitude(), target, delta));
    }

    [[nodiscard]] constexpr Vec2 lerp(const Vec2 target, const float delta) const {
        return {
            GameUtil::lerp(x, target.x, delta),
            GameUtil::lerp(y, target.y, delta),
        };
    }

    [[nodiscard]] constexpr Vec2 rotate(const float radians) const {
        return fromAngle(toAngle() + radians, magnitude()); // TODO: avoid atan2 call by doing the matrix multiplication
    }

    // Will not be able to resize of the current magnitude is 0
    [[nodiscard]] constexpr Vec2 resize(const float mag) const {
        const auto currentMag = magnitude();
        if (currentMag == 0) return {0, 0};

        return *this * (mag / currentMag);
    }

    [[nodiscard]] constexpr Vec2 round(const Vec2 toPlaceXY) const {
        return {
            std::round(x * toPlaceXY.x) / toPlaceXY.x,
            std::round(y * toPlaceXY.y) / toPlaceXY.y,
        };
    }

    constexpr explicit Vec2(const Vec2ui other) {
        x = static_cast<float>(other.x);
        y = static_cast<float>(other.y);
    }

    constexpr explicit operator Vec2ui() const {
        return {static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y)};
    }

    constexpr operator Vector2() const {
        return toRaylibVector2();
    }

    [[nodiscard]] constexpr Vector2 toRaylibVector2() const {
        return Vector2{x, y};
    }

    [[nodiscard]] constexpr std::string toString() const {
        return GameUtil::fmt("Vec2(x=%f, y=%f)", x, y);
    }
};

struct PercentVec2 { // todo: corner and more (maybe wont use this)
    float px, py;

    [[nodiscard]] Vec2 intoVec2(const int resX, const int resY) const {
        float x = static_cast<float>(resX) / px;
        float y = static_cast<float>(resY) / py;

        return {x, y};
    }
};

#endif //GAME_VEC_H