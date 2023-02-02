// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
using namespace gte;

class ConvolutionWindow2 : public Window2
{
public:
    ConvolutionWindow2(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    enum { MAX_RADIUS = 8 };

    bool SetEnvironment();
    void CreateShaders();
    void ExecuteShaders();
    std::shared_ptr<ConstantBuffer> GetKernel1(int32_t radius);
    std::shared_ptr<ConstantBuffer> GetKernel2(int32_t radius);

    std::shared_ptr<OverlayEffect> mOverlay[2];
    std::shared_ptr<Texture2> mImage[3];
    uint32_t mNumXGroups, mNumYGroups;
    int32_t mRadius;
    bool mShadersCreated;

    // 0 = convolve
    // 1 = convolve groupshared
    // 2 = convolve separable
    // 3 = convolve separable groupshared (one slice at a time)
    // 4 = convolve separable groupshared (slice processed as subslices)
    uint32_t mSelection;
    static std::string msName[5];

    // selection 0
    std::shared_ptr<ComputeProgram> mConvolve;

    // selection 1
    std::shared_ptr<ComputeProgram> mConvolveGS;

    // selection 2
    std::shared_ptr<ComputeProgram> mConvolveSeparableH;
    std::shared_ptr<ComputeProgram> mConvolveSeparableV;

    // selection 3
    std::shared_ptr<ComputeProgram> mConvolveSeparableHGS;
    std::shared_ptr<ComputeProgram> mConvolveSeparableVGS;

    // selection 4
    std::shared_ptr<ComputeProgram> mConvolveSeparableHGS2;
    std::shared_ptr<ComputeProgram> mConvolveSeparableVGS2;
};
