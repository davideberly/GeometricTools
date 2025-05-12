// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/SamplerState.h>
#include <Graphics/GL46/GL46DrawingState.h>

namespace gte
{
    class GL46SamplerState : public GL46DrawingState
    {
    public:
        // Construction and destruction.
        virtual ~GL46SamplerState();
        GL46SamplerState(SamplerState const* samplerState);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline SamplerState* GetSamplerState()
        {
            return static_cast<SamplerState*>(mGTObject);
        }

    private:
        // Conversions from GTEngine values to GL4 values.
        static GLint const msMode[];
    };
}

