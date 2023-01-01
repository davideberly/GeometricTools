// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ShortestPathWindow2.h"
#include <Mathematics/ImageUtility2.h>
#include <random>

ShortestPathWindow2::ShortestPathWindow2(Parameters& parameters)
    :
    Window2(parameters)
{
    if (!SetEnvironment() || !CreateWeightsShader())
    {
        parameters.created = false;
        return;
    }

#if defined(USE_CPU_SHORTEST_PATH)
    mCpuShortestPath = std::make_shared<CpuShortestPath>(mWeights);
#else
    bool created = false;
    mGpuShortestPath = std::make_shared<GpuShortestPath>(mEngine,
        mProgramFactory, mWeights, mEnvironment, created);
    if (!created)
    {
        // LogError calls were made in the GpuShortestPath constructor.
        parameters.created = false;
        return;
    }
#endif

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, ISIZE,
        ISIZE, ISIZE, ISIZE, SamplerState::Filter::MIN_P_MAG_P_MIP_P,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, true);
    mOverlay->SetTexture(mWeights);
}

void ShortestPathWindow2::OnDisplay()
{
    GenerateWeights();

    std::stack<std::pair<int32_t, int32_t>> path;
#if defined(USE_CPU_SHORTEST_PATH)
    mCpuShortestPath->Compute(path);
#else
    mGpuShortestPath->Compute(mEngine, path);
#endif
    DrawPath(path);

    mEngine->Draw(mOverlay);
    mEngine->DisplayColorBuffer(0);
}

bool ShortestPathWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Mathematics/ShortestPath/Shaders/");

    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("InitializeDiagToCol.cs"),
        mEngine->GetShaderName("InitializeDiagToRow.cs"),
        mEngine->GetShaderName("PartialSumsDiagToCol.cs"),
        mEngine->GetShaderName("PartialSumsDiagToRow.cs"),
        mEngine->GetShaderName("UpdateShader.cs"),
        mEngine->GetShaderName("WeightsShader.cs")
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

bool ShortestPathWindow2::CreateWeightsShader()
{
    // Perturb the smooth bicubic surface to avoid having a shortest path of
    // a small number of line segments.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.1f, 0.5f);
    mRandom = std::make_shared<Texture2>(DF_R32_FLOAT, ISIZE, ISIZE);
    float* random = mRandom->Get<float>();
    for (int32_t i = 0; i < ISIZE*ISIZE; ++i)
    {
        random[i] += rnd(mte);
    }

    mWeights = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, ISIZE, ISIZE);
    mWeights->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mWeights->SetCopy(Resource::Copy::BIDIRECTIONAL);

    std::string csPath = mEnvironment.GetPath(mEngine->GetShaderName("WeightsShader.cs"));
    int32_t const numThreads = 8;
    mNumGroups = ISIZE / numThreads;
    mProgramFactory->defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", numThreads);
    mWeightsProgram = mProgramFactory->CreateFromFile(csPath);
    mProgramFactory->defines.Clear();
    if (!mWeightsProgram)
    {
        return false;
    }
    auto const& cshader = mWeightsProgram->GetComputeShader();
    cshader->Set("ControlPoints", CreateBicubicMatrix());
    cshader->Set("random", mRandom);
    cshader->Set("weights", mWeights);
    return true;
}

void ShortestPathWindow2::GenerateWeights()
{
    // Generate the height field as gray scale.  The shortest path computed
    // on the GPU will be overlaid in color.
    mEngine->Execute(mWeightsProgram, mNumGroups, mNumGroups, 1);

    // Get a CPU copy of the weights to compute the shortest path using
    // CPU code.
    mEngine->CopyGpuToCpu(mWeights);
}

std::shared_ptr<ConstantBuffer> ShortestPathWindow2::CreateBicubicMatrix()
{
    // Generate random samples for the bicubic Bezier surface.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.05f, 1.0f);
    std::array<std::array<float, 4>, 4> P{};
    //float P[4][4];
    for (int32_t r = 0; r < 4; ++r)
    {
        for (int32_t c = 0; c < 4; ++c)
        {
            P[r][c] = rnd(mte);
        }
    }

    // Construct the control points from the samples.
    std::array<std::array<float, 4>, 4> control{};
    //float control[4][4];
    control[0][0] = P[0][0];
    control[0][1] = (
        -5.0f * P[0][0] + 18.0f * P[0][1] - 9.0f * P[0][2] + 2.0f * P[0][3]
        ) / 6.0f;
    control[0][2] = (
        +2.0f * P[0][0] - 9.0f * P[0][1] + 18.0f * P[0][2] - 5.0f * P[0][3]
        ) / 6.0f;
    control[0][3] = P[0][3];
    control[1][0] = (
        -5.0f * P[0][0] + 18.0f * P[1][0] - 9.0f * P[2][0] - 5.0f * P[3][0]
        ) / 6.0f;
    control[1][1] = (
        +25.0f * P[0][0] - 90.0f * P[0][1] + 45.0f * P[0][2] - 10.0f * P[0][3]
        - 90.0f * P[1][0] + 324.0f * P[1][1] - 162.0f * P[1][2] + 36.0f * P[1][3]
        + 45.0f * P[2][0] - 162.0f * P[2][1] + 81.0f * P[2][2] - 18.0f * P[2][3]
        - 10.0f * P[3][0] + 36.0f * P[3][1] - 18.0f * P[3][2] + 4.0f * P[3][3]
        ) / 36.0f;
    control[1][2] = (
        -10.0f * P[0][0] + 45.0f * P[0][1] - 90.0f * P[0][2] + 25.0f * P[0][3]
        + 36.0f * P[1][0] - 162.0f * P[1][1] + 324.0f * P[1][2] - 90.0f * P[1][3]
        - 18.0f * P[2][0] + 81.0f * P[2][1] - 162.0f * P[2][2] + 45.0f * P[2][3]
        + 4.0f * P[3][0] - 18.0f * P[3][1] + 36.0f * P[3][2] - 10.0f * P[3][3]
        ) / 36.0f;
    control[1][3] = (
        -5.0f * P[0][3] + 18.0f * P[1][3] - 9.0f * P[2][3] + 2.0f * P[3][3]
        ) / 6.0f;
    control[2][0] = (
        +2.0f * P[0][0] - 9.0f * P[1][0] + 18.0f * P[2][0] - 5.0f * P[3][0]
        ) / 6.0f;
    control[2][1] = (
        -10.0f * P[0][0] + 36.0f * P[0][1] - 18.0f * P[0][2] + 4.0f * P[0][3]
        + 45.0f * P[1][0] - 162.0f * P[1][1] + 81.0f * P[1][2] - 18.0f * P[1][3]
        - 90.0f * P[2][0] + 324.0f * P[2][1] - 162.0f * P[2][2] + 36.0f * P[2][3]
        + 25.0f * P[3][0] - 90.0f * P[3][1] + 45.0f * P[3][2] - 10.0f * P[3][3]
        ) / 36.0f;
    control[2][2] = (
        +4.0f * P[0][0] - 18.0f * P[0][1] + 36.0f * P[0][2] - 10.0f * P[0][3]
        - 18.0f * P[1][0] + 81.0f * P[1][1] - 162.0f * P[1][2] + 45.0f * P[1][3]
        + 36.0f * P[2][0] - 162.0f * P[2][1] + 324.0f * P[2][2] - 90.0f * P[2][3]
        - 10.0f * P[3][0] + 45.0f * P[3][1] - 90.0f * P[3][2] + 25.0f * P[3][3]
        ) / 36.0f;
    control[2][3] = (
        +2.0f * P[0][3] - 9.0f * P[1][3] + 18.0f * P[2][3] - 5.0f * P[3][3]
        ) / 6.0f;
    control[3][0] = P[3][0];
    control[3][1] = (
        -5.0f * P[3][0] + 18.0f * P[3][1] - 9.0f * P[3][2] + 2.0f * P[3][3]
        ) / 6.0f;
    control[3][2] = (
        +2.0f * P[3][0] - 9.0f * P[3][1] + 18.0f * P[3][2] - 5.0f * P[3][3]
        ) / 6.0f;
    control[3][3] = P[3][3];

    auto controlBuffer = std::make_shared<ConstantBuffer>(4 * sizeof(Vector4<float>), false);
    auto data = controlBuffer->Get<Vector4<float>>();
    for (int32_t r = 0; r < 4; ++r)
    {
        auto& trg = data[r];
        for (int32_t c = 0; c < 4; ++c)
        {
            trg[c] = control[r][c];
        }
    }

    return controlBuffer;
}

void ShortestPathWindow2::DrawPath(std::stack<std::pair<int32_t, int32_t>>& path)
{
    auto texels = mWeights->Get<Vector4<float>>();
    std::pair<int32_t, int32_t> loc0 = path.top();
    path.pop();
    while (path.size() > 0)
    {
        std::pair<int32_t, int32_t> loc1 = path.top();
        path.pop();

        ImageUtility2::DrawLine(
            loc0.first, loc0.second, loc1.first, loc1.second,
            [texels](int32_t x, int32_t y)
        {
            texels[x + ISIZE * y] = { 1.0f, 0.0f, 0.0f, 1.0f };
        }
        );

        loc0 = loc1;
    }
    mEngine->CopyCpuToGpu(mWeights);
}
