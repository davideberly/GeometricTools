// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/Buffer.h>
#include <Graphics/GL46/GL46Resource.h>

namespace gte
{
    class GL46Buffer : public GL46Resource
    {
    protected:
        // Abstract base class.
        virtual ~GL46Buffer();
        GL46Buffer(Buffer const* buffer, GLenum type);

        // Must be called by constructor.
        virtual void Initialize();

    public:
        // Member access.
        inline Buffer* GetBuffer() const
        {
            return static_cast<Buffer*>(mGTObject);
        }

        inline GLenum GetType() const
        {
            return mType;
        }

        inline GLenum GetUsage() const
        {
            return mUsage;
        }

        // TODO: This is a tentative interface; modify as needed.  Make these
        // pure virtual latter (if relevant).  Do we really need these to be
        // virtual?  Revisit the DX11 code and investigate why the choice was
        // made there.
        virtual bool Update();
        virtual bool CopyCpuToGpu();
        virtual bool CopyGpuToCpu();

    protected:
        GLenum mType;
        GLenum mUsage;
    };
}

