// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.12.02

#include "IntersectLineRectangleWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>

IntersectLineRectangleWindow3::IntersectLineRectangleWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mNoCullState{},
    mLinearMesh{},
    mRectangleMesh{},
    mSphereMesh{},
    mRectangle{}
#if defined(USE_LINE_RECTANGLE_QUERY)
    ,
    mLine{},
#endif
#if defined(USE_RAY_RECTANGLE_QUERY)
    ,
    mRay{},
#endif
#if defined(USE_SEGMENT_RECTANGLE_QUERY)
    ,
    mSegment{},
#endif
    mQuery{},
    mResult{}
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::NONE;
    mEngine->SetRasterizerState(mNoCullState);

    InitializeCamera(60.0f, GetAspectRatio(), 0.001f, 100.0f, 0.01f, 0.001f,
        { 0.0f, 0.0f, -8.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f});

    CreateScene();
    DoIntersectionQuery();
}

void IntersectLineRectangleWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mRectangleMesh);
    mEngine->Draw(mLinearMesh);
    if (mResult.intersect)
    {
        mEngine->Draw(mSphereMesh);
    }
    //mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectLineRectangleWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    float constexpr delta = 0.1f;

    switch (key)
    {
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

void IntersectLineRectangleWindow3::CreateScene()
{
#if defined(USE_LINE_RECTANGLE_QUERY)
    mLine.origin = { 0.0f, 0.0f, 0.0f };
    mLine.direction = { 0.0f, 0.0f, 1.0f };
#endif
#if defined(USE_RAY_RECTANGLE_QUERY)
    mRay.origin = { 0.0f, 0.0f, 0.0f };
    mRay.direction = { 0.0f, 0.0f, 1.0f };
#endif
#if defined(USE_SEGMENT_RECTANGLE_QUERY)
    mSegment.p[0] = { 0.0f, 0.0f, -0.5f };
    mSegment.p[1] = { 0.0f, 0.0f, +0.5f };
#endif

    mRectangle.axis[0] = { 1.0f, 0.0f, 0.0f };
    mRectangle.axis[1] = { 0.0f, 1.0f, 0.0f };
    mRectangle.extent[0] = 2.0f;
    mRectangle.extent[1] = 1.0f;

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf(vformat);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto* vertices = vbuffer->Get<Vector3<float>>();
#if defined(USE_LINE_RECTANGLE_QUERY)
    vertices[0] = mLine.origin - 32.0f * mLine.direction;
    vertices[1] = mLine.origin + 32.0f * mLine.direction;
#endif
#if defined(USE_RAY_RECTANGLE_QUERY)
    vertices[0] = mRay.origin;
    vertices[1] = mRay.origin + 32.0f * mRay.direction;
#endif
#if defined(USE_SEGMENT_RECTANGLE_QUERY)
    vertices[0] = mSegment.p[0];
    vertices[1] = mSegment.p[1];
#endif
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f });
    mLinearMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mLinearMesh);
    mTrackBall.Attach(mLinearMesh);

    mRectangleMesh = mf.CreateRectangle(2, 2, mRectangle.extent[0], mRectangle.extent[1]);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.75f, 0.0f, 0.0f, 1.0f });
    mRectangleMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mRectangleMesh);
    mTrackBall.Attach(mRectangleMesh);

    mSphereMesh = mf.CreateSphere(8, 8, 0.05f);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    mSphereMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mSphereMesh);
    mTrackBall.Attach(mSphereMesh);
}

void IntersectLineRectangleWindow3::Translate(int32_t direction, float delta)
{
    mRectangle.center[direction] += delta;
    mRectangleMesh->localTransform.SetTranslation(mRectangle.center);
    mRectangleMesh->Update();
    DoIntersectionQuery();
}

void IntersectLineRectangleWindow3::Rotate(int32_t direction, float delta)
{
    AxisAngle<3, float> aa{};
    Quaternion<float> incr{};

    if (direction == 0)
    {
        aa = AxisAngle<3, float>(mRectangle.axis[0], delta);
        incr = Rotation<3, float>(aa);
        mRectangle.axis[1] = gte::Rotate(incr, mRectangle.axis[1]);
    }
    else if (direction == 1)
    {
        aa = AxisAngle<3, float>(mRectangle.axis[1], delta);
        incr = Rotation<3, float>(aa);
        mRectangle.axis[0] = gte::Rotate(incr, mRectangle.axis[0]);
    }
    else // direction = 2
    {
        aa = AxisAngle<3, float>(Cross(mRectangle.axis[0], mRectangle.axis[1]), delta);
        incr = Rotation<3, float>(aa);
        mRectangle.axis[0] = gte::Rotate(incr, mRectangle.axis[0]);
        mRectangle.axis[1] = gte::Rotate(incr, mRectangle.axis[1]);
    }

    Quaternion<float> q{};
    mRectangleMesh->localTransform.GetRotation(q);
    mRectangleMesh->localTransform.SetRotation(incr * q);
    mRectangleMesh->Update();
    DoIntersectionQuery();
}

void IntersectLineRectangleWindow3::DoIntersectionQuery()
{
#if defined(USE_LINE_RECTANGLE_QUERY)
    mResult = mQuery(mLine, mRectangle);
    TIQuery<float, Line3<float>, Rectangle3<float>> tiQuery{};
    auto tiResult = tiQuery(mLine, mRectangle);
    if (tiResult.intersect != mResult.intersect)
    {
        int stophere{};
        stophere = 0;
    }
#endif
#if defined(USE_RAY_RECTANGLE_QUERY)
    mResult = mQuery(mRay, mRectangle);
    TIQuery<float, Ray3<float>, Rectangle3<float>> tiQuery{};
    auto tiResult = tiQuery(mRay, mRectangle);
    if (tiResult.intersect != mResult.intersect)
    {
        int stophere{};
        stophere = 0;
    }
#endif
#if defined(USE_SEGMENT_RECTANGLE_QUERY)
    mResult = mQuery(mSegment, mRectangle);
    TIQuery<float, Segment3<float>, Rectangle3<float>> tiQuery{};
    auto tiResult = tiQuery(mSegment, mRectangle);
    if (tiResult.intersect != mResult.intersect)
    {
        int stophere{};
        stophere = 0;
    }
#endif

    if (mResult.intersect)
    {
        mSphereMesh->localTransform.SetTranslation(mResult.point);
    }

    mTrackBall.Update();
    mPVWMatrices.Update();
}