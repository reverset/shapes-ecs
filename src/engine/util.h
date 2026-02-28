#ifndef GAME_UTIL_H
#define GAME_UTIL_H

#include <vector>
#include <functional>
#include <iostream>
#include <utility>
#include <memory>
#include <string>
#include <stdexcept>
#include <cmath>

#include "raylib.h"

namespace GameUtil {
    constexpr float PI_FOURTH = PI / 4.0f;
    constexpr float PI_SIXTH = PI / 6.0f;

    template <typename T>
    void insertSorted(std::vector<T>* list, T&& item, const std::function<int(const T&, const T&)>& comparator) {
        for (size_t i = 0; i < list->size(); i++) {
            if (comparator(list->at(i), item) >= 0) {
                list->insert(list->begin() + i, std::forward<T>(item));
                return;
            }
        }
        list->push_back(std::forward<T>(item));
    }


    // https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    template<typename ... Args>
    [[nodiscard]] std::string fmt( const std::string& format, Args ... args ) {
        int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
        if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
        auto size = static_cast<size_t>( size_s );
        std::unique_ptr<char[]> buf( new char[ size ] );
        std::snprintf( buf.get(), size, format.c_str(), args ... );
        return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
    }


    template <typename T>
    [[nodiscard]] constexpr T clamp(const T val, const T min, const T max) {
        return std::min(std::max(val, min), max);
    }

    template <typename T>
    [[nodiscard]] constexpr T moveTowards(const T val, const T target, const T delta) {
        T actualDelta = std::min(delta, std::abs(target - val));
        return val + std::copysign(actualDelta, target - val);
    }

    template <typename T>
    [[nodiscard]] constexpr T lerp(const T val, const T target, const T delta) {
        return val + (target - val) * delta;
    }

    template <typename T>
    [[nodiscard]] constexpr bool isApprox(const T lhs, const T rhs, float epsilon = 0.000001f) {
        return std::abs(lhs - rhs) < epsilon;
    }

    [[nodiscard]] inline Rectangle centeredRect(const float x, const float y, const float w, const float h) {
        return {
            x - w*0.5f,
            y - h*0.5f,
            w,
            h,
        };
    }

    // copied from raylib, just made it constexpr
    [[nodiscard]] constexpr Color fade1(const Color color, float alpha)
    {
        Color result = color;

        if (alpha < 0.0f) alpha = 0.0f;
        else if (alpha > 1.0f) alpha = 1.0f;

        result.a = static_cast<unsigned char>(255.0f * alpha);

        return result;
    }

    [[nodiscard]] constexpr Color colorLerp(const Color a, const Color b, float delta) {
        return {
            static_cast<unsigned char>(lerp(static_cast<float>(a.r), static_cast<float>(b.r), delta)),
            static_cast<unsigned char>(lerp(static_cast<float>(a.g), static_cast<float>(b.g), delta)),
            static_cast<unsigned char>(lerp(static_cast<float>(a.b), static_cast<float>(b.b), delta)),
            static_cast<unsigned char>(lerp(static_cast<float>(a.a), static_cast<float>(b.a), delta)),
        };
    }

    [[nodiscard]] constexpr std::string repeatStr(const std::string_view desired, const std::size_t amount) {
        std::string result;
        result.reserve(desired.size() * amount);

        for (std::size_t i = 0; i < amount; i++) {
            result.append(desired);
        }
        return result;
    }
}

#endif //GAME_UTIL_H