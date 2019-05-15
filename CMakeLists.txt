cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

project(NPadNoiseProgs LANGUAGES CXX)

add_executable(NPadNoiseGen src/NPadNoise.cpp src/NPadNoiseGen.cpp)
add_executable(NPadNoiseGenOctave src/NPadNoise.cpp src/NPadNoiseGenOctave.cpp)

option(USE_OPENMP "Use OpenMP" ON)

if(USE_OPENMP)
	find_package(OpenMP)

	if(OpenMP_CXX_FOUND)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
		message(STATUS "Using OpenMP (${OpenMP_CXX_FLAGS}). Current Flags: ${CMAKE_CXX_FLAGS}")
	else()
		message(WARNING "OpenMP is not supported by your compiler. Compiling without OpenMP!")
	endif()
endif()

install(TARGETS NPadNoiseGen DESTINATION bin)
install(TARGETS NPadNoiseGenOctave DESTINATION bin)
