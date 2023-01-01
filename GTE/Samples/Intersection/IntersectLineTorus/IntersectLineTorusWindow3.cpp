// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.28

#include "IntersectLineTorusWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

IntersectLineTorusWindow3::IntersectLineTorusWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mNoCullState{},
    mNoCullWireState{},
    mLineMesh{},
    mTorusMesh{},
    mSphereMesh{},
    mLineExtent(0.0f),
    mLine(Vector3<double>::Zero(), Vector3<double>::Zero()),
    mTorus(Vector3<double>::Zero(), Vector3<double>::Zero(),
        Vector3<double>::Zero(), Vector3<double>::Zero(), 0.0, 0.0),
    mQuery{},
    mResult{},
    mUseLinePoints(true)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { -16.0f, 0.0f, 2.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mLineExtent = 2.0f * mCamera->GetDMax();

    CreateScene();
}

void IntersectLineTorusWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mLineMesh);
    mEngine->Draw(mTorusMesh);
    for (size_t i = 0; i < mResult.numIntersections; ++i)
    {
        mEngine->Draw(mSphereMesh[i]);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectLineTorusWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
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

    case 'p':
    case 'P':
        mUseLinePoints = !mUseLinePoints;
        Update();
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool IntersectLineTorusWindow3::OnMouseClick(MouseButton button,
    MouseState state, int32_t x, int32_t y, uint32_t modifiers)
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

bool IntersectLineTorusWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Checkerboard.png") == "")
    {
        return false;
    }

    return true;
}

void IntersectLineTorusWindow3::CreateScene()
{
    CreateLine();
    CreateTorus();
    CreateSpheres();
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void IntersectLineTorusWindow3::CreateLine()
{
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto* vertices = vbuffer->Get<Vector3<float>>();
    Vector3<float> camPos = HProject(mCamera->GetPosition());
    Vector3<float> camDir = HProject(mCamera->GetDVector());
    vertices[0] = camPos;
    vertices[1] = camPos + mLineExtent * camDir;

    auto ibuffer = std::make_shared<IndexBuffer>(IPType::IP_POLYSEGMENT_DISJOINT, 1);
    
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    
    mLineMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mLineMesh);
    mTrackBall.Attach(mLineMesh);

    mLine.origin =
    {
        static_cast<double>(camPos[0]),
        static_cast<double>(camPos[1]),
        static_cast<double>(camPos[2])
    };

    mLine.direction =
    {
        static_cast<double>(camDir[0]),
        static_cast<double>(camDir[1]),
        static_cast<double>(camDir[2])
    };
}

void IntersectLineTorusWindow3::CreateTorus()
{

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf(vformat);

    float const outerRadius = 4.0f;
    float const innerRadius = 1.0f;
    mTorusMesh = mf.CreateTorus(16, 16, outerRadius, innerRadius);

    std::string path = mEnvironment.GetPath("Checkerboard.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mTorusMesh->SetEffect(effect);

    mPVWMatrices.Subscribe(mTorusMesh);
    mTrackBall.Attach(mTorusMesh);

    mTorus.center = { 0.0, 0.0, 0.0 };
    mTorus.normal = { 0.0, 0.0, 1.0 };
    mTorus.direction0 = { 1.0, 0.0, 0.0 };
    mTorus.direction1 = { 0.0, 1.0, 0.0 };
    mTorus.radius0 = static_cast<double>(outerRadius);
    mTorus.radius1 = static_cast<double>(innerRadius);
}

void IntersectLineTorusWindow3::CreateSpheres()
{
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf(vformat);

    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    for (size_t i = 0; i < 4; ++i)
    {
        mSphereMesh[i] = mf.CreateSphere(8, 8, 0.125f);
        auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);
        mSphereMesh[i]->SetEffect(effect);
        mPVWMatrices.Subscribe(mSphereMesh[i]);
        mTrackBall.Attach(mSphereMesh[i]);
    }
}

void IntersectLineTorusWindow3::DoPick(int32_t x, int32_t y)
{
    // Use the scene graph picking system to generate lines used in the
    // line-torus find-intersection queries.
    int32_t viewX, viewY, viewW, viewH;
    mEngine->GetViewport(viewX, viewY, viewW, viewH);
    Vector4<float> origin, direction;
    if (mCamera->GetPickLine(viewX, viewY, viewW, viewH, x, y, origin, direction))
    {
        auto const& invWMatrix = mTrackBall.GetRoot()->worldTransform.GetHInverse();
        origin = DoTransform(invWMatrix, origin);
        direction = DoTransform(invWMatrix, direction);
        Vector3<float> lineOrigin = HProject(origin);
        Vector3<float> lineDirection = HProject(direction);
        for (int32_t i = 0; i < 3; ++i)
        {
            mLine.origin[i] = static_cast<double>(lineOrigin[i]);
            mLine.direction[i] = static_cast<double>(lineDirection[i]);
        }

        auto const& vbuffer = mLineMesh->GetVertexBuffer();
        auto* vertices = vbuffer->Get<Vector3<float>>();
        vertices[0] = lineOrigin;
        vertices[1] = lineOrigin + mLineExtent * lineDirection;
        mEngine->Update(vbuffer);

        mResult = mQuery(mLine, mTorus);
        Update();
    }
}

void IntersectLineTorusWindow3::Update()
{
    if (!mResult.intersect)
    {
        return;
    }

    if (mUseLinePoints)
    {
        for (size_t i = 0; i < mResult.numIntersections; ++i)
        {
            mResult.point[i] = mLine.origin +
                mResult.lineParameter[i] * mLine.direction;

            mSphereMesh[i]->localTransform.SetTranslation(
                static_cast<float>(mResult.point[i][0]),
                static_cast<float>(mResult.point[i][1]),
                static_cast<float>(mResult.point[i][2]));

            auto const& effect = std::dynamic_pointer_cast<ConstantColorEffect>(mSphereMesh[i]->GetEffect());
            auto const& buffer = effect->GetColorConstant();
            auto& color = *buffer->Get<Vector4<float>>();
            color = Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f };
            mEngine->Update(buffer);
        }
    }
    else
    {
        for (size_t i = 0; i < mResult.numIntersections; ++i)
        {
            double u = mResult.torusParameter[i][0];
            double v = mResult.torusParameter[i][1];
            double csu = std::cos(u);
            double snu = std::sin(u);
            double csv = std::cos(v);
            double snv = std::sin(v);
            double term = mTorus.radius0 + mTorus.radius1 * csv;
            double coeffD0 = term * csu;
            double coeffD1 = term * snu;
            double coeffN = mTorus.radius1 * snv;

            mResult.point[i] = mTorus.center + coeffD0 * mTorus.direction0 +
                coeffD1 * mTorus.direction1 + coeffN * mTorus.normal;

            mSphereMesh[i]->localTransform.SetTranslation(
                static_cast<float>(mResult.point[i][0]),
                static_cast<float>(mResult.point[i][1]),
                static_cast<float>(mResult.point[i][2]));

            auto const& effect = std::dynamic_pointer_cast<ConstantColorEffect>(mSphereMesh[i]->GetEffect());
            auto const& buffer = effect->GetColorConstant();
            auto& color = *buffer->Get<Vector4<float>>();
            color = Vector4<float>{ 1.0f, 0.0f, 0.0f, 1.0f };
            mEngine->Update(buffer);
        }
    }

    mTrackBall.Update();
    mPVWMatrices.Update();
}
