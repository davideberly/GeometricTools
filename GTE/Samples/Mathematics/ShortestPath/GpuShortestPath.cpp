// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "GpuShortestPath.h"
#include <Mathematics/BitHacks.h>

GpuShortestPath::GpuShortestPath(std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Texture2> const& weights, Environment const& env,
    bool& created)
    :
    mSize(static_cast<int32_t>(weights->GetWidth()))
{
    created = false;
    mLogSize = BitHacks::Log2OfPowerOfTwo(mSize);

    mDistance = std::make_shared<Texture2>(DF_R32_FLOAT, mSize, mSize);
    mDistance->SetUsage(Resource::Usage::SHADER_OUTPUT);
    std::memset(mDistance->GetData(), 0, mDistance->GetNumBytes());

    mPrevious = std::make_shared<Texture2>(DF_R32G32_SINT, mSize, mSize);
    mPrevious->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mPrevious->SetCopy(Resource::Copy::STAGING_TO_CPU);

    mSegment = std::make_shared<ConstantBuffer>(3 * sizeof(int32_t), true);

    factory->PushDefines();
    factory->defines.Set("ISIZE", mSize);
    std::string csPath = env.GetPath(engine->GetShaderName("InitializeDiagToRow.cs"));
    mInitializeDiagToRow = factory->CreateFromFile(csPath);
    if (!mInitializeDiagToRow)
    {
        return;
    }
    std::shared_ptr<Shader> cshader = mInitializeDiagToRow->GetComputeShader();
    cshader->Set("weights", weights);
    cshader->Set("previous", mPrevious);
    cshader->Set("sum", mDistance);

    csPath = env.GetPath(engine->GetShaderName("InitializeDiagToCol.cs"));
    mInitializeDiagToCol = factory->CreateFromFile(csPath);
    if (!mInitializeDiagToCol)
    {
        return;
    }
    cshader = mInitializeDiagToCol->GetComputeShader();
    cshader->Set("weights", weights);
    cshader->Set("previous", mPrevious);
    cshader->Set("sum", mDistance);

    mPartialSumDiagToRow.resize(mLogSize);
    mPartialSumDiagToCol.resize(mLogSize);
    for (int32_t i = 0, p = 1; i < mLogSize; ++i, ++p)
    {
        factory->defines.Set("NUM_X_THREADS", (1 << (mLogSize - p)));
        factory->defines.Set("NUM_Y_THREADS", (1 << i));
        factory->defines.Set("TWO_P", (1 << p));
        factory->defines.Set("TWO_PM1", (1 << i));
        csPath = env.GetPath(engine->GetShaderName("PartialSumsDiagToRow.cs"));
        mPartialSumDiagToRow[i] = factory->CreateFromFile(csPath);
        if (!mPartialSumDiagToRow[i])
        {
            return;
        }
        mPartialSumDiagToRow[i]->GetComputeShader()->Set("sum", mDistance);

        csPath = env.GetPath(engine->GetShaderName("PartialSumsDiagToCol.cs"));
        mPartialSumDiagToCol[i] = factory->CreateFromFile(csPath);
        if (!mPartialSumDiagToCol[i])
        {
            return;
        }
        mPartialSumDiagToCol[i]->GetComputeShader()->Set("sum", mDistance);
    }

    csPath = env.GetPath(engine->GetShaderName("UpdateShader.cs"));
    mUpdate = factory->CreateFromFile(csPath);
    if (!mUpdate)
    {
        return;
    }
    cshader = mUpdate->GetComputeShader();
    cshader->Set("Segment", mSegment);
    cshader->Set("weights", weights);
    cshader->Set("distance", mDistance);
    cshader->Set("previous", mPrevious);

    factory->PopDefines();
    created = true;
}

void GpuShortestPath::Compute(std::shared_ptr<GraphicsEngine> const& engine,
    std::stack<std::pair<int32_t, int32_t>>& path)
{
    // Execute the shaders.
    engine->Execute(mInitializeDiagToRow, 1, 1, 1);
    for (int32_t i = 0; i < mLogSize; ++i)
    {
        engine->Execute(mPartialSumDiagToRow[i], 1, 1, 1);
    }

    engine->Execute(mInitializeDiagToCol, 1, 1, 1);
    for (int32_t i = 0; i < mLogSize; ++i)
    {
        engine->Execute(mPartialSumDiagToCol[i], 1, 1, 1);
    }

    auto segment = mSegment->Get<int32_t>();
    for (int32_t z = 2, numPixels = z - 1; z < mSize; ++z, ++numPixels)
    {
        segment[0] = 1;
        segment[1] = z - 1;
        segment[2] = numPixels;
        engine->Update(mSegment);
        engine->Execute(mUpdate, 1, 1, 1);
    }

    int32_t const zmax = 2 * (mSize - 1);
    for (int32_t z = mSize, numPixels = zmax - z + 1; z <= zmax; ++z, --numPixels)
    {
        segment[0] = z - (mSize - 1);
        segment[1] = mSize - 1;
        segment[2] = numPixels;
        engine->Update(mSegment);
        engine->Execute(mUpdate, 1, 1, 1);
    }

    // Read back the path from GPU memory.
    engine->CopyGpuToCpu(mPrevious);
    auto location = mPrevious->Get<std::array<int32_t, 2>>();

    // Create the path by starting at (mXSize-1,mYSize-1) and following the
    // previous links.
    int32_t x = mSize - 1, y = mSize - 1;
    while (x != -1 && y != -1)
    {
        path.push(std::make_pair(x, y));
        std::array<int32_t, 2> prev = location[x + mSize * y];
        x = prev[0];
        y = prev[1];
    }
}
