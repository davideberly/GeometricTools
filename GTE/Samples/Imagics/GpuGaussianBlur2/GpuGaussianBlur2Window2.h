// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.11

#pragma once

#include <Applications/Window2.h>
using namespace gte;

class GpuGaussianBlur2Window2 : public Window2
{
public:
    struct Parameters : public Window2::Parameters
    {
        Parameters(std::wstring const& inTitle, int inXOrigin, int inYOrigin,
            int inXSize, int inYSize, bool inUseDirichlet)
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
    unsigned int mNumXThreads, mNumYThreads;
    unsigned int mNumXGroups, mNumYGroups;
    bool mUseDirichlet;
};
