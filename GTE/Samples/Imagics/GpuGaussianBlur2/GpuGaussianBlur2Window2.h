// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
using namespace gte;

class GpuGaussianBlur2Window2 : public Window2
{
public:
    struct Parameters : public Window2::Parameters
    {
        Parameters(std::wstring const& inTitle, int32_t inXOrigin, int32_t inYOrigin,
            int32_t inXSize, int32_t inYSize, bool inUseDirichlet)
            :
            Window2::Parameters(inTitle, inXOrigin, inYOrigin, inXSize, inYSize),
            useDirichlet(inUseDirichlet)
        {
        }

        bool useDirichlet;
    };

    GpuGaussianBlur2Window2(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateImages();
    bool CreateShaders();

    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mImage[2];
    std::shared_ptr<Texture2> mMaskTexture;
    std::shared_ptr<Texture2> mOffsetTexture;
    std::shared_ptr<ConstantBuffer> mWeightBuffer;
    std::shared_ptr<ComputeProgram> mGaussianBlurProgram;
    std::shared_ptr<ComputeProgram> mBoundaryDirichletProgram;
    std::shared_ptr<ComputeProgram> mBoundaryNeumannProgram;
    uint32_t mNumXThreads, mNumYThreads;
    uint32_t mNumXGroups, mNumYGroups;
    bool mUseDirichlet;
};
