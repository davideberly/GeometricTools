// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/ConstantBuffer.h>
#include <Graphics/GL46/GL46Buffer.h>

namespace gte
{
    class GL46ConstantBuffer : public GL46Buffer
    {
    public:
        // Construction.
        virtual ~GL46ConstantBuffer() = default;
        GL46ConstantBuffer(ConstantBuffer const* cbuffer);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline ConstantBuffer* GetConstantBuffer() const
        {
            return static_cast<ConstantBuffer*>(mGTObject);
        }

        // Bind the constant buffer data to the specified uniform buffer unit.
        void AttachToUnit(GLint uniformBufferUnit);
    };
}


