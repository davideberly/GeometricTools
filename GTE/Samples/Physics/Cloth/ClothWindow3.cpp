// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ClothWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/Texture2Effect.h>

//#define SINGLE_STEP

ClothWindow3::ClothWindow3(Parameters& parameters)
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
    mEngine->SetRasterizerState(mNoCullState);

    mWireNoCullState = std::make_shared<RasterizerState>();
    mWireNoCullState->fill = RasterizerState::Fill::WIREFRAME;
    mWireNoCullState->cull = RasterizerState::Cull::NONE;

    mEngine->SetClearColor({ 0.85f, 0.85f, 1.0f, 1.0f });

    CreateSprings();
    CreateCloth();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.01f,
        { 0.0f, -1.75f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();

    mAnimStartTime = mAnimTimer.GetSeconds();
}

void ClothWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(SINGLE_STEP)
    PhysicsTick();
#endif

    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool ClothWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
    case 'W':
        if (mNoCullState != mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        else
        {
            mEngine->SetRasterizerState(mWireNoCullState);
        }
        return true;

#if defined(SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool ClothWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Cloth.png") == "")
    {
        LogError("Cannot find file Cloth.png.");
        return false;
    }

    return true;
}

void ClothWindow3::CreateSprings()
{
    // Set up the mass-spring system.
    int32_t numRows = 8;
    int32_t numCols = 16;
    float step = 0.01f;
    Vector3<float> gravity{ 0.0f, 0.0f, -1.0f };
    Vector3<float> wind{ 0.5f, 0.0f, 0.0f };
    float viscosity = 10.0f;
    float maxAmplitude = 2.0f;
    mModule = std::make_unique<PhysicsModule>(numRows, numCols, step, gravity,
        wind, viscosity, maxAmplitude);

    // The top r of the mesh is immovable (infinite mass).  All other masses
    // are constant.
    int32_t r, c;
    for (c = 0; c < numCols; ++c)
    {
        mModule->SetMass(numRows - 1, c, std::numeric_limits<float>::max());
    }
    for (r = 0; r < numRows - 1; ++r)
    {
        for (c = 0; c < numCols; ++c)
        {
            mModule->SetMass(r, c, 1.0f);
        }
    }

    // Initial position on a vertical axis-aligned rectangle, zero velocity.
    float rowFactor = 1.0f / static_cast<float>(numRows - 1);
    float colFactor = 1.0f / static_cast<float>(numCols - 1);
    for (r = 0; r < numRows; ++r)
    {
        for (c = 0; c < numCols; ++c)
        {
            float x = c * colFactor;
            float z = r * rowFactor;
            mModule->SetPosition(r, c, { x, 0.0f, z });
            mModule->SetVelocity(r, c, { 0.0f, 0.0f, 0.0f });
        }
    }

    // Springs are at rest in the initial configuration.
    float const rowConstant = 1000.0f;
    float const bottomConstant = 100.0f;
    Vector3<float> diff;
    for (r = 0; r < numRows; ++r)
    {
        for (c = 0; c < numCols - 1; ++c)
        {
            mModule->SetConstantC(r, c, rowConstant);
            diff = mModule->GetPosition(r, c + 1) - mModule->GetPosition(r, c);
            mModule->SetLengthC(r, c, Length(diff));
        }
    }

    for (r = 0; r < numRows - 1; ++r)
    {
        for (c = 0; c < numCols; ++c)
        {
            mModule->SetConstantR(r, c, bottomConstant);
            diff = mModule->GetPosition(r, c) - mModule->GetPosition(r + 1, c);
            mModule->SetLengthR(r, c, Length(diff));
        }
    }
}

void ClothWindow3::CreateCloth()
{
    MeshDescription desc(MeshTopology::RECTANGLE, 16, 32);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, desc.numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto vertices = vbuffer->Get<Vertex>();
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(uint32_t));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());

    BasisFunctionInput<float> input[2] =
    {
        BasisFunctionInput<float>(mModule->GetNumRows(), 2),
        BasisFunctionInput<float>(mModule->GetNumCols(), 2)
    };
    mSpline = std::make_shared<BSplineSurface<3, float>>(input, &mModule->GetPosition(0, 0));
    mSurface = std::make_unique<RectanglePatchMesh<float>>(desc, mSpline);

    std::string path = mEnvironment.GetPath("Cloth.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mCloth = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mCloth->UpdateModelBound();
    mCloth->localTransform.SetTranslation(-mCloth->modelBound.GetCenter());
    mPVWMatrices.Subscribe(mCloth->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mCloth);
    mTrackBall.Update();
}

void ClothWindow3::PhysicsTick()
{
    double currentTime = mAnimTimer.GetSeconds();
    double deltaTime = currentTime - mAnimStartTime;
    mModule->Update(static_cast<float>(deltaTime));

    // Update spline surface.  Remember that the spline maintains its own
    // copy of the control points, so this update is necessary.
    for (int32_t r = 0; r < mModule->GetNumRows(); ++r)
    {
        for (int32_t c = 0; c < mModule->GetNumCols(); ++c)
        {
            mSpline->SetControl(r, c, mModule->GetPosition(r, c));
        }
    }

    // Update the GPU copy of the vertices.
    mSurface->Update();
    mEngine->Update(mCloth->GetVertexBuffer());
    mPVWMatrices.Update();
}

void ClothWindow3::GraphicsTick()
{
    mEngine->ClearBuffers();
    mEngine->Draw(mCloth);
    mEngine->DisplayColorBuffer(0);
}
