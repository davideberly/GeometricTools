// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/DX11/HLSLByteAddressBuffer.h>
#include <Graphics/DX11/HLSLConstantBuffer.h>
#include <Graphics/DX11/HLSLParameter.h>
#include <Graphics/DX11/HLSLResourceBindInfo.h>
#include <Graphics/DX11/HLSLSamplerState.h>
#include <Graphics/DX11/HLSLStructuredBuffer.h>
#include <Graphics/DX11/HLSLTexture.h>
#include <Graphics/DX11/HLSLTextureArray.h>
#include <Graphics/DX11/HLSLTextureBuffer.h>
#include <cstdint>

namespace gte
{
    class HLSLReflection
    {
    public:
        struct Description
        {
            struct InstructionCount
            {
                InstructionCount()
                    :
                     numInstructions(0),
                     numTemporaryRegisters(0),
                     numTemporaryArrays(0),
                     numDefines(0),
                     numDeclarations(0),
                     numTextureNormal(0),
                     numTextureLoad(0),
                     numTextureComparison(0),
                     numTextureBias(0),
                     numTextureGradient(0),
                     numFloatArithmetic(0),
                     numSIntArithmetic(0),
                     numUIntArithmetic(0),
                     numStaticFlowControl(0),
                     numDynamicFlowControl(0),
                     numMacro(0),
                     numArray(0)
                {
                }

                uint32_t numInstructions;
                uint32_t numTemporaryRegisters;
                uint32_t numTemporaryArrays;
                uint32_t numDefines;
                uint32_t numDeclarations;
                uint32_t numTextureNormal;
                uint32_t numTextureLoad;
                uint32_t numTextureComparison;
                uint32_t numTextureBias;
                uint32_t numTextureGradient;
                uint32_t numFloatArithmetic;
                uint32_t numSIntArithmetic;
                uint32_t numUIntArithmetic;
                uint32_t numStaticFlowControl;
                uint32_t numDynamicFlowControl;
                uint32_t numMacro;
                uint32_t numArray;
            };


            struct GSParameters
            {
                GSParameters()
                    :
                    numCutInstructions(0),
                    numEmitInstructions(0),
                    inputPrimitive(D3D_PRIMITIVE_UNDEFINED),
                    outputTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED),
                    maxOutputVertices(0)
                {
                }

                uint32_t numCutInstructions;
                uint32_t numEmitInstructions;
                D3D_PRIMITIVE inputPrimitive;
                D3D_PRIMITIVE_TOPOLOGY outputTopology;
                uint32_t maxOutputVertices;
            };

            struct TSParameters
            {
                TSParameters()
                    :
                    numPatchConstants(0),
                    numGSInstances(0),
                    numControlPoints(0),
                    inputPrimitive(D3D_PRIMITIVE_UNDEFINED),
                    outputPrimitive(D3D_TESSELLATOR_OUTPUT_UNDEFINED),
                    partitioning(D3D_TESSELLATOR_PARTITIONING_UNDEFINED),
                    domain(D3D_TESSELLATOR_DOMAIN_UNDEFINED)
                {
                }

                uint32_t numPatchConstants;
                uint32_t numGSInstances;
                uint32_t numControlPoints;
                D3D_PRIMITIVE inputPrimitive;
                D3D_TESSELLATOR_OUTPUT_PRIMITIVE outputPrimitive;
                D3D_TESSELLATOR_PARTITIONING partitioning;
                D3D_TESSELLATOR_DOMAIN domain;
            };

            struct CSParameters
            {
                CSParameters()
                    :
                    numBarrierInstructions(0),
                    numInterlockedInstructions(0),
                    numTextureStoreInstructions(0)
                {
                }

                uint32_t numBarrierInstructions;
                uint32_t numInterlockedInstructions;
                uint32_t numTextureStoreInstructions;
            };

            Description()
                :
                creator(""),
                shaderType(D3D11_SHVER_PIXEL_SHADER),
                majorVersion(0),
                minorVersion(0),
                flags(0),
                numConstantBuffers(0),
                numBoundResources(0),
                numInputParameters(0),
                numOutputParameters(0),
                instructions{},
                gs{},
                ts{},
                cs{}
            {
            }

            std::string creator;
            D3D_SHADER_VERSION_TYPE shaderType;
            uint32_t majorVersion;
            uint32_t minorVersion;
            uint32_t flags;
            uint32_t numConstantBuffers;
            uint32_t numBoundResources;
            uint32_t numInputParameters;
            uint32_t numOutputParameters;
            InstructionCount instructions;
            GSParameters gs;
            TSParameters ts;
            CSParameters cs;
        };

        // Construction.
        HLSLReflection();

        // Test whether the shader was constructed properly.  The function tests
        // solely whether the name, entry, and target are nonempty strings and
        // that the compiled code array is nonempty; this is the common case when
        // HLSLShaderFactory is used to create the shader.
        bool IsValid() const;

        // Deferred construction for shader reflection.  These functions are
        // intended to be write-once.
        void SetDescription(D3D_SHADER_DESC const& desc);

        inline void SetName(std::string const& name)
        {
            mName = name;
        }

        inline void SetEntry(std::string const& entry)
        {
            mEntry = entry;
        }

        inline void SetTarget(std::string const& target)
        {
            mTarget = target;
        }

        inline void InsertInput(HLSLParameter const& parameter)
        {
            mInputs.push_back(parameter);
        }

        inline void InsertOutput(HLSLParameter const& parameter)
        {
            mOutputs.push_back(parameter);
        }

        inline void Insert(HLSLConstantBuffer const& cbuffer)
        {
            mCBuffers.push_back(cbuffer);
        }

        inline void Insert(HLSLTextureBuffer const& tbuffer)
        {
            mTBuffers.push_back(tbuffer);
        }

        inline void Insert(HLSLStructuredBuffer const& sbuffer)
        {
            mSBuffers.push_back(sbuffer);
        }

        inline void Insert(HLSLByteAddressBuffer const& rbuffer)
        {
            mRBuffers.push_back(rbuffer);
        }

        inline void Insert(HLSLTexture const& texture)
        {
            mTextures.push_back(texture);
        }

        inline void Insert(HLSLTextureArray const& textureArray)
        {
            mTextureArrays.push_back(textureArray);
        }

        inline void Insert(HLSLSamplerState const& samplerState)
        {
            mSamplerStates.push_back(samplerState);
        }

        inline void Insert(HLSLResourceBindInfo const& rbinfo)
        {
            mRBInfos.push_back(rbinfo);
        }

        void SetCompiledCode(size_t numBytes, void const* buffer);

        // Member access.
        inline HLSLReflection::Description const& GetDescription() const
        {
            return mDesc;
        }

        inline std::string const& GetName() const
        {
            return mName;
        }

        inline std::string const& GetEntry() const
        {
            return mEntry;
        }

        inline std::string const& GetTarget() const
        {
            return mTarget;
        }

        int32_t GetShaderTypeIndex() const;

        inline std::vector<HLSLParameter> const& GetInputs() const
        {
            return mInputs;
        }

        inline std::vector<HLSLParameter> const& GetOutputs() const
        {
            return mOutputs;
        }

        inline std::vector<HLSLConstantBuffer> const& GetCBuffers() const
        {
            return mCBuffers;
        }

        inline std::vector<HLSLTextureBuffer> const& GetTBuffers() const
        {
            return mTBuffers;
        }

        inline std::vector<HLSLStructuredBuffer> const& GetSBuffers() const
        {
            return mSBuffers;
        }

        inline std::vector<HLSLByteAddressBuffer> const& GetRBuffers() const
        {
            return mRBuffers;
        }

        inline std::vector<HLSLTexture> const& GetTextures() const
        {
            return mTextures;
        }

        inline std::vector<HLSLTextureArray> const& GetTextureArrays() const
        {
            return mTextureArrays;
        }

        inline std::vector<HLSLSamplerState> const& GetSamplerStates() const
        {
            return mSamplerStates;
        }

        inline std::vector<HLSLResourceBindInfo> const& GetResourceBindInfos() const
        {
            return mRBInfos;
        }

        inline std::vector<uint8_t> const& GetCompiledCode() const
        {
            return mCompiledCode;
        }

        // Compute shaders only.
        void SetNumThreads(uint32_t numXThreads, uint32_t numYThreads, uint32_t numZThreads);

        inline uint32_t GetNumXThreads() const
        {
            return mNumXThreads;
        }

        inline uint32_t GetNumYThreads() const
        {
            return mNumYThreads;
        }

        inline uint32_t GetNumZThreads() const
        {
            return mNumZThreads;
        }

        // Print to a text file for human readability.
        void Print(std::ofstream& output) const;

    private:
        Description mDesc;
        std::string mName;
        std::string mEntry;
        std::string mTarget;
        std::vector<HLSLParameter> mInputs;
        std::vector<HLSLParameter> mOutputs;
        std::vector<HLSLConstantBuffer> mCBuffers;
        std::vector<HLSLTextureBuffer> mTBuffers;
        std::vector<HLSLStructuredBuffer> mSBuffers;
        std::vector<HLSLByteAddressBuffer> mRBuffers;
        std::vector<HLSLTexture> mTextures;
        std::vector<HLSLTextureArray> mTextureArrays;
        std::vector<HLSLSamplerState> mSamplerStates;
        std::vector<HLSLResourceBindInfo> mRBInfos;
        std::vector<uint8_t> mCompiledCode;
        uint32_t mNumXThreads;
        uint32_t mNumYThreads;
        uint32_t mNumZThreads;

        // Support for Print.
        static std::string const msShaderType[];
        static std::string const msCompileFlags[];
        static std::string const msPrimitive[];
        static std::string const msPrimitiveTopology[];
        static std::string const msOutputPrimitive[];
        static std::string const msPartitioning[];
        static std::string const msDomain[];
    };
}
