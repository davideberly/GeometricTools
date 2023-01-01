// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectingBoxesWindow3.h"
#include <Graphics/MeshFactory.h>

IntersectingBoxesWindow3::IntersectingBoxesWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mDoSimulation(true),
    mLastIdle(0.0),
    mSize(256.0f),
    mPerturb(-4.0f, 4.0f)
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 10000.0f, 0.5f, 0.001f,
        { 0.0f, 0.0f, -mSize }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();

    mLastIdle = mSimulationTimer.GetSeconds();
}

void IntersectingBoxesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    PhysicsTick();
    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool IntersectingBoxesWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
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

    case 's':
    case 'S':
        mDoSimulation = !mDoSimulation;
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void IntersectingBoxesWindow3::CreateScene()
{
    // Create some axis-aligned boxes for intersection testing.
    std::uniform_real_distribution<float> rnd(16.0f, 64.0f), symr(-1.0f, 1.0f);
    for (int32_t i = 0; i < NUM_BOXES; ++i)
    {
        Vector3<float> min =
        {
            0.5f * mSize * symr(mMTE),
            0.5f * mSize * symr(mMTE),
            0.5f * mSize * symr(mMTE)
        };

        Vector3<float> max =
        {
            min[0] + rnd(mMTE),
            min[1] + rnd(mMTE),
            min[2] + rnd(mMTE)
        };

        mBoxes.push_back(AlignedBox3<float>(min, max));
    }
    mManager = std::make_unique<BoxManager<float>>(mBoxes);

    // Scene graph for the visual representation of the boxes.
    mScene = std::make_shared<Node>();

    // Effects for boxes, blue for nonintersecting and red for intersecting.
    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    Vector4<float> white{ 1.0f, 1.0f, 1.0f, 1.0f };
    auto blueMaterial = std::make_shared<Material>();
    blueMaterial->emissive = black;
    blueMaterial->ambient = { 0.25f, 0.25f, 0.25f, 1.0f };
    blueMaterial->diffuse = { 0.0f, 0.0f, 1.0f, 1.0f };
    blueMaterial->specular = black;

    auto redMaterial = std::make_shared<Material>();
    redMaterial->emissive = black;
    redMaterial->ambient = { 0.25f, 0.25f, 0.25f, 1.0f };
    redMaterial->diffuse = { 1.0f, 0.0f, 0.0f, 1.0f };
    redMaterial->specular = black;

    // A light for the effects.
    auto lighting = std::make_shared<Lighting>();
    lighting->ambient = white;
    lighting->diffuse = white;
    lighting->specular = black;

    auto geometry = std::make_shared<LightCameraGeometry>();
    geometry->lightModelDirection =  { 0.0f, 0.0f, 1.0f, 0.0f };

    // Create visual representations of the boxes.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    for (int32_t i = 0; i < NUM_BOXES; ++i)
    {
        Vector3<float> extent = 0.5f * (mBoxes[i].max - mBoxes[i].min);
        Vector3<float> center = 0.5f * (mBoxes[i].max + mBoxes[i].min);
        mBoxMesh[i] = mf.CreateBox(extent[0], extent[1], extent[2]);
        mBoxMesh[i]->GetVertexBuffer()->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
        auto const& vbuffer = mBoxMesh[i]->GetVertexBuffer();
        uint32_t const numVertices = vbuffer->GetNumElements();
        auto vertices = vbuffer->Get<Vertex>();
        for (uint32_t j = 0; j < numVertices; ++j)
        {
            vertices[j].position += center;
        }

        mNoIntersectEffect[i] = std::make_shared<DirectionalLightEffect>(mProgramFactory,
            mUpdater, 0, blueMaterial, lighting, geometry);

        mIntersectEffect[i] = std::make_shared<DirectionalLightEffect>(mProgramFactory,
            mUpdater, 0, redMaterial, lighting, geometry);

        mBoxMesh[i]->SetEffect(mNoIntersectEffect[i]);

        mPVWMatrices.Subscribe(mBoxMesh[i]->worldTransform,
            mNoIntersectEffect[i]->GetPVWMatrixConstant());

        mScene->AttachChild(mBoxMesh[i]);
    }

    mTrackBall.Attach(mScene);
}

void IntersectingBoxesWindow3::ModifyBoxes()
{
    for (int32_t i = 0; i < NUM_BOXES; ++i)
    {
        AlignedBox3<float> box = mBoxes[i];

        for (int32_t j = 0; j < 3; ++j)
        {
            float delta = mPerturb(mMTE);
            if (-mSize <= box.min[j] + delta && box.max[j] + delta <= mSize)
            {
                box.min[j] += delta;
                box.max[j] += delta;
            }
        }

        mManager->SetBox(i, box);
        ModifyMesh(i);
    }

    mManager->Update();
    mScene->Update();

    // Switch material to red for any box that overlaps another.
    for (int32_t i = 0; i < NUM_BOXES; ++i)
    {
        // Reset all boxes to blue.
        mPVWMatrices.Unsubscribe(mBoxMesh[i]->worldTransform);
        mBoxMesh[i]->SetEffect(mNoIntersectEffect[i]);
        mPVWMatrices.Subscribe(mBoxMesh[i]->worldTransform,
            mNoIntersectEffect[i]->GetPVWMatrixConstant());
    }

    for (auto const& overlap : mManager->GetOverlap())
    {
        // Set intersecting boxes to red.
        for (int32_t j = 0; j < 2; ++j)
        {
            int32_t v = overlap.V[j];
            mPVWMatrices.Unsubscribe(mBoxMesh[v]->worldTransform);
            mBoxMesh[v]->SetEffect(mIntersectEffect[v]);
            mPVWMatrices.Subscribe(mBoxMesh[v]->worldTransform,
                mIntersectEffect[v]->GetPVWMatrixConstant());
        }
    }

    mPVWMatrices.Update();
}

void IntersectingBoxesWindow3::ModifyMesh(int32_t i)
{
    Vector3<float> extent = 0.5f * (mBoxes[i].max - mBoxes[i].min);
    Vector3<float> center = 0.5f * (mBoxes[i].max + mBoxes[i].min);
    Vector3<float> xTerm = { extent[0], 0.0f, 0.0f };
    Vector3<float> yTerm = { 0.0f, extent[1], 0.0f };
    Vector3<float> zTerm = { 0.0f, 0.0f, extent[2] };

    auto const& vbuffer = mBoxMesh[i]->GetVertexBuffer();
    auto vertices = vbuffer->Get<Vertex>();
    vertices[0].position = center - xTerm - yTerm - zTerm;
    vertices[1].position = center + xTerm - yTerm - zTerm;
    vertices[2].position = center - xTerm + yTerm - zTerm;
    vertices[3].position = center + xTerm + yTerm - zTerm;
    vertices[4].position = center - xTerm - yTerm + zTerm;
    vertices[5].position = center + xTerm - yTerm + zTerm;
    vertices[6].position = center - xTerm + yTerm + zTerm;
    vertices[7].position = center + xTerm + yTerm + zTerm;

    mEngine->Update(vbuffer);
}

void IntersectingBoxesWindow3::PhysicsTick()
{
    if (mDoSimulation)
    {
        double currIdle = mSimulationTimer.GetSeconds();
        double diff = currIdle - mLastIdle;
        if (30.0 * diff - 1.0 >= 0.0)
        {
            ModifyBoxes();
            mLastIdle = currIdle;
        }
    }
}

void IntersectingBoxesWindow3::GraphicsTick()
{
    mEngine->ClearBuffers();
    for (int32_t i = 0; i < NUM_BOXES; ++i)
    {
        mEngine->Draw(mBoxMesh[i]);
    }
    mEngine->DisplayColorBuffer(0);
}
