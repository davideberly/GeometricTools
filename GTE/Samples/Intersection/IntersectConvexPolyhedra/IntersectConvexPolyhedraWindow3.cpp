// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectConvexPolyhedraWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>

IntersectConvexPolyhedraWindow3::IntersectConvexPolyhedraWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mMessage("Trackball rotates scene.")
{
    mTextColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 1000.0f, 0.01f, 0.001f,
        { 16.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
}

void IntersectConvexPolyhedraWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mMeshIntersection->culling == CullingMode::NEVER)
    {
        // Draw the intersection only when it exists.
        mEngine->Draw(mMeshIntersection);
    }

    mEngine->SetRasterizerState(mWireState);
    mEngine->Draw(mMeshPoly0);
    mEngine->Draw(mMeshPoly1);
    mEngine->SetDefaultRasterizerState();

    mEngine->Draw(8, 24, mTextColor, mMessage);
    mEngine->Draw(8, 48, mTextColor,
        "After rotating a single polyhedron, press 'i' to compute intersection.");
    mEngine->Draw(8, mYSize - 8, mTextColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectConvexPolyhedraWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '0':
        mTrackBall.Set(mMeshPoly0);
        mTrackBall.Update();
        mMessage = "Trackball rotates red polyhedron.";
        return true;
    case '1':
        mTrackBall.Set(mMeshPoly1);
        mTrackBall.Update();
        mMessage = "Trackball rotates blue polyhedron.";
        return true;
    case '2':
        mTrackBall.Set(mScene);
        mTrackBall.Update();
        mMessage = "Trackball rotates scene.";
        return true;
    case 'i':
    case 'I':
        ComputeIntersection();
        mTrackBall.Set(mScene);
        mTrackBall.Update();
        mMessage = "Trackball rotates scene.";
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void IntersectConvexPolyhedraWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);

    // Attach a dummy intersection mesh.  If the intersection is nonempty,
    // the Culling flag will be modified to CULL_DYNAMIC.  The intersection
    // is drawn as a solid.
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mMeshIntersection = mf.CreateTetrahedron();

    Vector4<float> green{ 0.0f, 1.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, green);
    mMeshIntersection->SetEffect(effect);
    mPVWMatrices.Subscribe(mMeshIntersection->worldTransform, effect->GetPVWMatrixConstant());
    mMeshIntersection->culling = CullingMode::ALWAYS;
    mScene->AttachChild(mMeshIntersection);

    // The first polyhedron is an ellipsoid.
    ConvexPolyhedron<float>::CreateEggShape(Vector3<float>::Zero(),
        1.0f, 1.0f, 2.0f, 2.0f, 4.0f, 4.0f, 3, mPoly0);

    int32_t numVertices = mPoly0.GetNumVertices();
    int32_t numTriangles = mPoly0.GetNumTriangles();
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, static_cast<uint32_t>(numVertices));
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH,
        static_cast<uint32_t>(numTriangles), sizeof(uint32_t));
    auto vertices = vbuffer->Get<Vector3<float>>();
    for (int32_t i = 0; i < numVertices; ++i)
    {
        vertices[i] = mPoly0.GetPoint(i);
    }
    auto indices = ibuffer->Get<int32_t>();
    for (int32_t i = 0; i < numTriangles; ++i)
    {
        MTTriangle const& triangle = mPoly0.GetTriangle(i);
        for (int32_t j = 0; j < 3; ++j)
        {
            indices[3 * i + j] = mPoly0.GetVLabel(triangle.GetVertex(j));
        }
    }

    Vector4<float> red{ 1.0f, 0.0f, 0.0f, 1.0f };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, red);
    mMeshPoly0 = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mMeshPoly0->localTransform.SetTranslation(0.0f, 2.0f, 0.0f);
    mPVWMatrices.Subscribe(mMeshPoly0->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mMeshPoly0);

    // The second polyhedron is egg shaped.
    ConvexPolyhedron<float>::CreateEggShape(Vector3<float>::Zero(),
        2.0f, 2.0f, 4.0f, 4.0f, 5.0f, 3.0f, 4, mPoly1);

    // Build the corresponding mesh.
    numVertices = mPoly1.GetNumVertices();
    numTriangles = mPoly1.GetNumTriangles();
    vbuffer = std::make_shared<VertexBuffer>(vformat, static_cast<uint32_t>(numVertices));
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH,
        static_cast<uint32_t>(numTriangles), sizeof(uint32_t));

    vertices = vbuffer->Get<Vector3<float>>();
    for (int32_t i = 0; i < numVertices; ++i)
    {
        vertices[i] = mPoly1.GetPoint(i);
    }
    indices = ibuffer->Get<int32_t>();
    for (int32_t i = 0; i < numTriangles; ++i)
    {
        MTTriangle const& triangle = mPoly1.GetTriangle(i);
        for (int32_t j = 0; j < 3; ++j)
        {
            indices[3 * i + j] = mPoly1.GetVLabel(triangle.GetVertex(j));
        }
    }

    Vector4<float> blue{ 0.0f, 0.0f, 1.0f, 1.0f };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, blue);
    mMeshPoly1 = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mMeshPoly1->localTransform.SetTranslation(0.0f, -2.0f, 0.0f);
    mPVWMatrices.Subscribe(mMeshPoly1->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mMeshPoly1);

    mTrackBall.Set(mScene);
    mTrackBall.Update();
    mPVWMatrices.Update();

    ComputeIntersection();
}

void IntersectConvexPolyhedraWindow3::ComputeIntersection()
{
    // Transform the model-space vertices to world space.
    std::shared_ptr<VertexBuffer> vbuffer = mMeshPoly0->GetVertexBuffer();
    int32_t numVertices = static_cast<int32_t>(vbuffer->GetNumElements());
    auto vertices = vbuffer->Get<Vector3<float>>();
    for (int32_t i = 0; i < numVertices; ++i)
    {
        Vector4<float> model = HLift(vertices[i], 1.0f);
        Vector4<float> local = mMeshPoly0->localTransform * model;
        mPoly0.SetPoint(i, HProject(local));
    }
    mPoly0.UpdatePlanes();

    vbuffer = mMeshPoly1->GetVertexBuffer();
    numVertices = static_cast<int32_t>(vbuffer->GetNumElements());
    vertices = vbuffer->Get<Vector3<float>>();
    for (int32_t i = 0; i < numVertices; ++i)
    {
        Vector4<float> model = HLift(vertices[i], 1.0f);
        Vector4<float> local = mMeshPoly1->localTransform * model;
        mPoly1.SetPoint(i, HProject(local));
    }
    mPoly1.UpdatePlanes();

    // Compute the intersection (if any) in world space.
    bool hasIntersection = mPoly0.FindIntersection(mPoly1, mIntersection);

    if (hasIntersection)
    {
        // Build the corresponding mesh.
        numVertices = mIntersection.GetNumVertices();
        int32_t numTriangles = mIntersection.GetNumTriangles();
        VertexFormat vformat = mMeshPoly0->GetVertexBuffer()->GetFormat();
        vbuffer = std::make_shared<VertexBuffer>(vformat, static_cast<uint32_t>(numVertices));
        vertices = vbuffer->Get<Vector3<float>>();
        auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH,
            static_cast<uint32_t>(numTriangles), sizeof(int32_t));
        for (int32_t i = 0; i < numVertices; ++i)
        {
            vertices[i] = mIntersection.GetPoint(i);
        }
        auto indices = ibuffer->Get<int32_t>();
        for (int32_t i = 0; i < numTriangles; ++i)
        {
            MTTriangle const& triangle = mIntersection.GetTriangle(i);
            for (int32_t j = 0; j < 3; ++j)
            {
                indices[3 * i + j] = mIntersection.GetVLabel(triangle.GetVertex(j));
            }
        }

        mMeshIntersection->SetVertexBuffer(vbuffer);
        mMeshIntersection->SetIndexBuffer(ibuffer);
        mMeshIntersection->culling = CullingMode::NEVER;
    }
    else
    {
        mMeshIntersection->culling = CullingMode::ALWAYS;
    }
}
