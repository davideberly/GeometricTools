## About ##

The Geometric Tools Engine is a collection of source code for computing in
the fields of mathematics, geometry, graphics, image analysis and physics.
The engine is written in C++ 14 and supports high-performance computing
using CPU multithreading and general purpose GPU programming (GPGPU).
Portions of the code are described in various books as well as in PDF
documents available at the
[Geometric Tools Website](https://www.geometrictools.com).

The Geometric Tools Engine (GTE) is licensed under the
[Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).

I am now in the process of finishing The Geometric Tools Library (GTL),
which is a major revision of GTE. The library is also licensed under the
[Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt). The
code comes with PDF documentation and hyperlinks in the source files to
the specific locations in the PDF file that describe the code and use.
The PDF itself has links to the source code files. Portions of the code
will be posted once finalized. The first batch is the Utility library and
documentation and is available at github and at my website.

## Supported Platforms ##

The mathematics code is in a header-only library, GTMathematics. A
mathematics library with GPU-based implementations is provided,
GTMathematicsGPU. The CPU-based common graphics engine code is in its
own library, GTGraphics. DirectX 11 wrappers are provided for graphics
and applications, GTGraphicsDX11 and GTApplicationsDX11, on Microsoft
Windows 10/11. OpenGL 4.5 wrappers are provided for graphics and
applications, GTGraphicsGL45 and GTApplicationsGL45, on Microsoft
Windows 10/11 and on Linux. A small number of files exist to use GLX
and X-Windows on Linux.

On Microsoft Windows 10/11, the code is maintained using Microsoft Visual
Studio 2019/2022 with Microsoft's compilers, LLVM clang-cl or with Intel C++
Compiler 2022.

On Ubuntu 20.04.1 LTS, the code is maintained using Visual Studio Code
1.49.2 and CMake 3.15.2, NVIDIA graphics drivers, OpenGL 4.5 and
gcc 9.3.0, 

On Fedora 35, the code is maintained using Visual Studio Code 1.49.2
and CMake 3.18.3, NVIDIA graphics drivers, OpenGL 4.5 and
gcc 11.2.1.

## Getting Started ##

The repository contains many sample applications to illustrate some
features of the engine. Top-level solutions/makefiles exist to build
everything in the repository. Please read the installation and release
notes to understand what is expected of your development environment.
 
## Technical Support Queue ##

The current queue of technical support issues, whether reported on github
or privately by email, is found at the Geometric Tools website. See the
document https://www.geometrictools.com/Downloads/TechnicalSupportQueue.pdf

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
  https://https://github.com/SebWouters/AabbCyl

* CodeReclaimers example of incorporating Dear ImGui into a GTE sample
  application (for Microsoft Windows). The repository link is
  https://github.com/CodeReclaimers/GTEImGuiExample
