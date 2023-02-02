// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/DirectionalLightEffect.h>
using namespace gte;

class ExtractLevelSurfacesWindow3 : public Window3
{
public:
    ExtractLevelSurfacesWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateMeshCubes();
    void CreateMeshTetrahedra();
    void UpdateConstants();

    std::shared_ptr<Node> mScene;
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mMeshCubes;
    std::shared_ptr<Visual> mMeshTetrahedra;
    std::shared_ptr<DirectionalLightEffect> mLightEffectCubes;
    std::shared_ptr<DirectionalLightEffect> mLightEffectTetrahedra;

    // An x-ray crystallography of a molecule.
    int32_t mXBound, mYBound, mZBound;
    std::vector<uint8_t> mImage;

    // Shader parameters shared by both meshes.
    std::shared_ptr<Material> mMaterial;
    std::shared_ptr<Lighting> mLighting;
    Vector4<float> mLightWorldDirection;

    bool mUseCubes;
};
