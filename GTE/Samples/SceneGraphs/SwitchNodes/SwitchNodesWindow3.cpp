// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.11

#include "SwitchNodesWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

SwitchNodesWindow3::SwitchNodesWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mSwitchNode{},
    mCuller{}
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    float angle = static_cast<float>(GTE_C_PI / 6.0);
    float cs = std::cos(angle), sn = std::sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 100.0f, 0.01f, 0.001f,
        { 0.0f, -4.0f, 2.0f }, { 0.0f, cs, -sn }, { 0.0f, sn, cs });

    CreateScene();

    mTrackBall.Update();
    mPVWMatrices.Update();
    mCuller.ComputeVisibleSet(mCamera, mScene);
}

void SwitchNodesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
        mCuller.ComputeVisibleSet(mCamera, mScene);
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mCuller.GetVisibleSet());
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool SwitchNodesWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'c':
    case 'C':
    {
        int32_t child = mSwitchNode->GetActiveChild();
        if (++child == mSwitchNode->GetNumChildren())
        {
            child = 0;
        }
        mSwitchNode->SetActiveChild(child);
        mCuller.ComputeVisibleSet(mCamera, mScene);
        return true;
    }
    }

    return Window3::OnCharPress(key, x, y);
}

bool SwitchNodesWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    std::vector<std::string> inputs =
    {
        "Flower.png"
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

void SwitchNodesWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();
    mTrackBall.Attach(mScene);

    mSwitchNode = std::make_shared<SwitchNode>();
    mScene->AttachChild(mSwitchNode);

    std::string textureFile = mEnvironment.GetPath("Flower.png");
    auto texture = WICFileIO::Load(textureFile, true);
    texture->AutogenerateMipmaps();

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf{};
    mf.SetVertexFormat(vformat);

    auto mesh = mf.CreateRectangle(4, 4, 1.0f, 1.0f);
    AttachEffect(mesh, texture);

    mesh = mf.CreateDisk(8, 16, 1.0f);
    AttachEffect(mesh, texture);

    mesh = mf.CreateBox(1.0f, 0.5f, 0.25f);
    AttachEffect(mesh, texture);

    mesh = mf.CreateCylinderClosed(8, 16, 1.0f, 2.0f);
    AttachEffect(mesh, texture);

    mesh = mf.CreateSphere(32, 16, 1.0f);
    AttachEffect(mesh, texture);

    mesh = mf.CreateTorus(16, 16, 1.0f, 0.25f);
    AttachEffect(mesh, texture);

    mesh = mf.CreateTetrahedron();
    AttachEffect(mesh, texture);

    mesh = mf.CreateHexahedron();
    AttachEffect(mesh, texture);

    mesh = mf.CreateOctahedron();
    AttachEffect(mesh, texture);

    mesh = mf.CreateDodecahedron();
    AttachEffect(mesh, texture);

    mesh = mf.CreateIcosahedron();
    AttachEffect(mesh, texture);

    mSwitchNode->SetActiveChild(0);
}

void SwitchNodesWindow3::AttachEffect(std::shared_ptr<Visual> const& mesh,
    std::shared_ptr<Texture2> const& texture)
{
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP);
    mesh->SetEffect(effect);
    mSwitchNode->AttachChild(mesh);
    mPVWMatrices.Subscribe(mesh);
}
