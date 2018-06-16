#pragma once
#include <cmath>
namespace blok::floats {
	bool almostEqual(float a, float b) {
		float result = std::nextafter(a, b);
		float r2 = std::nextafter(result, b);
		return r2 == b;
	}
}
