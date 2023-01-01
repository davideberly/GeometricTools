// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ConvolutionWindow2.h"
#include <Applications/WICFileIO.h>

ConvolutionWindow2::ConvolutionWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mNumXGroups(0),
    mNumYGroups(0),
    mRadius(1),
    mShadersCreated(false),
    mSelection(0)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Load the color image to be convolved.
    std::string path = mEnvironment.GetPath("MedicineBag.png");
    auto original = WICFileIO::Load(path, false);
    uint32_t const txWidth = original->GetWidth();
    uint32_t const txHeight = original->GetHeight();

    // Create images for shader inputs and outputs.
    for (int32_t i = 0; i < 3; ++i)
    {
        mImage[i] = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, txWidth, txHeight);
        mImage[i]->SetUsage(Resource::Usage::SHADER_OUTPUT);
    }

    // Map the 8-bit RGBA image to 32-bit RGBA for the numerical convolution.
    uint32_t const* src = original->Get<uint32_t>();
    float* trg = mImage[0]->Get<float>();
    for (uint32_t j = 0; j < txWidth*txHeight; ++j)
    {
        uint32_t rgba = *src++;
        *trg++ = (rgba & 0x000000FF) / 255.0f;
        *trg++ = ((rgba & 0x0000FF00) >> 8) / 255.0f;
        *trg++ = ((rgba & 0x00FF0000) >> 16) / 255.0f;
        *trg++ = 1.0f;
    }

    // Create two overlays, one for the original image and one for the
    // convolved image.
    std::array<int32_t, 4> rect[2] =
    {
        { 0, 0, mXSize / 2, mYSize },
        { mXSize / 2, 0, mXSize / 2, mYSize }
    };
    for (int32_t i = 0; i < 2; ++i)
    {
        mOverlay[i] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize, mYSize, txWidth, txHeight,
            SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, true);
        mOverlay[i]->SetOverlayRectangle(rect[i]);
        mOverlay[i]->SetTexture(mImage[i]);
    }

    CreateShaders();
}

void ConvolutionWindow2::OnIdle()
{
    mTimer.Measure();

    if (mShadersCreated)
    {
        ExecuteShaders();
        mEngine->Draw(mOverlay[0]);
        mEngine->Draw(mOverlay[1]);
        std::string message = "radius = " + std::to_string(mRadius);
        std::array<float, 4> textColor{ 1.0f, 1.0f, 0.0f, 1.0f };
        mEngine->Draw(8, mYSize - 40, textColor, msName[mSelection]);
        mEngine->Draw(8, mYSize - 24, textColor, message);
        mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
        mEngine->DisplayColorBuffer(0);
    }

    mTimer.UpdateFrameCount();
}

bool ConvolutionWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '0':
        mSelection = 0;
        CreateShaders();
        return true;

    case '1':
        mSelection = 1;
        CreateShaders();
        return true;

    case '2':
        mSelection = 2;
        CreateShaders();
        return true;

    case '3':
        mSelection = 3;
        CreateShaders();
        return true;

    case '4':
        mSelection = 4;
        CreateShaders();
        return true;

    case '+':
    case '=':
        if (mRadius < MAX_RADIUS)
        {
            ++mRadius;
            CreateShaders();
        }
        return true;

    case '-':
    case '_':
        if (mRadius > 1)
        {
            --mRadius;
            CreateShaders();
        }
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}

bool ConvolutionWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Imagics/Convolution/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");

    std::vector<std::string> inputs =
    {
        "MedicineBag.png",
        mEngine->GetShaderName("Convolve.cs"),
        mEngine->GetShaderName("ConvolveGS.cs"),
        mEngine->GetShaderName("ConvolveSeparableH.cs"),
        mEngine->GetShaderName("ConvolveSeparableHGS.cs"),
        mEngine->GetShaderName("ConvolveSeparableHGS2.cs"),
        mEngine->GetShaderName("ConvolveSeparableV.cs"),
        mEngine->GetShaderName("ConvolveSeparableVGS.cs"),
        mEngine->GetShaderName("ConvolveSeparableVGS2.cs")
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

void ConvolutionWindow2::CreateShaders()
{
    std::string path;
    std::shared_ptr<Shader> cshader;
    mShadersCreated = true;

    switch (mSelection)
    {
    case 0:
        mNumXGroups = mImage[0]->GetWidth() / 16;
        mNumYGroups = mImage[0]->GetHeight() / 16;
        mProgramFactory->defines.Set("NUM_X_THREADS", 16);
        mProgramFactory->defines.Set("NUM_Y_THREADS", 16);
        mProgramFactory->defines.Set("RADIUS", mRadius);
        path = mEnvironment.GetPath(mEngine->GetShaderName("Convolve.cs"));
        mConvolve = mProgramFactory->CreateFromFile(path);
        if (!mConvolve)
        {
            LogError("Failed to compile " + mEngine->GetShaderName("Convolve.cs"));
        }

        cshader = mConvolve->GetComputeShader();
        cshader->Set("inImage", mImage[0]);
        cshader->Set("outImage", mImage[1]);
        cshader->Set("Weights", GetKernel2(mRadius));
        mProgramFactory->defines.Clear();
        break;

    case 1:
        mNumXGroups = mImage[0]->GetWidth() / 16;
        mNumYGroups = mImage[0]->GetHeight() / 16;
        mProgramFactory->defines.Set("RADIUS", mRadius);
        mProgramFactory->defines.Set("NUM_X_THREADS", 16);
        mProgramFactory->defines.Set("NUM_Y_THREADS", 16);
        path = mEnvironment.GetPath(mEngine->GetShaderName("ConvolveGS.cs"));
        mConvolveGS = mProgramFactory->CreateFromFile(path);
        if (!mConvolveGS)
        {
            LogError("Failed to compile " + mEngine->GetShaderName("ConvolveGS.cs"));
        }

        cshader = mConvolveGS->GetComputeShader();
        cshader->Set("inImage", mImage[0]);
        cshader->Set("outImage", mImage[1]);
        cshader->Set("Weights", GetKernel2(mRadius));
        mProgramFactory->defines.Clear();
        break;

    case 2:
        mNumXGroups = mImage[0]->GetWidth() / 16;
        mNumYGroups = mImage[0]->GetHeight() / 16;
        mProgramFactory->defines.Set("NUM_X_THREADS", 16);
        mProgramFactory->defines.Set("NUM_Y_THREADS", 16);
        mProgramFactory->defines.Set("RADIUS", mRadius);

        path = mEnvironment.GetPath(mEngine->GetShaderName("ConvolveSeparableH.cs"));
        mConvolveSeparableH = mProgramFactory->CreateFromFile(path);
        if (!mConvolveSeparableH)
        {
            LogError("Failed to compile " + mEngine->GetShaderName("ConvolveSeparableH.cs"));
        }

        path = mEnvironment.GetPath(mEngine->GetShaderName("ConvolveSeparableV.cs"));
        mConvolveSeparableV = mProgramFactory->CreateFromFile(path);
        if (!mConvolveSeparableV)
        {
            LogError("Failed to compile " + mEngine->GetShaderName("ConvolveSeparableV.cs"));
        }

        cshader = mConvolveSeparableH->GetComputeShader();
        cshader->Set("inImage", mImage[0]);
        cshader->Set("outImage", mImage[2]);
        cshader->Set("Weights", GetKernel1(mRadius));
        cshader = mConvolveSeparableV->GetComputeShader();
        cshader->Set("inImage", mImage[2]);
        cshader->Set("outImage", mImage[1]);
        cshader->Set("Weights", GetKernel1(mRadius));
        mProgramFactory->defines.Clear();
        break;

    case 3:
        mProgramFactory->defines.Set("RADIUS", mRadius);

        path = mEnvironment.GetPath(mEngine->GetShaderName("ConvolveSeparableHGS.cs"));
        mConvolveSeparableHGS = mProgramFactory->CreateFromFile(path);
        if (!mConvolveSeparableHGS)
        {
            LogError("Failed to compile " + mEngine->GetShaderName("ConvolveSeparableHGS.cs"));
        }

        path = mEnvironment.GetPath(mEngine->GetShaderName("ConvolveSeparableVGS.cs"));
        mConvolveSeparableVGS = mProgramFactory->CreateFromFile(path);
        if (!mConvolveSeparableVGS)
        {
            LogError("Failed to compile " + mEngine->GetShaderName("ConvolveSeparableVGS.cs"));
        }

        cshader = mConvolveSeparableHGS->GetComputeShader();
        cshader->Set("inImage", mImage[0]);
        cshader->Set("outImage", mImage[2]);
        cshader->Set("Weights", GetKernel1(mRadius));
        cshader = mConvolveSeparableVGS->GetComputeShader();
        cshader->Set("inImage", mImage[2]);
        cshader->Set("outImage", mImage[1]);
        cshader->Set("Weights", GetKernel1(mRadius));
        mProgramFactory->defines.Clear();
        break;

    case 4:
        mProgramFactory->defines.Set("RADIUS", mRadius);

        path = mEnvironment.GetPath(mEngine->GetShaderName("ConvolveSeparableHGS2.cs"));
        mConvolveSeparableHGS2 = mProgramFactory->CreateFromFile(path);
        if (!mConvolveSeparableHGS2)
        {
            LogError("Failed to compile " + mEngine->GetShaderName("ConvolveSeparableHGS2.cs"));
        }

        path = mEnvironment.GetPath(mEngine->GetShaderName("ConvolveSeparableVGS2.cs"));
        mConvolveSeparableVGS2 = mProgramFactory->CreateFromFile(path);
        if (!mConvolveSeparableVGS2)
        {
            LogError("Failed to compile " + mEngine->GetShaderName("ConvolveSeparableVGS2.cs"));
        }

        cshader = mConvolveSeparableHGS2->GetComputeShader();
        cshader->Set("inImage", mImage[0]);
        cshader->Set("outImage", mImage[2]);
        cshader->Set("Weights", GetKernel1(mRadius));
        cshader = mConvolveSeparableVGS2->GetComputeShader();
        cshader->Set("inImage", mImage[2]);
        cshader->Set("outImage", mImage[1]);
        cshader->Set("Weights", GetKernel1(mRadius));
        mProgramFactory->defines.Clear();
        break;

    default:
        mShadersCreated = false;
        break;
    }
}

void ConvolutionWindow2::ExecuteShaders()
{
    switch (mSelection)
    {
    case 0:
        if (mConvolve)
        {
            mEngine->Execute(mConvolve, mNumXGroups, mNumYGroups, 1);
        }
        break;

    case 1:
        if (mConvolveGS)
        {
            mEngine->Execute(mConvolveGS, mNumXGroups, mNumYGroups, 1);
        }
        break;

    case 2:
        if (mConvolveSeparableH && mConvolveSeparableV)
        {
            mEngine->Execute(mConvolveSeparableH, mNumXGroups, mNumYGroups, 1);
            mEngine->Execute(mConvolveSeparableV, mNumXGroups, mNumYGroups, 1);
        }
        break;

    case 3:
        if (mConvolveSeparableHGS && mConvolveSeparableVGS)
        {
            mEngine->Execute(mConvolveSeparableHGS, 1, mImage[0]->GetHeight(), 1);
            mEngine->Execute(mConvolveSeparableVGS, mImage[0]->GetWidth(), 1, 1);
        }
        break;

    case 4:
        if (mConvolveSeparableHGS2 && mConvolveSeparableVGS2)
        {
            mEngine->Execute(mConvolveSeparableHGS2, 4, mImage[0]->GetHeight(), 1);
            mEngine->Execute(mConvolveSeparableVGS2, mImage[0]->GetWidth(), 4, 1);
        }
        break;
    }
}

std::shared_ptr<ConstantBuffer> ConvolutionWindow2::GetKernel1(int32_t radius)
{
    // If radius/sigma = ratio, then exp(-ratio^2/2) = 0.001.
    float const ratio = 3.7169221888498384469524067613045f;
    float sigma = radius / ratio;

    int32_t const numWeights = 2 * radius + 1;
    std::vector<float> weight(numWeights);
    float totalWeight = 0.0f;
    for (int32_t x = -radius, i = 0; x <= radius; ++x, ++i)
    {
        float fx = x / sigma;
        float value = std::exp(-0.5f * fx * fx);
        weight[i] = value;
        totalWeight += value;
    }

    for (auto& w : weight)
    {
        w /= totalWeight;
    }

    // The constant-buffer/uniform store one float per 4-tuple register.
    std::shared_ptr<ConstantBuffer> cbuffer = std::make_shared<ConstantBuffer>(
        numWeights * sizeof(Vector4<float>), false);
    Vector4<float>* data = cbuffer->Get<Vector4<float>>();
    for (int32_t i = 0; i < numWeights; ++i)
    {
        Vector4<float>& entry = data[i];
        entry[0] = weight[i];
        entry[1] = 0.0f;
        entry[2] = 0.0f;
        entry[3] = 0.0f;
    }
    return cbuffer;
}

std::shared_ptr<ConstantBuffer> ConvolutionWindow2::GetKernel2(int32_t radius)
{
    // If radius/sigma = ratio, then exp(-ratio^2/2) = 0.001.
    float const ratio = 3.7169221888498384469524067613045f;
    float sigma = radius / ratio;

    int32_t const length = 2 * radius + 1;
    int32_t const numWeights = length * length;
    std::vector<float> weight(numWeights);
    float totalWeight = 0.0f;
    for (int32_t y = -radius, i = 0; y <= radius; ++y)
    {
        float fy = y / sigma;
        for (int32_t x = -radius; x <= radius; ++x, ++i)
        {
            float fx = x / sigma;
            float value = std::exp(-0.5f*(fx*fx + fy*fy));
            weight[i] = value;
            totalWeight += value;
        }
    }

    for (auto& w : weight)
    {
        w /= totalWeight;
    }

    // The constant-buffer/uniform store one float per 4-tuple register.
    std::shared_ptr<ConstantBuffer> cbuffer = std::make_shared<ConstantBuffer>(
        numWeights * sizeof(Vector4<float>), false);
    Vector4<float>* data = cbuffer->Get<Vector4<float>>();
    for (int32_t i = 0; i < numWeights; ++i)
    {
        Vector4<float>& entry = data[i];
        entry[0] = weight[i];
        entry[1] = 0.0f;
        entry[2] = 0.0f;
        entry[3] = 0.0f;
    }
    return cbuffer;
}

std::string ConvolutionWindow2::msName[5] =
{
    "convolve",
    "convolve groupshared",
    "convolve separable",
    "convolve separable groupshared (one slice at a time)",
    "convolve separable groupshared (slice processed as subslices)"
};
