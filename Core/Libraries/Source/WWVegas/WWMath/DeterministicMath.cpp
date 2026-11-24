/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#include "DeterministicMath.h"

#include <string.h>

#if defined(_MSC_VER) && _MSC_VER < 1600
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#define DETERMINISTIC_U64(value) value##ui64
#else
#include <stdint.h>
#define DETERMINISTIC_U64(value) value##ULL
#endif

namespace
{

const float PI = 3.14159265358979323846f;
const float HALF_PI = 1.57079632679489661923f;
const float QUARTER_PI = 0.78539816339744830962f;
const float TWO_PI = 6.28318530717958647692f;
const double TWO_PI_DOUBLE = 6.283185307179586476925286766559;
const float TAN_PI_OVER_8 = 0.41421356237309504880f;
const float LN_2 = 0.69314718055994530942f;
const float INV_LN_2 = 1.44269504088896340736f;
const float SQRT_2 = 1.41421356237309504880f;

uint32_t floatBits(float value)
{
	uint32_t bits;
	memcpy(&bits, &value, sizeof(bits));
	return bits;
}

float bitsFloat(uint32_t bits)
{
	float value;
	memcpy(&value, &bits, sizeof(value));
	return value;
}

uint64_t doubleBits(double value)
{
	uint64_t bits;
	memcpy(&bits, &value, sizeof(bits));
	return bits;
}

double bitsDouble(uint64_t bits)
{
	double value;
	memcpy(&value, &bits, sizeof(value));
	return value;
}

float quietNaN()
{
	return bitsFloat(0x7fc00000U);
}

bool isNaNBits(uint32_t bits)
{
	return (bits & 0x7fffffffU) > 0x7f800000U;
}

bool isInfBits(uint32_t bits)
{
	return (bits & 0x7fffffffU) == 0x7f800000U;
}

bool isOddInteger(float value, bool &isInteger)
{
	if (value < -16777216.0f || value > 16777216.0f)
	{
		isInteger = true;
		return false;
	}

	const int integer = static_cast<int>(value);
	isInteger = static_cast<float>(integer) == value;
	return isInteger && (integer & 1) != 0;
}

float roundedAdd(float a, float b)
{
	volatile float result = a + b;
	return result;
}

float roundedSub(float a, float b)
{
	volatile float result = a - b;
	return result;
}

float roundedMul(float a, float b)
{
	volatile float result = a * b;
	return result;
}

float roundedDiv(float a, float b)
{
	volatile float result = a / b;
	return result;
}

float sinKernel(float x)
{
	const float s1 = -1.6666667163e-01f;
	const float s2 = 8.3333337680e-03f;
	const float s3 = -1.9841270114e-04f;
	const float s4 = 2.7557314297e-06f;
	const float s5 = -2.5050759689e-08f;
	const float s6 = 1.5896910177e-10f;

	const float z = roundedMul(x, x);
	float p = roundedAdd(s5, roundedMul(z, s6));
	p = roundedAdd(s4, roundedMul(z, p));
	p = roundedAdd(s3, roundedMul(z, p));
	p = roundedAdd(s2, roundedMul(z, p));
	p = roundedAdd(s1, roundedMul(z, p));
	return roundedAdd(x, roundedMul(roundedMul(x, z), p));
}

float cosKernel(float x)
{
	const float c1 = 4.1666667908e-02f;
	const float c2 = -1.3888889225e-03f;
	const float c3 = 2.4801587642e-05f;
	const float c4 = -2.7557314297e-07f;
	const float c5 = 2.0875723372e-09f;
	const float c6 = -1.1359647598e-11f;

	const float z = roundedMul(x, x);
	float p = roundedAdd(c5, roundedMul(z, c6));
	p = roundedAdd(c4, roundedMul(z, p));
	p = roundedAdd(c3, roundedMul(z, p));
	p = roundedAdd(c2, roundedMul(z, p));
	p = roundedAdd(c1, roundedMul(z, p));
	return roundedAdd(roundedSub(1.0f, roundedMul(0.5f, z)),
		roundedMul(roundedMul(z, z), p));
}

float reduceAngle(float value)
{
	if (value > -1000000000.0f && value < 1000000000.0f)
	{
		volatile double x = static_cast<double>(value);
		volatile double quotient = x / TWO_PI_DOUBLE;
		const int turns = static_cast<int>(quotient + (quotient >= 0.0 ? 0.5 : -0.5));
		volatile double product = static_cast<double>(turns) * TWO_PI_DOUBLE;
		volatile double reduced = x - product;
		return static_cast<float>(reduced);
	}
	return det_fmodf(value, TWO_PI);
}

float atanSeries(float x)
{
	const float z = roundedMul(x, x);
	float p = 1.0f / 19.0f;
	p = roundedAdd(-1.0f / 17.0f, roundedMul(z, p));
	p = roundedAdd(1.0f / 15.0f, roundedMul(z, p));
	p = roundedAdd(-1.0f / 13.0f, roundedMul(z, p));
	p = roundedAdd(1.0f / 11.0f, roundedMul(z, p));
	p = roundedAdd(-1.0f / 9.0f, roundedMul(z, p));
	p = roundedAdd(1.0f / 7.0f, roundedMul(z, p));
	p = roundedAdd(-1.0f / 5.0f, roundedMul(z, p));
	p = roundedAdd(1.0f / 3.0f, roundedMul(z, p));
	return roundedSub(x, roundedMul(roundedMul(x, z), p));
}

float atanPositive(float x)
{
	if (x > 1.0f)
		return roundedSub(HALF_PI, atanPositive(roundedDiv(1.0f, x)));
	if (x > TAN_PI_OVER_8)
		return roundedAdd(QUARTER_PI,
			atanSeries(roundedDiv(roundedSub(x, 1.0f), roundedAdd(x, 1.0f))));
	return atanSeries(x);
}

float logPositive(float value)
{
	uint32_t bits = floatBits(value);
	int exponent = static_cast<int>((bits >> 23) & 0xffU) - 127;
	if (((bits >> 23) & 0xffU) == 0)
	{
		value = roundedMul(value, 16777216.0f);
		bits = floatBits(value);
		exponent = static_cast<int>((bits >> 23) & 0xffU) - 127 - 24;
	}

	bits = (bits & 0x007fffffU) | 0x3f800000U;
	float mantissa = bitsFloat(bits);
	if (mantissa > SQRT_2)
	{
		mantissa = roundedMul(mantissa, 0.5f);
		++exponent;
	}

	const float z = roundedDiv(roundedSub(mantissa, 1.0f), roundedAdd(mantissa, 1.0f));
	const float z2 = roundedMul(z, z);
	float p = 1.0f / 15.0f;
	p = roundedAdd(1.0f / 13.0f, roundedMul(z2, p));
	p = roundedAdd(1.0f / 11.0f, roundedMul(z2, p));
	p = roundedAdd(1.0f / 9.0f, roundedMul(z2, p));
	p = roundedAdd(1.0f / 7.0f, roundedMul(z2, p));
	p = roundedAdd(1.0f / 5.0f, roundedMul(z2, p));
	p = roundedAdd(1.0f / 3.0f, roundedMul(z2, p));
	p = roundedAdd(1.0f, roundedMul(z2, p));
	const float logMantissa = roundedMul(2.0f, roundedMul(z, p));
	return roundedAdd(roundedMul(static_cast<float>(exponent), LN_2), logMantissa);
}

float expValue(float value)
{
	if (value > 88.72283935546875f)
		return bitsFloat(0x7f800000U);
	if (value < -103.97208404541016f)
		return 0.0f;

	const float scaled = roundedMul(value, INV_LN_2);
	const int exponent = static_cast<int>(roundedAdd(scaled, scaled >= 0.0f ? 0.5f : -0.5f));
	const float remainder = roundedSub(value, roundedMul(static_cast<float>(exponent), LN_2));

	float term = 1.0f;
	float sum = 1.0f;
	for (int i = 1; i <= 12; ++i)
	{
		term = roundedDiv(roundedMul(term, remainder), static_cast<float>(i));
		sum = roundedAdd(sum, term);
	}

	float scale = 1.0f;
	if (exponent > 0)
	{
		for (int i = 0; i < exponent; ++i)
			scale = roundedMul(scale, 2.0f);
	}
	else
	{
		for (int i = 0; i > exponent; --i)
			scale = roundedMul(scale, 0.5f);
	}
	return roundedMul(sum, scale);
}

} // namespace

float det_fmodf(float x, float y)
{
	uint32_t xBits = floatBits(x);
	uint32_t yBits = floatBits(y);
	const uint32_t sign = xBits & 0x80000000U;
	uint32_t xAbs = xBits & 0x7fffffffU;
	uint32_t yAbs = yBits & 0x7fffffffU;

	if (isNaNBits(xBits))
		return x;
	if (isNaNBits(yBits))
		return y;
	if (isInfBits(xBits) || yAbs == 0)
		return quietNaN();
	if (isInfBits(yBits) || xAbs == 0)
		return x;
	if (xAbs < yAbs)
		return x;
	if (xAbs == yAbs)
		return bitsFloat(sign);

	int xExponent = static_cast<int>((xAbs >> 23) & 0xffU);
	int yExponent = static_cast<int>((yAbs >> 23) & 0xffU);
	if (xExponent == 0)
	{
		xExponent = 1;
		while ((xAbs & 0x00800000U) == 0)
		{
			xAbs <<= 1;
			--xExponent;
		}
	}
	else
	{
		xAbs &= 0x007fffffU;
		xAbs |= 0x00800000U;
	}
	if (yExponent == 0)
	{
		yExponent = 1;
		while ((yAbs & 0x00800000U) == 0)
		{
			yAbs <<= 1;
			--yExponent;
		}
	}
	else
	{
		yAbs &= 0x007fffffU;
		yAbs |= 0x00800000U;
	}

	while (xExponent > yExponent)
	{
		const uint32_t difference = xAbs - yAbs;
		if ((difference & 0x80000000U) == 0)
		{
			if (difference == 0)
				return bitsFloat(sign);
			xAbs = difference;
		}
		xAbs <<= 1;
		--xExponent;
	}

	const uint32_t difference = xAbs - yAbs;
	if ((difference & 0x80000000U) == 0)
		xAbs = difference;
	if (xAbs == 0)
		return bitsFloat(sign);

	while ((xAbs & 0x00800000U) == 0)
	{
		xAbs <<= 1;
		--xExponent;
	}

	if (xExponent > 0)
	{
		xAbs -= 0x00800000U;
		xAbs |= static_cast<uint32_t>(xExponent) << 23;
	}
	else
	{
		xAbs >>= 1 - xExponent;
	}
	return bitsFloat(sign | xAbs);
}

float det_sinf(float value)
{
	const uint32_t bits = floatBits(value);
	if (isNaNBits(bits) || isInfBits(bits))
		return quietNaN();
	float x = reduceAngle(value);
	if (x > PI)
		x = roundedSub(x, TWO_PI);
	else if (x < -PI)
		x = roundedAdd(x, TWO_PI);
	if (x > HALF_PI)
		x = roundedSub(PI, x);
	else if (x < -HALF_PI)
		x = roundedSub(-PI, x);
	return sinKernel(x);
}

float det_cosf(float value)
{
	const uint32_t bits = floatBits(value);
	if (isNaNBits(bits) || isInfBits(bits))
		return quietNaN();
	float x = reduceAngle(value);
	if (x > PI)
		x = roundedSub(x, TWO_PI);
	else if (x < -PI)
		x = roundedAdd(x, TWO_PI);
	float sign = 1.0f;
	if (x > HALF_PI)
	{
		x = roundedSub(PI, x);
		sign = -1.0f;
	}
	else if (x < -HALF_PI)
	{
		x = roundedSub(-PI, x);
		sign = -1.0f;
	}
	return roundedMul(sign, cosKernel(x));
}

float det_tanf(float value)
{
	return roundedDiv(det_sinf(value), det_cosf(value));
}

float det_atanf(float value)
{
	const uint32_t bits = floatBits(value);
	if (isNaNBits(bits))
		return value;
	if (isInfBits(bits))
		return (bits & 0x80000000U) ? -HALF_PI : HALF_PI;
	const float magnitude = bitsFloat(bits & 0x7fffffffU);
	const float result = atanPositive(magnitude);
	return (bits & 0x80000000U) ? -result : result;
}

float det_atan2f(float y, float x)
{
	const uint32_t yBits = floatBits(y);
	const uint32_t xBits = floatBits(x);
	const bool yNegative = (yBits & 0x80000000U) != 0;
	const bool xNegative = (xBits & 0x80000000U) != 0;
	const float yAbs = bitsFloat(yBits & 0x7fffffffU);
	const float xAbs = bitsFloat(xBits & 0x7fffffffU);

	if (isNaNBits(yBits))
		return y;
	if (isNaNBits(xBits))
		return x;
	if (isInfBits(yBits) && isInfBits(xBits))
	{
		const float angle = xNegative ? 3.0f * QUARTER_PI : QUARTER_PI;
		return yNegative ? -angle : angle;
	}
	if (isInfBits(yBits))
		return yNegative ? -HALF_PI : HALF_PI;
	if (isInfBits(xBits))
	{
		if (!xNegative)
			return bitsFloat(yBits & 0x80000000U);
		return yNegative ? -PI : PI;
	}
	if (yAbs == 0.0f)
	{
		if (xNegative)
			return yNegative ? -PI : PI;
		return y;
	}
	if (xAbs == 0.0f)
		return yNegative ? -HALF_PI : HALF_PI;

	float angle;
	if (yAbs > xAbs)
		angle = roundedSub(HALF_PI, atanPositive(roundedDiv(xAbs, yAbs)));
	else
		angle = atanPositive(roundedDiv(yAbs, xAbs));
	if (xNegative)
		angle = roundedSub(PI, angle);
	return yNegative ? -angle : angle;
}

float det_sqrtf(float value)
{
	const uint32_t bits = floatBits(value);
	const uint32_t magnitude = bits & 0x7fffffffU;
	if (isNaNBits(bits))
		return value;
	if (magnitude == 0)
		return value;
	if ((bits & 0x80000000U) != 0)
		return quietNaN();
	if (isInfBits(bits))
		return value;

	float scale = 1.0f;
	if ((magnitude >> 23) == 0)
	{
		value = roundedMul(value, 16777216.0f);
		scale = 0.000244140625f;
	}

	float estimate = bitsFloat((floatBits(value) >> 1) + 0x1fc00000U);
	for (int i = 0; i < 6; ++i)
		estimate = roundedMul(0.5f, roundedAdd(estimate, roundedDiv(value, estimate)));
	return roundedMul(estimate, scale);
}

double det_sqrt(double value)
{
	const uint64_t bits = doubleBits(value);
	const uint64_t magnitude = bits & DETERMINISTIC_U64(0x7fffffffffffffff);
	if (magnitude > DETERMINISTIC_U64(0x7ff0000000000000))
		return value;
	if (magnitude == 0)
		return value;
	if ((bits & DETERMINISTIC_U64(0x8000000000000000)) != 0)
		return bitsDouble(DETERMINISTIC_U64(0x7ff8000000000000));
	if (magnitude == DETERMINISTIC_U64(0x7ff0000000000000))
		return value;

	double scale = 1.0;
	if ((magnitude >> 52) == 0)
	{
		value *= 18014398509481984.0;
		scale = 0.000000007450580596923828125;
	}

	double estimate = bitsDouble((doubleBits(value) >> 1) + DETERMINISTIC_U64(0x1ff8000000000000));
	for (int i = 0; i < 8; ++i)
	{
		volatile double quotient = value / estimate;
		volatile double sum = estimate + quotient;
		volatile double next = 0.5 * sum;
		estimate = next;
	}
	volatile double result = estimate * scale;
	return result;
}

float det_asinf(float value)
{
	const uint32_t bits = floatBits(value);
	const uint32_t magnitude = bits & 0x7fffffffU;
	if (isNaNBits(bits) || magnitude > 0x3f800000U)
		return quietNaN();
	if (magnitude == 0x3f800000U)
		return (bits & 0x80000000U) ? -HALF_PI : HALF_PI;

	const float radicand = roundedMul(roundedSub(1.0f, value), roundedAdd(1.0f, value));
	return det_atan2f(value, det_sqrtf(radicand));
}

float det_acosf(float value)
{
	const uint32_t bits = floatBits(value);
	if (isNaNBits(bits) || (bits & 0x7fffffffU) > 0x3f800000U)
		return quietNaN();
	return roundedSub(HALF_PI, det_asinf(value));
}

float det_powf(float x, float y)
{
	const uint32_t xBits = floatBits(x);
	const uint32_t yBits = floatBits(y);
	const uint32_t xAbsBits = xBits & 0x7fffffffU;
	const float xAbs = bitsFloat(xAbsBits);

	if (y == 0.0f || x == 1.0f)
		return 1.0f;
	if (isNaNBits(xBits))
		return x;
	if (isNaNBits(yBits))
		return y;
	if (isInfBits(yBits))
	{
		if (xAbs == 1.0f)
			return 1.0f;
		const bool grows = (xAbs > 1.0f) == ((yBits & 0x80000000U) == 0);
		return grows ? bitsFloat(0x7f800000U) : 0.0f;
	}
	if (xAbs == 0.0f)
	{
		bool isInteger = false;
		const bool oddExponent = isOddInteger(y, isInteger);
		if (!isInteger && (xBits & 0x80000000U) != 0)
			return quietNaN();
		const uint32_t sign = ((xBits & 0x80000000U) != 0 && oddExponent) ? 0x80000000U : 0;
		if ((yBits & 0x80000000U) != 0)
			return bitsFloat(sign | 0x7f800000U);
		return bitsFloat(sign);
	}
	if (isInfBits(xBits))
	{
		bool isInteger = false;
		const bool oddExponent = isOddInteger(y, isInteger);
		if ((xBits & 0x80000000U) != 0 && !isInteger)
			return quietNaN();
		const uint32_t sign = ((xBits & 0x80000000U) != 0 && oddExponent) ? 0x80000000U : 0;
		return (y > 0.0f) ? bitsFloat(sign | 0x7f800000U) : bitsFloat(sign);
	}

	bool negativeResult = false;
	if ((xBits & 0x80000000U) != 0)
	{
		bool isInteger = false;
		negativeResult = isOddInteger(y, isInteger);
		if (!isInteger)
			return quietNaN();
	}

	const float magnitude = expValue(roundedMul(y, logPositive(xAbs)));
	return negativeResult ? -magnitude : magnitude;
}
