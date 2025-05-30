// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Mathematics/Vector4.h>

namespace gte
{
    class Material
    {
    public:
        // Construction.
        Material();

        // (r,g,b,*): default (0,0,0,1)
        Vector4<float> emissive;

        // (r,g,b,*): default (0,0,0,1)
        Vector4<float> ambient;

        // (r,g,b,a): default (0,0,0,1)
        Vector4<float> diffuse;

        // (r,g,b,specularPower): default (0,0,0,1)
        Vector4<float> specular;
    };
}

