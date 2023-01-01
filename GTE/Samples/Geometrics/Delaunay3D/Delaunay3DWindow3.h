// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/VertexColorEffect.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Delaunay3.h>
#include <random>
using namespace gte;

class Delaunay3DWindow3 : public Window3
{
public:
    Delaunay3DWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    bool CreateScene();
    void CreateSphere();
    void CreateTetra(size_t index);
    void SetAllTetraWire();
    void SetTetraSolid(size_t index, Vector4<float> const& color);
    void SetLastTetraSolid(Vector4<float> const& color, Vector4<float> const& oppositeColor);
    void DoSearch();

    Vector4<float> mLightGray;
    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<VertexColorEffect> mVCEffect;
    std::shared_ptr<IndexBuffer> mIBuffer;

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mSphere;
    std::vector<std::shared_ptr<Visual>> mWireTetra;
    std::vector<std::shared_ptr<Visual>> mSolidTetra;

    std::vector<Vector3<float>> mVertices;
    std::mt19937 mRandomGenerator;
    std::uniform_real_distribution<float> mRandom[3];

    // The choice of 12 is empirical.  All the data sets tested in this
    // sample require at most 11 elements in the UIntegerFP32 array.
    //Delaunay3<float, BSNumber<UIntegerFP32<12>>> mDelaunay;
    //Delaunay3<float, BSNumber<UIntegerFP32<12>>>::SearchInfo mInfo;

    Delaunay3<float> mDelaunay;
    Delaunay3<float>::SearchInfo mInfo;
};
