#version 330

uniform float gameTime;

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

const vec3 PINK = vec3(1.0, 0.4117, 0.705);

const float PI = 3.14159265358979323846264338;
const float TWO_PI = PI * 2;


// there is probably a better and faster way to do this
vec2 sineCircle(float radius, float waves, float amplitude, float t) {
    return vec2(
        (radius + amplitude * sin(waves * t))*cos(t),
        (radius + amplitude * sin(waves * t))*sin(t)
    );
}

bool isPointWithinCircle(vec2 point) {
    const float THRESHOLD = 0.01;

    for (float i = 0; i < TWO_PI; i += 0.04) {
        vec2 test = sineCircle(0.35 + 0.04 * cos(5 * gameTime), 10, 0.02 + 0.01*cos(10 * gameTime), i);

        if (length(point - test) <= THRESHOLD) {
            return true;
        }
    }

    return false;
}

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);

    const vec2 center = vec2(0.51, 0.47);
    // not perfect center since the sprite is slightly offset
    // the sprite is 16x16, and the render texture is 32x32.

    vec2 d = fragTexCoord - center;

    float distance = length(d);

    finalColor = texelColor * colDiffuse * fragColor;

    if (isPointWithinCircle(fragTexCoord - center)) {
        finalColor = vec4(PINK, abs(cos(2 * gameTime)) + 0.5);
    }
}
