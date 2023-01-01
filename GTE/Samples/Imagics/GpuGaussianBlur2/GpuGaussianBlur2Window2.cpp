// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "GpuGaussianBlur2Window2.h"

GpuGaussianBlur2Window2::GpuGaussianBlur2Window2(Parameters& parameters)
    :
    Window2(parameters),
    mNumXThreads(8),
    mNumYThreads(8),
    mNumXGroups(mXSize / mNumXThreads),
    mNumYGroups(mYSize / mNumYThreads),
    mUseDirichlet(parameters.useDirichlet)
{
    if (!SetEnvironment() || !CreateImages() || !CreateShaders())
    {
        parameters.created = false;
        return;
    }

    // Create an overlay that covers the entire window.  The blurred image
    // is drawn by the overlay effect.
    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::Filter::MIN_P_MAG_P_MIP_P,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, false);
    mOverlay->SetTexture(mImage[0]);
}

void GpuGaussianBlur2Window2::OnIdle()
{
    mTimer.Measure();

    mEngine->Execute(mGaussianBlurProgram, mNumXGroups, mNumYGroups, 1);
    if (mUseDirichlet)
    {
        mEngine->Execute(mBoundaryDirichletProgram, mNumXGroups, mNumYGroups, 1);
    }
    else
    {
        mEngine->Execute(mBoundaryNeumannProgram, mNumXGroups, mNumYGroups, 1);
    }

    mEngine->Draw(mOverlay);

    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool GpuGaussianBlur2Window2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Imagics/GpuGaussianBlur2/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Head_U16_X256_Y256.binary",
        mEngine->GetShaderName("BoundaryDirichlet.cs"),
        mEngine->GetShaderName("BoundaryNeumann.cs"),
        mEngine->GetShaderName("GaussianBlur.cs")
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

bool GpuGaussianBlur2Window2::CreateImages()
{
    for (int32_t i = 0; i < 2; ++i)
    {
        mImage[i] = std::make_shared<Texture2>(DF_R32_FLOAT, mXSize, mYSize);
        mImage[i]->SetUsage(Resource::Usage::SHADER_OUTPUT);
    }

    std::string path = mEnvironment.GetPath("Head_U16_X256_Y256.binary");
    std::vector<uint16_t> original(static_cast<size_t>(mXSize) * static_cast<size_t>(mYSize));
    std::ifstream input(path, std::ios::binary);
    input.read((char*)original.data(), original.size() * sizeof(uint16_t));
    input.close();

    // The head image is known to store 10 bits per pixel.  Scale the
    // texture image to have values in [0,1).
    float const divisor = 1024.0f;
    auto* target = mImage[0]->Get<float>();
    for (int32_t i = 0; i < mXSize * mYSize; ++i)
    {
        target[i] = static_cast<float>(original[i]) / divisor;
    }

    // Create the mask texture for BoundaryDirichlet and the offset
    // texture for BoundaryNeumann.
    mMaskTexture = std::make_shared<Texture2>(DF_R32_FLOAT, mXSize, mYSize);
    auto* mask = mMaskTexture->Get<float>();
    mOffsetTexture = std::make_shared<Texture2>(DF_R32G32_SINT, mXSize, mYSize);
    auto* offset = mOffsetTexture->Get<std::array<int32_t, 2>>();
    int32_t xSizeM1 = mXSize - 1, ySizeM1 = mYSize - 1, index;

    // Interior.
    for (int32_t y = 1; y < ySizeM1; ++y)
    {
        for (int32_t x = 1; x < xSizeM1; ++x)
        {
            index = x + mXSize * y;
            mask[index] = 1.0f;
            offset[index] = { 0, 0 };
        }
    }

    // Edge-interior.
    for (int32_t x = 1; x < xSizeM1; ++x)
    {
        mask[x] = 0.0f;
        offset[x] = { 0, 1 };
        index = x + mXSize * ySizeM1;
        mask[index] = 0.0f;
        offset[index] = { 0, -1 };
    }
    for (int32_t y = 1; y < ySizeM1; ++y)
    {
        index = mXSize * y;
        mask[index] = 0.0f;
        offset[index] = { 1, 0 };
        index += xSizeM1;
        mask[index] = 0.0f;
        offset[index] = { -1, 0 };
    }

    // Corners.
    mask[0] = 0.0f;
    offset[0] = { 1, 1 };
    mask[xSizeM1] = 0.0f;
    offset[xSizeM1] = { -1, 1 };
    index = mXSize * ySizeM1;
    mask[index] = 0.0f;
    offset[index] = { 1, -1 };
    index += xSizeM1;
    mask[index] = 0.0f;
    offset[index] = { -1, -1 };

    mWeightBuffer = std::make_shared<ConstantBuffer>(sizeof(Vector4<float>), false);
    auto& weight = *mWeightBuffer->Get<Vector4<float>>();
    weight[0] = 0.01f;  // = kappa*DeltaT/DeltaX^2
    weight[1] = 0.01f;  // = kappa*DeltaT/DeltaY^2
    weight[2] = 1.0f - 2.0f * weight[0] - 2.0f * weight[1];  // positive value
    weight[3] = 0.0f;   // unused
    return true;
}

bool GpuGaussianBlur2Window2::CreateShaders()
{
    mProgramFactory->defines.Set("NUM_X_THREADS", mNumXThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", mNumYThreads);

    std::string csPath = mEnvironment.GetPath(mEngine->GetShaderName("GaussianBlur.cs"));
    mGaussianBlurProgram = mProgramFactory->CreateFromFile(csPath);
    if (!mGaussianBlurProgram)
    {
        return false;
    }

    csPath = mEnvironment.GetPath(mEngine->GetShaderName("BoundaryDirichlet.cs"));
    mBoundaryDirichletProgram = mProgramFactory->CreateFromFile(csPath);
    if (!mBoundaryDirichletProgram)
    {
        return false;
    }

    csPath = mEnvironment.GetPath(mEngine->GetShaderName("BoundaryNeumann.cs"));
    mBoundaryNeumannProgram = mProgramFactory->CreateFromFile(csPath);
    if (!mBoundaryNeumannProgram)
    {
        return false;
    }

    std::shared_ptr<Shader> cshader = mGaussianBlurProgram->GetComputeShader();
    cshader->Set("inImage", mImage[0]);
    cshader->Set("outImage", mImage[1]);
    cshader->Set("Weight", mWeightBuffer);

    cshader = mBoundaryDirichletProgram->GetComputeShader();
    cshader->Set("inImage", mImage[1]);
    cshader->Set("outImage", mImage[0]);
    cshader->Set("inMask", mMaskTexture);

    cshader = mBoundaryNeumannProgram->GetComputeShader();
    cshader->Set("inImage", mImage[1]);
    cshader->Set("outImage", mImage[0]);
    cshader->Set("inOffset", mOffsetTexture);

    return true;
}
