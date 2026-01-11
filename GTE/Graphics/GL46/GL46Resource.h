// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/Resource.h>
#include <Graphics/GL46/GL46GraphicsObject.h>

namespace gte
{
    class GL46Resource : public GL46GraphicsObject
    {
    public:
        // Abstract base class.
        virtual ~GL46Resource() = default;
    protected:
        GL46Resource(Resource const* gtResource);

    public:
        // Member access.
        inline Resource* GetResource() const
        {
            return static_cast<Resource*>(mGTObject);
        }

        // TODO: This is a tentative interface; modify as needed.  Make these
        // pure virtual latter (if relevant).
        void* MapForWrite(GLenum target);
        void Unmap(GLenum target);

        virtual bool Update()
        {
            return false;
        }

        virtual bool CopyCpuToGpu()
        {
            return false;
        }

        virtual bool CopyGpuToCpu()
        {
            return false;
        }

        virtual void CopyGpuToGpu(GL46Resource* target)
        {
            (void)target;
            throw std::logic_error(std::string(__FILE__) + "(" + std::string(__FUNCTION__) + "," + std::to_string(__LINE__) + "): Not yet implemented.\n");
        }

    protected:
        // Support for copying between CPU and GPU.
        bool PreparedForCopy(GLenum access) const;
    };
}


