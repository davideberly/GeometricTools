// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "HelixTubeSurfaceWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/Texture2Effect.h>
#include <Mathematics/TubeMesh.h>

HelixTubeSurfaceWindow3::HelixTubeSurfaceWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mMinCurveTime(0.0f),
    mMaxCurveTime(0.0f),
    mCurvePeriod(0.0f),
    mCurveTime(0.0f),
    mDeltaTime(0.0f)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    CreateScene();

    // Disable the default camera rig and work directly with the camera.
    // The coordinate frame will be set by the MoveCamera() member function.
    mCameraRig.ClearMotions();
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.01f, 10.0f);

    MoveCamera(mMinCurveTime);
    mPVWMatrices.Update();
}

void HelixTubeSurfaceWindow3::OnIdle()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    mEngine->Draw(mHelixTube);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool HelixTubeSurfaceWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mWireState == mEngine->GetRasterizerState())
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;
    case '+':
    case '=':
        mDeltaTime *= 2.0f;
        return true;
    case '-':
    case '_':
        mDeltaTime *= 0.5f;
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool HelixTubeSurfaceWindow3::OnKeyDown(int32_t key, int32_t x, int32_t y)
{
    if (key == KEY_UP)
    {
        mCurveTime += mDeltaTime;
        if (mCurveTime > mMaxCurveTime)
        {
            mCurveTime -= mCurvePeriod;
        }
        MoveCamera(mCurveTime);
        return true;
    }

    if (key == KEY_DOWN)
    {
        mCurveTime -= mDeltaTime;
        if (mCurveTime < mMinCurveTime)
        {
            mCurveTime += mCurvePeriod;
        }
        MoveCamera(mCurveTime);
        return true;
    }

    return Window3::OnKeyDown(key, x, y);
}

bool HelixTubeSurfaceWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Grating.png") == "")
    {
        LogError("Cannot find file Grating.png");
        return false;
    }

    return true;
}

void HelixTubeSurfaceWindow3::CreateScene()
{
    MeshDescription desc(MeshTopology::CYLINDER, 256, 32);
    desc.wantCCW = false;

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, desc.numVertices);
    auto vertices = vbuffer->Get<Vertex>();
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(uint32_t));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());

    CreateCurve();
    TubeMesh<float> surface(desc, mMedial, [](float) { return 0.0625f; },
        false, false, { 0.0f, 0.0f, 1.0f });

    // The texture coordinates are in [0,1]^2.  Allow the texture to repeat
    // in the direction along the medial curve.
    for (uint32_t i = 0; i < vbuffer->GetNumElements(); ++i)
    {
        vertices[i].tcoord[1] *= 32.0f;
    }

    auto texture = WICFileIO::Load(mEnvironment.GetPath("Grating.png"), false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mHelixTube = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mHelixTube->Update();
    mPVWMatrices.Subscribe(mHelixTube->worldTransform, effect->GetPVWMatrixConstant());
}

void HelixTubeSurfaceWindow3::CreateCurve()
{
    // Sample points on a looped helix (first and last point must match).
    float const fourPi = static_cast<float>(2.0 * GTE_C_TWO_PI);
    int32_t const numSegments = 32;
    int32_t const numSegmentsP1 = numSegments + 1;
    float const invNumSegments = 1.0f / static_cast<float>(numSegments);
    float const invNumSegmentsP1 = 1.0f / static_cast<float>(numSegmentsP1);
    std::vector<float> times(numSegmentsP1);
    std::vector<Vector3<float>> points(numSegmentsP1);
    float t;
    int32_t i;

    for (i = 0; i <= numSegmentsP1 / 2; ++i)
    {
        t = i * fourPi * invNumSegmentsP1;
        times[i] = t;
        points[i] = { std::cos(t), std::sin(t), t };
    }
    for (/**/; i < numSegments; ++i)
    {
        t = i * fourPi * invNumSegments;
        times[i] = t;
        points[i] = { 2.0f - std::cos(t), std::sin(t), fourPi - t };
    }

    times[numSegments] = fourPi;
    points[numSegments] = points[0];

    // Save min and max times.
    mMinCurveTime = 0.0f;
    mMaxCurveTime = fourPi;
    mCurvePeriod = mMaxCurveTime - mMinCurveTime;
    mCurveTime = mMinCurveTime;
    mDeltaTime = 0.01f;

    // Create a closed cubic curve containing the sample points.
    mMedial = std::make_shared<NaturalSplineCurve<3, float>>(false,
        numSegmentsP1, points.data(), times.data());
}

void HelixTubeSurfaceWindow3::MoveCamera(float time)
{
    Vector3<float> values[4];
    mMedial->Evaluate(time, 1, values);
    Vector4<float> position = HLift(values[0], 1.0f);
    Vector4<float> tangent = HLift(values[1], 0.0f);
    Vector4<float> binormal = UnitCross(tangent, Vector4<float>{ 0.0f, 0.0f, 1.0f, 0.0f });
    Vector4<float> normal = UnitCross(binormal, tangent);
    binormal -= Dot(binormal, normal) * normal;
    Normalize(binormal);
    tangent = Cross(normal, binormal);
    mCamera->SetFrame(position, tangent, normal, binormal);
    mPVWMatrices.Update();
}
