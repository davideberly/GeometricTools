// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.06

#pragma once

#include <Graphics/ConstantColorEffect.h>
#include <Graphics/GraphicsEngine.h>
#include <Graphics/Node.h>
#include <Graphics/ProgramFactory.h>
#include <Graphics/PVWUpdater.h>
#include <Graphics/Visual.h>
#include <cstdint>

namespace gte
{
    class PlanarShadowEffect
    {
    public:
        struct LightProjector
        {
            LightProjector()
                :
                position{ 0.0f, 0.0f, 0.0f, 0.0f },
                direction{ 0.0f, 0.0f, 0.0f, 0.0f },
                isPointLight(false)
            {
            }

            // The position and direction must be in world coordinates.
            Vector4<float> position, direction;

            // true for point light, false for directional light
            bool isPointLight;
        };

        // The number of planes supported depends on the number of stencil
        // bits (256 for an 8-bit stencil buffer). The planes must be opaque.
        // The vertex formats for the vertex buffer of the planeVisuals must
        // have VASemantic::POSITION with unit 0. The positions must be
        // 3-tuple or 4-tuple floats and must occur as the first semantic
        // of the formats.
        PlanarShadowEffect(std::shared_ptr<ProgramFactory> const& factory,
            std::shared_ptr<Node> const& shadowCaster,
            std::shared_ptr<LightProjector> const& lightProjector,
            std::vector<std::shared_ptr<Visual>> const& planeVisuals,
            std::vector<Vector4<float>> const& shadowColors);

        ~PlanarShadowEffect() = default;

        // The constructor input 'light' can represent a point light or a
        // directional light. Set 'isPointLight' to 'true' for a point light
        // or to 'false' for a directional light.
        void Draw(std::shared_ptr<GraphicsEngine> const& engine,
            PVWUpdater& pvwMatrices);

    protected:
        void GatherVisuals(std::shared_ptr<ProgramFactory> const& factory,
            std::shared_ptr<Spatial> const& spatial);

        void GetModelSpaceTriangles();

        bool GetProjectionMatrix(size_t i, Matrix4x4<float>& projectionMatrix);

        // Constructor inputs.
        std::shared_ptr<Node> mShadowCaster;
        std::shared_ptr<LightProjector> mLightProjector;
        std::vector<std::shared_ptr<Visual>> mPlaneVisuals;
        std::vector<Vector4<float>> mShadowColors;

        // Each Visual in the shadow caster hierarchy needs a visual
        // effect for drawing the shadow cast by it.
        std::vector<std::shared_ptr<Visual>> mCasterVisuals;
        std::vector<std::shared_ptr<ConstantColorEffect>> mCasterEffects;
        std::vector<std::shared_ptr<VisualEffect>> mSaveVisualEffects;

        // Model-space triangles for the planes. These are transformed to
        // world space and used to compute the projection matrix of the
        // light.
        std::vector<std::array<Vector4<float>, 3>> mModelSpaceTriangles;

        // Global state for the drawing passes.
        std::shared_ptr<BlendState> mShadowBlend;
        std::shared_ptr<DepthStencilState> mDSPass0, mDSPass1;

        // TODO: The stencil buffer reference values for the planes during
        // the mDSPass1 drawing are not being set to zero in the GL45
        // version of this sample. This leads to shadowed pixels that should
        // not be shadowed. If I clear the stencil buffer after each plane is
        // processed, the drawing is correct. I need to figure out why this
        // happens.
        int32_t mAPI;
    };
}
