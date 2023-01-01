// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "CubeMapsWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>
#include <random>

CubeMapsWindow3::CubeMapsWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;

    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 10.0f, 0.01f, 0.01f,
        { 0.0f, 0.0f, -0.85f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    CreateScene();
    mPVWMatrices.Update();
    mCuller.ComputeVisibleSet(mCamera, mScene);
}

void CubeMapsWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        if (mCubeMapEffect->DynamicUpdates())
        {
            // Cull the sphere object because it is the object that
            // reflects the environment.
            mSphere->culling = CullingMode::ALWAYS;

            // You can take a snapshot of the environment from any camera
            // position and camera orientation.  In this application, the
            // environment is always rendered from the center of the cube
            // object and using the axes of that cube for the orientation.
            mCubeMapEffect->UpdateFaces(mEngine, mScene, mCuller,
                { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f, 0.0f });

            // Restore the sphere object's culling state.
            mSphere->culling = CullingMode::DYNAMIC;
        }

        mPVWMatrices.Update();
        mCuller.ComputeVisibleSet(mCamera, mScene);
    }

    mCubeMapEffect->SetWMatrix(mSphere->worldTransform);
    mEngine->Update(mCubeMapEffect->GetWMatrixConstant());
    mCubeMapEffect->SetCameraWorldPosition(mCamera->GetPosition());
    mEngine->Update(mCubeMapEffect->GetCameraWorldPositionConstant());

    mEngine->ClearBuffers();
    for (auto const& visual : mCuller.GetVisibleSet())
    {
        mEngine->Draw(visual);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool CubeMapsWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'c':
    case 'C':
        if (mEngine->GetRasterizerState() == mNoCullState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool CubeMapsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    std::vector<std::string> inputs =
    {
        "XmFace.png",
        "XpFace.png",
        "YmFace.png",
        "YpFace.png",
        "ZmFace.png",
        "ZpFace.png"
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

void CubeMapsWindow3::CreateScene()
{
    // Create the root of the scene.
    mScene = std::make_shared<Node>();

    // Create the walls of the cube room.  Each of the six texture images is
    // RGBA 64-by-64.
    auto room = std::make_shared<Node>();
    mScene->AttachChild(room);

    // The vertex format shared by the room walls.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    // The index buffer shared by the room walls.
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(uint32_t));
    auto* indices = ibuffer->Get<uint32_t>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 3;
    indices[3] = 0;  indices[4] = 3;  indices[5] = 2;

    std::shared_ptr<VertexBuffer> vbuffer;
    Vertex* vertices;
    std::shared_ptr<Texture2> texture;
    std::shared_ptr<Texture2Effect> effect;
    std::shared_ptr<Visual> wall;
    SamplerState::Filter filter = SamplerState::Filter::MIN_L_MAG_L_MIP_L;
    SamplerState::Mode mode = SamplerState::Mode::WRAP;

    // +x wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertices = vbuffer->Get<Vertex>();
    vertices[0] = { { +1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f } };
    vertices[1] = { { +1.0f, -1.0f, +1.0f }, { 0.0f, 1.0f } };
    vertices[2] = { { +1.0f, +1.0f, -1.0f }, { 1.0f, 0.0f } };
    vertices[3] = { { +1.0f, +1.0f, +1.0f }, { 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("XpFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "+x wall";

    // -x wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertices = vbuffer->Get<Vertex>();
    vertices[0] = { { -1.0f, -1.0f, +1.0f },{ 1.0f, 1.0f } };
    vertices[1] = { { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f } };
    vertices[2] = { { -1.0f, +1.0f, +1.0f },{ 1.0f, 0.0f } };
    vertices[3] = { { -1.0f, +1.0f, -1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("XmFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "-x wall";

    // +y wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertices = vbuffer->Get<Vertex>();
    vertices[0] = { { +1.0f, +1.0f, +1.0f },{ 1.0f, 1.0f } };
    vertices[1] = { { -1.0f, +1.0f, +1.0f },{ 0.0f, 1.0f } };
    vertices[2] = { { +1.0f, +1.0f, -1.0f },{ 1.0f, 0.0f } };
    vertices[3] = { { -1.0f, +1.0f, -1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("YpFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "+y wall";

    // -y wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertices = vbuffer->Get<Vertex>();
    vertices[0] = { { +1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f } };
    vertices[1] = { { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f } };
    vertices[2] = { { +1.0f, -1.0f, +1.0f },{ 1.0f, 0.0f } };
    vertices[3] = { { -1.0f, -1.0f, +1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("YmFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "-y wall";

    // +z wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertices = vbuffer->Get<Vertex>();
    vertices[0] = { { +1.0f, -1.0f, +1.0f },{ 1.0f, 1.0f } };
    vertices[1] = { { -1.0f, -1.0f, +1.0f },{ 0.0f, 1.0f } };
    vertices[2] = { { +1.0f, +1.0f, +1.0f },{ 1.0f, 0.0f } };
    vertices[3] = { { -1.0f, +1.0f, +1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("ZpFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "+z wall";

    // -z wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertices = vbuffer->Get<Vertex>();
    vertices[0] = { { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f } };
    vertices[1] = { { +1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f } };
    vertices[2] = { { -1.0f, +1.0f, -1.0f },{ 1.0f, 0.0f } };
    vertices[3] = { { +1.0f, +1.0f, -1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("ZmFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "-z wall";

    // A sphere to reflect the environment via a cube map.  The colors will
    // be used to modulate the cube map texture.
    struct SVertex
    {
        Vector3<float> position, normal, color;
    };
    VertexFormat svformat;
    svformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    svformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    svformat.Bind(VASemantic::COLOR, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(svformat);
    mSphere = mf.CreateSphere(64, 64, 0.125f);
    mSphere->UpdateModelBound();
    room->AttachChild(mSphere);

    // Generate random vertex colors for the sphere.  The MeshFactory class
    // produces a sphere with duplicated vertices along a longitude line.
    // This allows texture coordinates to be assigned in a manner that treats
    // the sphere as if it were a rectangle mesh.  For vertex colors, we want
    // the duplicated vertices to have the same color, so a hash table is used
    // to look up vertex colors for the duplicates.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rndG(0.5f, 0.75f);
    std::uniform_real_distribution<float> rndB(0.75f, 1.0f);
    vbuffer = mSphere->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumElements();
    SVertex* svertex = vbuffer->Get<SVertex>();
    std::map<Vector3<float>, Vector3<float>> dataMap;
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        auto const& element = dataMap.find(svertex[i].position);
        if (element != dataMap.end())
        {
            svertex[i].color = element->second;
        }
        else
        {
            svertex[i].color = { 0.0f, rndG(mte), rndB(mte) };
        }
    }

    // Create the cube map effect.
    std::string name[6] =
    {
        "XpFace.png",
        "XmFace.png",
        "YpFace.png",
        "YmFace.png",
        "ZpFace.png",
        "ZmFace.png"
    };

    // The cube-map faces are 64x64 textures.
    mCubeTexture = std::make_shared<TextureCube>(DF_R8G8B8A8_UNORM, 64, true);
    mCubeTexture->AutogenerateMipmaps();
    mCubeTexture->SetCopy(Resource::Copy::CPU_TO_STAGING);
    for (int32_t face = 0; face < 6; ++face)
    {
        std::string textureName = mEnvironment.GetPath(name[face]);
        texture = WICFileIO::Load(textureName, true);
        std::memcpy(mCubeTexture->GetDataFor(face, 0), texture->GetData(), texture->GetNumBytes());
    }
    float const reflectivity = 0.5f;
    mCubeMapEffect = std::make_shared<CubeMapEffect>(mProgramFactory, mCubeTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP, reflectivity);

    mSphere->SetEffect(mCubeMapEffect);
    mPVWMatrices.Subscribe(mSphere->worldTransform, mCubeMapEffect->GetPVWMatrixConstant());

    mScene->Update();
}
