// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.08

#include "ProjectTemplate.v17.h"

TemplateV17::TemplateV17(std::string const& gteRelativePath)
    :
    Template(gteRelativePath)
{
}

std::string const TemplateV17::msGTMathematicsGUID("0CADDB12-31D9-4F60-A8C0-678D1773DA03");
std::string const TemplateV17::msGTGraphicsGUID("B15CB711-F650-4063-9C7C-E7A5F88139A2");
std::string const TemplateV17::msGTGraphicsDX11GUID("00DEFF42-0F7C-4A7C-9C1F-59322B9E9004");
std::string const TemplateV17::msGTApplicationsDX11GUID("8BB237C2-2D3A-41AD-9B98-A35BA75D5DEA");
std::string const TemplateV17::msGTGraphicsGL45GUID("7373CB9F-6595-4FA4-9500-88985A74F0E7");
std::string const TemplateV17::msGTApplicationsGL45GUID("C67471F4-761E-4292-A811-B1FA78E60C21");

std::string const TemplateV17::msSolutionLines =
R"raw(Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 17
VisualStudioVersion = 17.0.32014.148
MinimumVisualStudioVersion = 10.0.40219.1
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "_PROJECT_NAME__GRAPHICS_API_.v17", "_PROJECT_NAME__GRAPHICS_API_.v17.vcxproj", "{_PROJECT_GUID_}"
EndProject
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "Required", "Required", "{_REQUIRED_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTMathematics.v17", "_GTE_RELATIVE_PATH_GTMathematics.v17.vcxproj", "{_GTMATHEMATICS_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTGraphics.v17", "_GTE_RELATIVE_PATH_GTGraphics.v17.vcxproj", "{_GTGRAPHICS_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTGraphics_GRAPHICS_API_.v17", "_GTE_RELATIVE_PATH_GTGraphics_GRAPHICS_API_.v17.vcxproj", "{_GTGRAPHICSAPI_GUID_}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GTApplications_GRAPHICS_API_.v17", "_GTE_RELATIVE_PATH_GTApplications_GRAPHICS_API_.v17.vcxproj", "{_GTAPPLICATIONSAPI_GUID_}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|x64 = Debug|x64
		Debug|x86 = Debug|x86
		Release|x64 = Release|x64
		Release|x86 = Release|x86
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{_PROJECT_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_PROJECT_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_PROJECT_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_PROJECT_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_PROJECT_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_PROJECT_GUID_}.Release|x64.Build.0 = Release|x64
		{_PROJECT_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_PROJECT_GUID_}.Release|x86.Build.0 = Release|Win32
		{_GTMATHEMATICS_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_GTMATHEMATICS_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_GTMATHEMATICS_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_GTMATHEMATICS_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_GTMATHEMATICS_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_GTMATHEMATICS_GUID_}.Release|x64.Build.0 = Release|x64
		{_GTMATHEMATICS_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_GTMATHEMATICS_GUID_}.Release|x86.Build.0 = Release|Win32
		{_GTGRAPHICS_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_GTGRAPHICS_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_GTGRAPHICS_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_GTGRAPHICS_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_GTGRAPHICS_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_GTGRAPHICS_GUID_}.Release|x64.Build.0 = Release|x64
		{_GTGRAPHICS_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_GTGRAPHICS_GUID_}.Release|x86.Build.0 = Release|Win32
		{_GTGRAPHICSAPI_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_GTGRAPHICSAPI_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_GTGRAPHICSAPI_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_GTGRAPHICSAPI_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_GTGRAPHICSAPI_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_GTGRAPHICSAPI_GUID_}.Release|x64.Build.0 = Release|x64
		{_GTGRAPHICSAPI_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_GTGRAPHICSAPI_GUID_}.Release|x86.Build.0 = Release|Win32
		{_GTAPPLICATIONSAPI_GUID_}.Debug|x64.ActiveCfg = Debug|x64
		{_GTAPPLICATIONSAPI_GUID_}.Debug|x64.Build.0 = Debug|x64
		{_GTAPPLICATIONSAPI_GUID_}.Debug|x86.ActiveCfg = Debug|Win32
		{_GTAPPLICATIONSAPI_GUID_}.Debug|x86.Build.0 = Debug|Win32
		{_GTAPPLICATIONSAPI_GUID_}.Release|x64.ActiveCfg = Release|x64
		{_GTAPPLICATIONSAPI_GUID_}.Release|x64.Build.0 = Release|x64
		{_GTAPPLICATIONSAPI_GUID_}.Release|x86.ActiveCfg = Release|Win32
		{_GTAPPLICATIONSAPI_GUID_}.Release|x86.Build.0 = Release|Win32
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(NestedProjects) = preSolution
		{_GTMATHEMATICS_GUID_} = {_REQUIRED_GUID_}
		{_GTGRAPHICS_GUID_} = {_REQUIRED_GUID_}
		{_GTGRAPHICSAPI_GUID_} = {_REQUIRED_GUID_}
		{_GTAPPLICATIONSAPI_GUID_} = {_REQUIRED_GUID_}
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {_SOLUTION_GUID_}
	EndGlobalSection
EndGlobal)raw";

std::string const TemplateV17::msProjectLines =
R"raw(<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <ProjectGuid>{_PROJECT_GUID_}</ProjectGuid>
    <Keyword>Win32Proj</Keyword>
    <RootNamespace>_PROJECT_NAME_.v17</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
  </ImportGroup>
  <ImportGroup Label="Shared">
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="PropertySheets">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="PropertySheets">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <LinkIncremental>true</LinkIncremental>
    <OutDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <LinkIncremental>true</LinkIncremental>
    <OutDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <LinkIncremental>false</LinkIncremental>
    <OutDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <LinkIncremental>false</LinkIncremental>
    <OutDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>_Output\_GRAPHICS_API_\$(PlatformToolset)\$(Platform)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <ClCompile>
      <PrecompiledHeader>
      </PrecompiledHeader>
      <WarningLevel>Level4</WarningLevel>
      <Optimization>Disabled</Optimization>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>_GTE_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <DisableSpecificWarnings>6262;26812</DisableSpecificWarnings>
      <TreatWarningAsError>true</TreatWarningAsError>
      <ConformanceMode>true</ConformanceMode>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <PrecompiledHeader>
      </PrecompiledHeader>
      <WarningLevel>Level4</WarningLevel>
      <Optimization>Disabled</Optimization>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>_GTE_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <DisableSpecificWarnings>6262;26812</DisableSpecificWarnings>
      <TreatWarningAsError>true</TreatWarningAsError>
      <ConformanceMode>true</ConformanceMode>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <PrecompiledHeader>
      </PrecompiledHeader>
      <Optimization>MaxSpeed</Optimization>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>_GTE_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <DisableSpecificWarnings>6262;26812</DisableSpecificWarnings>
      <TreatWarningAsError>true</TreatWarningAsError>
      <ConformanceMode>true</ConformanceMode>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level4</WarningLevel>
      <PrecompiledHeader>
      </PrecompiledHeader>
      <Optimization>MaxSpeed</Optimization>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <PreprocessorDefinitions>GTE_USE_MSWINDOWS;GTE_USE_ROW_MAJOR;GTE_USE_MAT_VEC;_GRAPHICS_MACRO_;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>_GTE_RELATIVE_PATH_.</AdditionalIncludeDirectories>
      <DisableSpecificWarnings>6262;26812</DisableSpecificWarnings>
      <TreatWarningAsError>true</TreatWarningAsError>
      <ConformanceMode>true</ConformanceMode>
      <PrecompiledHeaderFile />
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <AdditionalDependencies>_LINK_LIBRARY_%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="_PROJECT_NAME__APPTYPE_.cpp" />
    <ClCompile Include="_PROJECT_NAME_Main.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="_PROJECT_NAME__APPTYPE_.h" />
  </ItemGroup>
  <ItemGroup>
    <ProjectReference Include="_GTE_RELATIVE_PATH_GTApplications_GRAPHICS_API_.v17.vcxproj">
      <Project>{_GTAPPLICATIONSAPI_GUID_}</Project>
    </ProjectReference>
    <ProjectReference Include="_GTE_RELATIVE_PATH_GTGraphics.v17.vcxproj">
      <Project>{_GTGRAPHICS_GUID_}</Project>
    </ProjectReference>
    <ProjectReference Include="_GTE_RELATIVE_PATH_GTGraphics_GRAPHICS_API_.v17.vcxproj">
      <Project>{_GTGRAPHICSAPI_GUID_}</Project>
    </ProjectReference>
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>)raw";

std::string const TemplateV17::msFilterLines =
R"raw(<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Source Files">
      <UniqueIdentifier>{4FC737F1-C7A5-4376-A066-2A32D752A2FF}</UniqueIdentifier>
      <Extensions>cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx</Extensions>
    </Filter>
    <Filter Include="Header Files">
      <UniqueIdentifier>{93995380-89BD-4b04-88EB-625FBE52EBFB}</UniqueIdentifier>
      <Extensions>h;hh;hpp;hxx;hm;inl;inc;xsd</Extensions>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="_PROJECT_NAME__APPTYPE_.cpp">
      <Filter>Source Files</Filter>
    </ClCompile>
    <ClCompile Include="_PROJECT_NAME_Main.cpp">
      <Filter>Source Files</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="_PROJECT_NAME__APPTYPE_.h">
      <Filter>Header Files</Filter>
    </ClInclude>
  </ItemGroup>
</Project>)raw";
