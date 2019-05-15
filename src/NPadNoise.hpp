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

#ifndef NPADNOISE_H
#define NPADNOISE_H

// STL
#include <algorithm>
#include <functional>
#include <vector>

// Version
#define NPADNOISE_VERSION 20190514

namespace npadnoise
{

bool fill(float *ptr, size_t width, size_t height, float roughness = 0.5);
bool fill(float *ptr, size_t width, size_t height, std::function<float()> rng, float roughness = 0.5);
bool fill(double *ptr, size_t width, size_t height, double roughness = 0.5);
bool fill(double *ptr, size_t width, size_t height, std::function<double()> rng, double roughness = 0.5f);

inline std::vector<double> create(size_t width, size_t height, double roughness = 0.5)
{
	if (width % 4 != 0 || height % 4 != 0)
		return std::vector<double>(); // empty vector on failure
	
	std::vector<double> data = std::vector<double>(width * height);
	fill(&data[0], width, height, roughness);
	return data;
}

}

#endif
