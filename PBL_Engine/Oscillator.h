#pragma once
#include <cmath>

class Oscillator {
  private:
    float minimum;
    float maximum;

    static float mod(float num, float div) {
        float ratio = num / div;
        return div * (ratio - std::floor(ratio));
    }

  public:
    Oscillator(float a, float b)
        : minimum(a < b ? a : b), maximum(a > b ? a : b) {}

    float range() { return maximum - minimum; }

    float cycle_length() { return 2 * range(); }

    float normalize(float val) {
        float state = mod(val - minimum, cycle_length());

        if (state > range()) state = cycle_length() - state;

        return state + minimum;
    }
};