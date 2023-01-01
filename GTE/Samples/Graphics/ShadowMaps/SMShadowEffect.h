// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#pragma once

#include <Graphics/GraphicsEngine.h>
#include <Graphics/ViewVolume.h>
#include <Graphics/VisualEffect.h>
using namespace gte;

class SMShadowEffect : public VisualEffect
{
public:
    struct Geometry
    {
        Geometry()
            :
            worldMatrix{},
            lightPVMatrix{}
        {
        }

        Matrix4x4<float> worldMatrix;
        Matrix4x4<float> lightPVMatrix;
    };

    SMShadowEffect(
        std::shared_ptr<ProgramFactory> const& factory,
        std::string const& vsPath,
        std::string const& psPath,
        Geometry const& geometry);

    inline std::shared_ptr<ConstantBuffer> const& GetGeometryBuffer() const
    {
        return mGeometryBuffer;
    }

private:
    // Vertex shader constants.
    std::shared_ptr<ConstantBuffer> mGeometryBuffer;
};
