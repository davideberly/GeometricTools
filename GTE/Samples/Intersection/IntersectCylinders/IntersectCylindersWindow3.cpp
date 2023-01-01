// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectCylindersWindow3.h"
#include <Mathematics/Rotation.h>
#include <Graphics/MeshFactory.h>

IntersectCylindersWindow3::IntersectCylindersWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mNumLines(2048),
    mCylinder{},
    mQuery(mNumLines),
    mMotionObject(0),
    mCylinderBasis{}
{
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

void IntersectCylindersWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mCylinderMesh[0]);
    mEngine->Draw(mCylinderMesh[1]);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectCylindersWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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

    case '0':  // move cylinder[0]
        mMotionObject = 0;
        return true;

    case '1':  // move cylinder[1]
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
    }

    return Window3::OnCharPress(key, x, y);
}

void IntersectCylindersWindow3::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);

    mRedEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 1.0f, 0.0f, 0.0f, 1.0f });
    mMagentaEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 1.0f, 0.0f, 1.0f, 1.0f });
    mBlueEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    mCyanEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 1.0f, 1.0f, 1.0f });

    // Create the cylinder meshes.
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetVertexBufferUsage(Resource::Usage::DYNAMIC_UPDATE);

    mCylinderMesh[0] = mf.CreateCylinderClosed(8, 128, mCylinder[0].radius, mCylinder[0].height);
    mCylinderMesh[0]->SetEffect(mRedEffect);
    mPVWMatrices.Subscribe(mCylinderMesh[0]);
    mTrackBall.Attach(mCylinderMesh[0]);

    mCylinderMesh[1] = mf.CreateCylinderClosed(8, 128, mCylinder[1].radius, mCylinder[1].height);
    mCylinderMesh[1]->SetEffect(mBlueEffect);
    mPVWMatrices.Subscribe(mCylinderMesh[1]);
    mTrackBall.Attach(mCylinderMesh[1]);

    // Create the cylinders.
    mCylinder[0].axis.origin = { 0.0f, 0.0f, 0.0f };
    mCylinder[0].axis.direction = { 1.0f, 1.0f, 1.0f };
    Normalize(mCylinder[0].axis.direction);
    mCylinder[0].radius = 1.0f;
    mCylinder[0].height = 2.0f;

    mCylinder[1].axis.origin = { 0.0, 0.0, 1.5 };
    mCylinder[1].axis.direction = { 3.0, 2.0, 1.0 };
    Normalize(mCylinder[1].axis.direction);
    mCylinder[1].radius = 0.125;
    mCylinder[1].height = 1.0;

    // Create bases for use by Translate(...) and Rotate(...).
    std::array<Vector3<float>, 3> basis{};
    for (size_t i = 0; i < 2; ++i)
    {
        basis[0] = mCylinder[i].axis.direction;
        ComputeOrthogonalComplement(1, basis.data());
        mCylinderBasis[i].SetCol(0, basis[1]);
        mCylinderBasis[i].SetCol(1, basis[2]);
        mCylinderBasis[i].SetCol(2, basis[0]);
        mMotionObject = i;
        UpdateCylinderMesh();
    }

    DoIntersectionQuery();
}

void IntersectCylindersWindow3::Translate(int32_t direction, float delta)
{
    mCylinder[mMotionObject].axis.origin[direction] += delta;

    UpdateCylinderMesh();
    DoIntersectionQuery();
}

void IntersectCylindersWindow3::Rotate(int32_t direction, float delta)
{
    auto& cylinder = mCylinder[mMotionObject];
    auto& cylinderBasis = mCylinderBasis[mMotionObject];

    float const cs = std::cos(delta);
    float const sn = std::sin(delta);
    if (direction == 0)
    {
        Vector3<float> temp1 = cs * cylinderBasis.GetCol(1) - sn * cylinderBasis.GetCol(2);
        Vector3<float> temp2 = sn * cylinderBasis.GetCol(1) + cs * cylinderBasis.GetCol(2);
        cylinderBasis.SetCol(1, temp1);
        cylinderBasis.SetCol(2, temp2);
    }
    else // direction == 1
    {
        Vector3<float> temp0 = cs * cylinderBasis.GetCol(0) - sn * cylinderBasis.GetCol(2);
        Vector3<float> temp2 = sn * cylinderBasis.GetCol(0) + cs * cylinderBasis.GetCol(2);
        cylinderBasis.SetCol(0, temp0);
        cylinderBasis.SetCol(2, temp2);
    }

    cylinder.axis.direction = cylinderBasis.GetCol(2);

    UpdateCylinderMesh();
    DoIntersectionQuery();
}

void IntersectCylindersWindow3::UpdateCylinderMesh()
{
    auto& cylinder = mCylinder[mMotionObject];
    auto& cylinderBasis = mCylinderBasis[mMotionObject];
    auto const& cylinderMesh = mCylinderMesh[mMotionObject];

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto mesh = mf.CreateCylinderClosed(8, 128, cylinder.radius, cylinder.height);
    auto const& sourceVBuffer = mesh->GetVertexBuffer();
    uint32_t numVertices = sourceVBuffer->GetNumElements();
    auto const* source = sourceVBuffer->Get<Vector3<float>>();
    auto const& targetVBuffer = cylinderMesh->GetVertexBuffer();
    auto* target = targetVBuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        target[i] = cylinder.axis.origin + cylinderBasis * source[i];
    }
    mEngine->Update(targetVBuffer);
}

void IntersectCylindersWindow3::DoIntersectionQuery()
{
    mPVWMatrices.Unsubscribe(mCylinderMesh[0]);
    mPVWMatrices.Unsubscribe(mCylinderMesh[1]);

    auto result = mQuery(mCylinder[0], mCylinder[1]);
    if (result.separated)
    {
        mCylinderMesh[0]->SetEffect(mRedEffect);
        mCylinderMesh[1]->SetEffect(mBlueEffect);
    }
    else
    {
        mCylinderMesh[0]->SetEffect(mMagentaEffect);
        mCylinderMesh[1]->SetEffect(mCyanEffect);
    }

    mPVWMatrices.Subscribe(mCylinderMesh[0]);
    mPVWMatrices.Subscribe(mCylinderMesh[1]);
    mPVWMatrices.Update();
}
