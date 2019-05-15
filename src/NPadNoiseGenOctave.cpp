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

// This function creates NPad noise with specified width and height
// plus with its half-sized variant then merge it.

#include "NPadNoise.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

constexpr double PId = 3.141592653589793238462643383279502884;
constexpr float PIf = 3.141592653589793238462643383279502884f;

inline unsigned int to8(double v)
{
	return (int) floor(v * 255.0 + 0.5);
}

inline double cosLerp(double a, double b, double t)
{
	double x = (1.0 - cos(t * PId)) * 0.5;
	return a * (1.0 - x) + b * x;
}

// form
// c00 c01
// c10 c11
inline double cosBilerp(double c00, double c01, double c10, double c11, double x, double y)
{
	return cosLerp(cosLerp(c00, c01, x), cosLerp(c10, c11, x), y);
}

inline double wrap(double a, double b)
{
	return a - floor(a / b) * b;
}

inline double sampleData(double *ptr, double x, double y, size_t w, size_t h)
{
	double fx = fmod(x, 1.0), fy = fmod(y, 1.0);
	size_t cx = (size_t) wrap(floor(x), (double) w);
	size_t cy = (size_t) wrap(floor(y), (double) h);
	size_t nx = (size_t) wrap(floor(x + 0.5), (double) w);
	size_t ny = (size_t) wrap(floor(y + 0.5), (double) h);

	return cosBilerp(
		ptr[cy * w + cx],
		ptr[cy * w + nx],
		ptr[ny * w + cx],
		ptr[ny * w + nx],
		fx, fy
	);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <width> [height = width]" << std::endl;
		std::cerr << "The noise will be octaved, down to 4x4 when possible." << std::endl;
		std::cerr << "Width and height must be divisible by 4." << std::endl;
		std::cerr << "The result image will be printed into stdout" << std::endl;
		return 1;
	}

	size_t width, height;
	try
	{
		width = height = std::stoul(argv[1]);
	}
	catch (std::out_of_range)
	{
		std::cerr << "ERROR: Invalid width and/or height is specified." << std::endl;
		return 1;
	}

	if (width % 4 != 0 || height % 4 != 0)
	{
		std::cerr << "ERROR: width and/or height is not divisible by 4." << std::endl;
		return 1;
	}

	std::vector<double> nd = npadnoise::create(width, height);
	size_t ndSize = width * height;
	// octaved noise
	for (size_t i = 1, hw = width >> 1, hh = height >> 1; hw >= 4 && hh >= 4; hw >>= 1, hh >>= 1, i++)
	{
		std::vector<double> halved = npadnoise::create(hw, hh);
		double pwDiv = pow(2.0, -((double) i));

		// combine the noise
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for (int64_t i = 0; ((size_t) i) < ndSize; i++)
		{
			size_t x = ((size_t) i) % width;
			size_t y = ((size_t) i) / width;

			nd[i] = (nd[i] + sampleData(&halved[0], ((double) x) * pwDiv, ((double) y) * pwDiv, hw, hh) * pwDiv) / (1.0 + pwDiv);
		}
	}

	// dump to PGM (P2)
	std::cout << "P2" << std::endl;
	std::cout << width << " " << height << std::endl;
	std::cout << "255";

	for (size_t i = 0; i < height; i++)
	{
		std::cout << std::endl;

		for (size_t j = 0; j < width; j += 4)
		{
			size_t p = i * width + j;
			std::cout << to8(nd[p + 0]) << " " << to8(nd[p + 1]) << " " << to8(nd[p + 2]) << " " << to8(nd[p + 3]) << " ";
		}
	}

	return 0;
}
