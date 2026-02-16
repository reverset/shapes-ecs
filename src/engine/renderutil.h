#ifndef GAME_RENDERUTIL_H
#define GAME_RENDERUTIL_H

#include "raylib.h"
#include "vec.h"

namespace RenderUtil {
    inline void DrawObtuseTriangleFacing(const Vec2 center, const Vec2 direction, const float scale, const Color color) {
        const float halfScale = scale * 0.5f;

        const auto p1 = center + (direction * halfScale);

        const auto p2 = center + direction.rotate(GameUtil::PI_SIXTH * 2) * halfScale;
        const auto p3 = center + direction.rotate(-GameUtil::PI_SIXTH * 2) * halfScale;


        // DrawCircleV(p1, 1.0f, BLUE);
        // DrawCircleV(p2, 1.0f, BLUE);
        // DrawCircleV(p3, 1.0f, BLUE);
        DrawTriangle(p1, p3, p2, color);
    }
}

#endif //GAME_RENDERUTIL_H