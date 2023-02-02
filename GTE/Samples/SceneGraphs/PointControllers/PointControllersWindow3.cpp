// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "PointControllersWindow3.h"
#include <Graphics/VertexColorEffect.h>

PointControllersWindow3::PointControllersWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mApplicationTime(0.0),
    mApplicationDeltaTime(0.001)
{
    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.01f, 0.01f,
        { 4.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
    mTrackBall.Update();
}

void PointControllersWindow3::OnIdle()
{
    mTimer.Measure();

    mPoints->Update(mApplicationTime);
    mApplicationTime += mApplicationDeltaTime;

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mPoints);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

void PointControllersWindow3::CreateScene()
{
    std::default_random_engine dre;
    std::uniform_real_distribution<float> urd(-1.0f, 1.0f);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    uint32_t numVertices = 1024;
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].position = { urd(dre), urd(dre), urd(dre) };
        vertices[i].color = { urd(dre), urd(dre), urd(dre), 1.0f };
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, numVertices);

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    mPoints = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mPoints->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mPoints);

    mRandomController = std::make_shared<RandomController>(mUpdater);
    mPoints->AttachController(mRandomController);
}
