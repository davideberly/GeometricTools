// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/GEObject.h>
#include <Graphics/GL46/GL46.h>
#include <Mathematics/Logger.h>

namespace gte
{
    class GL46GraphicsObject : public GEObject
    {
    public:
        // Abstract base class.
        virtual ~GL46GraphicsObject() = default;
    protected:
        GL46GraphicsObject(GraphicsObject const* gtObject);

    public:
        // Member access.
        inline GLuint GetGLHandle() const
        {
            return mGLHandle;
        }

        // Support for debugging.
        virtual void SetName(std::string const& name) override;

    protected:
        GLuint mGLHandle;
    };
}

