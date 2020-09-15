// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/GPU/GPUFluid2AdjustVelocity.h>
#include <Mathematics/GPU/GPUFluid2ComputeDivergence.h>
#include <Mathematics/GPU/GPUFluid2EnforceStateBoundary.h>
#include <Mathematics/GPU/GPUFluid2InitializeSource.h>
#include <Mathematics/GPU/GPUFluid2InitializeState.h>
#include <Mathematics/GPU/GPUFluid2SolvePoisson.h>
#include <Mathematics/GPU/GPUFluid2UpdateState.h>

namespace gte
{
    class GPUFluid2
    {
    public:
        // Construction.  The (x,y) grid covers [0,1]^2.
        GPUFluid2(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<ProgramFactory> const& factory,
            int xSize, int ySize, float dt, float densityViscosity, float velocityViscosity);

        void Initialize();
        void DoSimulationStep();

        inline std::shared_ptr<Texture2> const& GetState() const
        {
            return mStateTTexture;
        }

    private:
        // Constructor inputs.
        std::shared_ptr<GraphicsEngine> mEngine;
        int mXSize, mYSize;
        float mDt;

        // Current simulation time.
        float mTime;

        std::shared_ptr<ConstantBuffer> mParameters;
        std::shared_ptr<GPUFluid2InitializeSource> mInitializeSource;
        std::shared_ptr<GPUFluid2InitializeState> mInitializeState;
        std::shared_ptr<GPUFluid2EnforceStateBoundary> mEnforceStateBoundary;
        std::shared_ptr<GPUFluid2UpdateState> mUpdateState;
        std::shared_ptr<GPUFluid2ComputeDivergence> mComputeDivergence;
        std::shared_ptr<GPUFluid2SolvePoisson> mSolvePoisson;
        std::shared_ptr<GPUFluid2AdjustVelocity> mAdjustVelocity;

        std::shared_ptr<Texture2> mSourceTexture;
        std::shared_ptr<Texture2> mStateTm1Texture;
        std::shared_ptr<Texture2> mStateTTexture;
        std::shared_ptr<Texture2> mStateTp1Texture;
        std::shared_ptr<Texture2> mDivergenceTexture;
        std::shared_ptr<Texture2> mPoissonTexture;
    };
}
