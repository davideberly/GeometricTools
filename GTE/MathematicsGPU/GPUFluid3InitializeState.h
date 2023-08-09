// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Graphics/GraphicsEngine.h>
#include <Graphics/ProgramFactory.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/Texture3.h>
#include <cstdint>
#include <memory>
#include <string>

namespace gte
{
    class GPUFluid3InitializeState
    {
    public:
        // Construction.  The initial velocity is zero and the initial density
        // is randomly generated with values in [0,1].
        GPUFluid3InitializeState(std::shared_ptr<ProgramFactory> const& factory,
            int32_t xSize, int32_t ySize, int32_t zSize, int32_t numXThreads, int32_t numYThreads, int32_t numZThreads);

        // Member access.  The texels are (velocity.xyz, density).
        inline std::shared_ptr<Texture3> const& GetStateTm1() const
        {
            return mStateTm1;
        }

        inline std::shared_ptr<Texture3> const& GetStateT() const
        {
            return mStateT;
        }

        // Compute the initial density and initial velocity for the fluid
        // simulation.
        void Execute(std::shared_ptr<GraphicsEngine> const& engine);

    private:
        int32_t mNumXGroups, mNumYGroups, mNumZGroups;
        std::shared_ptr<ComputeProgram> mInitializeState;
        std::shared_ptr<Texture3> mDensity;
        std::shared_ptr<Texture3> mVelocity;
        std::shared_ptr<Texture3> mStateTm1;
        std::shared_ptr<Texture3> mStateT;

        // Shader source code as strings.
        static std::string const msGLSLSource;
        static std::string const msHLSLSource;
        static ProgramSources const msSource;
    };
}
