// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.07.10

#include <Applications/GTApplicationsPCH.h>
#include <Applications/MSW/Console.h>
using namespace gte;

// The singleton used to create and destroy consoles for applications.
namespace gte
{
    ConsoleSystem TheConsoleSystem;
}

#if defined(GTE_USE_DIRECTX)

#include <Graphics/DX11/DXGIAdapter.h>

void ConsoleSystem::CreateEngineAndProgramFactory(Console::Parameters& parameters)
{
    // The adapterManager must be declared outside the if-then-else
    // statement so that it persists long enough to create the 'engine'
    // object.
    DXGIAdapter adapterManager;
    IDXGIAdapter* adapter = nullptr;
    if ((parameters.deviceCreationFlags & D3D11_CREATE_DEVICE_DEBUG) == 0)
    {
        // The GPU adapter is selected using the following algorithm. If a
        // discrete adapter is available (NVIDIA, AMD or other manufacturer),
        // it is selected. If a discrete adapter is not available, Intel
        // Integrated Graphics is chosen. Although these days Intel Core
        // architecture is the norm, in the event Intel Integrated Graphics is
        // not found, the fallback is to Microsoft WARP which is a software
        // implementation for DirectX 11 that is multithreaded and has decent
        // performance.
        adapterManager = DXGIAdapter::GetMostPowerful();
        adapter = adapterManager.GetAdapter();
    }
    else
    {
        // If parameters.deviceCreationFlags is 0 (no flags specified), the
        // first adapter in the adapter enumeration is selected. This is
        // invariably the adapter to which the display monitors are attached.
        //
        // If the debug layer is selected using D3D11_CREATE_DEVICE_DEBUG,
        // choosing a non-null adapter does not work. It will cause the
        // D3D11CreateDevice funtion to throw an exception and not return
        // an HRESULT code.
        adapter = nullptr;
    }

    auto engine = std::make_shared<DX11Engine>(adapter, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, parameters.deviceCreationFlags);

    if (engine->GetDevice())
    {
        parameters.engine = engine;
        parameters.factory = std::make_shared<HLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create compute engine.");
    }
}
#endif

#if defined(GTE_USE_OPENGL)
void ConsoleSystem::CreateEngineAndProgramFactory(Console::Parameters& parameters)
{
    bool saveDriverInfo = ((parameters.deviceCreationFlags & 0x00000001) != 0);
    auto engine = std::make_shared<WGLEngine>(false, saveDriverInfo);
    if (!engine->MeetsRequirements())
    {
        LogError("OpenGL 4.5 or later is required.");
    }

    if (engine->GetDevice())
    {
        parameters.engine = engine;
        parameters.factory = std::make_shared<GLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create compute engine.");
    }
}
#endif
