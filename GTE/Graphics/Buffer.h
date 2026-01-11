// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/Resource.h>
#include <cstdint>
#include <functional>

namespace gte
{
    class Buffer : public Resource
    {
    public:
        // Abstract base class.
        virtual ~Buffer() = default;
    protected:
        Buffer(uint32_t numElements, size_t elementSize, bool createStorage = true);
    };

    typedef std::function<void(std::shared_ptr<Buffer> const&)> BufferUpdater;
}


