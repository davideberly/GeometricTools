// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "VertexColoringWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/VertexColorEffect.h>

VertexColoringWindow3::VertexColoringWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 0.0f, 0.0f, 1.25f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize, mYSize);
    mTarget->GetRTTexture(0)->SetCopy(Resource::Copy::STAGING_TO_CPU);
#endif
}

void VertexColoringWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mTriangle);
    mEngine->DisplayColorBuffer(0);

#if defined(SAVE_RENDERING_TO_DISK)
    mEngine->Enable(mTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mTriangle);
    mEngine->Disable(mTarget);
    mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
    WICFileIO::SaveToPNG("VertexColoring.png", mTarget->GetRTTexture(0));
#endif

    mTimer.UpdateFrameCount();
}

void VertexColoringWindow3::CreateScene()
{
    // Create a vertex buffer for a single triangle.
    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 3);
    auto* vertices = vbuffer->Get<Vertex>();
    vertices[0].position = { 0.0f, 0.0f, 0.0f };
    vertices[0].color = { 1.0f, 0.0f, 0.0f, 1.0f };
    vertices[1].position = { 1.0f, 0.0f, 0.0f };
    vertices[1].color = { 0.0f, 1.0f, 0.0f, 1.0f };
    vertices[2].position = { 0.0f, 1.0f, 0.0f };
    vertices[2].color = { 0.0f, 0.0f, 1.0f, 1.0f };

    // Create an indexless buffer for a triangle mesh with one triangle.
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 1);

    // Create an effect for the vertex and pixel shaders.
    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mTriangle = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    float const negOneThird = -1.0f / 3.0f;
    mTriangle->localTransform.SetTranslation(negOneThird, negOneThird, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mPVWMatrices.Subscribe(mTriangle->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mTriangle);
    mTrackBall.Update();
}
