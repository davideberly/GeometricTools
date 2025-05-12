// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/RawBuffer.h>
#include <Graphics/GL46/GL46Buffer.h>

namespace gte
{
    class GL46AtomicCounterBuffer : public GL46Buffer
    {
    public:
        // Construction.
        virtual ~GL46AtomicCounterBuffer() = default;
        GL46AtomicCounterBuffer(RawBuffer const* cbuffer);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline RawBuffer* GetRawBuffer() const
        {
            return static_cast<RawBuffer*>(mGTObject);
        }

        // Bind the raw buffer data to the specified atomic counter buffer unit.
        void AttachToUnit(GLint atomicCounterBufferUnit);
    };
}

