static const float PI = 3.14159265359f;
static const float TWO_PI = 2.0f * PI;

float harmonic(float A, float T, float lambda, float phi, float t, float x) {
    return A * sin((TWO_PI / T) * t - (TWO_PI / lambda) * x + phi);
}
