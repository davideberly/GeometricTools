// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectPlaneConvexPolyhedronWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>

IntersectPlaneConvexPolyhedronWindow3::IntersectPlaneConvexPolyhedronWindow3(
    Parameters& parameters)
    :
    Window3(parameters),
    mAlpha(0.5f),
    mDeltaDistance(0.01f),
    mDeltaTheta(0.1f),
    mDeltaPhi(0.1f),
    mDistance(0.0f),
    mTheta(0.0f),
    mPhi(0.0f),
    mValidPosPolyMesh(false),
    mValidNegPolyMesh(false),
    mValidPolygonCurve(false),
    mValidPolygonMesh(false),
    mDrawPosPolyMesh(true),
    mDrawNegPolyMesh(true),
    mDrawPolygonCurve(true),
    mDrawPolygonMesh(true)
{
    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    mDepthReadNoWriteState = std::make_shared<DepthStencilState>();
    mDepthReadNoWriteState->depthEnable = true;
    mDepthReadNoWriteState->writeMask = DepthStencilState::WriteMask::ZERO;

    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->fill = RasterizerState::Fill::SOLID;
    mNoCullSolidState->cull = RasterizerState::Cull::NONE;
    mEngine->SetRasterizerState(mNoCullSolidState);

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mNoCullWireState->cull = RasterizerState::Cull::NONE;

    CreateQueryObjects();
    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.01f,
        { 0.0f, -4.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();

    DoQuery();
}

void IntersectPlaneConvexPolyhedronWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mValidPolygonCurve && mDrawPolygonCurve)
    {
        mEngine->Draw(mPolygonCurve);
    }

    auto const& previousBlendState = mEngine->GetBlendState();
    mEngine->SetBlendState(mBlendState);
    mEngine->SetDepthStencilState(mDepthReadNoWriteState);
    {
        mEngine->Draw(mPlaneMesh);
        if (mValidPosPolyMesh && mDrawPosPolyMesh)
        {
            mEngine->Draw(mPosPolyMesh);
        }
        if (mValidNegPolyMesh && mDrawNegPolyMesh)
        {
            mEngine->Draw(mNegPolyMesh);
        }

        if (mValidPolygonMesh && mDrawPolygonMesh)
        {
            mEngine->Draw(mPolygonMesh);
        }
    }
    mEngine->SetDefaultDepthStencilState();
    mEngine->SetBlendState(previousBlendState);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectPlaneConvexPolyhedronWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullSolidState == mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullSolidState);
        }
        return true;

    case 'p':
    case 'P':
        mDrawPosPolyMesh = !mDrawPosPolyMesh;
        return true;

    case 'n':
    case 'N':
        mDrawNegPolyMesh = !mDrawNegPolyMesh;
        return true;

    case 'c':
    case 'C':
        mDrawPolygonCurve = !mDrawPolygonCurve;
        return true;

    case 'm':
    case 'M':
        mDrawPolygonMesh = !mDrawPolygonMesh;
        return true;

    case '-':
    case '_':
        mDistance -= mDeltaDistance;
        UpdatePlane();
        return true;

    case '+':
    case '=':
        mDistance += mDeltaDistance;
        UpdatePlane();
        return true;

    case 'a':
        mTheta -= mDeltaTheta;
        UpdatePlane();
        return true;

    case 'A':
        mTheta += mDeltaTheta;
        UpdatePlane();
        return true;

    case 'b':
        mPhi -= mDeltaPhi;
        UpdatePlane();
        return true;

    case 'B':
        mPhi += mDeltaPhi;
        UpdatePlane();
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

void IntersectPlaneConvexPolyhedronWindow3::CreateQueryObjects()
{
    // Create the convex polyhedron to use in query.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto visual = mf.CreateDodecahedron();
    auto const& vbuffer = visual->GetVertexBuffer();
    auto vertices = vbuffer->Get<Vector3<float>>();
    auto const& ibuffer = visual->GetIndexBuffer();
    auto triangles = ibuffer->Get<std::array<int32_t, 3>>();

    mPolyhedron.configuration = CM::CFG_POLYHEDRON;

    mPolyhedron.vertices.resize(vbuffer->GetNumElements());
    for (size_t i = 0; i < mPolyhedron.vertices.size(); ++i)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            mPolyhedron.vertices[i][j] = vertices[i][j];
        }
    }

    mPolyhedron.triangles.resize(ibuffer->GetNumPrimitives());
    for (size_t i = 0; i < mPolyhedron.triangles.size(); ++i)
    {
        mPolyhedron.triangles[i] = triangles[i];
    }

    // Create the plane to use in the query.
    Rational const zero(0), one(1);
    mPlane.normal = { zero, zero, one };
    mPlane.constant = zero;
}

void IntersectPlaneConvexPolyhedronWindow3::CreateScene()
{
    // Create vertex buffers large enough to store the maximum number of
    // vertices for the polyhedra. The dodecahedron has 20 vertices,
    // 36 triangles and 54 edges. The cap on a polyhedron uses an average
    // point, The maximum number of vertices is 20 + 54 + 1 = 75. Create index
    // buffers large enough to store the maximum number of triangles for the
    // polyhedra. The number shown here is a conservative estimate (twice the
    // number of polyhedron triangle faces).
    uint32_t const maxNumVertices = 75;
    uint32_t const maxNumTriangles = 72;
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);

    auto vbufferPos = std::make_shared<VertexBuffer>(vformat, maxNumVertices);
    vbufferPos->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto ibufferPos = std::make_shared<IndexBuffer>(IP_TRIMESH, maxNumTriangles, sizeof(int32_t));
    ibufferPos->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto effectPos = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 1.0f, 0.0f, 0.0f, mAlpha });
    mPosPolyMesh = std::make_shared<Visual>(vbufferPos, ibufferPos, effectPos);
    mPVWMatrices.Subscribe(mPosPolyMesh);
    mTrackBall.Attach(mPosPolyMesh);

    auto vbufferNeg = std::make_shared<VertexBuffer>(vformat, maxNumVertices);
    vbufferNeg->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto ibufferNeg = std::make_shared<IndexBuffer>(IP_TRIMESH, maxNumTriangles, sizeof(int32_t));
    ibufferNeg->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto effectNeg = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, mAlpha });
    mNegPolyMesh = std::make_shared<Visual>(vbufferNeg, ibufferNeg, effectNeg);
    mPVWMatrices.Subscribe(mNegPolyMesh);
    mTrackBall.Attach(mNegPolyMesh);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, maxNumVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS,
        maxNumTriangles);
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f });
    mPolygonCurve = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mPolygonCurve);
    mTrackBall.Attach(mPolygonCurve);

    vbuffer = std::make_shared<VertexBuffer>(vformat, maxNumVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, maxNumTriangles, sizeof(int32_t));
    ibuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 1.0f, 0.0f, mAlpha });
    mPolygonMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mPolygonMesh);
    mTrackBall.Attach(mPolygonMesh);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mPlaneMesh = mf.CreateRectangle(2, 2, 2.0f, 2.0f);
    auto effectPlane = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.75f, 0.75f, 0.75f, mAlpha });
    mPlaneMesh->SetEffect(effectPlane);
    mPVWMatrices.Subscribe(mPlaneMesh);
    mTrackBall.Attach(mPlaneMesh);

    mTrackBall.Update();
}

void IntersectPlaneConvexPolyhedronWindow3::DoQuery()
{
    mResult = mQuery(mPolyhedron, mPlane, Query::REQ_ALL);

    mValidPosPolyMesh = (mResult.configuration & Query::CFG_POS_SIDE) != 0;
    if (mValidPosPolyMesh)
    {
        CM const& poly = mResult.positivePolyhedron;
        auto const& vbuffer = mPosPolyMesh->GetVertexBuffer();
        auto vertices = vbuffer->Get<Vector3<float>>();
        for (size_t i = 0; i < poly.vertices.size(); ++i)
        {
            for (int32_t j = 0; j < 3; ++j)
            {
                vertices[i][j] = poly.vertices[i][j];
            }
        }
        uint32_t const numVertices = static_cast<uint32_t>(poly.vertices.size());
        vbuffer->SetNumActiveElements(numVertices);
        mEngine->Update(vbuffer);

        auto const& ibuffer = mPosPolyMesh->GetIndexBuffer();
        auto indices = ibuffer->Get<int32_t>();
        size_t const numBytes = poly.triangles.size() * sizeof(std::array<int32_t, 3>);
        std::memcpy(indices, poly.triangles.data(), numBytes);
        uint32_t const numTriangles = static_cast<uint32_t>(poly.triangles.size());
        ibuffer->SetNumActivePrimitives(numTriangles);
        mEngine->Update(ibuffer);
    }

    mValidNegPolyMesh = (mResult.configuration & Query::CFG_NEG_SIDE) != 0;
    if (mValidNegPolyMesh)
    {
        CM const& poly = mResult.negativePolyhedron;
        auto const& vbuffer = mNegPolyMesh->GetVertexBuffer();
        auto vertices = vbuffer->Get<Vector3<float>>();
        for (size_t i = 0; i < poly.vertices.size(); ++i)
        {
            for (int32_t j = 0; j < 3; ++j)
            {
                vertices[i][j] = poly.vertices[i][j];
            }
        }
        uint32_t const numVertices = static_cast<uint32_t>(poly.vertices.size());
        vbuffer->SetNumActiveElements(numVertices);
        mEngine->Update(vbuffer);

        auto const& ibuffer = mNegPolyMesh->GetIndexBuffer();
        auto indices = ibuffer->Get<int32_t>();
        size_t const numBytes = poly.triangles.size() * sizeof(std::array<int32_t, 3>);
        std::memcpy(indices, poly.triangles.data(), numBytes);
        uint32_t const numTriangles = static_cast<uint32_t>(poly.triangles.size());
        ibuffer->SetNumActivePrimitives(numTriangles);
        mEngine->Update(ibuffer);
    }

    auto const& polyVertices = mResult.intersectionPolygon;
    size_t const numPolyVertices = polyVertices.size();
    mValidPolygonCurve = (numPolyVertices > 0);
    if (mValidPolygonCurve)
    {
        auto const& vbuffer = mPolygonCurve->GetVertexBuffer();
        auto vertices = vbuffer->Get<Vector3<float>>();
        for (size_t i = 0; i < numPolyVertices; ++i)
        {
            for (int32_t j = 0; j < 3; ++j)
            {
                vertices[i][j] = polyVertices[i][j];
            }
        }
        for (int32_t j = 0; j < 3; ++j)
        {
            vertices[numPolyVertices][j] = polyVertices[0][j];
        }

        uint32_t const numVertices = static_cast<uint32_t>(numPolyVertices + 1);
        vbuffer->SetNumActiveElements(numVertices);
        mEngine->Update(vbuffer);
    }

    auto const& imesh = mResult.intersectionMesh;
    mValidPolygonMesh = (imesh.vertices.size() > 0);
    if (mValidPolygonMesh)
    {
        auto const& vbuffer = mPolygonMesh->GetVertexBuffer();
        auto vertices = vbuffer->Get<Vector3<float>>();
        for (size_t i = 0; i < imesh.vertices.size(); ++i)
        {
            for (int32_t j = 0; j < 3; ++j)
            {
                vertices[i][j] = imesh.vertices[i][j];
            }
        }
        uint32_t const numVertices = static_cast<uint32_t>(imesh.vertices.size());
        vbuffer->SetNumActiveElements(numVertices);
        mEngine->Update(vbuffer);

        auto const& ibuffer = mPolygonMesh->GetIndexBuffer();
        auto indices = ibuffer->Get<int32_t>();
        size_t const numBytes = imesh.triangles.size() * sizeof(std::array<int32_t, 3>);
        std::memcpy(indices, imesh.triangles.data(), numBytes);
        uint32_t const numTriangles = static_cast<uint32_t>(imesh.triangles.size());
        ibuffer->SetNumActivePrimitives(numTriangles);
        mEngine->Update(ibuffer);
    }
}

void IntersectPlaneConvexPolyhedronWindow3::UpdatePlane()
{
    Vector3<float> center = mPlaneMesh->localTransform.GetTranslation();
    float cs0 = std::cos(mTheta), sn0 = std::sin(mTheta);
    float cs1 = std::cos(mPhi), sn1 = std::sin(mPhi);
    Vector3<float> normal = { cs0 * sn1, sn0 * sn1, cs1 };
    center = mDistance * normal;

    std::array<Vector3<float>, 3> basis{};
    basis[0] = normal;
    ComputeOrthogonalComplement(1, basis.data());
    Matrix3x3<float> rotate{};
    rotate.SetCol(0, basis[1]);
    rotate.SetCol(1, basis[2]);
    rotate.SetCol(2, basis[0]);

    mPlaneMesh->localTransform.SetTranslation(center);
    mPlaneMesh->localTransform.SetRotation(rotate);
    mPlaneMesh->Update();
    mPVWMatrices.Update();

    float constant = Dot(normal, center);
    mPlane.normal = { normal[0], normal[1], normal[2] };
    mPlane.constant = constant;

    DoQuery();
}
