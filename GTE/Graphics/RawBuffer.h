// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Buffer.h>
#include <cstdint>

// RawBuffer is currently supported only in the DirectX graphics engine.

namespace gte
{
    class RawBuffer : public Buffer
    {
    public:
        // Construction.  The element size is always 4 bytes.
        RawBuffer(uint32_t numElements, bool createStorage = true);

    public:
        // For use by the Shader class for storing reflection information.
        static int32_t const shaderDataLookup = 3;
    };
}
