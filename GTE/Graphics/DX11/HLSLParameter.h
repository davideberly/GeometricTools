// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/DX11/HLSLResource.h>
#include <cstdint>

namespace gte
{
    class HLSLParameter
    {
    public:
        struct Description
        {
            Description()
                :
                semanticName(""),
                semanticIndex(0),
                registerIndex(0),
                systemValueType(D3D_NAME_UNDEFINED),
                componentType(D3D_REGISTER_COMPONENT_UNKNOWN),
                mask(0),
                readWriteMask(0),
                stream(0),
                minPrecision(D3D_MIN_PRECISION_DEFAULT)
            {
            }

            std::string semanticName;
            uint32_t semanticIndex;
            uint32_t registerIndex;
            D3D_NAME systemValueType;
            D3D_REGISTER_COMPONENT_TYPE componentType;
            uint32_t mask;
            uint32_t readWriteMask;
            uint32_t stream;
            D3D_MIN_PRECISION minPrecision;
        };

        // Construction.  Parameters are reported for inputs, outputs and
        // patch constants.
        HLSLParameter(D3D_SIGNATURE_PARAMETER_DESC const& desc);

        // Member access.
        inline std::string const& GetSemanticName() const
        {
            return mDesc.semanticName;
        }

        inline uint32_t GetSemanticIndex() const
        {
            return mDesc.semanticIndex;
        }

        inline uint32_t GetRegisterIndex() const
        {
            return mDesc.registerIndex;
        }

        inline D3D_NAME GetSystemValueType() const
        {
            return mDesc.systemValueType;
        }

        inline D3D_REGISTER_COMPONENT_TYPE GetComponentType() const
        {
            return mDesc.componentType;
        }

        inline uint32_t GetMask() const
        {
            return mDesc.mask;
        }

        inline uint32_t GetReadWriteMask() const
        {
            return mDesc.readWriteMask;
        }

        inline uint32_t GetStream() const
        {
            return mDesc.stream;
        }

        inline D3D_MIN_PRECISION GetMinPrecision() const
        {
            return mDesc.minPrecision;
        }

        // Print to a text file for human readability.
        void Print(std::ofstream& output) const;

    private:
        Description mDesc;

        // Support for Print.
        static std::string const msSVName[];
        static std::string const msComponentType[];
        static std::string const msMinPrecision[];
    };
}
