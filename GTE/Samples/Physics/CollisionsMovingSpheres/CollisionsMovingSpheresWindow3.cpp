// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.20

#include "CollisionsMovingSpheresWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/VertexColorEffect.h>
#include <random>

//#define SINGLE_STEP

CollisionsMovingSpheresWindow3::CollisionsMovingSpheresWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mMesh0{},
    mMesh1{},
    mSphere0(Vector3<float>{ 0.0f, 0.75f, 0.0f }, 0.1f),
    mSphere1(Vector3<float>{ 0.0f, -0.75f, 0.0f }, 0.2f),
    mBoundingSphere(Vector3<float>{ 0.0f, 0.0f, 0.0f }, 1.0f),
    mVelocity0{ 0.0f, -1.0f, 0.0f },
    mVelocity1{ 0.0f, 0.0f, 1.0f },
    mColliders(mSphere0, mSphere1),
    mSimulationTime(0.0f),
    mSimulationDeltaTime(0.0f)
{
#if defined(SINGLE_STEP)
    mSimulationTime = 0.01f;
    mSimulationDeltaTime = 0.01f;
#else
    mSimulationTime = 0.0001f;
    mSimulationDeltaTime = 0.0001f;
#endif

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 3.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
}

void CollisionsMovingSpheresWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(SINGLE_STEP)
    UpdateSpheres();
#endif

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh0);
    mEngine->Draw(mMesh1);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool CollisionsMovingSpheresWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
#if defined(SINGLE_STEP)
    switch (key)
    {
    case 'g':
    case 'G':
        UpdateSpheres();
        return true;
    }
#endif
    return Window3::OnCharPress(key, x, y);
}

void CollisionsMovingSpheresWindow3::CreateScene()
{
    std::default_random_engine dre{};
    std::uniform_real_distribution<float> urd(0.0f, 1.0f);

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    MeshFactory mf{};
    mf.SetVertexFormat(vformat);

    mMesh0 = mf.CreateSphere(16, 16, mSphere0.radius);
    auto const& vbuffer0 = mMesh0->GetVertexBuffer();
    auto* vertices0 = vbuffer0->Get<Vertex>();
    uint32_t const numVertices0 = vbuffer0->GetNumElements();
    for (uint32_t i = 0; i < numVertices0; ++i)
    {
        vertices0[i].color = { urd(dre), 0.0f, 0.0f, 1.0f };
    }
    auto effect0 = std::make_shared<VertexColorEffect>(mProgramFactory);
    mMesh0->SetEffect(effect0);
    mMesh0->localTransform.SetTranslation(mSphere0.center);
    mPVWMatrices.Subscribe(mMesh0);
    mTrackBall.Attach(mMesh0);

    mMesh1 = mf.CreateSphere(16, 16, mSphere1.radius);
    auto const& vbuffer1 = mMesh1->GetVertexBuffer();
    auto* vertices1 = vbuffer1->Get<Vertex>();
    uint32_t const numVertices1 = vbuffer1->GetNumElements();
    for (uint32_t i = 0; i < numVertices1; ++i)
    {
        vertices1[i].color = { 0.0f, 0.0f, urd(dre), 1.0f };
    }
    auto effect1 = std::make_shared<VertexColorEffect>(mProgramFactory);
    mMesh1->SetEffect(effect1);
    mMesh1->localTransform.SetTranslation(mSphere1.center);
    mPVWMatrices.Subscribe(mMesh1);
    mTrackBall.Attach(mMesh1);

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void CollisionsMovingSpheresWindow3::UpdateSpheres()
{
    float contactTime{};
    (void)mColliders.Find(mSimulationTime, mVelocity0, mVelocity1, contactTime);
    if (contactTime < 0.0f)
    {
        contactTime = 0.0f;
    }

    if (contactTime <= mSimulationTime)
    {
        // Move the spheres through the time to make contact.
        mSphere0.center += contactTime * mVelocity0;
        mSphere1.center += contactTime * mVelocity1;
        Vector3<float> contactPoint = mColliders.GetContactPoint();

        // Compute the unit-length normals at the contact point.
        Vector3<float> normal0 = contactPoint - mSphere0.center;
        (void)Normalize(normal0);
        Vector3<float> normal1 = contactPoint - mSphere1.center;
        (void)Normalize(normal1);

        // Reflect the velocities through the normals.
        mVelocity0 -= 2.0f * Dot(mVelocity0, normal1) * normal1;
        mVelocity1 -= 2.0f * Dot(mVelocity1, normal0) * normal0;
        mSimulationTime -= contactTime;
    }
    else
    {
        mSphere0.center += mSimulationDeltaTime * mVelocity0;
        mSphere1.center += mSimulationDeltaTime * mVelocity1;
        mSimulationTime = mSimulationDeltaTime;
    }

    // Keep the spheres inside the world sphere.
    Vector3<float> diff = mSphere0.center - mBoundingSphere.center;
    float length = Normalize(diff);
    if (length >= mBoundingSphere.radius)
    {
        mSphere0.center = mBoundingSphere.radius * diff;
        mVelocity0 -= 2.0f * Dot(mVelocity0, diff) * diff;
    }
    diff = mSphere1.center - mBoundingSphere.center;
    length = Normalize(diff);
    if (length >= mBoundingSphere.radius)
    {
        mSphere1.center = mBoundingSphere.radius * diff;
        mVelocity1 -= 2.0f * Dot(mVelocity1, diff) * diff;
    }

    mMesh0->localTransform.SetTranslation(mSphere0.center);
    mMesh1->localTransform.SetTranslation(mSphere1.center);
    mTrackBall.Update();
    mPVWMatrices.Update();
}
