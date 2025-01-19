## Geometric Tools Engine ##

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
[Current Update History](https://www.geometrictools.com/Downloads/Gte7UpdateHistory.pdf).
The update history for all versions of GTE is [Full Update History](https://www.geometrictools.com/Downloads/GteFullUpdateHistory.pdf).

## Supported Platforms ##

The mathematics code is in a header-only library, GTMathematics. A
mathematics library with GPU-based implementations is provided,s
GTMathematicsGPU. The CPU-based common graphics engine code is in its
own library, GTGraphics. DirectX 11 wrappers are provided for graphics
and applications, GTGraphicsDX11 and GTApplicationsDX11, on Microsoft
Windows 10/11. OpenGL 4.5 wrappers are provided for graphics and
applications, GTGraphicsGL45 and GTApplicationsGL45, on Microsoft
Windows 10/11 and on Linux. A small number of files exist to use GLX
and X-Windows on Linux.

On Microsoft Windows 10/11, the code is maintained using Microsoft Visual
Studio 2019/2022 with Microsoft's compilers, LLVM clang-cl or with Intel C++
Compilers 2024/2025.

On Ubuntu 24.04.1 LTS, the code is maintained using Visual Studio Code
1.85.1 and CMake 3.28.1, NVIDIA graphics drivers, OpenGL 4.5 and
gcc 13.2.0.

On Fedora 41, the code is maintained using Visual Studio Code 1.92.1
and CMake 3.28.2, NVIDIA graphics drivers, OpenGL 4.5 and gcc 14.2.1.

On openSUSE Leap 15.5, the code is maintained using Visual Studio Code 1.85.1
and CMake 3.20.4, NVIDIA graphics drivers, OpenGL 4.5 and gcc 7.5.0.

## Getting Started ##

The repository contains many sample applications to illustrate some
features of the engine. Top-level solutions/makefiles exist to build
everything in the repository. Please read the
[Installation and Release Notes](https://github.com/davideberly/GeometricTools/blob/master/GTE/Gte7p2InstallationRelease.pdf)
to understand what is expected of your development environment.
  