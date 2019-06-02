/**
 * Copyright (c) 2019 MikuAuahDark
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "NPadNoise.hpp"

#include <cmath>

#include <chrono>
#include <random>

constexpr double PId = 3.141592653589793238462643383279502884;
constexpr float PIf = 3.141592653589793238462643383279502884f;

template<typename T> inline T clamp(T v, T a, T b)
{
	return v < a ? a : (v > b ? b : v);
}

template<typename T> inline T& IDX(T *p, size_t x, size_t y, size_t w)
{
	return p[y * w + x];
}

namespace npadnoise
{

template<typename T> bool fillCommon(T *ptr, size_t width, size_t height, std::function<T()> rng, T jitter)
{
	if (width % 4 != 0 || height % 4 != 0)
		return false;
	
	// NPad noise operates on 4x4 block
	// 1. Fill the corner with random values and calculate the middle point
	for (size_t offy = 0; offy < height; offy += 4)
	{
		for (size_t offx = 0; offx < width; offx += 4)
		{
			// a. Random point value by each corner
			// c1----c2
			// ||    ||
			// ||    ||
			// c3----c4
			T c1 = rng(), c2 = rng(), c3 = rng(), c4 = rng();
			// b. Calculate average middle points
			// c1----c2
			// || mn ||
			// || op ||
			// c3----c4
			// m = clamp((c1 + c2 + c3) / 3 + jitter, 0, 1)
			// n = clamp((c1 + c2 + c4) / 3 + jitter, 0, 1)
			// o = clamp((c1 + c3 + c4) / 3 + jitter, 0, 1)
			// p = clamp((c2 + c3 + c4) / 3 + jitter, 0, 1)
			// Where jitter is randomized for each time accessed
			// and calculated as follows:
			// jitter = (random() * 2 - 1) * jitterPower
			// where random() returns random value from 0 to 1 with fraction.

			IDX(ptr, offx + 1, offy + 1, width) = clamp<T>((c1 + c2 + c3) / ((T) 3.0) + (rng() * ((T) 2.0) - ((T) 1.0)) * ((T) jitter), 0, 1);
			IDX(ptr, offx + 2, offy + 1, width) = clamp<T>((c1 + c2 + c4) / ((T) 3.0) + (rng() * ((T) 2.0) - ((T) 1.0)) * ((T) jitter), 0, 1);
			IDX(ptr, offx + 1, offy + 2, width) = clamp<T>((c1 + c3 + c4) / ((T) 3.0) + (rng() * ((T) 2.0) - ((T) 1.0)) * ((T) jitter), 0, 1);
			IDX(ptr, offx + 2, offy + 2, width) = clamp<T>((c2 + c3 + c4) / ((T) 3.0) + (rng() * ((T) 2.0) - ((T) 1.0)) * ((T) jitter), 0, 1);
			IDX(ptr, offx + 0, offy + 0, width) = c1;
			IDX(ptr, offx + 3, offy + 0, width) = c2;
			IDX(ptr, offx + 0, offy + 3, width) = c3;
			IDX(ptr, offx + 3, offy + 3, width) = c4;
		}
	}
	
	// 2. Fill the sides by using neighboor values, wrapping to first/last if necessary
	// This part can be parallelized as values can be set in independent order (ex. OpenMP)
	size_t loopCount = (width >> 2) * (height >> 2);
#ifdef _OPENMP
#pragma omp parallel for
	for (int64_t pos = 0; pos < ((int64_t) loopCount); pos++)
	{
#else
	for (size_t pos = 0; pos < loopCount; pos++)
	{
#endif
		size_t offx = (pos % (width >> 2)) * 4;
		size_t offy = pos / (width >> 2) * 4;
		// Calculate the sides using neighboor values
		// Regions displayed may necessarily wrap to first/last regions
		// in the array
		// Diagram (wrapped):
		// a -- j -- k -- b
		// |    |    |    |
		// e -- m -- n -- d
		// |    |    |    |
		// g -- o -- p -- f
		// |    |    |    |
		// c -- h -- i -- d
		// For left/right sides:
		// (prefix "p" for previous, "n" for next)
		// (suffix "x" for horizontal, "y" for vertical)
		// d = clamp(sqrt(m * n + nmx * nnx + b * d + nax * ncx), 0, 1)
		// e = clamp(sqrt(m * n + pmx * pnx + a * c + pbx * pdx), 0, 1)
		// f = clamp(sqrt(o * p + nox * npx + b * d + nax * ncx), 0, 1)
		// g = clamp(sqrt(o * p + pox * ppx + a * c + pbx * pdx), 0, 1)
		// For top/bottom sides:
		// h = clamp(sqrt(m * o + nmy * noy + c * d + nay * nby), 0, 1)
		// i = clamp(sqrt(n * p + nny * npy + c * d + nay * nby), 0, 1)
		// j = clamp(sqrt(m * o + pmy * poy + a * b + pcy * pdy), 0, 1)
		// k = clamp(sqrt(n * p + pny * ppy + a * b + pcy * pdy), 0, 1)
		T
		a = IDX(ptr, offx + 0, offy + 0, width),
		b = IDX(ptr, offx + 3, offy + 0, width),
		c = IDX(ptr, offx + 0, offy + 3, width),
		d = IDX(ptr, offx + 3, offy + 3, width),
		m = IDX(ptr, offx + 1, offy + 1, width),
		n = IDX(ptr, offx + 2, offy + 1, width),
		o = IDX(ptr, offx + 1, offy + 2, width),
		p = IDX(ptr, offx + 2, offy + 2, width);
		
		size_t
		poffx = (offx == 0) ? (width - 4) : (offx - 4),
		poffy = (offy == 0) ? (height - 4) : (offy - 4),
		noffx = (offx == (width - 4)) ? 0 : (offx + 4),
		noffy = (offy == (height - 4)) ? 0 : (offy + 4);

		T
		nax = IDX(ptr, noffx + 0, offy + 0, width),
		ncx = IDX(ptr, noffx + 0, offy + 3, width),
		pbx = IDX(ptr, poffx + 3, offy + 0, width),
		pdx = IDX(ptr, poffx + 3, offy + 3, width),
		nay = IDX(ptr, offx + 0, noffy + 0, width),
		nby = IDX(ptr, offx + 3, noffy + 0, width),
		pcy = IDX(ptr, offx + 0, poffy + 3, width),
		pdy = IDX(ptr, offx + 3, poffy + 3, width),

		dv = clamp<T>(sqrt((
			IDX(ptr, noffx + 1, offy + 1, width) * IDX(ptr, noffx + 2, offy + 1, width) +
			m * n + b * d + nax * ncx) / ((T) 4)), 0, 1),
		e = clamp<T>(sqrt((
			IDX(ptr, poffx + 1, offy + 1, width) * IDX(ptr, poffx + 2, offy + 1, width) +
			m * n + a * c + nax * ncx) / ((T) 4)), 0, 1),
		f = clamp<T>(sqrt((
			IDX(ptr, noffx + 1, offy + 2, width) * IDX(ptr, noffx + 2, offy + 2, width) +
			o * p + b * d + nax * ncx) / ((T) 4)), 0, 1),
		g = clamp<T>(sqrt((
			IDX(ptr, poffx + 1, offy + 2, width) * IDX(ptr, poffx + 2, offy + 2, width) +
			o * p + a * c + nax * ncx) / ((T) 4)), 0, 1),
		h = clamp<T>(sqrt((
			IDX(ptr, offx + 1, noffy + 1, width) * IDX(ptr, offx + 1, noffy + 2, width) +
			m * o + c * d + nay * nby) / ((T) 4)), 0, 1),
		i = clamp<T>(sqrt((
			IDX(ptr, offx + 2, noffy + 1, width) * IDX(ptr, offx + 2, noffy + 2, width) +
			n * p + c * d + nay * nby) / ((T) 4)), 0, 1),
		j = clamp<T>(sqrt((
			IDX(ptr, offx + 1, poffy + 1, width) * IDX(ptr, offx + 1, poffy + 2, width) +
			m * o + a * b + pcy * pdy) / ((T) 4)), 0, 1),
		k = clamp<T>(sqrt((
			IDX(ptr, offx + 2, poffy + 1, width) * IDX(ptr, offx + 2, poffy + 2, width) +
			n * p + a * b + pcy * pdy) / ((T) 4)), 0, 1);
		
		IDX(ptr, offx + 1, offy + 0, width) = j;
		IDX(ptr, offx + 2, offy + 0, width) = k;
		IDX(ptr, offx + 1, offy + 3, width) = h;
		IDX(ptr, offx + 2, offy + 3, width) = i;
		IDX(ptr, offx + 3, offy + 1, width) = dv;
		IDX(ptr, offx + 3, offy + 2, width) = f;
		IDX(ptr, offx + 0, offy + 1, width) = e;
		IDX(ptr, offx + 0, offy + 2, width) = g;
	}

	return true;
}

bool fill(float *ptr, size_t width, size_t height, float roughness)
{
	std::mt19937 mtrng((unsigned int) std::chrono::system_clock::now().time_since_epoch().count());
	return fill(ptr, width, height, [&]() -> float {
		return ((float) (mtrng() >> 16)) / ((float) UINT16_MAX);
	}, roughness);
}

bool fill(float *ptr, size_t width, size_t height, std::function<float()> rng, float roughness)
{
	return fillCommon(ptr, width, height, rng, roughness);
}

bool fill(double *ptr, size_t width, size_t height, double roughness)
{
	std::mt19937 mtrng((unsigned int) std::chrono::system_clock::now().time_since_epoch().count());
	return fill(ptr, width, height, [&]() -> double {
		return ((double) mtrng()) / ((double) UINT32_MAX);
	}, roughness);
}

bool fill(double *ptr, size_t width, size_t height, std::function<double()> rng, double roughness)
{
	return fillCommon(ptr, width, height, rng, roughness);
}

}
