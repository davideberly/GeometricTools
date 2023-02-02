// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "FlowingSkirtWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/Texture2Effect.h>
#include <random>

//#define SINGLE_STEP

FlowingSkirtWindow3::FlowingSkirtWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mNumCtrl(32),
    mDegree(3),
    mATop(1.0f),
    mBTop(1.5f),
    mABottom(2.0f),
    mBBottom(3.0f),
    mFrequencies(mNumCtrl)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mWireNoCullState = std::make_shared<RasterizerState>();
    mWireNoCullState->cull = RasterizerState::Cull::NONE;
    mWireNoCullState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);

    CreateScene();

    // Center-and-fit for camera viewing.
    mScene->Update();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.005f, 0.01f,
        { 0.0f, 0.0f, -2.5f * mScene->worldBound.GetRadius() },
        { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mSkirt->localTransform.SetTranslation(-mScene->worldBound.GetCenter());
    mSkirt->Update();
    mPVWMatrices.Update();
}

void FlowingSkirtWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(SINGLE_STEP)
    ModifyCurves();
#endif

    mEngine->ClearBuffers();
    mEngine->Draw(mSkirt);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool FlowingSkirtWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
    case 'W':
        if (mWireNoCullState == mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        else
        {
            mEngine->SetRasterizerState(mWireNoCullState);
        }
        return true;
#if defined(SINGLE_STEP)
    case 'g':
    case 'G':
        ModifyCurves();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool FlowingSkirtWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Flower.png") == "")
    {
        LogError("Cannot find file Flower.png.");
        return false;
    }

    return true;
}

void FlowingSkirtWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();

    // The skirt top and bottom boundary curves are chosen to be periodic,
    // looped B-spline curves.  The top control points are generated on an
    // ellipse (x/a0)^2 + (z/b0)^2 = 1 with y = 4.  The bottom control points
    // are generated on an ellipse (x/a1)^2 + (z/b1)^2 = 1 with y = 0.

    // The vertex storage is used for the B-spline control points.  The
    // curve objects make a copy of the input points.  The vertex storage is
    // then used for the skirt mesh vertices themselves.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    // Use random numbers for the frequencies.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.5f, 1.0f);

    uint32_t numVertices = 2 * mNumCtrl;
    std::vector<Vector3<float>> positions(numVertices);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto vertices = vbuffer->Get<Vertex>();
    int32_t i, j;
    for (i = 0, j = mNumCtrl; i < mNumCtrl; ++i, ++j)
    {
        float ratio = static_cast<float>(i) / static_cast<float>(mNumCtrl);
        float angle = ratio * static_cast<float>(GTE_C_TWO_PI);
        float sn = std::sin(angle);
        float cs = std::cos(angle);
        float v = 1.0f - std::fabs(2.0f * ratio - 1.0f);

        // Set a vertex for the skirt top.
        positions[i] = { mATop * cs, 4.0f, mBTop * sn };
        vertices[i].position = positions[i];
        vertices[i].tcoord = { 1.0f, v };

        // Set a vertex for the skirt bottom.
        positions[j] = { mABottom * cs, 0.0f, mBBottom * sn };
        vertices[j].position = positions[j];
        vertices[j].tcoord = { 0.0f, v };

        // Frequency of sinusoidal motion for skirt bottom.
        mFrequencies[i] = rnd(mte);
    }

    // The control points are copied by the curve objects.
    BasisFunctionInput<float> bfInput;
    bfInput.numControls = mNumCtrl;
    bfInput.degree = mDegree;
    bfInput.uniform = false;
    bfInput.periodic = true;
    bfInput.numUniqueKnots = mNumCtrl + mDegree + 1;
    bfInput.uniqueKnots.resize(bfInput.numUniqueKnots);
    float invNmD = 1.0f / static_cast<float>(mNumCtrl - mDegree);
    for (i = 0; i < bfInput.numUniqueKnots; ++i)
    {
        bfInput.uniqueKnots[i].t = static_cast<float>(i - mDegree) * invNmD;
        bfInput.uniqueKnots[i].multiplicity = 1;
    }
    mSkirtTop = std::make_unique<BSplineCurve<3, float>>(bfInput, positions.data());
    mSkirtBottom = std::make_unique<BSplineCurve<3, float>>(bfInput, positions.data() + mNumCtrl);

    // Generate the triangle connectivity (cylinder connectivity).
    uint32_t numTriangles = numVertices;
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(uint32_t));
    auto indices = ibuffer->Get<uint32_t>();
    int32_t i0 = 0, i1 = 1, i2 = mNumCtrl, i3 = mNumCtrl + 1;
    for (i = 0; i1 < mNumCtrl; i0 = i1++, i2 = i3++)
    {
        indices[i++] = i0;
        indices[i++] = i1;
        indices[i++] = i3;
        indices[i++] = i0;
        indices[i++] = i3;
        indices[i++] = i2;
    }
    indices[i++] = mNumCtrl - 1;
    indices[i++] = 0;
    indices[i++] = mNumCtrl;
    indices[i++] = mNumCtrl - 1;
    indices[i++] = mNumCtrl;
    indices[i++] = 2 * mNumCtrl - 1;

    std::string path = mEnvironment.GetPath("Flower.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);

    mSkirt = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSkirt->UpdateModelBound();
    mPVWMatrices.Subscribe(mSkirt->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mSkirt);
    mTrackBall.Attach(mScene);

    // Compute the vertex values for the current B-spline curves.
    UpdateSkirt();
}

void FlowingSkirtWindow3::UpdateSkirt()
{
    auto const& vbuffer = mSkirt->GetVertexBuffer();
    auto vertices = vbuffer->Get<Vertex>();
    for (int32_t i = 0, j = mNumCtrl; i < mNumCtrl; ++i, ++j)
    {
        float t = static_cast<float>(i) / static_cast<float>(mNumCtrl);
        Vector3<float> values[4];
        mSkirtTop->Evaluate(t, 0, values);
        vertices[i].position = values[0];
        mSkirtBottom->Evaluate(t, 0, values);
        vertices[j].position = values[0];
    }

    mSkirt->Update();
    mPVWMatrices.Update();
    mEngine->Update(vbuffer);
}

void FlowingSkirtWindow3::ModifyCurves()
{
    // Perturb the skirt bottom.
    float time = static_cast<float>(mAnimTimer.GetSeconds());
    for (int32_t i = 0; i < mNumCtrl; ++i)
    {
        float ratio = static_cast<float>(i) / static_cast<float>(mNumCtrl);
        float angle = ratio * static_cast<float>(GTE_C_TWO_PI);
        float sn = std::sin(angle);
        float cs = std::cos(angle);

        float amplitude = 1.0f + 0.25f * std::cos(mFrequencies[i] * time);
        Vector3<float> ctrl{ amplitude * mABottom * cs, 0.0f, amplitude * mBBottom * sn };
        mSkirtBottom->SetControl(i, ctrl);
    }

    UpdateSkirt();
}
