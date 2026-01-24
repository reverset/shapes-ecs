#ifndef GAME_VEC_H
#define GAME_VEC_H

#include <raylib.h>
#include <cmath>
#include <string>

#include "RandomGen.h"
#include "util.h"

struct Vec2 {
    float x, y;

    static Vec2 fromAngle(const float angle, const float magnitude) {
        const float x = std::cos(angle);
        const float y = std::sin(angle);

        const Vec2 res = {x, y};
        const float desiredMag = magnitude / res.magnitude(); // normalize then multiply by magnitude
        return res * desiredMag;
    }

    static Vec2 randomDirection(const float magnitude) {
        const double angle = RandomGen::randomNormalized() * PI * 2;
        return fromAngle(static_cast<float>(angle), magnitude);
    }

    Vec2(const Vector2 other) {
        x = other.x;
        y = other.y;
    }

    Vec2(const float x, const float y) {
        this->x = x;
        this->y = y;
    }

    [[nodiscard]] int xInt() const {
        return static_cast<int>(x);
    }

    [[nodiscard]] int yInt() const {
        return static_cast<int>(y);
    }

    Vec2 operator+(const Vec2& b) const {
        return {x + b.x, y + b.y};
    }

    Vec2 operator+(const float b) const {
        return {x + b, y + b};
    }

    Vec2 operator-(const Vec2& b) const {
        return {x - b.x, y - b.y};
    }

    Vec2 operator*(const float b) const {
        return {x * b, y * b};
    }

    Vec2 operator*(const Vec2& b) const {
        return {x * b.x, y * b.y};
    }

    Vec2 operator+=(const Vec2& b) {
        x += b.x;
        y += b.y;
        return *this;
    }

    [[nodiscard]] float toAngle() const {
        return std::atan2(y, x);
    }

    [[nodiscard]] float dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }

    [[nodiscard]] float magnitude() const {
       return std::sqrt(dot(*this)) ;
    }

    [[nodiscard]] Vec2 normalizeOrZero() const {
        const float mag = magnitude();
        if (mag == 0) return {0, 0};
        return {x/mag, y/mag};
    }

    [[nodiscard]] Vec2 moveTowards(const Vec2 target, const float delta) const {
        return {
            GameUtil::moveTowards(x, target.x, delta),
            GameUtil::moveTowards(y, target.y, delta),
        };
    }

    [[nodiscard]] Vec2 moveTowardsMagnitude(const float target, const float delta) const {
        const float angle = toAngle();
        const float mag = magnitude();

        return fromAngle(angle, GameUtil::moveTowards(mag, target, delta));
    }

    operator Vector2() const {
        return toRaylibVector2();
    }

    [[nodiscard]] Vector2 toRaylibVector2() const {
        return Vector2{x, y};
    }

    [[nodiscard]] std::string toString() const {
        return GameUtil::string_format("Vec2(x=%f, y=%f)", x, y);
    }
};


const Vec2 VEC2_ZERO = {0, 0};

struct PercentVec2 { // todo: corner and more (maybe wont use this)
    float px, py;

    [[nodiscard]] Vec2 intoVec2(const int resX, const int resY) const {
        float x = static_cast<float>(resX) / px;
        float y = static_cast<float>(resY) / py;

        return {x, y};
    }
};

#endif //GAME_VEC_H