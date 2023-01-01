// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#include <Windows.h>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <direct.h>
#include <io.h>

bool Modify(std::string const& filename, std::string const& toolset)
{
    std::ifstream input(filename);
    if (!input)
    {
        return false;
    }
    std::ofstream output("tempproject.txt");
    if (!output)
    {
        input.close();
        return false;
    }

    std::string line{};
    while (!input.eof())
    {
        std::getline(input, line);
        if (line != "")
        {
            auto index0 = line.find("<PlatformToolset>");
            if (index0 != std::string::npos)
            {
                auto index1 = line.find("</PlatformToolset>");
                if (index1 != std::string::npos)
                {
                    output << "    <PlatformToolset>" << toolset << "</PlatformToolset>" << std::endl;
                    continue;
                }
            }
            output << line << std::endl;
        }
    }

    output.close();
    input.close();

    std::string command = "copy tempproject.txt " + filename;
    system(command.c_str());
    command = "del tempproject.txt";
    system(command.c_str());
    return true;
}

void FindSource(std::string const& toolset)
{
    bool foundFirstSamples = true;
    bool foundFirstTools = true;

    struct _finddata_t fileInfo;
    intptr_t hFile = _findfirst("*", &fileInfo);
    while (hFile != -1L)
    {
        if (fileInfo.attrib & _A_SUBDIR)
        {
            if (memcmp(fileInfo.name, ".", 1) != 0
                && memcmp(fileInfo.name, "..", 2) != 0)
            {
                std::string name(fileInfo.name);
                if (name == "Samples")
                {
                    if (foundFirstSamples)
                    {
                        foundFirstSamples = false;
                        (void)_chdir(fileInfo.name);
                        FindSource(toolset);
                        continue;
                    }
                }
                if (name == "Tools")
                {
                    if (foundFirstTools)
                    {
                        foundFirstTools = false;
                        (void)_chdir(fileInfo.name);
                        FindSource(toolset);
                        continue;
                    }
                }
                if (name != "Internal")
                {
                    (void)_chdir(fileInfo.name);
                    FindSource(toolset);
                }
            }
        }
        else
        {
            char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME];
            char ext[_MAX_EXT];
            _splitpath_s(fileInfo.name, drive, dir, fname, ext);

            std::string name(fileInfo.name);
            std::string suffix(ext);
            auto projectV16Ext = name.find(".v17.vcxproj");
            if (projectV16Ext != std::string::npos && suffix == ".vcxproj")
            {
                if (name != "ChangePlatformToolset.v17.vcxproj")
                {
                    Modify(name, toolset);
                }
            }
        }

        if (_findnext(hFile, &fileInfo) != 0)
        {
            (void)_chdir("..");
            break;
        }
    }
}

int main(int numArguments, char* arguments[])
{
    if (numArguments == 2)
    {
        std::string toolset(arguments[1]);
        FindSource(toolset);
        return 0;
    }
    else
    {
        std::cout << "Invalid input to main." << std::endl;
        return -1;
    }
}
