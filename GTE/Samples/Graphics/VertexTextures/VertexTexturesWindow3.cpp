// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "VertexTexturesWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>

VertexTexturesWindow3::VertexTexturesWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 10000.0f, 0.01f, 0.01f,
        { 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    CreateMesh();
    mPVWMatrices.Update();
}

void VertexTexturesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mHeightMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool VertexTexturesWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("HeightField.png") == "")
    {
        LogError("Cannot find file HeightField.png");
        return false;
    }

    return true;
}

void VertexTexturesWindow3::CreateMesh()
{
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mHeightMesh = mf.CreateRectangle(32, 32, 1.0f, 1.0f);

    std::string path = mEnvironment.GetPath("HeightField.png");
    auto texture = WICFileIO::Load(path, false);
    mEffect = std::make_shared<DisplacementEffect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mHeightMesh->SetEffect(mEffect);

    mPVWMatrices.Subscribe(mHeightMesh->worldTransform, mEffect->GetPVWMatrixConstant());
    mTrackBall.Attach(mHeightMesh);
}
