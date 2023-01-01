// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ExtremalQueryWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/VertexColorEffect.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/ConvexHull3.h>
#include <random>

#if defined(MEASURE_TIMING_OF_QUERY)
#include <Applications/Timer.h>
#endif

ExtremalQueryWindow3::ExtremalQueryWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    // Set up an orthogonal camera.  This projection type is used to make it
    // clear that the displayed extreme points really are extreme; the
    // perspective projection is deceptive.
    mCamera = std::make_shared<Camera>(false, mEngine->HasDepthRange01());
    mCamera->SetFrustum(1.0f, 1000.0f, -1.5f, 1.5f, -2.0, 2.0f);
    Vector4<float> camPosition{ 4.0f, 0.0f, 0.0f, 1.0f };
    Vector4<float> camDVector{ -1.0f, 0.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mPVWMatrices.Set(mCamera, mUpdater);
    mTrackBall.Set(mXSize, mYSize, mCamera);

    CreateScene();
}

void ExtremalQueryWindow3::OnIdle()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    mEngine->Draw(mConvexMesh);
    mEngine->Draw(mMaxSphere);
    mEngine->Draw(mMinSphere);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ExtremalQueryWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mWireState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool ExtremalQueryWindow3::OnMouseMotion(MouseButton button, int32_t x, int32_t y, uint32_t modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        UpdateExtremePoints();
    }
    return true;
}

void ExtremalQueryWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();

    // Create a convex polyhedron that is the hull of numVertices randomly
    // generated points.
    int32_t const numVertices = 32;
    CreateConvexPolyhedron(numVertices);
    CreateVisualConvexPolyhedron();

    // Use small spheres to show the extreme points in the camera's right
    // direction.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mMaxSphere = mf.CreateSphere(8, 8, 0.05f);
    mMinSphere = mf.CreateSphere(8, 8, 0.05f);

    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);
    mMaxSphere->SetEffect(effect);
    mPVWMatrices.Subscribe(mMaxSphere->worldTransform, effect->GetPVWMatrixConstant());

    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);
    mMinSphere->SetEffect(effect);
    mPVWMatrices.Subscribe(mMinSphere->worldTransform, effect->GetPVWMatrixConstant());

    mScene->AttachChild(mMaxSphere);
    mScene->AttachChild(mMinSphere);

    mTrackBall.Attach(mScene);
    mTrackBall.Update();
    UpdateExtremePoints();
}

void ExtremalQueryWindow3::CreateConvexPolyhedron(int32_t numVertices)
{
    // Create the convex hull of a randomly generated set of points on the unit sphere.
    auto vertexPool = std::make_shared<std::vector<Vector3<float>>>(numVertices);
    auto& vertices = *vertexPool.get();
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    for (auto& vertex : vertices)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            vertex[j] = rnd(mte);
        }
        Normalize(vertex);
    }

    ConvexHull3<float> ch3;
    ch3(vertices, 0);

    auto const& triangles = ch3.GetHull();
    int32_t numIndices = 3 * static_cast<int32_t>(triangles.size());
    std::vector<int32_t> indices(numIndices);
    size_t k = 0;
    for (auto index : triangles)
    {
        indices[k++] = static_cast<int32_t>(index);
    }
    mConvexPolyhedron = std::make_unique<Polyhedron3<float>>(vertexPool, numIndices,
        indices.data(), true);

#ifdef USE_BSP_QUERY
    mExtremalQuery = std::make_unique<ExtremalQuery3BSP<float>>(*mConvexPolyhedron);
#else
    mExtremalQuery = std::make_unique<ExtremalQuery3PRJ<float>>(*mConvexPolyhedron);
#endif

#ifdef MEASURE_TIMING_OF_QUERY
    // For timing purposes and determination of asymptotic order.
    int32_t const imax = 10000000;
    std::vector<Vector3<float>> directions(imax);
    for (auto& direction : directions)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            direction[j] = rnd(mte);
        }
        Normalize(direction);
    }

    Timer timer;
    for (int32_t i = 0; i < imax; ++i)
    {
        int32_t pos, neg;
        mExtremalQuery->GetExtremeVertices(directions[i], pos, neg);
    }
    double duration = timer.GetSeconds();
    std::ofstream outFile("timing.txt");
    outFile << "duration = " << duration << " seconds" << std::endl;
    outFile.close();
#endif
}

void ExtremalQueryWindow3::CreateVisualConvexPolyhedron()
{
    auto vertexPool = mConvexPolyhedron->GetVertices();
    auto const& polyIndices = mConvexPolyhedron->GetIndices();
    int32_t const numIndices = static_cast<int32_t>(polyIndices.size());
    int32_t const numTriangles = numIndices / 3;

    // Visualize the convex polyhedron as a collection of face-colored triangles.
    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numIndices);
    auto vertices = vbuffer->Get<Vertex>();

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(uint32_t));
    auto indices = ibuffer->Get<int32_t>();
    for (int32_t i = 0; i < numIndices; ++i)
    {
        vertices[i].position = vertexPool[polyIndices[i]];
        indices[i] = i;
    }

    // Use randomly generated vertex colors.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    for (int32_t t = 0; t < numTriangles; ++t)
    {
        Vector4<float> color{ rnd(mte), rnd(mte), rnd(mte), 1.0f };
        for (int32_t j = 0; j < 3; ++j)
        {
            vertices[3 * t + j].color = color;
        }
    }

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    mConvexMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mConvexMesh->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mConvexMesh);
}

void ExtremalQueryWindow3::UpdateExtremePoints()
{
    Matrix4x4<float> invWMatrix = mScene->worldTransform.GetHInverse();
    Vector4<float> rVector = DoTransform(invWMatrix, mCamera->GetRVector());
    Vector3<float> direction = HProject<4, float>(rVector);

    int32_t posDir, negDir;
    mExtremalQuery->GetExtremeVertices(direction, posDir, negDir);

    auto vertexPool = mConvexPolyhedron->GetVertices();
    mMaxSphere->localTransform.SetTranslation(vertexPool[posDir]);
    mMinSphere->localTransform.SetTranslation(vertexPool[negDir]);

    mTrackBall.Update();
    mPVWMatrices.Update();
}
