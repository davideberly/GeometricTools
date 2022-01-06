// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Environment.h>
#include <Graphics/Graphics.h>
#include <Graphics/Texture1.h>
#include <Graphics/Texture2.h>
#include <Graphics/VisualEffect.h>
#include <Mathematics/Vector2.h>

namespace gte
{
    class BlendedTerrainEffect : public VisualEffect
    {
    public:
        // Construction.
        BlendedTerrainEffect(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<ProgramFactory> const& factory,
            Environment const& environment, bool& created);

        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline void SetFlowDirection(Vector2<float> const& flowDirection)
        {
            *mFlowDirection = flowDirection;
        }

        inline Vector2<float> const& GetFlowDirection() const
        {
            return *mFlowDirection;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetFlowDirectionConstant() const
        {
            return mFlowDirectionConstant;
        }

        inline void SetPowerFactor(float powerFactor)
        {
            *mPowerFactor = powerFactor;
        }

        inline float GetPowerFactor() const
        {
            return *mPowerFactor;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetPowerFactorConstant() const
        {
            return mPowerFactorConstant;
        }

        inline std::shared_ptr<Texture1> const& GetBlendTexture() const
        {
            return mBlendTexture;
        }

        inline std::shared_ptr<Texture2> const& GetGrassTexture() const
        {
            return mGrassTexture;
        }

        inline std::shared_ptr<Texture2> const& GetStoneTexture() const
        {
            return mStoneTexture;
        }

        inline std::shared_ptr<Texture2> const& GetCloudTexture() const
        {
            return mCloudTexture;
        }

        inline std::shared_ptr<SamplerState> const& GetCommonSampler() const
        {
            return mCommonSampler;
        }

        inline std::shared_ptr<SamplerState> const& GetBlendSampler() const
        {
            return mBlendSampler;
        }

    private:
        // Vertex shader parameter.
        std::shared_ptr<ConstantBuffer> mFlowDirectionConstant;

        // Pixel shader parameters.
        std::shared_ptr<ConstantBuffer> mPowerFactorConstant;
        std::shared_ptr<Texture1> mBlendTexture;
        std::shared_ptr<Texture2> mGrassTexture;
        std::shared_ptr<Texture2> mStoneTexture;
        std::shared_ptr<Texture2> mCloudTexture;
        std::shared_ptr<SamplerState> mCommonSampler;
        std::shared_ptr<SamplerState> mBlendSampler;

        // Convenience pointers.
        Vector2<float>* mFlowDirection;
        float* mPowerFactor;
    };
}
