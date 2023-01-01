// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "MinimumVolumeSphere3DWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>

MinimumVolumeSphere3DWindow3::MinimumVolumeSphere3DWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mNumActive(2),
    mVertices(NUM_POINTS)
{
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullWireState);

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.1f, 0.01f,
        { 0.0f, 0.0f, -4.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();

    mMVS3(mNumActive, &mVertices[0], mMinimalSphere);
    UpdateScene();
}

void MinimumVolumeSphere3DWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    for (int32_t i = 0; i < mNumActive; ++i)
    {
        mEngine->Draw(mPoints[i]);
    }
    mEngine->Draw(mSegments);
    mEngine->Draw(mSphere);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MinimumVolumeSphere3DWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'n':
    case 'N':
        if (mNumActive < NUM_POINTS)
        {
            mMVS3(++mNumActive, &mVertices[0], mMinimalSphere);
            UpdateScene();
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void MinimumVolumeSphere3DWindow3::CreateScene()
{
    // Create the points.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    for (auto& v : mVertices)
    {
        v = { rnd(mte), rnd(mte), rnd(mte) };
    }

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    std::shared_ptr<ConstantColorEffect> effect;
    Vector4<float> gray{ 0.5f, 0.5f, 0.5f, 1.0f };
    for (int32_t i = 0; i < NUM_POINTS; ++i)
    {
        mPoints[i] = mf.CreateSphere(6, 6, 0.01f);
        effect = std::make_shared<ConstantColorEffect>(mProgramFactory, gray);
        mPoints[i]->SetEffect(effect);
        mPVWMatrices.Subscribe(mPoints[i]->worldTransform, effect->GetPVWMatrixConstant());

        auto const& vbuffer = mPoints[i]->GetVertexBuffer();
        auto* vertex = vbuffer->Get<Vector3<float>>();
        Vector3<float> offset = mVertices[i];
        for (uint32_t j = 0; j < vbuffer->GetNumElements(); ++j)
        {
            vertex[j] += offset;
        }
    }

    // Create the segments.
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 12);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 6);
    Vector4<float> red{ 0.5f, 0.0f, 0.0f, 1.0f };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, red);
    mSegments = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mSegments->worldTransform, effect->GetPVWMatrixConstant());
    mSegments->Update();

    // Create the sphere.
    mSphere = mf.CreateSphere(16, 16, 1.0f);

    Vector4<float> blue{ 0.0f, 0.0f, 0.5f, 1.0f };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, blue);

    mSphere->SetEffect(effect);
    mPVWMatrices.Subscribe(mSphere->worldTransform, effect->GetPVWMatrixConstant());
}

void MinimumVolumeSphere3DWindow3::UpdateScene()
{
    // Update the segments.
    auto const& vbuffer = mSegments->GetVertexBuffer();
    auto* vertex = vbuffer->Get<Vector3<float>>();

    int32_t numSupport = mMVS3.GetNumSupport();
    std::array<int32_t, 4> support = mMVS3.GetSupport();

    if (numSupport >= 2)
    {
        vertex[0] = mVertices[support[0]];
        vertex[1] = mVertices[support[1]];
        vbuffer->SetNumActiveElements(2);
    }

    if (numSupport >= 3)
    {
        vertex[2] = mVertices[support[1]];
        vertex[3] = mVertices[support[2]];
        vertex[4] = mVertices[support[2]];
        vertex[5] = mVertices[support[0]];
        vbuffer->SetNumActiveElements(6);
    }

    if (numSupport == 4)
    {
        vertex[ 6] = mVertices[support[3]];
        vertex[ 7] = mVertices[support[0]];
        vertex[ 8] = mVertices[support[3]];
        vertex[ 9] = mVertices[support[1]];
        vertex[10] = mVertices[support[3]];
        vertex[11] = mVertices[support[2]];
        vbuffer->SetNumActiveElements(12);
    }

    mEngine->Update(vbuffer);

    // Update the sphere.
    mSphere->localTransform.SetTranslation(mMinimalSphere.center);
    mSphere->localTransform.SetUniformScale(mMinimalSphere.radius);
    mSphere->Update();

    mPVWMatrices.Update();
}
