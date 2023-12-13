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
[Current Update History](https://github.com/davideberly/GeometricTools/blob/master/GTE/Gte6UpdateHistory.pdf). The update history for all versions of GTE is [Full Update History](https://github.com/davideberly/GeometricTools/blob/master/GTE/GteFullUpdateHistory.pdf).

I am in the process of writing The Geometric Tools Library (GTL),
which is a major revision of GTE. A large portion of GTL development
has been porting code from GTE, dealing with size_t versus signed
integer compiler complaints, and adding new features. An equally
large portion has been writing unit tests and end-to-end tests for
the mathematics code. The project has taking a significant amount
of time and effort, but the completion is drawing near. GTL is also
licensed under the
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
Compiler 2023.

On Ubuntu 22.04.1 LTS, the code is maintained using Visual Studio Code
1.49.2 and CMake 3.15.2, NVIDIA graphics drivers, OpenGL 4.5 and
gcc 11.3.0, 

On Fedora 38, the code is maintained using Visual Studio Code 1.49.2
and CMake 3.22.2, NVIDIA graphics drivers, OpenGL 4.5 and
gcc 13.1.1.

## Getting Started ##

The repository contains many sample applications to illustrate some
features of the engine. Top-level solutions/makefiles exist to build
everything in the repository. Please read the
[Installation and Release Notes](https://github.com/davideberly/GeometricTools/blob/master/GTE/Gte6p8InstallationRelease.pdf)
to understand what is expected of your development environment.

## Known 3rd Party Problems ##

* After installing NVIDIA GeForce driver version 531.18, all my DirectX 11 samples
  throw 2 exceptions in D3D11CreateDevice, both tagged as Poco::NotFoundException.
  If you continue execution after these exceptions, the applications perform
  correctly. When the D3D11 device is released, another Poco::NotFoundException is
  generated. You can continue execution after this exception. The exceptions still
  occur through driver version 546.33 released on 12 December 2023. A moderator for
  NVIDIA forums posted on 14 November 2023 that the issue has been addressed but
  does not know when the fix will occur in a future driver release. See
  [Bug status](https://forums.developer.nvidia.com/t/poco-notfoundexception-thrown-in-driver-version-531-18/245285/28).
    
  WORKAROUND: This assumes you have checked the box in the MSVS Exception Settings
  window that says "<All C++ Exceptions not in this list>". If you do not have this
  checked, set a breakpoint at a line of code before the call to D3D11CreateDevice
  and run to the breakpoint. Then check the aforementioned box. Continue the
  execution and an exception dialog is launched when the call is made to
  D3D11CreateDevice. That dialog has a box checked "Break when this exception type
  is thrown". Uncheck the box. You will see a new item in the Exception Settings
  window under the "C++ Exceptions" that says "Poco::NotFoundException". It is
  unchecked, which means the debugger will not stop execution for that particular
  exception. This information is stored in the *.user files. If you delete the *.user
  file, you lose the exception settings, which reapplying the workaround steps.

* On shutting down the DX11 graphics in application, Intel(R) Iris(R) Xe Graphics
  driver version 31.0.101.4146 throws two C++ exceptions (in igc64.dll). The first is
  MONZA\::DdiThreadingContext&lt;MONZA::AdapterTraits_Gen12LP&gt;\::msg_end and the
  second is MONZA\::IgcThreadingContext&lt;MONZA::AdapterTraits_Gen12LP&gt;\::msg_end. These
  exceptions can be ignored.
 
## Links to GTE-Based Projects ##
* Seb Wouter's improvement for my LCP-based test-intersection query between
  a box and a finite cylinder. When using floating-point arithmetic, the LCP
  solver had some significant rounding errors to produce incorrect results.
  His approach clips the box by the cylinder enddisk planes to form a convex
  polyhedron, projects that to a plane perpendicular to the cylinder axis to
  obtain a polygon, and then tests for polygon-circle intersection. The code
  uses a generic convex hull finder to create the convex polyhedron. My current
  GTE code avoids the generic hull finder and takes advantage of the pairs of
  parallel planes for the box to amortize the computational costs for a faster
  algorithm. The repository link is
  https://github.com/SebWouters/AabbCyl

* CodeReclaimers example of incorporating Dear ImGui into a GTE sample
  application (for Microsoft Windows). The repository link is
  https://github.com/CodeReclaimers/GTEImGuiExample
