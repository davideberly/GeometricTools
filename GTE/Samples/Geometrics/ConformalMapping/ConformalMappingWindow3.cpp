// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ConformalMappingWindow3.h"
#include <Graphics/VertexColorEffect.h>
#include <Mathematics/ConformalMapGenus0.h>
#include <Mathematics/MeshCurvature.h>

ConformalMappingWindow3::ConformalMappingWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mExtreme(10.0f)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.4f, 0.5f, 0.6f, 1.0f });
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.01f,
        { 0.0f, 0.0f, -6.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    CreateScene();

    mPVWMatrices.Update();
}

void ConformalMappingWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Draw(mSphere);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ConformalMappingWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
        if (mEngine->GetRasterizerState() == mWireState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;
    case 'm':
        // Rotate only the brain mesh.
        mTrackBall.Set(mMeshNode);
        mTrackBall.Update();
        return true;
    case 's':
        // Rotate only the sphere mesh.
        mTrackBall.Set(mSphereNode);
        mTrackBall.Update();
        return true;
    case 'b':
        // Rotate both the brain and sphere meshes simultaneously.
        mTrackBall.Set(mScene);
        mTrackBall.Update();
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool ConformalMappingWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    if (mEnvironment.GetPath("Brain_V4098_T8192.binary") == "")
    {
        LogError("Cannot find file Brain_V4098_T8192.binary");
        return false;
    }
    return true;
}

void ConformalMappingWindow3::LoadBrain(std::vector<Vector3<float>>& positions,
    std::vector<Vector4<float>>& colors, std::vector<uint32_t>& indices)
{
    // Load the brain mesh, which has the topology of a sphere.
    uint32_t const numPositions = NUM_BRAIN_VERTICES;
    uint32_t const numTriangles = NUM_BRAIN_TRIANGLES;
    positions.resize(numPositions);
    colors.resize(numPositions);
    indices.resize(3 * numTriangles);

    std::string path = mEnvironment.GetPath("Brain_V4098_T8192.binary");
    std::ifstream input(path, std::ios::binary);
    input.read((char*)positions.data(), positions.size() * sizeof(positions[0]));
    input.read((char*)indices.data(), indices.size() * sizeof(indices[0]));
    input.close();

    // Scale the data to the cube [-10,10]^3 for numerical preconditioning
    // of the conformal mapping.
    float minValue = positions[0][0], maxValue = minValue;
    for (uint32_t i = 0; i < numPositions; ++i)
    {
        auto const& position = positions[i];
        for (int32_t j = 0; j < 3; ++j)
        {
            if (position[j] < minValue)
            {
                minValue = position[j];
            }
            else if (position[j] > maxValue)
            {
                maxValue = position[j];
            }
        }
    }
    float halfRange = 0.5f * (maxValue - minValue);
    float mult = mExtreme / halfRange;
    for (uint32_t i = 0; i < numPositions; ++i)
    {
        auto& position = positions[i];
        for (int32_t j = 0; j < 3; ++j)
        {
            position[j] = -mExtreme + mult * (position[j] - minValue);
        }
    }

    // Assign vertex colors according to mean curvature.
    MeshCurvature<float>mc;
    mc(positions, indices, 1e-06f);
    auto const& minCurvatures = mc.GetMinCurvatures();
    auto const& maxCurvatures = mc.GetMaxCurvatures();
    std::vector<float> meanCurvatures(numPositions);
    float minMeanCurvature = minCurvatures[0] + maxCurvatures[0];
    float maxMeanCurvature = minMeanCurvature;
    for (uint32_t i = 0; i < numPositions; ++i)
    {
        meanCurvatures[i] = minCurvatures[i] + maxCurvatures[i];
        if (meanCurvatures[i] < minMeanCurvature)
        {
            minMeanCurvature = meanCurvatures[i];
        }
        else if (meanCurvatures[i] > maxMeanCurvature)
        {
            maxMeanCurvature = meanCurvatures[i];
        }
    }

    for (uint32_t i = 0; i < numPositions; ++i)
    {
        auto& color = colors[i];
        if (meanCurvatures[i] > 0.0f)
        {
            color[0] = 0.5f * (1.0f + meanCurvatures[i] / maxMeanCurvature);
            color[1] = color[0];
            color[2] = 0.0f;
        }
        else if (meanCurvatures[i] < 0.0f)
        {
            color[0] = 0.0f;
            color[1] = 0.0f;
            color[2] = 0.5f * (1.0f - meanCurvatures[i] / minMeanCurvature);
        }
        else
        {
            color[0] = 0.0f;
            color[1] = 0.0f;
            color[2] = 0.0f;
        }
        color[3] = 1.0f;
    }
}

void ConformalMappingWindow3::CreateScene()
{
    // Load and preprocess the brain data set.
    std::vector<Vector3<float>> positions;
    std::vector<Vector4<float>> colors;
    std::vector<uint32_t> indices;
    LoadBrain(positions, colors, indices);

    // Create the brain mesh.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, NUM_BRAIN_VERTICES);
    auto vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < NUM_BRAIN_VERTICES; ++i)
    {
        vertices[i].position = positions[i];
        vertices[i].color = colors[i];
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, NUM_BRAIN_TRIANGLES, sizeof(uint32_t));
    std::memcpy(ibuffer->GetData(), indices.data(), ibuffer->GetNumBytes());

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mMesh->UpdateModelBound();
    mPVWMatrices.Subscribe(mMesh->worldTransform, effect->GetPVWMatrixConstant());

    // Select the first triangle as the puncture triangle and use red
    // vertex colors for it.
    size_t punctureTriangle = 100;
    Vector4<float> red{ 1.0f, 0.0f, 0.0f, 1.0f };
    vertices[indices[3 * punctureTriangle + 0]].color = red;
    vertices[indices[3 * punctureTriangle + 1]].color = red;
    vertices[indices[3 * punctureTriangle + 2]].color = red;

    // Conformally map the mesh to a sphere.
    ConformalMapGenus0<float> cm;
    cm(static_cast<int32_t>(NUM_BRAIN_VERTICES), positions.data(),
        static_cast<int32_t>(NUM_BRAIN_TRIANGLES), ibuffer->Get<int32_t>(),
        static_cast<int32_t>(punctureTriangle));
    auto const& sphereCoordinates = cm.GetSphereCoordinates();

    vbuffer = std::make_shared<VertexBuffer>(vformat, NUM_BRAIN_VERTICES);
    vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < NUM_BRAIN_VERTICES; ++i)
    {
        vertices[i].position = sphereCoordinates[i];
        vertices[i].color = colors[i];
    }
    vertices[indices[3 * punctureTriangle + 0]].color = red;
    vertices[indices[3 * punctureTriangle + 1]].color = red;
    vertices[indices[3 * punctureTriangle + 2]].color = red;

    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mSphere = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSphere->UpdateModelBound();
    mPVWMatrices.Subscribe(mSphere->worldTransform, effect->GetPVWMatrixConstant());

    // Create a subtree for the mesh.  This allows for the trackball to
    // manipulate only the mesh.
    mScene = std::make_shared<Node>();
    mMeshNode = std::make_shared<Node>();
    mMeshNode->localTransform.SetTranslation(2.0f, 0.0f, 0.0f);
    mMeshNode->localTransform.SetUniformScale(1.0f / mExtreme);
    auto meshParent = std::make_shared<Node>();
    meshParent->localTransform.SetTranslation(-mMesh->modelBound.GetCenter());

    // Create a subtree for the sphere.  This allows for the trackball to
    // manipulate only the sphere.
    mSphereNode = std::make_shared<Node>();
    mSphereNode->localTransform.SetTranslation(-2.0f, 0.0f, 0.0f);
    auto sphereParent = std::make_shared<Node>();
    sphereParent->localTransform.SetTranslation(-mSphere->modelBound.GetCenter());

    // Create the scene graph.  The trackball manipulates the entire scene
    // graph initially.
    mScene->AttachChild(mMeshNode);
    mScene->AttachChild(mSphereNode);
    mMeshNode->AttachChild(meshParent);
    meshParent->AttachChild(mMesh);
    mSphereNode->AttachChild(sphereParent);
    sphereParent->AttachChild(mSphere);

    mTrackBall.Set(mScene);
    mTrackBall.Update();
}
