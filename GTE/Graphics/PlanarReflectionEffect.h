// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.06

#pragma once

#include <Mathematics/Matrix4x4.h>
#include <Graphics/GraphicsEngine.h>
#include <Graphics/Node.h>
#include <Graphics/PVWUpdater.h>
#include <Graphics/Visual.h>
#include <cstdint>

namespace gte
{
    class PlanarReflectionEffect
    {
    public:
        // The number of planes supported depends on the number of stencil
        // bits (256 for an 8-bit stencil buffer). The planes must be opaque.
        PlanarReflectionEffect(
            std::shared_ptr<Node> const& reflectionCaster,
            std::vector<std::shared_ptr<Visual>> const& planeVisuals,
            std::vector<float> const& reflectances);

        ~PlanarReflectionEffect() = default;

        void Draw(std::shared_ptr<GraphicsEngine> const& engine,
            PVWUpdater& pvwMatrices);

        // Member access.
        inline std::vector<std::shared_ptr<Visual>> const& GetPlaneVisuals() const
        {
            return mPlaneVisuals;
        }

        inline std::vector<Vector4<float>> const& GetPlaneOrigins() const
        {
            return mPlaneOrigins;
        }

        inline std::vector<Vector4<float>> const& GetPlaneNormals() const
        {
            return mPlaneNormals;
        }

        inline void SetReflectance(size_t i, float reflectance)
        {
            mReflectances[i] = reflectance;
        }

        inline float GetReflectance(size_t i) const
        {
            return mReflectances[i];
        }

    private:
        void GatherVisuals(std::shared_ptr<Spatial> const& spatial);
        void GetModelSpacePlanes();

        // Constructor inputs.
        std::shared_ptr<Node> mReflectionCaster;
        std::vector<std::shared_ptr<Visual>> mPlaneVisuals;
        std::vector<float> mReflectances;

        std::vector<std::shared_ptr<Visual>> mCasterVisuals;
        std::vector<Vector4<float>> mPlaneOrigins;
        std::vector<Vector4<float>> mPlaneNormals;

        // Global state for the drawing passes.
        std::shared_ptr<BlendState> mNoColorWrites, mReflectanceBlend;
        std::shared_ptr<RasterizerState> mCullReverse;
        std::shared_ptr<DepthStencilState> mDSPass0, mDSPass1, mDSPass2, mDSPass3;
    };
}
