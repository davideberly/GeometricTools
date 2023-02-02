// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.11

#pragma once

#include <regex>
#include <string>

class TemplateVSCode
{
public:
    TemplateVSCode();

    bool Execute(std::string const& projectName, std::string const& appType);

private:
    bool CreateDotVSCodeFolderAndFiles();
    bool CreateCMakeSample();
    bool CreateCMakeVariants();
    bool CreateCodeWorkspace(std::string const& projectName);
    bool CreateCMakeLists(std::string const& projectName,
        std::string const& applicationType);

    // TODO: Do I have to even worry about Visual Studio Code on Linux
    // wanting newline 0x0A but properly handling Windows newline 0x0D 0x0A?
    std::string ReplaceCRLFbyLF(std::string const& source, bool isUTF8);

    static std::regex const msProjectNamePattern;
    static std::regex const msApplicationTypePattern;
    static std::string const msLaunch;
    static std::string const msSettings;
    static std::string const msCMakeSample;
    static std::string const msCMakeVariants;
    static std::string const msCodeWorkspace;
    static std::string const msCMakeLists;
};
