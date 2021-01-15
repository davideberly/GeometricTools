// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "GpuMassSpringVolume.h"

GpuMassSpringVolume::GpuMassSpringVolume(std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<ProgramFactory> const& factory, int numColumns, int numRows,
    int numSlices, float step, float viscosity, Environment& environment, bool& created)
    :
    mNumColumns(numColumns),
    mNumRows(numRows),
    mNumSlices(numSlices)
{
    created = false;

    // Create the shaders.
    int const numThreads = 4;
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numThreads);
    factory->defines.Set("NUM_Y_THREADS", numThreads);
    factory->defines.Set("NUM_Z_THREADS", numThreads);

    for (int i = 0; i < 8; ++i)
    {
        std::string name = "RungeKutta" +
            std::to_string(1 + i / 2) + ((i & 1) == 0 ? "a" : "b") +
            ".cs";
        std::string csPath = environment.GetPath(engine->GetShaderName(name));
        mRK4Shader[i] = factory->CreateFromFile(csPath);
        if (!mRK4Shader[i])
        {
            return;
        }
    }

    // The cbuffer is tightly packed.  Only time, halfTime, and fullTime vary.
    mParameters = std::make_shared<ConstantBuffer>(sizeof(SimulationParameters), true);
    SimulationParameters& p = *mParameters->Get<SimulationParameters>();
    p.dimensions[0] = numColumns;
    p.dimensions[1] = numRows;
    p.dimensions[2] = numSlices;
    p.dimensions[3] = numColumns * numRows;
    p.viscosity = viscosity;
    p.time = 0.0f;
    p.delta = step;
    p.halfDelta = p.delta / 2.0f;
    p.sixthDelta = p.delta / 6.0f;
    p.halfTime = p.time + p.halfDelta;
    p.fullTime = p.time + p.delta;

    unsigned int const numParticles = p.dimensions[2] * p.dimensions[3];
    size_t const vecsize = sizeof(Vector4<float>);
    mMass = std::make_shared<StructuredBuffer>(numParticles, sizeof(float));
    mInvMass = std::make_shared<StructuredBuffer>(numParticles, sizeof(float));
    mPosition = std::make_shared<StructuredBuffer>(numParticles, vecsize);
    mPosition->SetUsage(Resource::SHADER_OUTPUT);
    mPosition->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    mVelocity = std::make_shared<StructuredBuffer>(numParticles, vecsize);
    mVelocity->SetUsage(Resource::SHADER_OUTPUT);
    mConstantC = std::make_shared<StructuredBuffer>(numParticles, sizeof(float));
    mLengthC = std::make_shared<StructuredBuffer>(numParticles, sizeof(float));
    mConstantR = std::make_shared<StructuredBuffer>(numParticles, sizeof(float));
    mLengthR = std::make_shared<StructuredBuffer>(numParticles, sizeof(float));
    mConstantS = std::make_shared<StructuredBuffer>(numParticles, sizeof(float));
    mLengthS = std::make_shared<StructuredBuffer>(numParticles, sizeof(float));

    mPTmp = std::make_shared<StructuredBuffer>(numParticles, vecsize, true);
    mPTmp->SetUsage(Resource::SHADER_OUTPUT);
    mPTmp->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    mPAllTmp = std::make_shared<StructuredBuffer>(numParticles, 4 * vecsize, true);
    mPAllTmp->SetUsage(Resource::SHADER_OUTPUT);
    mPAllTmp->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    mVTmp = std::make_shared<StructuredBuffer>(numParticles, vecsize, true);
    mVTmp->SetUsage(Resource::SHADER_OUTPUT);
    mVTmp->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    mVAllTmp = std::make_shared<StructuredBuffer>(numParticles, 4 * vecsize, true);
    mVAllTmp->SetUsage(Resource::SHADER_OUTPUT);
    mVAllTmp->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    mNumXGroups = p.dimensions[0] / numThreads;
    mNumYGroups = p.dimensions[1] / numThreads;
    mNumZGroups = p.dimensions[2] / numThreads;

    // Step 1a
    auto cshader = mRK4Shader[0]->GetComputeShader();
    cshader->Set("SimulationParameters", mParameters);
    cshader->Set("invMass", mInvMass);
    cshader->Set("constantC", mConstantC);
    cshader->Set("lengthC", mLengthC);
    cshader->Set("constantR", mConstantR);
    cshader->Set("lengthR", mLengthR);
    cshader->Set("constantS", mConstantS);
    cshader->Set("lengthS", mLengthS);
    cshader->Set("pAllTmp", mPAllTmp);
    cshader->Set("vAllTmp", mVAllTmp);
    cshader->Set("position", mPosition);
    cshader->Set("velocity", mVelocity);

    // Step 1b
    cshader = mRK4Shader[1]->GetComputeShader();
    cshader->Set("SimulationParameters", mParameters);
    cshader->Set("invMass", mInvMass);
    cshader->Set("pTmp", mPTmp);
    cshader->Set("vTmp", mVTmp);
    cshader->Set("pAllTmp", mPAllTmp);
    cshader->Set("vAllTmp", mVAllTmp);
    cshader->Set("position", mPosition);
    cshader->Set("velocity", mVelocity);

    // Step 2a
    cshader = mRK4Shader[2]->GetComputeShader();
    cshader->Set("SimulationParameters", mParameters);
    cshader->Set("invMass", mInvMass);
    cshader->Set("constantC", mConstantC);
    cshader->Set("lengthC", mLengthC);
    cshader->Set("constantR", mConstantR);
    cshader->Set("lengthR", mLengthR);
    cshader->Set("constantS", mConstantS);
    cshader->Set("lengthS", mLengthS);
    cshader->Set("pTmp", mPTmp);
    cshader->Set("vTmp", mVTmp);
    cshader->Set("pAllTmp", mPAllTmp);
    cshader->Set("vAllTmp", mVAllTmp);
    cshader->Set("velocity", mVelocity);

    // Step 2b
    cshader = mRK4Shader[3]->GetComputeShader();
    cshader->Set("SimulationParameters", mParameters);
    cshader->Set("invMass", mInvMass);
    cshader->Set("pTmp", mPTmp);
    cshader->Set("vTmp", mVTmp);
    cshader->Set("pAllTmp", mPAllTmp);
    cshader->Set("vAllTmp", mVAllTmp);
    cshader->Set("position", mPosition);
    cshader->Set("velocity", mVelocity);

    // Step 3a
    cshader = mRK4Shader[4]->GetComputeShader();
    cshader->Set("SimulationParameters", mParameters);
    cshader->Set("invMass", mInvMass);
    cshader->Set("constantC", mConstantC);
    cshader->Set("lengthC", mLengthC);
    cshader->Set("constantR", mConstantR);
    cshader->Set("lengthR", mLengthR);
    cshader->Set("constantS", mConstantS);
    cshader->Set("lengthS", mLengthS);
    cshader->Set("pTmp", mPTmp);
    cshader->Set("vTmp", mVTmp);
    cshader->Set("pAllTmp", mPAllTmp);
    cshader->Set("vAllTmp", mVAllTmp);
    cshader->Set("velocity", mVelocity);

    // Step 3b
    cshader = mRK4Shader[5]->GetComputeShader();
    cshader->Set("SimulationParameters", mParameters);
    cshader->Set("invMass", mInvMass);
    cshader->Set("pTmp", mPTmp);
    cshader->Set("vTmp", mVTmp);
    cshader->Set("pAllTmp", mPAllTmp);
    cshader->Set("vAllTmp", mVAllTmp);
    cshader->Set("position", mPosition);
    cshader->Set("velocity", mVelocity);

    // Step 4a
    cshader = mRK4Shader[6]->GetComputeShader();
    cshader->Set("SimulationParameters", mParameters);
    cshader->Set("invMass", mInvMass);
    cshader->Set("constantC", mConstantC);
    cshader->Set("lengthC", mLengthC);
    cshader->Set("constantR", mConstantR);
    cshader->Set("lengthR", mLengthR);
    cshader->Set("constantS", mConstantS);
    cshader->Set("lengthS", mLengthS);
    cshader->Set("pTmp", mPTmp);
    cshader->Set("vTmp", mVTmp);
    cshader->Set("pAllTmp", mPAllTmp);
    cshader->Set("vAllTmp", mVAllTmp);
    cshader->Set("velocity", mVelocity);

    // Step 4b
    cshader = mRK4Shader[7]->GetComputeShader();
    cshader->Set("SimulationParameters", mParameters);
    cshader->Set("invMass", mInvMass);
    cshader->Set("position", mPosition);
    cshader->Set("velocity", mVelocity);
    cshader->Set("pAllTmp", mPAllTmp);
    cshader->Set("vAllTmp", mVAllTmp);

    factory->PopDefines();
    created = true;
}

void GpuMassSpringVolume::SetMass(int c, int r, int s, float mass)
{
    if (0.0f < mass && mass < std::numeric_limits<float>::max())
    {
        mMass->Get<float>()[GetIndex(c, r, s)] = mass;
        mInvMass->Get<float>()[GetIndex(c, r, s)] = 1.0f / mass;
    }
    else
    {
        mMass->Get<float>()[GetIndex(c, r, s)] = std::numeric_limits<float>::max();
        mInvMass->Get<float>()[GetIndex(c, r, s)] = 0.0f;
    }
}

void GpuMassSpringVolume::SetPosition(int c, int r, int s,
    Vector3<float> const& position)
{
    mPosition->Get<Vector4<float>>()[GetIndex(c, r, s)] = HLift(position, 1.0f);
}

void GpuMassSpringVolume::SetVelocity(int c, int r, int s,
    Vector3<float> const& velocity)
{
    mVelocity->Get<Vector4<float>>()[GetIndex(c, r, s)] = HLift(velocity, 0.0f);
}

void GpuMassSpringVolume::SetConstantC(int c, int r, int s, float v)
{
    mConstantC->Get<float>()[GetIndex(c, r, s)] = v;
}

void GpuMassSpringVolume::SetLengthC(int c, int r, int s, float v)
{
    mLengthC->Get<float>()[GetIndex(c, r, s)] = v;
}

void GpuMassSpringVolume::SetConstantR(int c, int r, int s, float v)
{
    mConstantR->Get<float>()[GetIndex(c, r, s)] = v;
}

void GpuMassSpringVolume::SetLengthR(int c, int r, int s, float v)
{
    mLengthR->Get<float>()[GetIndex(c, r, s)] = v;
}

void GpuMassSpringVolume::SetConstantS(int c, int r, int s, float v)
{
    mConstantS->Get<float>()[GetIndex(c, r, s)] = v;
}

void GpuMassSpringVolume::SetLengthS(int c, int r, int s, float v)
{
    mLengthS->Get<float>()[GetIndex(c, r, s)] = v;
}

Vector3<float> GpuMassSpringVolume::GetPosition(int c, int r, int s) const
{
    return HProject(mPosition->Get<Vector4<float>>()[GetIndex(c, r, s)]);
}

std::shared_ptr<StructuredBuffer>& GpuMassSpringVolume::GetPosition()
{
    return mPosition;
}

void GpuMassSpringVolume::Update(float time, std::shared_ptr<GraphicsEngine> const& engine)
{
    SimulationParameters& p = *mParameters->Get<SimulationParameters>();
    p.time = time;
    p.halfTime = p.time + p.halfDelta;
    p.fullTime = p.time + p.delta;
    engine->Update(mParameters);

    for (int i = 0; i < 8; ++i)
    {
        engine->Execute(mRK4Shader[i], mNumXGroups, mNumYGroups, mNumZGroups);
    }
}
