## About ##

The Geometric Tools Engine is a collection of source code for computing in
the fields of mathematics, geometry, graphics, image analysis and physics.
The engine is written in C++ 14 and supports high-performance computing
using CPU multithreading and general purpose GPU programming (GPGPU).
Portions of the code are described in various books as well as in PDF
documents available at the
[Geometric Tools Website](https://www.geometrictools.com).

The Geometric Tools Engine is licensed under the
[Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).

## Supported Platforms ##

The mathematics code is in a header-only library, GTMathematics. A
mathematics library with GPU-based implementations is provided,
GTMathematicsGPU. The CPU-based common graphics engine code is in its
own library, GTGraphics. DirectX 11 wrappers are provided for graphics
and applications, GTGraphicsDX11 and GTApplicationsDX11, on Microsoft
Windows 10. OpenGL 4.5 wrappers are provided for graphics and
applications, GTGraphicsGL45 and GTApplicationsGL45, on Microsoft
Windows 10 and on Linux. A small number of files exist to use GLX
and X-Windows on Linux.

On Microsoft Windows 10, the code is maintained using Microsoft Visual
Studio 2015, 2017 and 2019 with Microsoft's compilers or with Intel
C++ Compilers 17, 18 and 19.1.

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
 
## Citing ##

Here is a Bibtex entry you can use to cite this project in a publication.

```
@misc{GeometricToolsEngine5,
  title = {{G}eometric {T}ools {E}ngine, Version 5},
  author = {David Eberly},
  howpublished = {\url{https://github.com/davideberly/GeometricTools}}
}
```
## Current Technical Support Queue ##

* Replacing the LCP approach by a geometric approach for the
  intersection of a finite cylinder and a box to fix the robustness
  problem reported by Seb Wouters. This is a longer-term project
  than I had planned. Wouters' approach uses convex hulls of 2D
  projections. I am implementing an approach that avoids this when
  the cylinder direction has 1 or 2 zero components. When the direction
  has no zero components, the explicit projection can be avoided and
  reduces computation time. The project includes writing a PDF that
  describes the ideas (of LCP and its robustness failure, of Wouters'
  approach, and my variation).
* Adding a minor improvement to the TriangulateEC algorithm as
  describe in my online PDF.
* The neverending attempts to finish the GTL implementation. Adding
  unit tests takes a lot of time, and technical support of GTE takes
  higher precedence when bugs are reported.
