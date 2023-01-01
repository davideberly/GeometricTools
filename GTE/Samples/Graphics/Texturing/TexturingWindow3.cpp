// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "TexturingWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/Texture2Effect.h>

TexturingWindow3::TexturingWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 0.0f, 0.0f, 1.25f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void TexturingWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

#if defined(SAVE_RENDERING_TO_DISK)
    mEngine->Enable(mTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);
    mEngine->Disable(mTarget);
    mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
    WICFileIO::SaveToPNG("Texturing.png", mTarget->GetRTTexture(0));
#endif

    mTimer.UpdateFrameCount();
}

bool TexturingWindow3::SetEnvironment()
{
    // Set the search path to find images to load.
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("StoneWall.png") == "")
    {
        LogError("Cannot find file Stonewall.png");
        return false;
    }

    return true;
}

void TexturingWindow3::CreateScene()
{
    // Create a vertex buffer for a two-triangles square.  The PNG is stored
    // in left-handed coordinates.  The texture coordinates are chosen to
    // reflect the texture in the y-direction.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto* vertices = vbuffer->Get<Vertex>();
    vertices[0].position = { 0.0f, 0.0f, 0.0f };
    vertices[0].tcoord = { 0.0f, 1.0f };
    vertices[1].position = { 1.0f, 0.0f, 0.0f };
    vertices[1].tcoord = { 1.0f, 1.0f };
    vertices[2].position = { 0.0f, 1.0f, 0.0f };
    vertices[2].tcoord = { 0.0f, 0.0f };
    vertices[3].position = { 1.0f, 1.0f, 0.0f };
    vertices[3].tcoord = { 1.0f, 0.0f };

    // Create an indexless buffer for a triangle mesh with two triangles.
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRISTRIP, 2);

    // Create an effect for the vertex and pixel shaders.  The texture is
    // bilinearly filtered and the texture coordinates are clamped to [0,1]^2.
    auto myTexture = WICFileIO::Load(mEnvironment.GetPath("StoneWall.png"), false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, myTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSquare->localTransform.SetTranslation(-0.5f, -0.5f, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mPVWMatrices.Subscribe(mSquare->worldTransform, effect->GetPVWMatrixConstant());

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize,
        mYSize);
    mTarget->GetRTTexture(0)->SetCopy(Resource::Copy::STAGING_TO_CPU);
#endif

    mTrackBall.Attach(mSquare);
    mTrackBall.Update();
}
