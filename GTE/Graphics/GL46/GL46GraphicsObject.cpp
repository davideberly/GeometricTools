// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46GraphicsObject.h>
using namespace gte;

GL46GraphicsObject::GL46GraphicsObject(GraphicsObject const* gtObject)
    :
    GEObject(gtObject),
    mGLHandle(0)
{
}

void GL46GraphicsObject::SetName(std::string const& name)
{
    // TODO:  Determine how to tag OpenGL objects with names?
    mName = name;
}


