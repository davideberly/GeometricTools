## About ##

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

I am in the process of writing The Geometric Tools Library (GTL),
which is a major revision of GTE. A large portion of GTL development
has been porting code from GTE, dealing with size_t versus signed
integer compiler complaints, and adding new features. An equally
large portion has been writing unit tests and end-to-end tests for
the mathematics code. The project has taking a significant amount
of time and effort. I do not yet have a reliable schedul to post.
GTL is also licensed under the
[Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).

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

## Known 3rd Party Problems ##

* After installing NVIDIA GeForce driver version 531.18, all my DirectX 11 samples
  throw 2 exceptions in D3D11CreateDevice, both tagged as Poco::NotFoundException.
  If you continue execution after these exceptions, the applications perform
  correctly. When the D3D11 device is released, another Poco::NotFoundException is
  generated. You can continue execution after this exception. The exceptions still
  occur through driver version 546.65 released on 17 January 2024. Driver version
  551.52 has been released on 13 February 2024; the exceptions no longer occur.
    
 * On shutting down the DX11 graphics in application, Intel(R) Iris(R) Xe Graphics
  driver version 31.0.101.4146 throws two C++ exceptions (in igc64.dll). The first is
  MONZA\::DdiThreadingContext&lt;MONZA::AdapterTraits_Gen12LP&gt;\::msg_end and the
  second is MONZA\::IgcThreadingContext&lt;MONZA::AdapterTraits_Gen12LP&gt;\::msg_end. These
  exceptions can be ignored.

  ## Geometric Tools Library (GTL) ##
  This is a major update of GTE. I am starting to push the initial distribution.
  This will take some time.
  