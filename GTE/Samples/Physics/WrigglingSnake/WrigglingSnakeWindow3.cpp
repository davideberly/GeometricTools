// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.12.14

#include "WrigglingSnakeWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/Texture2Effect.h>
#include <Graphics/VertexColorEffect.h>

//#define SINGLE_STEP

WrigglingSnakeWindow3::WrigglingSnakeWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mNumCtrlPoints(32),
    mDegree(3),
    mRadius(0.0625f),
    mMedial{},
    mRadial{},
    mNumMedialSamples(128),
    mNumSliceSamples(32),
    mAmplitudes(mNumCtrlPoints),
    mPhases(mNumCtrlPoints),
    mNumShells(4),
    mSlice(mNumSliceSamples + 1)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mRadial = std::make_shared<std::function<float(float)>>(
        [this](float t)
        {
            return mRadius * (2.0f * t) / (1.0f + t);
        }
    );
        
    mEngine->SetClearColor({ 1.0f, 0.823529f, 0.607843f, 1.0f });
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::WIREFRAME;

    // The camera position was chosen based on precomputed information
    // about the minimum-volume sphere containing the snake vertices.
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 100.0f, 0.01f, 0.001f,
        { 0.0400751755f, 1.97405100f, -0.0681254268f },
        { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void WrigglingSnakeWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#ifndef SINGLE_STEP
    ModifyCurve();
#endif

    mEngine->ClearBuffers();
    mEngine->Draw(mSnakeSurface);
    mEngine->Draw(mSnakeHead);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool WrigglingSnakeWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
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
#ifdef SINGLE_STEP
    case 'g':
    case 'G':
        ModifyCurve();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool WrigglingSnakeWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    std::vector<std::string> inputs =
    {
        "Snake.png"
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

void WrigglingSnakeWindow3::CreateScene()
{
    CreateSnakeBody();
    CreateSnakeHead();
    UpdateSnake();
}

void WrigglingSnakeWindow3::CreateSnakeBody()
{
    // Create the B-spline curve for the snake body.
    std::vector<Vector3<float>> ctrlPoints(mNumCtrlPoints);
    for (size_t i = 0; i < mNumCtrlPoints; ++i)
    {
        // Control points for a snake.
        float const pi = static_cast<float>(GTE_C_PI);
        float ratio = static_cast<float>(i) / static_cast<float>(mNumCtrlPoints - 1);
        float x = -1.0f + 2.0f * ratio;
        float xMod = 10.0f * x - 4.0f;
        ctrlPoints[i][0] = x;
        ctrlPoints[i][1] = mRadius * (1.5f + std::atan(xMod) / pi);
        ctrlPoints[i][2] = 0.0f;

        // Sinusoidal motion for snake.
        mAmplitudes[i] = 0.1f + ratio * std::exp(-ratio);
        mPhases[i] = 3.0f * pi * ratio;
    }

    // The control points are copied by the curve objects.
    BasisFunctionInput<float> input(static_cast<int32_t>(mNumCtrlPoints), static_cast<int32_t>(mDegree));
    mMedial = std::make_shared<BSplineCurve<3, float>>(input, ctrlPoints.data());

    // Generate a tube surface.
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    Vector2<float> tcoordMin{ 0.0f, 0.0f }, tcoordMax{ 1.0f, 16.0f };

    mSnakeBody = std::make_shared<TubeSurface>(mMedial, mRadial,
        Vector3<float>{ 0.0f, 1.0f, 0.0f }, mNumMedialSamples, mNumSliceSamples,
        vformat, tcoordMin, tcoordMax, false, false, false, true);

    mSnakeSurface = mSnakeBody->GetSurface();

    // Attach a texture to the snake body.
    std::string snakeFile = mEnvironment.GetPath("Snake.png");
    auto texture = WICFileIO::Load(snakeFile, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mSnakeSurface->SetEffect(effect);

    mPVWMatrices.Subscribe(mSnakeSurface);
    mTrackBall.Attach(mSnakeSurface);
}

void WrigglingSnakeWindow3::CreateSnakeHead()
{
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);

    // Create the snake head as a paraboloid that is attached to the last
    // ring of vertices on the snake body. These vertices are generated
    // for t = 1.
    mSlice.resize(mNumSliceSamples + 1);

    // Number of rays (determined by slice samples of tube surface).
    uint32_t numRays = static_cast<uint32_t>(mNumSliceSamples - 1);

    // Number of shells less one (your choice, specified in application
    // constructor).
    uint32_t numShellsM1 = static_cast<uint32_t>(mNumShells - 1);

    // Generate vertices. The positions are filled in by UpdateSnake().
    uint32_t numVertices = 1 + numRays * numShellsM1;
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto* vertices = vbuffer->Get<VertexPC>();
    Vector4<float> darkGreen{ 0.0f, 0.25f, 0.0f, 1.0f };
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].color = darkGreen;
    }

    // Generate triangles. TODO: Use MeshFactory::CreateDisk.
    uint32_t numTriangles = numRays * (2 * numShellsM1 - 1);
    auto ibuffer = std::make_shared<IndexBuffer>(IPType::IP_TRIMESH, numTriangles, sizeof(uint32_t));
    auto* indices = ibuffer->Get<uint32_t>();
    for (uint32_t r0 = numRays - 1, r1 = 0; r1 < numRays; r0 = r1++)
    {
        *indices++ = 0;
        *indices++ = 1 + numShellsM1 * r0;
        *indices++ = 1 + numShellsM1 * r1;
        for (uint32_t s = 1; s < numShellsM1; ++s)
        {
            uint32_t i00 = s + numShellsM1 * r0;
            uint32_t i01 = s + numShellsM1 * r1;
            uint32_t i10 = i00 + 1;
            uint32_t i11 = i01 + 1;
            *indices++ = i00;
            *indices++ = i10;
            *indices++ = i11;
            *indices++ = i00;
            *indices++ = i11;
            *indices++ = i01;
        }
    }

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mSnakeHead = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mSnakeHead);
    mTrackBall.Attach(mSnakeHead);
}

void WrigglingSnakeWindow3::UpdateSnake()
{
    // The snake head uses the last ring of vertices in the tube surface of
    // the snake body, so the body must be updated first.
    mSnakeBody->UpdateSurface();
    mEngine->Update(mSnakeSurface->GetVertexBuffer());

    // Get the ring of vertices at the head-end of the tube.
    mSnakeBody->GetTMaxSlice(mSlice);

    // Compute the center of the slice vertices.
    Vector3<float> center = mSlice[0];
    for (size_t i = 1; i <= mNumSliceSamples; ++i)
    {
        center += mSlice[i];
    }
    center /= static_cast<float>(mNumSliceSamples + 1);

    // Compute a unit-length normal of the plane of the vertices. The normal
    // points away from tube and is used to extrude the paraboloid surface
    // for the head.
    Vector3<float> edge1 = mSlice[1] - mSlice[0];
    Vector3<float> edge2 = mSlice[2] - mSlice[0];
    Vector3<float> normal = UnitCross(edge1, edge2);

    // Adjust the normal length to include the height of the ellipsoid vertex
    // above the plane of the slice.
    normal *= 3.0f * mRadius;

    // Set the head origin.
    auto const& vbuffer = mSnakeHead->GetVertexBuffer();
    auto* vertices = vbuffer->Get<VertexPC>();
    vertices[0].position = center + normal;

    // Set the remaining shells.
    size_t const numShellsM1 = mNumShells - 1;
    float factor = 1.0f / static_cast<float>(numShellsM1);
    for (size_t r = 0; r + 1 < mNumSliceSamples; ++r)
    {
        for (size_t s = 1; s < mNumShells; ++s)
        {
            float t = factor * s;
            float oneMinusT = std::min(std::max(1.0f - t, 0.0f), 1.0f);
            size_t i = s + numShellsM1 * r;
            vertices[i].position = oneMinusT * center +
                t * mSlice[r] + std::pow(oneMinusT, 0.25f) * normal;
        }
    }

    mEngine->Update(vbuffer);
}

void WrigglingSnakeWindow3::ModifyCurve()
{
    // Perturb the snake medial curve.
    float time = static_cast<float>(mMotionTimer.GetSeconds());
    int32_t j = 0;
    for (size_t i = 0; i < mNumCtrlPoints; ++i, ++j)
    {
        Vector3<float> ctrl = mMedial->GetControl(j);
        ctrl[2] = mAmplitudes[i] * std::sin(3.0f * time + mPhases[i]);
        mMedial->SetControl(j, ctrl);
    }

    UpdateSnake();
}
