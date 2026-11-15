## Geometric Tools Engine

The Geometric Tools Engine (GTE) is a collection of source code for computing
in the fields of mathematics, geometry, graphics, image analysis and physics.
The engine is written in C++ 14 and supports high-performance computing using
CPU multithreading and general purpose GPU programming (GPGPU). Portions of
the code are described in various books as well as in PDF documents available
at the
[Geometric Tools Website](https://www.geometrictools.com).
GTE is licensed under the
[Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).
The update history for the current version of GTE is
[Current Update History](https://www.geometrictools.com/Downloads/Gte8UpdateHistory.pdf).
The update history for all versions of GTE is [Full Update History](https://www.geometrictools.com/Downloads/GteFullUpdateHistory.pdf).
The CMake system is provided by Michal Lurie using the MIT library.

## Supported Platforms

The mathematics code is in a header-only library, GTMathematics. A
mathematics library with GPU-based implementations is provided,s
GTMathematicsGPU. The CPU-based common graphics engine code is in its
own library, GTGraphics. DirectX 11 wrappers are provided for graphics
and applications, GTGraphicsDX11 and GTApplicationsDX11, on Microsoft
Windows 10/11. OpenGL 4.6 wrappers are provided for graphics and
applications, GTGraphicsGL46 and GTApplicationsGL46, on Microsoft
Windows 10/11 and on Linux. A small number of files exist to use GLX
and X-Windows on Linux.

On Microsoft Windows 11, the code is maintained using Microsoft Visual
Studio 2022 or 2026 with Microsoft's compilers, LLVM clang-cl or with Intel C++
Compilers 2026. Support is also provided for mingw-w64 using MSYS2 (GCC),
Visual Code v1.145.2, CMake 4.4.5, NVIDIA graphics drivers, OpenGL 4.6,
and gcc 16.2.1.

On Ubuntu 26.04.1 LTS, the code is maintained using Visual Code
1.146.1, CMake 4.2.3, NVIDIA graphics drivers, OpenGL 4.6, and
gcc 15.2.0.

On Fedora 45, the code is maintained using Visual Code 1.146.0,
and CMake 4.3.0, NVIDIA graphics drivers, OpenGL 4.6, and gcc 16.2.1.

## Getting Started

The repository contains many sample applications to illustrate some
features of the engine. Top-level solutions/makefiles exist to build
everything in the repository. Please read the
[Installation and Release Notes](https://github.com/davideberly/GeometricTools/blob/master/GTE/Gte8p1InstallationRelease.pdf)
to understand what is expected of your development environment.
  
## Pull Requests

You are welcome to create pull requests for me to examine if you believe
you have found a bug. When you post the pull request, please also modify
the file version (line 6 of the comment header in the files). The version
is of the form major.minor.year.month.day. The major.minor portion is when
the file was introduced into the GTE distribution; do not modify this. The
year is a 4-digit number, the month is a 2-digit number, and the day is a
2-digit number. For example, if you submitted the pull request on
February 20, 2025, then major.minor.2025.02.20 is the new file version.
