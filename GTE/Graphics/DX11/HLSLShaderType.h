// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/DX11/HLSLResource.h>
#include <cstdint>
#include <string>
#include <vector>

namespace gte
{
    class HLSLShaderType
    {
    public:
        struct Description
        {
            Description()
                :
                varClass(D3D_SVC_SCALAR),
                varType(D3D_SVT_VOID),
                numRows(0),
                numColumns(0),
                numElements(0),
                numChildren(0),
                offset(0),
                typeName("")
            {
            }

            D3D_SHADER_VARIABLE_CLASS varClass;
            D3D_SHADER_VARIABLE_TYPE varType;
            uint32_t numRows;
            uint32_t numColumns;
            uint32_t numElements;
            uint32_t numChildren;
            uint32_t offset;
            std::string typeName;
        };

        // Construction.
        HLSLShaderType();

        // Deferred construction for shader reflection.  These functions are
        // intended to be write-once.
        void SetDescription(D3D_SHADER_TYPE_DESC const& desc);

        inline void SetName(std::string const& name)
        {
            mName = name;
        }

        // This is non-const and is intended to be used as part of the Set*
        // write-once system.  HLSLShaderFactory::{GetVariables,GetTypes} are
        // the clients and they ensure that i is a valid index.
        HLSLShaderType& GetChild(uint32_t i);

        // For use in construction of lookup tables for name-offset pairs.
        HLSLShaderType const& GetChild(uint32_t i) const;

        // Member access.
        inline std::string const& GetName() const
        {
            return mName;
        }

        inline D3D_SHADER_VARIABLE_CLASS GetClass() const
        {
            return mDesc.varClass;
        }

        inline D3D_SHADER_VARIABLE_TYPE GetType() const
        {
            return mDesc.varType;
        }

        inline uint32_t GetNumRows() const
        {
            return mDesc.numRows;
        }

        inline uint32_t GetNumColumns() const
        {
            return mDesc.numColumns;
        }

        inline uint32_t GetNumElements() const
        {
            return mDesc.numElements;
        }

        inline uint32_t GetNumChildren() const
        {
            return mDesc.numChildren;
        }

        inline uint32_t GetOffset() const
        {
            return mDesc.offset;
        }

        inline std::string const& GetTypeName() const
        {
            return mDesc.typeName;
        }

        inline std::vector<HLSLShaderType> const& GetChildren() const
        {
            return mChildren;
        }

        // Print to a text file for human readability.
        void Print(std::ofstream& output, int32_t indent) const;

    private:
        Description mDesc;
        std::string mName;
        std::vector<HLSLShaderType> mChildren;

        // Support for Print.
        static std::string const msVarClass[];
        static std::string const msVarType[];
    };
}
