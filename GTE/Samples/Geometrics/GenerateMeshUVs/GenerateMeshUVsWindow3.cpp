// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "GenerateMeshUVsWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/PlanarMesh.h>
#include <iostream>
#include <random>

#if defined(GENERATE_MESH_UVS_GPU)
#include <Mathematics/GPU/GPUGenerateMeshUV.h>
#else
#include <Mathematics/GenerateMeshUV.h>
#endif

GenerateMeshUVsWindow3::GenerateMeshUVsWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mDrawMeshOriginal(true)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mEngine->SetRasterizerState(mNoCullState);

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 1000.0f, 0.001f, 0.001f,
        { -3.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void GenerateMeshUVsWindow3::OnIdle()
{
    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    if (mDrawMeshOriginal)
    {
        mEngine->Draw(mMeshOriginal);
    }
    else
    {
        mEngine->Draw(mMeshResampled);
    }
    mEngine->DisplayColorBuffer(0);
}

bool GenerateMeshUVsWindow3::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullState == mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;
    case 'm':
    case 'M':
        mDrawMeshOriginal = !mDrawMeshOriginal;
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool GenerateMeshUVsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("MedicineBag.png") == "")
    {
        LogError("Cannot find file MedicineBag.png.");
        return false;
    }

    return true;
}

void GenerateMeshUVsWindow3::CreateScene()
{
    CreateMeshOriginal();
    CreateMeshResampled();
    mTrackBall.Update();
}

void GenerateMeshUVsWindow3::CreateMeshOriginal()
{
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);

    // Generate a perturbed hemisphere.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mMeshOriginal = mf.CreateDisk(16, 16, 1.0f);
    float height = 0.25f;
    float radius = std::sqrt(1.0f - height*height);
    auto vbuffer = mMeshOriginal->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vertex* vertices = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        // Start with a hemisphere.
        float x = radius * vertices[i].position[0];
        float y = radius * vertices[i].position[1];
        float z = std::sqrt(std::max(1.0f - x*x - y*y, 0.0f));

        // Perturb points along rays, which preserves non-self-intersection.
        float r = 1.0f + 0.125f*rnd(mte);
        vertices[i].position = { r * x, r * y, r * z };
    }

    std::string path = mEnvironment.GetPath("MedicineBag.png");
    auto texture = WICFileIO::Load(path, false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP);
    mMeshOriginal->SetEffect(effect);

    mPVWMatrices.Subscribe(mMeshOriginal->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mMeshOriginal);
}

void GenerateMeshUVsWindow3::CreateMeshResampled()
{
    auto vbuffer = mMeshOriginal->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vertex* vertices = vbuffer->Get<Vertex>();

    auto ibuffer = mMeshOriginal->GetIndexBuffer();
    int numIndices = (int)ibuffer->GetNumElements();
    int const* indices = ibuffer->Get<int>();
    std::vector<Vector3<double>> dvertices(numVertices);
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            dvertices[i][j] = vertices[i].position[j];
        }
    }

    // Generate texture coordinates.
#if defined(GENERATE_MESH_UVS_CPU_SINGLE_THREADED)
    // Use the main application thread.
    uint32_t const numThreads = 0;
    GenerateMeshUV<double> pm(numThreads);
#endif
#if defined(GENERATE_MESH_UVS_CPU_MULTITHREADED)
    // Use half the number of hardware threads on the CPU.
    uint32_t const numThreads = std::thread::hardware_concurrency() / 2;
    GenerateMeshUV<double> pm(numThreads);
#endif
#if defined(GENERATE_MESH_UVS_GPU)
    // Use the GPU, whether DX11/HLSL or GL45/GLSL.
    GPUGenerateMeshUV<double> pm(mEngine, mProgramFactory);
#endif
    std::vector<Vector2<double>> tcoords(numVertices);
    unsigned int numGaussSeidelIterations = 128;
    pm(numGaussSeidelIterations, true, numVertices, &dvertices[0], numIndices,
        indices, &tcoords[0]);

    // Resample the mesh.
    typedef BSNumber<UIntegerAP32> Numeric;
    typedef BSRational<UIntegerAP32> Rational;
    int numTriangles = numIndices / 3;
    PlanarMesh<double, Numeric, Rational> pmesh(numVertices, &tcoords[0],
        numTriangles, &indices[0]);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int size = 64;
    double dsize = (double)size;
    mMeshResampled = mf.CreateRectangle(size, size, 1.0f, 1.0f);
    vertices = mMeshResampled->GetVertexBuffer()->Get<Vertex>();

    Vector2<double> P;
    Vector3<double> resampled;
    int triangle = 0;
    std::array<double, 3> bary;
    std::array<int, 3> lookup;
    for (int y = 0; y < size; ++y)
    {
        P[1] = y / dsize;
        for (int x = 0; x < size; ++x)
        {
            P[0] = x / dsize;
            triangle = pmesh.GetContainingTriangle(P, triangle);
            if (triangle >= 0)
            {
                pmesh.GetBarycentrics(triangle, P, bary);
                pmesh.GetIndices(triangle, lookup);
                resampled =
                    bary[0] * dvertices[lookup[0]] +
                    bary[1] * dvertices[lookup[1]] +
                    bary[2] * dvertices[lookup[2]];
                for (int i = 0; i < 3; ++i)
                {
                    vertices[x + size*y].position[i] = (float)resampled[i];
                }
            }
            else
            {
                std::cout << "failed at (" << x << "," << y << ")"
                    << std::endl;
                triangle = 0;
                for (int i = 0; i < 3; ++i)
                {
                    vertices[x + size*y].position[i] = 0.0f;
                }
            }
        }
    }

    std::string path = mEnvironment.GetPath("MedicineBag.png");
    auto texture = WICFileIO::Load(path, false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP,
        SamplerState::CLAMP);
    mMeshResampled->SetEffect(effect);

    mPVWMatrices.Subscribe(mMeshResampled->worldTransform,  effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mMeshResampled);
}
