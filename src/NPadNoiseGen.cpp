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

// This function creates NPad noise with specified width and height.

#include "NPadNoise.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

inline unsigned int to8(double v)
{
	return (int) floor(v * 255.0 + 0.5);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <width> [height = width]" << std::endl;
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
