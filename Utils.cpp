#include "Utils.h"
#include <cmath>

double roundDown(double value) {
    return std::floor(value * 100.0) / 100.0;
}
