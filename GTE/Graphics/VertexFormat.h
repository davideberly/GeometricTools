// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/DataFormat.h>
#include <cstdint>

namespace gte
{
    // Enumerations for DX11.  TODO: Add a GLSL subsystem to allow
    // hooking up 'location' to the semantic.
    enum VASemantic
    {
        NONE,
        POSITION,
        BLENDWEIGHT,
        BLENDINDICES,
        NORMAL,
        PSIZE,
        TEXCOORD,
        TANGENT,
        BINORMAL,
        TESSFACTOR,
        POSITIONT,
        COLOR,
        FOG,
        DEPTH,
        SAMPLE,
        NUM_SEMANTICS
    };

    enum VAConstant
    {
        // TODO:  Modify to the numbers for Shader Model 5 (DX11).

        // The maximum number of attributes for a vertex format.
        MAX_ATTRIBUTES = 16,

        // The maximum number of texture coordinate units.
        MAX_TCOORD_UNITS = 8,

        // The maximum number of color units.
        MAX_COLOR_UNITS = 2
    };

    class VertexFormat
    {
    public:
        // Construction.
        VertexFormat();

        // Support for reusing a VertexFormat object within a scope.  This
        // call resets the object to the state produced by the default
        // constructor call.
        void Reset();

        // Create a packed vertex format, where all attributes are contiguous
        // in memory.  The order of the attributes is determined by the order
        // of Bind calls.
        void Bind(VASemantic semantic, DFType type, uint32_t unit);

        // Member access.  GetAttribute returns 'true' when the input i is
        // such that 0 <= i < GetNumAttributes(), in which case the returned
        // semantic, type, unit, and offset are valid.
        inline uint32_t GetVertexSize() const
        {
            return mVertexSize;
        }

        inline int32_t GetNumAttributes() const
        {
            return mNumAttributes;
        }

        void GetAttribute(int32_t i, VASemantic& semantic, DFType& type,
            uint32_t& unit, uint32_t& offset) const;

        // Determine whether a semantic/unit exists.  If so, return the
        // index i that can be used to obtain more information about the
        // attribute by the functions after this.  If not, return -1.
        int32_t GetIndex(VASemantic semantic, uint32_t unit) const;
        DFType GetType(int32_t i) const;
        uint32_t GetOffset(int32_t i) const;

    private:
        class Attribute
        {
        public:
            Attribute()
                :
                semantic(VASemantic::NONE),
                type(DF_UNKNOWN),
                unit(0),
                offset(0)
            {
            }

            VASemantic semantic;
            DFType type;
            uint32_t unit;
            uint32_t offset;
        };

        int32_t mNumAttributes;
        uint32_t mVertexSize;
        std::array<Attribute, VAConstant::MAX_ATTRIBUTES> mAttributes;
    };
}
