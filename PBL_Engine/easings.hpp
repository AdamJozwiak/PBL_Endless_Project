#pragma once

// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include <cmath>

// ////////////////////////////////////////////////////////////////// Easings //
float interpolate(float (*easing)(float const), float const a, float const b,
                  float const alpha, float const deltaTime) {
    return std::lerp(a, b, easing(1.0f - std::pow(alpha, deltaTime)));
}

// ------------------------------------------------------------------ Sine -- //
float easeOutSine(float const x) { return sin((x * M_PI) / 2.0f); }
float easeInSine(float const x) { return 1.0f - std::cos((x * M_PI) / 2.0f); }
float easeInOutSine(float const x) {
    return -(std::cos(M_PI * x) - 1.0f) / 2.0f;
}

// ------------------------------------------------------------------ Quad -- //
float easeOutQuad(float const x) { return 1.0f - (1.0f - x) * (1.0f - x); }
float easeInQuad(float const x) { return x * x; }
float easeInOutQuad(float const x) {
    return x < 0.5f ? 2.0f * x * x
                    : 1.0f - std::pow(-2.0f * x + 2.0f, 2.0f) / 2.0f;
}

// ----------------------------------------------------------------- Cubic -- //
float easeOutCubic(float const x) { return 1.0f - std::pow(1.0f - x, 3.0f); }
float easeInCubic(float const x) { return x * x * x; }
float easeInOutCubic(float const x) {
    return x < 0.5f ? 4.0f * x * x * x
                    : 1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f;
}

// ----------------------------------------------------------------- Quart -- //
float easeOutQuart(float const x) { return 1.0f - std::pow(1.0f - x, 4.0f); }
float easeInQuart(float const x) { return x * x * x * x; }
float easeInOutQuart(float const x) {
    return x < 0.5f ? 8.0f * x * x * x * x
                    : 1.0f - std::pow(-2.0f * x + 2.0f, 4.0f) / 2.0f;
}

// ----------------------------------------------------------------- Quint -- //
float easeOutQuint(float const x) { return 1.0f - std::pow(1.0f - x, 5.0f); }
float easeInQuint(float const x) { return x * x * x * x * x; }
float easeInOutQuint(float const x) {
    return x < 0.5f ? 16.0f * x * x * x * x * x
                    : 1.0f - std::pow(-2.0f * x + 2.0f, 5.0f) / 2.0f;
}

// ------------------------------------------------------------------ Expo -- //
float easeOutExpo(float const x) {
    return x == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * x);
}
float easeInExpo(float const x) {
    return x == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * x - 10.0f);
}
float easeInOutExpo(float const x) {
    return x == 0.0f
               ? 0.0f
               : x == 1.0f
                     ? 1.0f
                     : x < 0.5f
                           ? std::pow(2.0f, 20.0f * x - 10.0f) / 2.0f
                           : (2.0f - std::pow(2.0f, -20.0f * x + 10.0f)) / 2.0f;
}

// ------------------------------------------------------------------ Circ -- //
float easeOutCirc(float const x) {
    return std::sqrt(1.0f - std::pow(x - 1.0f, 2.0f));
}
float easeInCirc(float const x) {
    return 1.0f - std::sqrt(1.0f - std::pow(x, 2.0f));
}
float easeInOutCirc(float const x) {
    return x < 0.5f
               ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * x, 2.0f))) / 2.0f
               : (std::sqrt(1.0f - std::pow(-2.0f * x + 2.0f, 2.0f)) + 1.0f) /
                     2.0f;
}

// ------------------------------------------------------------------ Back -- //
float easeOutBack(float const x) {
    float const c1 = 1.70158f;
    float const c3 = c1 + 1.0f;

    return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
}
float easeInBack(float const x) {
    float const c1 = 1.70158f;
    float const c3 = c1 + 1.0f;

    return c3 * x * x * x - c1 * x * x;
}
float easeInOutBack(float const x) {
    float const c1 = 1.70158f;
    float const c2 = c1 * 1.525f;

    return x < 0.5f
               ? (std::pow(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) /
                     2.0f
               : (std::pow(2.0f * x - 2.0f, 2.0f) *
                      ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) +
                  2.0f) /
                     2.0f;
}

// --------------------------------------------------------------- Elastic -- //
float easeOutElastic(float const x) {
    float const c4 = (2.0f * M_PI) / 3.0f;

    return x == 0.0f ? 0.0f
                     : x == 1.0f ? 1.0f
                                 : std::pow(2.0f, -10.0f * x) *
                                           sin((x * 10.0f - 0.75f) * c4) +
                                       1.0f;
}
float easeInElastic(float const x) {
    float const c4 = (2.0f * M_PI) / 3.0f;

    return x == 0.0f ? 0.0f
                     : x == 1.0f ? 1.0f
                                 : -std::pow(2.0f, 10.0f * x - 10.0f) *
                                       sin((x * 10.0f - 10.75f) * c4);
}
float easeInOutElastic(float const x) {
    float const c5 = (2.0f * M_PI) / 4.5f;

    return x == 0.0f
               ? 0.0f
               : x == 1.0f ? 1.0f
                           : x < 0.5f ? -(std::pow(2.0f, 20.0f * x - 10.0f) *
                                          sin((20.0f * x - 11.125f) * c5)) /
                                            2.0f
                                      : (std::pow(2.0f, -20.0f * x + 10.0f) *
                                         sin((20.0f * x - 11.125f) * c5)) /
                                                2.0f +
                                            1.0f;
}

// ---------------------------------------------------------------- Bounce -- //
float easeOutBounce(float x) {
    float const n1 = 7.5625f;
    float const d1 = 2.75f;

    if (x < 1.0f / d1) {
        return n1 * x * x;
    } else if (x < 2.0f / d1) {
        x -= 1.5f;
        return n1 * (x / d1) * x + 0.75f;
    } else if (x < 2.5f / d1) {
        x -= 2.25f;
        return n1 * (x / d1) * x + 0.9375f;
    } else {
        x -= 2.625f;
        return n1 * (x / d1) * x + 0.984375f;
    }
}
float easeInBounce(float const x) { return 1.0f - easeOutBounce(1.0f - x); }
float easeInOutBounce(float const x) {
    return x < 0.5f ? (1.0f - easeOutBounce(1.0f - 2.0f * x)) / 2.0f
                    : (1.0f + easeOutBounce(2.0f * x - 1.0f)) / 2.0f;
}

// ////////////////////////////////////////////////////////////////////////// //
