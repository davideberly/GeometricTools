The execution of ChangePlatformToolset modifies all *.v16.vcxproj
files and replaces the current_option string in the XML element
  <PlatformToolset>current_option</PlatformToolset>
by the option passed to the executable. This is much faster than
using the Visual Studio IDE to modify hundreds of project files.

1. Build the ChangePlatformToolset project.
2. Copy the executable to the GeometricTools/GTE folder.
3. Execute the program using one of the following:
   a. ChangePlatformToolset "v142"
   b. ChangePlatformToolset "ClangCL"
   c. ChangePlatformToolset "Intel C++ Compiler 2021"

Item 3.a. is the default which uses the Microsoft compiler.
Item 3.b. is for using clang as the compiler.
Item 3.c. is for using Intel's compiler.

Generally, your compiler options can be found by launching
the Properties dialog for a Visual Studio project. Select
Configuration Properties and then General. In the right pane
of the dialog there is an item Platform Toolset. The default
shows "Visual Studio 2019 (v142)". If you click on that, you
can get a drop-down list. On my machines, I have options for
older versions of Visual Studio. I also had installed the clang
tools, so an option is "LLVM (clang-cl)". I have Intel Parallel
Studio 2021 installed, and two compiler options occur. The first
is item 3.c. The second is "Intel C++ Compiler 19.2".
