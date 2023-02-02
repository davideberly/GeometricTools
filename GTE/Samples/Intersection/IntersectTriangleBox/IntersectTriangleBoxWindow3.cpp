// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectTriangleBoxWindow3.h"
#include <Graphics/MeshFactory.h>

IntersectTriangleBoxWindow3::IntersectTriangleBoxWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mEngine->SetRasterizerState(mNoCullState);

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;
    mEngine->SetBlendState(mBlendState);

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.01f,
        { 0.0f, 0.0f, -8.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    DoIntersectionQuery();
}

void IntersectTriangleBoxWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mOutsideTriangleMesh->culling == CullingMode::NEVER)
    {
        mEngine->Draw(mOutsideTriangleMesh);
    }

    if (mInsideTriangleMesh->culling == CullingMode::NEVER)
    {
        mEngine->Draw(mInsideTriangleMesh);
    }

    mEngine->Draw(mBoxMesh);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectTriangleBoxWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    float const delta = 0.1f;

    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullState)
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;

    case ' ':  // To re-test during debugging without moving the box.
        DoIntersectionQuery();
        return true;

    case 'x':  // decrement x-center of box
        Translate(0, -delta);
        return true;

    case 'X':  // increment x-center of box
        Translate(0, +delta);
        return true;

    case 'y':  // decrement y-center of box
        Translate(1, -delta);
        return true;

    case 'Y':  // increment y-center of box
        Translate(1, +delta);
        return true;

    case 'z':  // decrement z-center of box
        Translate(2, -delta);
        return true;

    case 'Z':  // increment z-center of box
        Translate(2, +delta);
        return true;

    case 'p':  // rotate about axis[0]
        Rotate(0, -delta);
        return true;

    case 'P':  // rotate about axis[0]
        Rotate(0, +delta);
        return true;

    case 'r':  // rotate about axis[1]
        Rotate(1, -delta);
        return true;

    case 'R':  // rotate about axis[1]
        Rotate(1, +delta);
        return true;

    case 'h':  // rotate about axis[2]
        Rotate(2, -delta);
        return true;

    case 'H':  // rotate about axis[2]
        Rotate(2, +delta);
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void IntersectTriangleBoxWindow3::CreateScene()
{
    // Initialize the objects used in the intersection queries.  The objects
    // are not intersecting initially.
    mBox.center = { 0.0f, 0.0f, 0.0f };
    mBox.axis[0] = { 1.0f, 0.0f, 0.0f };
    mBox.axis[1] = { 0.0f, 1.0f, 0.0f };
    mBox.axis[2] = { 0.0f, 0.0f, 1.0f };
    mBox.extent = { 1.0f, 2.0f, 3.0f };

    mTriangle.v[0] = { 2.0f, 0.0f, 0.0f };
    mTriangle.v[1] = { 2.0f, 1.0f, 0.0f };
    mTriangle.v[2] = { 2.0f, 0.0f, 1.0f };

    // The mesh objects use constant color; only vertex position is required.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    // Create the constant color effects.
    mRedEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 1.0f, 0.0f, 0.0f, 0.5f });

    mBlueEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 0.5f });

    mGreenEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 1.0f, 0.0f, 0.5f });

    mGrayEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.5f, 0.5f, 0.5f, 0.5f });

    // Create a visual representation of the box.
    mBoxMesh = mf.CreateBox(mBox.extent[0], mBox.extent[1], mBox.extent[2]);
    mBoxMesh->SetEffect(mBlueEffect);
    mPVWMatrices.Subscribe(mBoxMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());

    // Create the mesh to store the clipped triangle outside the box.
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 32);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 32, sizeof(uint32_t));
    ibuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    ibuffer->SetTriangle(0, 0, 1, 2);
    vbuffer->SetNumActiveElements(3);
    ibuffer->SetNumActivePrimitives(1);
    mOutsideTriangleMesh = std::make_shared<Visual>(vbuffer, ibuffer);
    mOutsideTriangleMesh->SetEffect(mGreenEffect);
    mPVWMatrices.Subscribe(mOutsideTriangleMesh->worldTransform, mGreenEffect->GetPVWMatrixConstant());
    mOutsideTriangleMesh->culling = CullingMode::NEVER;

    // Create the mesh to store the clipped triangle inside the box.
    vbuffer = std::make_shared<VertexBuffer>(vformat, 32);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 32, sizeof(uint32_t));
    ibuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    ibuffer->SetTriangle(0, 0, 1, 2);
    auto position = vbuffer->Get<Vector3<float>>();
    position[0] = mTriangle.v[0];
    position[1] = mTriangle.v[1];
    position[2] = mTriangle.v[2];
    vbuffer->SetNumActiveElements(3);
    ibuffer->SetNumActivePrimitives(1);
    mInsideTriangleMesh = std::make_shared<Visual>(vbuffer, ibuffer);
    mInsideTriangleMesh->SetEffect(mGrayEffect);
    mPVWMatrices.Subscribe(mInsideTriangleMesh->worldTransform, mGrayEffect->GetPVWMatrixConstant());
    mInsideTriangleMesh->culling = CullingMode::ALWAYS;

    mTrackBall.Attach(mBoxMesh);
    mTrackBall.Attach(mOutsideTriangleMesh);
    mTrackBall.Attach(mInsideTriangleMesh);
    mTrackBall.Update();
}

void IntersectTriangleBoxWindow3::Translate(int32_t direction, float delta)
{
    mBox.center[direction] += delta;
    mBoxMesh->localTransform.SetTranslation(mBox.center);
    mBoxMesh->Update();
    DoIntersectionQuery();
}

void IntersectTriangleBoxWindow3::Rotate(int32_t direction, float delta)
{
    AxisAngle<3, float> aa(mBox.axis[direction], delta);
    Quaternion<float> incr = Rotation<3, float>(aa);
    for (int32_t i = 0; i < 3; ++i)
    {
        if (i != direction)
        {
            mBox.axis[i] = HProject(gte::Rotate(incr, HLift(mBox.axis[i], 0.0f)));
        }
    }
    Quaternion<float> q;
    mBoxMesh->localTransform.GetRotation(q);
    mBoxMesh->localTransform.SetRotation(incr * q);
    mBoxMesh->Update();
    DoIntersectionQuery();
}

void IntersectTriangleBoxWindow3::DoIntersectionQuery()
{
    // Use the find-intersection query for a triangle and a box.  See the
    // note later in this function about using the test-intersection query.
    auto fiResult = mFIQuery(mTriangle, mBox);
    bool intersects = (fiResult.insidePolygon.size() >= 3);
    if (intersects)
    {
        uint32_t numVertices = static_cast<uint32_t>(fiResult.insidePolygon.size());
        uint32_t numTriangles = numVertices - 2;
        auto const& vbuffer = mInsideTriangleMesh->GetVertexBuffer();
        auto const& ibuffer = mInsideTriangleMesh->GetIndexBuffer();
        auto position = vbuffer->Get<Vector3<float>>();
        for (uint32_t i = 0; i < numVertices; ++i)
        {
            position[i] = fiResult.insidePolygon[i];
        }
        for (uint32_t t = 0; t < numTriangles; ++t)
        {
            ibuffer->SetTriangle(t, 0, t + 1, t + 2);
        }
        vbuffer->SetNumActiveElements(numVertices);
        ibuffer->SetNumActivePrimitives(numTriangles);
        mInsideTriangleMesh->culling = CullingMode::NEVER;
        mEngine->Update(vbuffer);
        mEngine->Update(ibuffer);
    }
    else
    {
        mInsideTriangleMesh->culling = CullingMode::ALWAYS;
    }

    if (fiResult.outsidePolygons.size() > 0)
    {
        auto const& vbuffer = mOutsideTriangleMesh->GetVertexBuffer();
        auto const& ibuffer = mOutsideTriangleMesh->GetIndexBuffer();
        auto position = vbuffer->Get<Vector3<float>>();
        auto indices = ibuffer->Get<uint32_t>();
        uint32_t totalVertices = 0;
        uint32_t totalTriangles = 0;
        for (auto const& outsidePolygon : fiResult.outsidePolygons)
        {
            uint32_t numVertices = static_cast<uint32_t>(outsidePolygon.size());
            uint32_t numTriangles = numVertices - 2;
            for (uint32_t i = 0; i < numVertices; ++i)
            {
                *position++ = outsidePolygon[i];
            }
            for (uint32_t t = 0; t < numTriangles; ++t)
            {
                *indices++ = totalVertices;
                *indices++ = totalVertices + t + 1;
                *indices++ = totalVertices + t + 2;
            }
            totalVertices += numVertices;
            totalTriangles += numTriangles;
        }
        vbuffer->SetNumActiveElements(totalVertices);
        ibuffer->SetNumActivePrimitives(totalTriangles);
        mOutsideTriangleMesh->culling = CullingMode::NEVER;
        mEngine->Update(vbuffer);
        mEngine->Update(ibuffer);
    }
    else
    {
        mOutsideTriangleMesh->culling = CullingMode::ALWAYS;
    }

#if defined(USE_TIQUERY_OVERRIDE)
    // Test the TIQuery code.  If this block of code is disabled, then
    // the box color updates use the FIQuery 'intersects', which in theory
    // should match those of TIQuery.
    auto tiResult = mTIQuery(mTriangle, mBox);
    intersects = tiResult.intersect;
#endif

    mPVWMatrices.Unsubscribe(mBoxMesh->worldTransform);
    if (intersects)
    {
        mBoxMesh->SetEffect(mRedEffect);
        mPVWMatrices.Subscribe(mBoxMesh->worldTransform, mRedEffect->GetPVWMatrixConstant());
    }
    else
    {
        mBoxMesh->SetEffect(mBlueEffect);
        mPVWMatrices.Subscribe(mBoxMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());
    }
    mPVWMatrices.Update();
}
