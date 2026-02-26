#version 330

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
    const float THRESHOLD = 0.02;

    for (float i = 0; i < TWO_PI; i += 0.04) {
        vec2 test = sineCircle(0.2, 10, 0.02, i);

        if (length(point - test) <= THRESHOLD) {
            return true;
        }
    }

    return false;
}

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);

    const vec2 center = vec2(0.51, 0.49);

    vec2 d = fragTexCoord - center;
    // not perfect center since the sprite is slightly offset
    // the sprite is 16x16, and the render texture is 32x32.

    float distance = length(d);

    float innerRadius = 0.2;

    float outerRadius = innerRadius + 0.01;

    finalColor = texelColor * colDiffuse * fragColor;
    // if (distance >= innerRadius && distance <= outerRadius) {
    //     finalColor = vec4(PINK, 1.0);
    // }
    if (isPointWithinCircle(fragTexCoord - vec2(0.51, 0.49))) {
        finalColor = vec4(PINK, 1.0);
    }
}
