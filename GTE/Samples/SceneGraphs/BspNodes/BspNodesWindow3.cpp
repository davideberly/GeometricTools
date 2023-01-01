// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BspNodesWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

BspNodesWindow3::BspNodesWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 0.0f, -1.0f, 0.1f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    DoCullSort();
}

void BspNodesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        DoCullSort();
    }

    mEngine->ClearBuffers();

    for (auto visual : mVisibleOpaque)
    {
        mEngine->Draw(visual);
    }

    mEngine->SetRasterizerState(mNoCullWireState);
    for (auto visual : mVisibleNoCullWire)
    {
        mEngine->Draw(visual);
    }
    mEngine->SetDefaultRasterizerState();

    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BspNodesWindow3::OnResize(int32_t xSize, int32_t ySize)
{
    if (Window3::OnResize(xSize, ySize))
    {
        DoCullSort();
    }
    return true;
}

bool BspNodesWindow3::OnMouseMotion(MouseButton button, int32_t x, int32_t y, uint32_t modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        DoCullSort();
    }
    return true;
}

bool BspNodesWindow3::SetEnvironment()
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

void BspNodesWindow3::CreateScene()
{
    // Create the scene graph.
    //
    // 1. The rectangles represent the BSP planes of the BSP tree.  They
    //    share a VertexColorEffect.  You can see a plane from either side
    //    (backface culling disabled).  The planes do not interfere with view
    //    of the solid objects (wirestate enabled).
    //
    // 2. The sphere, tetrahedron, and cube share a Texture2Effect.  These
    //    objects are convex.  The backfacing triangles are discarded
    //    (backface culling enabled).  The front facing triangles are drawn
    //    correctly by convexity, so depthbuffer reads are disabled and
    //    depthbuffer writes are enabled.  The BSP-based sorting of objects
    //    guarantees that front faces of convex objects in the foreground
    //    are drawn after the front faces of convex objects in the background,
    //    which allows us to set the depthbuffer state as we have.  That is,
    //    BSPNode sorts from back to front.
    //
    // 3. The torus has backface culling enabled and depth buffering enabled.
    //    This is necessary, because the torus is not convex.
    //
    // 4. Generally, if all objects are opaque, then you want to draw from
    //    front to back with depth buffering fully enabled.  You need to
    //    reverse-order the elements of the visible set before drawing.  If
    //    any of the objects are semitransparent, then drawing back to front
    //    is the correct order to handle transparency.  However, you do not
    //    get the benefit of early z-rejection for opaque objects.  A better
    //    BSP sorter needs to be built to produce a visible set with opaque
    //    objects listed first (front-to-back order) and semitransparent
    //    objects listed last (back-to-front order).
    //
    // trackball
    //     scene
    //         ground
    //         bsp0
    //             rectangle0
    //             bsp1
    //                 rectangle1
    //                 bsp3
    //                     rectangle3
    //                     torus
    //                     sphere
    //                 tetrahedron
    //             bsp2
    //                 rectangle2
    //                 cube
    //                 octahedron

    mScene = std::make_shared<Node>();
    mTrackBall.Attach(mScene);

    Quaternion<float> q{ 0.0757066011f, 0.0260398518f, -0.0892945006f, 0.992782414f };
    mScene->localTransform.SetRotation(q);

    // The scene consists of 6 opaque visuals, so create a visible set large
    // enough to store them.
    mVisibleOpaque.reserve(6);

    // Create the ground.  It covers a square with vertices (1,1,0), (1,-1,0),
    // (-1,1,0), and (-1,-1,0).  Multiply the texture coordinates by a factor
    // to enhance the wrap-around.
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

    mGround = mf.CreateRectangle(2, 2, 16.0f, 16.0f);
    mVisualOpaque.insert(mGround.get());
    mScene->AttachChild(mGround);
    auto const& vbuffer = mGround->GetVertexBuffer();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < vbuffer->GetNumElements(); ++i)
    {
        vertex[i].tcoord *= 128.0f;
    }

    std::string path = mEnvironment.GetPath("BlueGrid.png");
    std::shared_ptr<Texture2> groundTexture = WICFileIO::Load(path, true);
    groundTexture->AutogenerateMipmaps();
    std::shared_ptr<Texture2Effect> txEffect =
        std::make_shared<Texture2Effect>(mProgramFactory, groundTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
            SamplerState::Mode::WRAP);
    mGround->SetEffect(txEffect);

    // Partition the region above the ground into 5 convex pieces.  Each plane
    // is perpendicular to the ground (not required generally).
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;

    std::array<Vector4<float>, 4> color =
    {
        Vector4<float>{ 1.0f, 0.0f, 0.0f, 1.0f },
        Vector4<float>{ 0.0f, 0.5f, 0.0f, 1.0f },
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f },
        Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f }
    };

    std::array<Vector2<float>, 9> v =
    {
        Vector2<float>{ -1.0f, 1.0f },
        Vector2<float>{ 1.0f, -1.0f },
        Vector2<float>{ -0.25f, 0.25f },
        Vector2<float>{ -1.0f, -1.0f },
        Vector2<float>{ 0.0f, 0.0f },
        Vector2<float>{ 1.0f, 0.5f },
        Vector2<float>{ -0.75f, -7.0f / 12.0f },
        Vector2<float>{ -0.75f, 0.75f },
        Vector2<float>{ 1.0f, 1.0f }
    };

    for (int32_t i = 0; i < 4; ++i)
    {
        size_t twoI = 2 * static_cast<size_t>(i);
        mVCEffect[i] = std::make_shared<VertexColorEffect>(mProgramFactory);
        mBSPNode[i] = CreateNode(i, v[twoI], v[twoI + 1], color[i]);
    }

    mScene->AttachChild(mBSPNode[0]);
    mBSPNode[0]->AttachPositiveChild(mBSPNode[1]);
    mBSPNode[0]->AttachNegativeChild(mBSPNode[2]);
    mBSPNode[1]->AttachPositiveChild(mBSPNode[3]);

    // Attach an object in each convex region.
    float height = 0.1f;
    Vector2<float> center;

    // The texture image shared by the objects.
    path = mEnvironment.GetPath("Flower.png");
    std::shared_ptr<Texture2> flowerTexture = WICFileIO::Load(path, true);
    flowerTexture->AutogenerateMipmaps();

    // Region 0: Create a torus mesh.
    mTorus = mf.CreateTorus(16, 16, 1.0f, 0.25f);
    mVisualOpaque.insert(mTorus.get());
    txEffect = std::make_shared<Texture2Effect>(mProgramFactory,
        flowerTexture, SamplerState::Filter::MIN_L_MAG_L_MIP_L,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mTorus->SetEffect(txEffect);
    mTorus->localTransform.SetUniformScale(0.1f);
    center = (v[2] + v[6] + v[7]) / 3.0f;
    mTorus->localTransform.SetTranslation(center[0], center[1], height);
    mBSPNode[3]->AttachPositiveChild(mTorus);

    // Region 1: Create a sphere mesh.
    mSphere = mf.CreateSphere(32, 16, 1.0f);
    mVisualOpaque.insert(mSphere.get());
    txEffect = std::make_shared<Texture2Effect>(mProgramFactory,
        flowerTexture, SamplerState::Filter::MIN_L_MAG_L_MIP_L,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mSphere->SetEffect(txEffect);
    mSphere->localTransform.SetUniformScale(0.1f);
    center = (v[0] + v[3] + v[6] + v[7]) / 4.0f;
    mSphere->localTransform.SetTranslation(center[0], center[1], height);
    mBSPNode[3]->AttachNegativeChild(mSphere);

    // Region 2: Create a tetrahedron.
    mTetrahedron = mf.CreateTetrahedron();
    mVisualOpaque.insert(mTetrahedron.get());
    txEffect = std::make_shared<Texture2Effect>(mProgramFactory,
        flowerTexture, SamplerState::Filter::MIN_L_MAG_L_MIP_L,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mTetrahedron->SetEffect(txEffect);
    mTetrahedron->localTransform.SetUniformScale(0.1f);
    center = (v[1] + v[2] + v[3]) / 3.0f;
    mTetrahedron->localTransform.SetTranslation(center[0], center[1], height);
    mBSPNode[1]->AttachNegativeChild(mTetrahedron);

    // Region 3: Create a cube.
    mCube = mf.CreateHexahedron();
    mVisualOpaque.insert(mCube.get());
    txEffect = std::make_shared<Texture2Effect>(mProgramFactory,
        flowerTexture, SamplerState::Filter::MIN_L_MAG_L_MIP_L,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mCube->SetEffect(txEffect);
    mCube->localTransform.SetUniformScale(0.1f);
    center = (v[1] + v[4] + v[5]) / 3.0f;
    mCube->localTransform.SetTranslation(center[0], center[1], height);
    mBSPNode[2]->AttachPositiveChild(mCube);

    // Region 4: Create an octahedron.
    mOctahedron = mf.CreateOctahedron();
    mVisualOpaque.insert(mOctahedron.get());
    txEffect = std::make_shared<Texture2Effect>(mProgramFactory,
        flowerTexture, SamplerState::Filter::MIN_L_MAG_L_MIP_L,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mOctahedron->SetEffect(txEffect);
    mOctahedron->localTransform.SetUniformScale(0.1f);
    center = (v[0] + v[4] + v[5] + v[8]) / 4.0f;
    mOctahedron->localTransform.SetTranslation(center[0], center[1], height);
    mBSPNode[2]->AttachNegativeChild(mOctahedron);

    mTrackBall.Update();
}

void BspNodesWindow3::DoCullSort()
{
    mCuller.ComputeVisibleSet(mCamera, mScene);
    mPVWMatrices.Update(mCuller.GetVisibleSet());

    mVisibleOpaque.clear();
    mVisibleNoCullWire.clear();

    for (auto const& visual : mCuller.GetVisibleSet())
    {
        if (mVisualOpaque.find(visual) != mVisualOpaque.end())
        {
            mVisibleOpaque.push_back(visual);
        }
        else
        {
            mVisibleNoCullWire.push_back(visual);
        }
    }
}

std::shared_ptr<BspNode> BspNodesWindow3::CreateNode(int32_t i,
    Vector2<float> const& v0, Vector2<float> const& v1,
    Vector4<float> const& color)
{
    // Create the model-space separating plane.
    Vector2<float> dir = v1 - v0;
    Vector3<float> normal{ dir[1], -dir[0], 0.0f };
    Normalize(normal);
    float constant = normal[0] * v0[0] + normal[1] * v0[1];
    Plane3<float> modelPlane(normal, constant);

    // Create the BSP node.
    std::shared_ptr<BspNode> bsp = std::make_shared<BspNode>(modelPlane);

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    // Create the rectangle representation of the model plane and set the
    // vertex colors to the specified color.
    float xExtent = 0.5f * Length(dir);
    float yExtent = 0.125f;
    mRectangle[i] = mf.CreateRectangle(2, 2, xExtent, yExtent);
    auto vbuffer = mRectangle[i]->GetVertexBuffer().get();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (int32_t j = 0; j < 4; ++j)
    {
        vertex[j].color = color;
    }
    mRectangle[i]->SetEffect(mVCEffect[i]);

    // Set the position and orientation for the world-space plane.
    Vector3<float> trn{ 0.5f*(v0[0] + v1[0]), 0.5f*(v0[1] + v1[1]), yExtent + 0.001f };

    Matrix4x4<float> zRotate = Rotation<4, float>(AxisAngle<4, float>(
        Vector4<float>::Unit(2), std::atan2(dir[1], dir[0])));

    Matrix4x4<float> xRotate = Rotation<4, float>(AxisAngle<4, float>(
        Vector4<float>::Unit(0), (float)GTE_C_HALF_PI));

    Matrix4x4<float> rotate = DoTransform(zRotate, xRotate);

    mRectangle[i]->localTransform.SetTranslation(trn);
    mRectangle[i]->localTransform.SetRotation(rotate);

    bsp->AttachCoplanarChild(mRectangle[i]);
    return bsp;
}
