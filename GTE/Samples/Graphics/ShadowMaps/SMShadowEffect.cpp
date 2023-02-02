// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#include "SMShadowEffect.h"

SMShadowEffect::SMShadowEffect(
    std::shared_ptr<ProgramFactory> const& factory,
    std::string const& vsPath,
    std::string const& psPath,
    Geometry const& geometry)
    :
    mGeometryBuffer{}
{
    mProgram = factory->CreateFromFiles(vsPath, psPath, "");
    LogAssert(
        mProgram,
        "Cannot compile " + vsPath + " or " + psPath);

    mGeometryBuffer = std::make_shared<ConstantBuffer>(sizeof(Geometry), true);
    *mGeometryBuffer->Get<Geometry>() = geometry;

    auto const& vshader = mProgram->GetVertexShader();
    vshader->Set("Geometry", mGeometryBuffer);
}
