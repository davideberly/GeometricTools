This tool applies to projects created with MSVS 2019 or 2022.

The execution of ChangePlatformToolset modifies all *.v17.vcxproj files and
replaces the current_option string in the XML element
  <PlatformToolset>current_option</PlatformToolset>
by the option passed to the executable. This is much faster than using the
Visual Studio IDE to modify hundreds of project files.

1. Build the ChangePlatformToolset project.
2. Copy the executable to the GeometricTools/GTE folder.
3. Execute the program using one of the following. Option (a) is for Microsoft
   Visual Studio 2019. Option (b) is for Microsoft Visual Studio 2022. Option
   (c) is for Clang. Option (d) is for Intel oneAPI 2024. Option (e) is for
   Intel oneAPI 2025.
   a. ChangePlatformToolset "v142"
   b. ChangePlatformToolset "v143"
   c. ChangePlatformToolset "ClangCL"
   d. ChangePlatformToolset "Intel C++ Compiler 2024"
   e. ChangePlatformToolset "Intel C++ Compiler 2025"

Generally, your compiler options can be found by launching the Properties
dialog for a Visual Studio project. Select Configuration Properties and then
General. In the right pane of the dialog there is an item Platform Toolset.
The default shows "Visual Studio 2019 (v142)" for MSVS 2019 or "Visual Studio
2022 (v143)" for MSVS 2022. If you click on that, you can get a drop-down
list. On my machines, I have options for older versions of Visual Studio. I
also had installed the clang tools, so an option is "LLVM (clang-cl)". I have
Intel oneAPI 2025.0 installed, so there is an option "Intel C++ Compiler 2024".
2025.0 installed, so there is an option "Intel C++ Compiler 2024".
