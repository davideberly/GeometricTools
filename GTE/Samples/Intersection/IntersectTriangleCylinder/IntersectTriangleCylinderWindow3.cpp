// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectTriangleCylinderWindow3.h"
#include <Mathematics/Rotation.h>
#include <Graphics/MeshFactory.h>

namespace gte
{
    template class TIQuery<float, Triangle3<float>, Cylinder3<float>>;
}

IntersectTriangleCylinderWindow3::IntersectTriangleCylinderWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mTriangle(
        Vector3<float>{ 0.0f, 1.125f, 3.0f },
        Vector3<float>{ -0.25f, 1.125f, 0.0f },
        Vector3<float>{ 1.0f, 1.125f, -1.0f }),
    mCylinder(
        Line3<float>(Vector3<float>{ 0.0f, 0.0f, 0.0f }, Vector3<float>{ 0.0f, 0.0f, 1.0f }),
        1.0f,
        0.5f),
    mQuery{},
    mMotionObject(0),
    mTriangleCenter{ 0.0f, 0.0f, 0.0f },
    mTriangleCoord{},
    mTriangleBasis{},
    mCylinderBasis{}
{
    Vector3<float> edge10 = mTriangle.v[1] - mTriangle.v[0];
    Vector3<float> edge20 = mTriangle.v[2] - mTriangle.v[0];
    Vector3<float> normal = UnitCross(edge10, edge20);
    std::array<Vector3<float>, 3> basis{};
    basis[0] = normal;
    ComputeOrthogonalComplement(1, basis.data());
    mTriangleBasis.SetCol(0, basis[1]);
    mTriangleBasis.SetCol(1, basis[2]);
    mTriangleBasis.SetCol(2, basis[0]);
    mTriangleCenter = (mTriangle.v[0] + mTriangle.v[1] + mTriangle.v[2]) / 3.0f;
    for (int32_t i = 0; i < 3; ++i)
    {
        Vector3<float> diff = mTriangle.v[i] - mTriangleCenter;
        mTriangleCoord[i][0] = Dot(mTriangleBasis.GetCol(0), diff);
        mTriangleCoord[i][1] = Dot(mTriangleBasis.GetCol(1), diff);
    }

    basis[0] = mCylinder.axis.direction;
    ComputeOrthogonalComplement(1, basis.data());
    mCylinderBasis.SetCol(0, basis[1]);
    mCylinderBasis.SetCol(1, basis[2]);
    mCylinderBasis.SetCol(2, basis[0]);

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.001f, 0.001f,
        { 6.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void IntersectTriangleCylinderWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mTriangleMesh);
    mEngine->Draw(mCylinderMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectTriangleCylinderWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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

    case '0':  // move the triangle
        mMotionObject = 0;
        return true;

    case '1':  // move the cylinder
        mMotionObject = 1;
        return true;

    case 'x': // decrement x-center of motion object
        Translate(0, -delta);
        return true;

    case 'X': // increment x-center of motion object
        Translate(0, +delta);
        return true;

    case 'y': // decrement y-center of motion object
        Translate(1, -delta);
        return true;

    case 'Y': // increment y-center of motion object
        Translate(1, +delta);
        return true;

    case 'z': // decrement z-center of motion object
        Translate(2, -delta);
        return true;

    case 'Z': // increment z-center of motion object
        Translate(2, +delta);
        return true;

    case 'p': // rotate about axis[0] of motion object
        Rotate(0, -delta);
        return true;

    case 'P': // rotate about axis[0] of motion object
        Rotate(0, +delta);
        return true;

    case 'r': // rotate about axis[1] of motion object
        Rotate(1, -delta);
        return true;

    case 'R': // rotate about axis[1] of motion object
        Rotate(1, +delta);
        return true;

    case 'h': // rotate about axis[2] of motion object
        Rotate(2, -delta);
        return true;

    case 'H': // rotate about axis[2] of motion object
        Rotate(2, +delta);
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void IntersectTriangleCylinderWindow3::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);

    mRedEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 1.0f, 0.0f, 0.0f, 1.0f });
    mGreenEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 1.0f, 0.0f, 1.0f });
    mBlueEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });

    // Create the triangle mesh.
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 3);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto vertices = vbuffer->Get<Vector3<float>>();
    for (size_t i = 0; i < 3; ++i)
    {
        vertices[i] = mTriangle.v[i];
    }
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 1);
    mTriangleMesh = std::make_shared<Visual>(vbuffer, ibuffer, mBlueEffect);
    mPVWMatrices.Subscribe(mTriangleMesh);
    mTrackBall.Attach(mTriangleMesh);

    // Create the cylinder mesh.
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetVertexBufferUsage(Resource::Usage::DYNAMIC_UPDATE);
    mCylinderMesh = mf.CreateCylinderClosed(8, 16, mCylinder.radius, mCylinder.height);
    mCylinderMesh->SetEffect(mRedEffect);
    mPVWMatrices.Subscribe(mCylinderMesh);
    mTrackBall.Attach(mCylinderMesh);
}

void IntersectTriangleCylinderWindow3::Translate(int32_t direction, float delta)
{
    if (mMotionObject == 0)
    {
        // Translate the triangle.
        for (int32_t i = 0; i < 3; ++i)
        {
            mTriangle.v[i][direction] += delta;
        }
        mTriangleCenter = (mTriangle.v[0] + mTriangle.v[1] + mTriangle.v[2]) / 3.0f;

        auto const& targetVBuffer = mTriangleMesh->GetVertexBuffer();
        auto* target = targetVBuffer->Get<Vector3<float>>();
        for (int32_t i = 0; i < 3; ++i)
        {
            target[i] = mTriangle.v[i];
        }
        mEngine->Update(targetVBuffer);
    }
    else
    {
        // Translate the cylinder.
        mCylinder.axis.origin[direction] += delta;

        VertexFormat vformat;
        vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
        MeshFactory mf;
        mf.SetVertexFormat(vformat);
        auto mesh = mf.CreateCylinderClosed(8, 16, mCylinder.radius, mCylinder.height);
        auto const& sourceVBuffer = mesh->GetVertexBuffer();
        uint32_t numVertices = sourceVBuffer->GetNumElements();
        auto const* source = sourceVBuffer->Get<Vector3<float>>();
        auto const& targetVBuffer = mCylinderMesh->GetVertexBuffer();
        auto* target = targetVBuffer->Get<Vector3<float>>();
        for (uint32_t i = 0; i < numVertices; ++i)
        {
            target[i] = mCylinder.axis.origin + mCylinderBasis * source[i];
        }
        mEngine->Update(targetVBuffer);
    }

    DoIntersectionQuery();
}

void IntersectTriangleCylinderWindow3::Rotate(int32_t direction, float delta)
{
    float const cs = std::cos(delta);
    float const sn = std::sin(delta);

    if (mMotionObject == 0)
    {
        // Rotate the triangle about an axis through the triangle center.
        if (direction == 0)
        {
            Vector3<float> temp1 = cs * mTriangleBasis.GetCol(1) - sn * mTriangleBasis.GetCol(2);
            Vector3<float> temp2 = sn * mTriangleBasis.GetCol(1) + cs * mTriangleBasis.GetCol(2);
            mTriangleBasis.SetCol(1, temp1);
            mTriangleBasis.SetCol(2, temp2);
        }
        else if (direction == 1)
        {
            Vector3<float> temp0 = cs * mTriangleBasis.GetCol(0) - sn * mTriangleBasis.GetCol(2);
            Vector3<float> temp2 = sn * mTriangleBasis.GetCol(0) + cs * mTriangleBasis.GetCol(2);
            mTriangleBasis.SetCol(0, temp0);
            mTriangleBasis.SetCol(2, temp2);
        }
        else
        {
            Vector3<float> temp0 = cs * mTriangleBasis.GetCol(0) - sn * mTriangleBasis.GetCol(1);
            Vector3<float> temp1 = sn * mTriangleBasis.GetCol(0) + cs * mTriangleBasis.GetCol(1);
            mTriangleBasis.SetCol(0, temp0);
            mTriangleBasis.SetCol(1, temp1);
        }

        for (int32_t i = 0; i < 3; ++i)
        {
            mTriangle.v[i] = mTriangleCenter
                + mTriangleCoord[i][0] * mTriangleBasis.GetCol(0)
                + mTriangleCoord[i][1] * mTriangleBasis.GetCol(1);
        }
        mTriangleCenter = (mTriangle.v[0] + mTriangle.v[1] + mTriangle.v[2]) / 3.0f;

        auto const& targetVBuffer = mTriangleMesh->GetVertexBuffer();
        auto* target = targetVBuffer->Get<Vector3<float>>();
        for (int32_t i = 0; i < 3; ++i)
        {
            target[i] = mTriangle.v[i];
        }
        mEngine->Update(targetVBuffer);
    }
    else
    {
        // Rotate the cylinder an axis through the cylinder center.
        if (direction == 0)
        {
            Vector3<float> temp1 = cs * mCylinderBasis.GetCol(1) - sn * mCylinderBasis.GetCol(2);
            Vector3<float> temp2 = sn * mCylinderBasis.GetCol(1) + cs * mCylinderBasis.GetCol(2);
            mCylinderBasis.SetCol(1, temp1);
            mCylinderBasis.SetCol(2, temp2);
        }
        else if (direction == 1)
        {
            Vector3<float> temp0 = cs * mCylinderBasis.GetCol(0) - sn * mCylinderBasis.GetCol(2);
            Vector3<float> temp2 = sn * mCylinderBasis.GetCol(0) + cs * mCylinderBasis.GetCol(2);
            mCylinderBasis.SetCol(0, temp0);
            mCylinderBasis.SetCol(2, temp2);
        }
        else
        {
            Vector3<float> temp0 = cs * mCylinderBasis.GetCol(0) - sn * mCylinderBasis.GetCol(1);
            Vector3<float> temp1 = sn * mCylinderBasis.GetCol(0) + cs * mCylinderBasis.GetCol(1);
            mCylinderBasis.SetCol(0, temp0);
            mCylinderBasis.SetCol(1, temp1);
        }

        mCylinder.axis.direction = mCylinderBasis.GetCol(2);

        VertexFormat vformat;
        vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
        MeshFactory mf;
        mf.SetVertexFormat(vformat);
        auto mesh = mf.CreateCylinderClosed(8, 16, mCylinder.radius, mCylinder.height);
        auto const& sourceVBuffer = mesh->GetVertexBuffer();
        uint32_t numVertices = sourceVBuffer->GetNumElements();
        auto const* source = sourceVBuffer->Get<Vector3<float>>();
        auto const& targetVBuffer = mCylinderMesh->GetVertexBuffer();
        auto* target = targetVBuffer->Get<Vector3<float>>();
        for (uint32_t i = 0; i < numVertices; ++i)
        {
            target[i] = mCylinder.axis.origin + mCylinderBasis * source[i];
        }
        mEngine->Update(targetVBuffer);
    }

    DoIntersectionQuery();
}

void IntersectTriangleCylinderWindow3::DoIntersectionQuery()
{
    mPVWMatrices.Unsubscribe(mTriangleMesh);

    auto result = mQuery(mTriangle, mCylinder);
    if (result.intersect)
    {
        mTriangleMesh->SetEffect(mGreenEffect);
    }
    else
    {
        mTriangleMesh->SetEffect(mBlueEffect);
    }

    mPVWMatrices.Subscribe(mTriangleMesh);
    mPVWMatrices.Update();
}
