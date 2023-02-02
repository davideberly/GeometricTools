// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

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
