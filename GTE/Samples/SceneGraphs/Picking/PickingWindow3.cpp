// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "PickingWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/Texture2Effect.h>

PickingWindow3::PickingWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mNumActiveSpheres(0)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { -16.0f, 0.0f, 2.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void PickingWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mTorus);
    mEngine->Draw(mDodecahedron);
    mEngine->Draw(mPoints);
    mEngine->Draw(mSegments);
    for (int32_t i = 0; i < mNumActiveSpheres; ++i)
    {
        mEngine->Draw(mSphere[i]);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool PickingWindow3::OnMouseClick(MouseButton button, MouseState state, int32_t x,
    int32_t y, uint32_t modifiers)
{
    if (!Window3::OnMouseClick(button, state, x, y, modifiers))
    {
        if (button == MOUSE_RIGHT && state == MOUSE_DOWN)
        {
            DoPick(x, mYSize - 1 - y);
        }
    }
    return true;
}

bool PickingWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Checkerboard.png") == "")
    {
        LogError("Cannot open file Checkerboard.png.");
        return false;
    }

    return true;
}

void PickingWindow3::CreateScene()
{
    std::string path = mEnvironment.GetPath("Checkerboard.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, false);

    mScene = std::make_shared<Node>();

    VertexFormat vformat0;
    vformat0.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat0.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    // The torus and dodecahedron are created by the mesh factory in which
    // the 'visual' model bounds are computed.  The points and segments
    // primitives are created explicitly here, so we need to compute their
    // model bounds to be used by the picking system.
    MeshFactory mf;
    mf.SetVertexFormat(vformat0);

    mTorus = mf.CreateTorus(16, 16, 4.0f, 1.0f);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mTorus->SetEffect(effect);
    mPVWMatrices.Subscribe(mTorus->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mTorus);

    mDodecahedron = mf.CreateDodecahedron();
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP);
    mDodecahedron->SetEffect(effect);
    mPVWMatrices.Subscribe(mDodecahedron->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mDodecahedron);

    VertexFormat vformat1;
    vformat1.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat1, 4);
    auto* vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { 1.0f, 1.0f, 4.0f };
    vertices[1] = { 1.0f, 2.0f, 5.0f };
    vertices[2] = { 2.0f, 2.0f, 6.0f };
    vertices[3] = { 2.0f, 1.0f, 7.0f };
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, 4);
    auto cceffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.5f, 0.0f, 0.0f, 1.0f });
    mPoints = std::make_shared<Visual>(vbuffer, ibuffer, cceffect);
    mPoints->UpdateModelBound();
    mPVWMatrices.Subscribe(mPoints->worldTransform, cceffect->GetPVWMatrixConstant());
    mScene->AttachChild(mPoints);

    vbuffer = std::make_shared<VertexBuffer>(vformat1, 4);
    vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { -1.0f, -1.0f, 4.0f };
    vertices[1] = { -1.0f, -2.0f, 5.0f };
    vertices[2] = { -2.0f, -1.0f, 6.0f };
    vertices[3] = { -2.0f, -2.0f, 7.0f };
    ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, 3, sizeof(int32_t));
    ibuffer->SetSegment(0, 0, 1);
    ibuffer->SetSegment(1, 1, 2);
    ibuffer->SetSegment(2, 2, 3);
    cceffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.5f, 1.0f });
    mSegments = std::make_shared<Visual>(vbuffer, ibuffer, cceffect);
    mSegments->UpdateModelBound();
    mPVWMatrices.Subscribe(mSegments->worldTransform, cceffect->GetPVWMatrixConstant());
    mScene->AttachChild(mSegments);

    for (int32_t i = 0; i < SPHERE_BUDGET; ++i)
    {
        mSphere[i] = mf.CreateSphere(8, 8, 0.125f);
        cceffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
            Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f });
        mSphere[i]->SetEffect(cceffect);
        mPVWMatrices.Subscribe(mSphere[i]->worldTransform, cceffect->GetPVWMatrixConstant());
        mScene->AttachChild(mSphere[i]);
    }

    mTrackBall.Attach(mScene);
    mTrackBall.Update();
}

void PickingWindow3::DoPick(int32_t x, int32_t y)
{
    int32_t viewX, viewY, viewW, viewH;
    mEngine->GetViewport(viewX, viewY, viewW, viewH);
    Vector4<float> origin, direction;
    if (mCamera->GetPickLine(viewX, viewY, viewW, viewH, x, y, origin, direction))
    {
        // Use a ray for picking.
        float tmin = 0.0f;
        float constexpr tmax = std::numeric_limits<float>::max();

        // Set the distance tolerance for point and segment primitives.
        mPicker.SetMaxDistance(0.0625f);

        // Request the results in model-space coordinates.  All the objects
        // in the scene have the same model space, so we can set the sphere
        // centers in model-space coordinates.
        mPicker(mScene, origin, direction, tmin, tmax);
        mNumActiveSpheres = std::min((int32_t)mPicker.records.size(), (int32_t)SPHERE_BUDGET);
        if (mNumActiveSpheres > 0)
        {
            // Place spheres at the picked locations.
            Matrix4x4<float> const& invWMatrix = mScene->worldTransform.GetHInverse();
            for (int32_t i = 0; i < mNumActiveSpheres; ++i)
            {
                Vector4<float> modelPosition = DoTransform(invWMatrix, mPicker.records[i].primitivePoint);
                mSphere[i]->localTransform.SetTranslation(modelPosition);
            }

            mTrackBall.Update();
            mPVWMatrices.Update();
        }
    }
}
