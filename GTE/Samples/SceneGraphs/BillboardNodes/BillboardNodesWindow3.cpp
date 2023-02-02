// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BillboardNodesWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

BillboardNodesWindow3::BillboardNodesWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({0.9f, 0.9f, 0.9f, 1.0f});

    // InitializeCamera(...) occurs before CreateScene() because the billboard
    // node creation requires the camera to be initialized.
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.005f, 0.002f,
        { 0.0f, -1.0f, 0.25f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    CreateScene();
    mPVWMatrices.Update();
    mCuller.ComputeVisibleSet(mCamera, mScene);
}

void BillboardNodesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
        mCuller.ComputeVisibleSet(mCamera, mScene);
    }

    mEngine->ClearBuffers();
    for (auto const& visual : mCuller.GetVisibleSet())
    {
        mEngine->Draw(visual);
    }

#if defined(DEMONSTRATE_VIEWPORT_BOUNDING_RECTANGLE)
    ComputeTorusBoundingRectangle();
    mEngine->SetBlendState(mBlendState);
    auto const& rstate = mEngine->GetRasterizerState();
    mEngine->SetRasterizerState(mNoCullState);
    mEngine->Draw(mOverlay);
    mEngine->SetRasterizerState(rstate);
    mEngine->SetDefaultBlendState();
#endif

    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BillboardNodesWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'p':
    case 'P':
        if (mEngine->GetRasterizerState() != mCullCWState)
        {
            Matrix4x4<float> xReflect = Matrix4x4<float>::Identity();
            xReflect(0, 0) = -1.0f;
            mCamera->SetPostProjectionMatrix(xReflect);
            mEngine->SetRasterizerState(mCullCWState);
        }
        else
        {
            mCamera->SetPostProjectionMatrix(Matrix4x4<float>::Identity());
            mEngine->SetDefaultRasterizerState();
        }
        mPVWMatrices.Update();
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool BillboardNodesWindow3::OnMouseMotion(MouseButton button, int32_t x, int32_t y,
    uint32_t modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        mPVWMatrices.Update();
        mCuller.ComputeVisibleSet(mCamera, mScene);
    }
    return true;
}

bool BillboardNodesWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "BlueGrid.png",
        "RedSky.png"
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

void BillboardNodesWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();

    std::string path = mEnvironment.GetPath("BlueGrid.png");
    mGroundTexture = WICFileIO::Load(path, true);
    mGroundTexture->AutogenerateMipmaps();

    path = mEnvironment.GetPath("RedSky.png");
    mSkyTexture = WICFileIO::Load(path, false);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    // Create the ground.  It covers a square with vertices (1,1,0), (1,-1,0),
    // (-1,1,0), and (-1,-1,0).  Multiply the texture coordinates by a factor
    // to enhance the wrap-around.
    mGround = mf.CreateRectangle(2, 2, 16.0f, 16.0f);
    mScene->AttachChild(mGround);
    auto const& vbuffer = mGround->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].tcoord *= 128.0f;
    }

    // Create a texture effect for the ground.
    auto groundEffect = std::make_shared<Texture2Effect>(mProgramFactory, mGroundTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP, SamplerState::Mode::WRAP);
    mGround->SetEffect(groundEffect);

    // Create a rectangle mesh.  The mesh is in the xy-plane.  Do not apply
    // local transformations to the mesh.  Use the billboard node transforms
    // to control the mesh location and orientation.
    mRectangle = mf.CreateRectangle(2, 2, 0.125f, 0.25f);

    // Create a texture effect for the rectangle.
    auto rectEffect = std::make_shared<Texture2Effect>(mProgramFactory, mSkyTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mRectangle->SetEffect(rectEffect);

    // Create a torus mesh.  Do not apply local transformations to the mesh.
    // Use the billboard node transforms to control the mesh location and
    // orientation.
    mTorus = mf.CreateTorus(16, 16, 1.0f, 0.25f);
    mTorus->localTransform.SetUniformScale(0.1f);

    // Create a texture effect for the torus.
    auto torusEffect = std::make_shared<Texture2Effect>(mProgramFactory, mSkyTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mTorus->SetEffect(torusEffect);

    // Create a billboard node that causes a rectangle always to be facing the
    // camera.  This is the type of billboard for an avatar.
    mBillboard0 = std::make_shared<BillboardNode>(mCamera);
    mBillboard0->AttachChild(mRectangle);
    mScene->AttachChild(mBillboard0);

    // The billboard rotation is about its model-space up-vector (0,1,0).  In
    // this application, world-space up is (0,0,1).  Locally rotate the
    // billboard so it's up-vector matches the world's.
    AxisAngle<4, float> aa(Vector4<float>::Unit(0), (float)GTE_C_HALF_PI);
    mBillboard0->localTransform.SetTranslation(-0.25f, 0.0f, 0.25f);
    mBillboard0->localTransform.SetRotation(aa);

    // Create a billboard node that causes the torus always to be oriented the
    // same way relative to the camera.
    mBillboard1 = std::make_shared<BillboardNode>(mCamera);
    mBillboard1->AttachChild(mTorus);
    mScene->AttachChild(mBillboard1);

    // The billboard rotation is about its model-space up-vector (0,1,0).  In
    // this application, world-space up is (0,0,1).  Locally rotate the
    // billboard so it's up-vector matches the world's.
    mBillboard1->localTransform.SetTranslation(0.25f, 0.0f, 0.25f);
    mBillboard1->localTransform.SetRotation(aa);

    // When the trackball moves, automatically update the PVW matrices that
    // are used by the effects.
    mPVWMatrices.Subscribe(mGround->worldTransform, groundEffect->GetPVWMatrixConstant());
    mPVWMatrices.Subscribe(mRectangle->worldTransform, rectEffect->GetPVWMatrixConstant());
    mPVWMatrices.Subscribe(mTorus->worldTransform, torusEffect->GetPVWMatrixConstant());

    // Attach the scene to the virtual trackball.  When the trackball moves,
    // the W matrix of the scene is updated automatically.  The W matrices
    // of the child objects are also updated by the hierarchical update.
    mTrackBall.Attach(mScene);
    mTrackBall.Update();

#if defined(DEMONSTRATE_VIEWPORT_BOUNDING_RECTANGLE)
    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, 1, 1, SamplerState::Filter::MIN_P_MAG_P_MIP_P, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP, true);
    auto overlayTexture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, 1, 1);
    mOverlay->SetTexture(overlayTexture);
    uint32_t& texel = *overlayTexture->Get<uint32_t>();
    texel = 0x40FF0000;  // (r,g,b,a) = (0,0,255,64)

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
#endif

#if defined(DEMONSTRATE_POST_PROJECTION_REFLECTION)
    mCullCWState = std::make_shared<RasterizerState>();
    mCullCWState->cull = RasterizerState::Cull::FRONT;
#endif
}

#if defined(DEMONSTRATE_VIEWPORT_BOUNDING_RECTANGLE)

void BillboardNodesWindow3::ComputeTorusBoundingRectangle()
{
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float> wMatrix = mTorus->worldTransform;
    Matrix4x4<float> pvwMatrix = DoTransform(pvMatrix, wMatrix);

    auto const& vbuffer = mTorus->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    auto const* vertex = vbuffer->Get<Vertex>();

    // Compute the extremes of the normalized display coordinates.
    float constexpr maxFloat = std::numeric_limits<float>::max();
    float xmin = maxFloat, xmax = -maxFloat;
    float ymin = maxFloat, ymax = -maxFloat;
    for (uint32_t i = 0; i < numVertices; ++i, ++vertex)
    {
        Vector4<float> input{ vertex->position[0], vertex->position[1], vertex->position[2], 1.0f };
        Vector4<float> output = DoTransform(pvwMatrix, input);

        // Reflect the y-values because the normalized display coordinates
        // are right-handed but the overlay rectangle coordinates are
        // left-handed.
        float invW = 1.0f / output[3];
        float x = output[0] * invW;
        float y = -output[1] * invW;
        if (x < xmin)
        {
            xmin = x;
        }
        if (x > xmax)
        {
            xmax = x;
        }
        if (y < ymin)
        {
            ymin = y;
        }
        if (y > ymax)
        {
            ymax = y;
        }
    }

    // Map normalized display coordinates [-1,1]^2 to [0,1]^2.
    xmin = 0.5f * (xmin + 1.0f);
    xmax = 0.5f * (xmax + 1.0f);
    ymin = 0.5f * (ymin + 1.0f);
    ymax = 0.5f * (ymax + 1.0f);

    // Update the overlay to the region covered by the bounding rectangle.
    std::array<int32_t, 4> rectangle{};
    rectangle[0] = static_cast<int32_t>(xmin * mXSize);
    rectangle[1] = static_cast<int32_t>(ymin * mYSize);
    rectangle[2] = static_cast<int32_t>((xmax - xmin) * mXSize);
    rectangle[3] = static_cast<int32_t>((ymax - ymin) * mYSize);
    mOverlay->SetOverlayRectangle(rectangle);
    mEngine->Update(mOverlay->GetVertexBuffer());
}

#endif
