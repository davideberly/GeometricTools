// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Read the comments in Mathematics/GenerateMeshUV.h for information about
// the algorithm.  The class GenerateMeshUV header file has a CPU-based
// implementation.  The class GPUGenerateMeshUV derives from GenerateMeshUV
// and provides a GPU-based implementation using DX11/HLSL or GL45/GLSL.

#include <Mathematics/GenerateMeshUV.h>
#include <Graphics/GraphicsEngine.h>
#include <Graphics/ProgramFactory.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <utility>

namespace gte
{
    template <typename Real>
    class GPUGenerateMeshUV : public GenerateMeshUV<Real>
    {
    public:
        // Construction.
        GPUGenerateMeshUV(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<ProgramFactory> const& factory,
            std::function<void(uint32_t)> const* progress = nullptr)
            :
            GenerateMeshUV<Real>(std::numeric_limits<uint32_t>::max(), progress),
            mEngine(engine),
            mFactory(factory)
        {
        }

        virtual ~GPUGenerateMeshUV() = default;

    protected:
        virtual void SolveSystemInternal(uint32_t numIterations) override
        {
            int32_t api = mFactory->GetAPI();
            mFactory->defines.Set("NUM_X_THREADS", 8);
            mFactory->defines.Set("NUM_Y_THREADS", 8);
            if (std::numeric_limits<Real>::max() == std::numeric_limits<float>::max())
            {
                mFactory->defines.Set("Real", "float");
                if (api == ProgramFactory::PF_GLSL)
                {
                    mFactory->defines.Set("Real2", "vec2");
                }
                else  // api == ProgramFactory::PF_HLSL
                {
                    mFactory->defines.Set("Real2", "float2");
                }
            }
            else
            {
                mFactory->defines.Set("Real", "double");
                if (api == ProgramFactory::PF_GLSL)
                {
                    mFactory->defines.Set("Real2", "dvec2");
                }
                else  // api == ProgramFactory::PF_HLSL
                {
                    mFactory->defines.Set("Real2", "double2");
                }
            }

            // TODO: Test mSolveSystem for null and respond accordingly.
            auto solveSystem = mFactory->CreateFromSource(ShaderSource(api));
            LogAssert(solveSystem, "Failed to compile shader.");
            auto const& cshader = solveSystem->GetComputeShader();

            // Compute the number of thread groups.
            int32_t numInputs = this->mNumVertices - this->mNumBoundaryEdges;
            Real factor0 = std::ceil(std::sqrt((Real)numInputs));
            Real factor1 = std::ceil((Real)numInputs / factor0);
            int32_t xElements = static_cast<int32_t>(factor0);
            int32_t yElements = static_cast<int32_t>(factor1);
            int32_t xRem = (xElements % 8);
            if (xRem > 0)
            {
                xElements += 8 - xRem;
            }
            int32_t yRem = (yElements % 8);
            if (yRem > 0)
            {
                yElements += 8 - yRem;
            }
            uint32_t numXGroups = xElements / 8;
            uint32_t numYGroups = yElements / 8;

            auto boundBuffer = std::make_shared<ConstantBuffer>(4 * sizeof(int32_t), false);
            auto data = boundBuffer->Get<int32_t>();
            data[0] = xElements;
            data[1] = yElements;
            data[2] = this->mNumBoundaryEdges;
            data[3] = numInputs;
            cshader->Set("Bounds", boundBuffer);

            uint32_t const vgSize = static_cast<uint32_t>(this->mVertexGraph.size());
            auto vgBuffer = std::make_shared<StructuredBuffer>(vgSize, sizeof(typename GenerateMeshUV<Real>::Vertex));
            std::memcpy(vgBuffer->GetData(), &this->mVertexGraph[0], vgBuffer->GetNumBytes());
            cshader->Set("vertexGraph", vgBuffer);

            uint32_t const vgdSize = static_cast<uint32_t>(this->mVertexGraphData.size());
            auto vgdBuffer = std::make_shared<StructuredBuffer>(vgdSize, sizeof(std::pair<int32_t, Real>));
            std::memcpy(vgdBuffer->GetData(), &this->mVertexGraphData[0], vgdBuffer->GetNumBytes());
            cshader->Set("vertexGraphData", vgdBuffer);

            uint32_t const ovSize = static_cast<uint32_t>(this->mOrderedVertices.size());
            auto ovBuffer = std::make_shared<StructuredBuffer>(ovSize, sizeof(int32_t));
            std::memcpy(ovBuffer->GetData(), &this->mOrderedVertices[0], ovBuffer->GetNumBytes());
            cshader->Set("orderedVertices", ovBuffer);

            std::array<std::shared_ptr<StructuredBuffer>, 2> tcoordsBuffer;
            for (size_t j = 0; j < 2; ++j)
            {
                tcoordsBuffer[j] = std::make_shared<StructuredBuffer>(this->mNumVertices, sizeof(Vector2<Real>));
                tcoordsBuffer[j]->SetUsage(Resource::Usage::SHADER_OUTPUT);
                std::memcpy(tcoordsBuffer[j]->GetData(), this->mTCoords, tcoordsBuffer[j]->GetNumBytes());
            }
            tcoordsBuffer[0]->SetCopy(Resource::Copy::STAGING_TO_CPU);

            // The value numIterations is even, so we always swap an even
            // number of times.  This ensures that on exit from the loop,
            // mTCoordsBuffer[0] has the final output.
            for (uint32_t i = 1; i <= numIterations; ++i)
            {
                if (this->mProgress)
                {
                    (*this->mProgress)(i);
                }

                cshader->Set("inTCoords", tcoordsBuffer[0]);
                cshader->Set("outTCoords", tcoordsBuffer[1]);
                mEngine->Execute(solveSystem, numXGroups, numYGroups, 1);
                std::swap(tcoordsBuffer[0], tcoordsBuffer[1]);
            }

            mEngine->CopyGpuToCpu(tcoordsBuffer[0]);
            std::memcpy(this->mTCoords, tcoordsBuffer[0]->GetData(), tcoordsBuffer[0]->GetNumBytes());
        }

    private:
        std::shared_ptr<GraphicsEngine> mEngine;
        std::shared_ptr<ProgramFactory> mFactory;

        static std::string const& ShaderSource(int32_t api)
        {
            static std::array<std::string, 2> source =
            {
                // GLSL compute shader
                R"(
                    uniform Bounds
                    {
                        ivec2 bound;
                        int32_t numBoundaryEdges;
                        int32_t numInputs;
                    };

                    struct VertexGraphData
                    {
                        int32_t adjacent;
                        Real weight;
                    };

                    buffer vertexGraph { ivec4 data[]; } vertexGraphSB;
                    buffer vertexGraphData { VertexGraphData data[]; } vertexGraphDataSB;
                    buffer orderedVertices { int32_t data[]; } orderedVerticesSB;
                    buffer inTCoords { Real2 data[]; } inTCoordsSB;
                    buffer outTCoords { Real2 data[]; } outTCoordsSB;

                    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
                    void main()
                    {
                        ivec2 t = ivec2(gl_GlobalInvocationID.xy);
                        int32_t index = t.x + bound.x * t.y;
                        if (step(index, numInputs-1) == 1)
                        {
                            int32_t v = orderedVerticesSB.data[numBoundaryEdges + index];
                            ivec2 range = vertexGraphSB.data[v].yz;
                            Real2 tcoord = Real2(0, 0);
                            Real weightSum = 0;
                            for (int32_t j = 0; j < range.y; ++j)
                            {
                                VertexGraphData vgd = vertexGraphDataSB.data[range.x + j];
                                weightSum += vgd.weight;
                                tcoord += vgd.weight * inTCoordsSB.data[vgd.adjacent];
                            }
                            tcoord /= weightSum;
                            outTCoordsSB.data[v] = tcoord;
                        }
                    }
                )",

                // HLSL compute shader
                R"(
                    cbuffer Bounds
                    {
                        int2 bound;
                        int32_t numBoundaryEdges;
                        int32_t numInputs;
                    };

                    struct VertexGraphData
                    {
                        int32_t adjacent;
                        Real weight;
                    };

                    StructuredBuffer<int4> vertexGraph;
                    StructuredBuffer<VertexGraphData> vertexGraphData;
                    StructuredBuffer<int32_t> orderedVertices;
                    StructuredBuffer<Real2> inTCoords;
                    RWStructuredBuffer<Real2> outTCoords;

                    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
                    void CSMain(int2 t : SV_DispatchThreadID)
                    {
                        int32_t index = t.x + bound.x * t.y;
                        if (step(index, numInputs-1))
                        {
                            int32_t v = orderedVertices[numBoundaryEdges + index];
                            int2 range = vertexGraph[v].yz;
                            Real2 tcoord = Real2(0, 0);
                            Real weightSum = 0;
                            for (int32_t j = 0; j < range.y; ++j)
                            {
                                VertexGraphData vgd = vertexGraphData[range.x + j];
                                weightSum += vgd.weight;
                                tcoord += vgd.weight * inTCoords[vgd.adjacent];
                            }
                            tcoord /= weightSum;
                            outTCoords[v] = tcoord;
                        }
                    }
                )"
            };
            return source[api];
        }
    };
}
