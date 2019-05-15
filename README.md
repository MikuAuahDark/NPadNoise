NPad Noise
==========

C++11 [Null-Pointer AuahDark](https://www.facebook.com/NPad.Memes) noise function implementation.

Building
--------

Just put `NPadNoise.h` and `NPadNoise.cpp` in `src` into your project. Other files are only for executables.

Notes:

* In some Linux distro, math library is separated so you want to link with it too.

* It will automatically parallel some code execution if [OpenMP](https://www.openmp.org/) compiler option is passed.

* CMake files is only for the executable.
