// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
using namespace gte;

class GpuGaussianBlur3Window2 : public Window2
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

    GpuGaussianBlur3Window2(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateImages();
    bool CreateShaders();

    inline int32_t Map3Dto1D(int32_t x, int32_t y, int32_t z) const
    {
        int32_t u = x + (z % 8) * 128;
        int32_t v = y + (z / 8) * 128;
        return u + 1024 * v;
    }

    inline void Map3Dto2D(int32_t x, int32_t y, int32_t z, int32_t& u, int32_t& v) const
    {
        u = x + (z % 8) * 128;
        v = y + (z / 8) * 128;
    }

    inline int32_t Map2Dto1D(int32_t u, int32_t v) const
    {
        return u + 1024 * v;
    }

    inline void Map2Dto3D(int32_t u, int32_t v, int32_t& x, int32_t& y, int32_t& z) const
    {
        x = u % 128;
        y = v % 128;
        z = (u / 128) + (v / 128) * 8;
    }

    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mImage[2];
    std::shared_ptr<Texture2> mMaskTexture;
    std::shared_ptr<Texture2> mZNeighborTexture;
    std::shared_ptr<Texture2> mNeumannOffsetTexture;
    std::shared_ptr<ConstantBuffer> mWeightBuffer;
    std::shared_ptr<ComputeProgram> mGaussianBlurProgram;
    std::shared_ptr<ComputeProgram> mBoundaryDirichletProgram;
    std::shared_ptr<ComputeProgram> mBoundaryNeumannProgram;
    uint32_t mNumXThreads, mNumYThreads;
    uint32_t mNumXGroups, mNumYGroups;
    bool mUseDirichlet;
};
