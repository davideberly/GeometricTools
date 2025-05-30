// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/GraphicsObject.h>

namespace gte
{
    class GEObject
    {
    public:
        // Abstract base class.
        virtual ~GEObject();
    protected:
        GEObject(GraphicsObject const* gtObject);

    public:
        // Member access.
        inline GraphicsObject* GetGraphicsObject() const
        {
            return mGTObject;
        }

        // Support for debugging.
        virtual void SetName(std::string const& name) = 0;

        inline std::string const& GetName() const
        {
            return mName;
        }

    protected:
        GraphicsObject* mGTObject;
        std::string mName;
    };
}

