// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "NURBSSphereWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>

NURBSSphereWindow3::NURBSSphereWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullWireState);

    CreateScene();

    InitializeCamera(60.0f, GetAspectRatio(), 0.001f, 100.0f, 0.001f, 0.001f,
        { 4.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void NURBSSphereWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mCurrentVisual);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool NURBSSphereWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullSolidState == mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullSolidState);
        }
        return true;

    case '0':
        mCurrentVisual = mEighthSphereVisual;
        return true;
    case '1':
        mCurrentVisual = mHalfSphereVisual;
        return true;
    case '2':
        mCurrentVisual = mFullSphereVisual;
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

void NURBSSphereWindow3::CreateScene()
{
    CreateEighthSphere();
    CreateHalfSphere();
    CreateFullSphere();
    mCurrentVisual = mEighthSphereVisual;
}

void NURBSSphereWindow3::CreateEighthSphere()
{
    int32_t const density = 32;
    Vector3<float> values[6];

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, density * density);
    auto vertices = vbuffer->Get<Vector3<float>>();
    std::memset(vbuffer->GetData(), 0, vbuffer->GetNumBytes());
    float const divisor = static_cast<float>(density - 1);
    for (int32_t iv = 0; iv <= density - 1; ++iv)
    {
        float v = static_cast<float>(iv) / divisor;
        for (int32_t iu = 0; iu + iv <= density - 1; ++iu)
        {
            float u = static_cast<float>(iu) / divisor;
            mEighthSphere.Evaluate(u, v, 0, values);
            vertices[iu + density * iv] = values[0];
        }
    }

    std::vector<int32_t> indices;
    for (int32_t iv = 0; iv <= density - 2; ++iv)
    {
        // two triangles per square
        int32_t iu, j0, j1, j2, j3;
        for (iu = 0; iu + iv <= density - 3; ++iu)
        {
            j0 = iu + density * iv;
            j1 = j0 + 1;
            j2 = j0 + density;
            j3 = j2 + 1;
            indices.push_back(j0);
            indices.push_back(j1);
            indices.push_back(j2);
            indices.push_back(j1);
            indices.push_back(j3);
            indices.push_back(j2);
        }

        // last triangle in row is singleton
        j0 = iu + density * iv;
        j1 = j0 + 1;
        j2 = j0 + density;
        indices.push_back(j0);
        indices.push_back(j1);
        indices.push_back(j2);
    }

    uint32_t numTriangles = static_cast<uint32_t>(indices.size() / 3);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(int32_t));
    std::memcpy(ibuffer->GetData(), indices.data(), indices.size() * sizeof(int32_t));

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    mEighthSphereVisual = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mEighthSphereVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mEighthSphereVisual);
}

void NURBSSphereWindow3::CreateHalfSphere()
{
    int32_t const density = 32;
    Vector3<float> values[6];

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mHalfSphereVisual = mf.CreateRectangle(density, density, 1.0f, 1.0f);
    auto const& vbuffer = mHalfSphereVisual->GetVertexBuffer();
    auto vertices = vbuffer->Get<Vector3<float>>();
    float const divisor = static_cast<float>(density - 1);
    for (int32_t iv = 0; iv < density; ++iv)
    {
        float v = static_cast<float>(iv) / divisor;
        for (int32_t iu = 0; iu < density; ++iu)
        {
            float u = static_cast<float>(iu) / divisor;
            mHalfSphere.Evaluate(u, v, 0, values);
            vertices[iu + density * iv] = values[0];
        }
    }

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    mHalfSphereVisual->SetEffect(effect);
    mPVWMatrices.Subscribe(mHalfSphereVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mHalfSphereVisual);
}

void NURBSSphereWindow3::CreateFullSphere()
{
    int32_t const density = 32;
    Vector3<float> values[6];

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mFullSphereVisual = mf.CreateRectangle(density, density, 1.0f, 1.0f);
    auto const& vbuffer = mFullSphereVisual->GetVertexBuffer();
    auto vertices = vbuffer->Get<Vector3<float>>();
    float const divisor = static_cast<float>(density - 1);
    for (int32_t iv = 0; iv < density; ++iv)
    {
        float v = static_cast<float>(iv) / divisor;
        for (int32_t iu = 0; iu < density; ++iu)
        {
            float u = static_cast<float>(iu) / divisor;
            mFullSphere.Evaluate(u, v, 0, values);
            vertices[iu + density * iv] = values[0];
        }
    }

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    mFullSphereVisual->SetEffect(effect);
    mPVWMatrices.Subscribe(mFullSphereVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mFullSphereVisual);
}
