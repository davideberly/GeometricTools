// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.28

#include "SkinningWindow3.h"

SkinningWindow3::SkinningWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mWireState{},
    mMesh{},
    mSkinningEffect{},
    mSkinningTimer{}
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.1f, 0.001f,
        { 0.0f, 0.0f, -90.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    CreateScene();
}

void SkinningWindow3::OnIdle()
{
    mTimer.Measure();

    UpdateConstants(static_cast<float>(mSkinningTimer.GetSeconds()));

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool SkinningWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mWireState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void SkinningWindow3::CreateScene()
{
    // The skinned object is a cylinder.
    uint32_t const numRadialSamples = 10;
    uint32_t const numAxisSamples = 7;
    float const radius = 10.0f;
    float const height = 80.0f;
    float const invRS = 1.0f / static_cast<float>(numRadialSamples);
    float const invASm1 = 1.0f / static_cast<float>(numAxisSamples - 1);
    float const halfHeight = 0.5f * height;
    Vector3<float> const center{ 0.0f, 0.0f, 0.0f };
    Vector3<float> const u{ 0.0f, 0.0f, -1.0f };
    Vector3<float> const v{ 0.0f, 1.0f, 0.0f };
    Vector3<float> const axis{ 1.0f, 0.0f, 0.0f };

    // Generate geometry.
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32B32A32_FLOAT, 0);

    // Generate points on the unit circle to be used in computing the mesh
    // points on a cylinder slice.
    uint32_t numVertices = numAxisSamples * (numRadialSamples + 1);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto* vertices = vbuffer->Get<Vertex>();
    std::vector<float> cs(numRadialSamples + 1);
    std::vector<float> sn(numRadialSamples + 1);
    float const twoPi = static_cast<float>(GTE_C_TWO_PI);
    for (uint32_t r = 0; r < numRadialSamples; ++r)
    {
        float angle = twoPi * invRS * static_cast<float>(r);
        cs[r] = std::cos(angle);
        sn[r] = std::sin(angle);
    }
    cs[numRadialSamples] = cs[0];
    sn[numRadialSamples] = sn[0];

    // Generate the cylinder itself.
    for (uint32_t a = 0, i = 0; a < numAxisSamples; ++a, ++i)
    {
        float axisFraction = a * invASm1;  // in [0,1]
        float z = -halfHeight + height * axisFraction;

        // Compute center of slice.
        Vector3<float> sliceCenter = center + z * axis;

        // Compute slice vertices with duplication at endpoint.
        Vector4<float> color{ axisFraction, 1.0f - axisFraction, 0.3f, 1.0f };
        uint32_t save = i;
        for (uint32_t r = 0; r < numRadialSamples; ++r, ++i)
        {
            Vector3<float> normal = cs[r] * u + sn[r] * v;
            vertices[i].position = sliceCenter + radius * normal;
            vertices[i].color = color;
            vertices[i].weights = ComputeWeights(a);
        }

        vertices[i].position = vertices[save].position;
        vertices[i].color = vertices[save].color;
        vertices[i].weights = ComputeWeights(a);
    }

    // Generate topology.
    uint32_t const numTriangles = 2 * (numAxisSamples - 1) * numRadialSamples;
    auto ibuffer = std::make_shared<IndexBuffer>(IPType::IP_TRIMESH,
        numTriangles, sizeof(uint32_t));
    uint32_t* indices = ibuffer->Get<uint32_t>();
    for (uint32_t a = 0, aStart = 0; a < numAxisSamples - 1; ++a)
    {
        uint32_t i0 = aStart;
        uint32_t i1 = i0 + 1;
        aStart += numRadialSamples + 1;
        uint32_t i2 = aStart;
        uint32_t i3 = i2 + 1;
        for (uint32_t i = 0; i < numRadialSamples; ++i, indices += 6)
        {
            indices[0] = i0++;
            indices[1] = i1;
            indices[2] = i2;
            indices[3] = i1++;
            indices[4] = i3++;
            indices[5] = i2++;
        }
    }

    // Generate the skinning effect.
    mSkinningEffect = std::make_shared<SkinningEffect>(mProgramFactory);

    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, mSkinningEffect);
    mPVWMatrices.Subscribe(mMesh);
    mTrackBall.Attach(mMesh);

    mTrackBall.Update();
    mPVWMatrices.Update();
}

Vector4<float> SkinningWindow3::ComputeWeights(uint32_t a)
{
    Vector4<float> tcoord{};

    if (a == 0)
    {
        tcoord[0] = 1.0f;
        tcoord[1] = 0.0f;
        tcoord[2] = 0.0f;
        tcoord[3] = 0.0f;
    }
    else if (a == 1)
    {
        tcoord[0] = 0.5f;
        tcoord[1] = 0.5f;
        tcoord[2] = 0.0f;
        tcoord[3] = 0.0f;
    }
    else if (a == 2)
    {
        tcoord[0] = 0.0f;
        tcoord[1] = 1.0f;
        tcoord[2] = 0.0f;
        tcoord[3] = 0.0f;
    }
    else if (a == 3)
    {
        tcoord[0] = 0.0f;
        tcoord[1] = 0.5f;
        tcoord[2] = 0.5f;
        tcoord[3] = 0.0f;
    }
    else if (a == 4)
    {
        tcoord[0] = 0.0f;
        tcoord[1] = 0.0f;
        tcoord[2] = 1.0f;
        tcoord[3] = 0.0f;
    }
    else if (a == 5)
    {
        tcoord[0] = 0.0f;
        tcoord[1] = 0.0f;
        tcoord[2] = 0.5f;
        tcoord[3] = 0.5f;
    }
    else
    {
        tcoord[0] = 0.0f;
        tcoord[1] = 0.0f;
        tcoord[2] = 0.0f;
        tcoord[3] = 1.0f;
    }

    return tcoord;
}

void SkinningWindow3::UpdateConstants(float time)
{
    // Create some arbitrary skinning transformations.
    float const factor = static_cast<float>(GTE_C_PI) / 1.25f;
    float const div = std::trunc(time / factor);

    // The angle in in [-factor/4, +factor/4].
    float const maxAngle =
        std::fabs(time - (div + 0.5f) * factor) - 0.25f * factor;

    auto const& cbuffer = mSkinningEffect->GetSkinningMatricesConstant();
    auto* skinningMatrices = cbuffer->Get<Matrix4x4<float>>();
    for (size_t i = 0; i < 4; ++i)
    {
        float fi = static_cast<float>(i);
        float angle{};
        if (static_cast<int32_t>(time / factor + 0.25f) & 1)
        {
            angle = std::fabs(fi - 1.5f) * maxAngle;
        }
        else
        {
            angle = (fi - 1.5f) * maxAngle;
        }

        float cs = std::cos(angle);
        float sn = std::sin(angle);
        float yTrn = 10.0f * std::sin(time + 0.5f * fi);

        auto& skinningMatrix = skinningMatrices[i];
        skinningMatrix[0] = cs;
        skinningMatrix[1] = -sn;
        skinningMatrix[2] = 0.0f;
        skinningMatrix[3] = 0.0f;
        skinningMatrix[4] = sn;
        skinningMatrix[5] = cs;
        skinningMatrix[6] = 0.0f;
        skinningMatrix[7] = yTrn;
        skinningMatrix[8] = 0.0f;
        skinningMatrix[9] = 0.0f;
        skinningMatrix[10] = 1.0f;
        skinningMatrix[11] = 0.0f;
        skinningMatrix[12] = 0.0f;
        skinningMatrix[13] = 0.0f;
        skinningMatrix[14] = 0.0f;
        skinningMatrix[15] = 1.0f;
    }

    mEngine->Update(cbuffer);
}
