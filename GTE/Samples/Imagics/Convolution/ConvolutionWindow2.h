// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window2.h>
using namespace gte;

class ConvolutionWindow2 : public Window2
{
public:
    ConvolutionWindow2(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    enum { MAX_RADIUS = 8 };

    bool SetEnvironment();
    void CreateShaders();
    void ExecuteShaders();
    std::shared_ptr<ConstantBuffer> GetKernel1(int radius);
    std::shared_ptr<ConstantBuffer> GetKernel2(int radius);

    std::shared_ptr<OverlayEffect> mOverlay[2];
    std::shared_ptr<Texture2> mImage[3];
    unsigned int mNumXGroups, mNumYGroups;
    int mRadius;
    bool mShadersCreated;

    // 0 = convolve
    // 1 = convolve groupshared
    // 2 = convolve separable
    // 3 = convolve separable groupshared (one slice at a time)
    // 4 = convolve separable groupshared (slice processed as subslices)
    unsigned int mSelection;
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
