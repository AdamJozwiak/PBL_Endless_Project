#pragma once

// ////////////////////////////////////////////////////////////////// Defines //
#define _USE_MATH_DEFINES

// ///////////////////////////////////////////////////////////////// Includes //
#include <cmath>

#include "math-operators.hpp"

// ////////////////////////////////////////////////////////////////// Easings //
template <typename T>
inline T interpolate(float (*easing)(float const), T const &a, T const &b,
                     float const alpha, float const deltaTime) {
    return lerp(a, b, easing(1.0f - std::pow(alpha, deltaTime)));
}

// ------------------------------------------------------------------ Sine -- //
inline float easeOutSine(float const x) {
    return sin((x * static_cast<float>(M_PI)) / 2.0f);
}
inline float easeInSine(float const x) {
    return 1.0f - std::cos((x * static_cast<float>(M_PI)) / 2.0f);
}
inline float easeInOutSine(float const x) {
    return -(std::cos(static_cast<float>(M_PI) * x) - 1.0f) / 2.0f;
}

// ------------------------------------------------------------------ Quad -- //
inline float easeOutQuad(float const x) {
    return 1.0f - (1.0f - x) * (1.0f - x);
}
inline float easeInQuad(float const x) { return x * x; }
inline float easeInOutQuad(float const x) {
    return x < 0.5f ? 2.0f * x * x
                    : 1.0f - std::pow(-2.0f * x + 2.0f, 2.0f) / 2.0f;
}

// ----------------------------------------------------------------- Cubic -- //
inline float easeOutCubic(float const x) {
    return 1.0f - std::pow(1.0f - x, 3.0f);
}
inline float easeInCubic(float const x) { return x * x * x; }
inline float easeInOutCubic(float const x) {
    return x < 0.5f ? 4.0f * x * x * x
                    : 1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f;
}

// ----------------------------------------------------------------- Quart -- //
inline float easeOutQuart(float const x) {
    return 1.0f - std::pow(1.0f - x, 4.0f);
}
inline float easeInQuart(float const x) { return x * x * x * x; }
inline float easeInOutQuart(float const x) {
    return x < 0.5f ? 8.0f * x * x * x * x
                    : 1.0f - std::pow(-2.0f * x + 2.0f, 4.0f) / 2.0f;
}

// ----------------------------------------------------------------- Quint -- //
inline float easeOutQuint(float const x) {
    return 1.0f - std::pow(1.0f - x, 5.0f);
}
inline float easeInQuint(float const x) { return x * x * x * x * x; }
inline float easeInOutQuint(float const x) {
    return x < 0.5f ? 16.0f * x * x * x * x * x
                    : 1.0f - std::pow(-2.0f * x + 2.0f, 5.0f) / 2.0f;
}

// ------------------------------------------------------------------ Expo -- //
inline float easeOutExpo(float const x) {
    return x == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * x);
}
inline float easeInExpo(float const x) {
    return x == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * x - 10.0f);
}
inline float easeInOutExpo(float const x) {
    return x == 0.0f   ? 0.0f
           : x == 1.0f ? 1.0f
           : x < 0.5f  ? std::pow(2.0f, 20.0f * x - 10.0f) / 2.0f
                       : (2.0f - std::pow(2.0f, -20.0f * x + 10.0f)) / 2.0f;
}

// ------------------------------------------------------------------ Circ -- //
inline float easeOutCirc(float const x) {
    return std::sqrt(1.0f - std::pow(x - 1.0f, 2.0f));
}
inline float easeInCirc(float const x) {
    return 1.0f - std::sqrt(1.0f - std::pow(x, 2.0f));
}
inline float easeInOutCirc(float const x) {
    return x < 0.5f
               ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * x, 2.0f))) / 2.0f
               : (std::sqrt(1.0f - std::pow(-2.0f * x + 2.0f, 2.0f)) + 1.0f) /
                     2.0f;
}

// ------------------------------------------------------------------ Back -- //
inline float easeOutBack(float const x) {
    float const c1 = 1.70158f;
    float const c3 = c1 + 1.0f;

    return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
}
inline float easeInBack(float const x) {
    float const c1 = 1.70158f;
    float const c3 = c1 + 1.0f;

    return c3 * x * x * x - c1 * x * x;
}
inline float easeInOutBack(float const x) {
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
inline float easeOutElastic(float const x) {
    float const c4 = (2.0f * static_cast<float>(M_PI)) / 3.0f;

    return x == 0.0f ? 0.0f
           : x == 1.0f
               ? 1.0f
               : std::pow(2.0f, -10.0f * x) * sin((x * 10.0f - 0.75f) * c4) +
                     1.0f;
}
inline float easeInElastic(float const x) {
    float const c4 = (2.0f * static_cast<float>(M_PI)) / 3.0f;

    return x == 0.0f   ? 0.0f
           : x == 1.0f ? 1.0f
                       : -std::pow(2.0f, 10.0f * x - 10.0f) *
                             sin((x * 10.0f - 10.75f) * c4);
}
inline float easeInOutElastic(float const x) {
    float const c5 = (2.0f * static_cast<float>(M_PI)) / 4.5f;

    return x == 0.0f   ? 0.0f
           : x == 1.0f ? 1.0f
           : x < 0.5f  ? -(std::pow(2.0f, 20.0f * x - 10.0f) *
                          sin((20.0f * x - 11.125f) * c5)) /
                            2.0f
                      : (std::pow(2.0f, -20.0f * x + 10.0f) *
                         sin((20.0f * x - 11.125f) * c5)) /
                                2.0f +
                            1.0f;
}

// ---------------------------------------------------------------- Bounce -- //
inline float easeOutBounce(float x) {
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
inline float easeInBounce(float const x) {
    return 1.0f - easeOutBounce(1.0f - x);
}
inline float easeInOutBounce(float const x) {
    return x < 0.5f ? (1.0f - easeOutBounce(1.0f - 2.0f * x)) / 2.0f
                    : (1.0f + easeOutBounce(2.0f * x - 1.0f)) / 2.0f;
}

// ////////////////////////////////////////////////////////////////////////// //
