// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <MathematicsGPU/GPUFluid2Parameters.h>
#include <Graphics/GraphicsEngine.h>
#include <Graphics/ProgramFactory.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/Texture3.h>
#include <cstdint>
#include <memory>
#include <string>

namespace gte
{
    class GPUFluid3ComputeDivergence
    {
    public:
        // Construction.  Compute the divergence of the velocity vector field:
        // Divergence(V(x,y,z)) = dV/dx + dV/dy + dV/dz.  The derivatives are
        // estimated using centered finite differences,
        //   dV/dx = (V(x+dx,y,z) - V(x-dx,y,z))/(2*dx)
        //   dV/dy = (V(x,y+dy,z) - V(x,y-dy,z))/(2*dy)
        //   dV/dz = (V(x,y,z+dz) - V(x,y,z-dz))/(2*dz)
        GPUFluid3ComputeDivergence(std::shared_ptr<ProgramFactory> const& factory,
            int32_t xSize, int32_t ySize, int32_t zSize, int32_t numXThreads, int32_t numYThreads, int32_t numZThreads,
            std::shared_ptr<ConstantBuffer> const& parameters);

        // Member access.
        inline std::shared_ptr<Texture3> const& GetDivergence() const
        {
            return mDivergence;
        }

        // Compute the divergence of the velocity vector field for the input
        // state.
        void Execute(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<Texture3> const& state);

    private:
        int32_t mNumXGroups, mNumYGroups, mNumZGroups;
        std::shared_ptr<ComputeProgram> mComputeDivergence;
        std::shared_ptr<Texture3> mDivergence;

        // Shader source code as strings.
        static std::string const msGLSLSource;
        static std::string const msHLSLSource;
        static ProgramSources const msSource;
    };
}
