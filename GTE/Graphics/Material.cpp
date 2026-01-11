// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Material.h>
using namespace gte;

Material::Material()
    :
    emissive{ 0.0f, 0.0f, 0.0f, 1.0f },
    ambient{ 0.0f, 0.0f, 0.0f, 1.0f },
    diffuse{ 0.0f, 0.0f, 0.0f, 1.0f },
    specular{ 0.0f, 0.0f, 0.0f, 1.0f }
{
}


