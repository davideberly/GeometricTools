// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/VertexFormat.h>
#include <Mathematics/Logger.h>
using namespace gte;

VertexFormat::VertexFormat()
    :
    mNumAttributes(0),
    mVertexSize(0),
    mAttributes{}
{
}

void VertexFormat::Reset()
{
    mNumAttributes = 0;
    mVertexSize = 0;
    for (int32_t i = 0; i < VAConstant::MAX_ATTRIBUTES; ++i)
    {
        mAttributes[i] = Attribute();
    }
}

void VertexFormat::Bind(VASemantic semantic, DFType type, uint32_t unit)
{
    // Validate the inputs.
    LogAssert(0 <= mNumAttributes && mNumAttributes < VAConstant::MAX_ATTRIBUTES, "Exceeded maximum attributes.");
    if (semantic == VASemantic::COLOR)
    {
        LogAssert(unit < VAConstant::MAX_COLOR_UNITS, "Invalid color unit.");
    }
    else if (semantic == VASemantic::TEXCOORD)
    {
        LogAssert(unit < VAConstant::MAX_TCOORD_UNITS, "Invalid texture unit.");
    }
    else
    {
        LogAssert(unit == 0, "Invalid semantic unit.");
    }

    // Accept the attribute.
    Attribute& attribute = mAttributes[mNumAttributes];
    attribute.semantic = semantic;
    attribute.type = type;
    attribute.unit = unit;
    attribute.offset = mVertexSize;
    ++mNumAttributes;

    // Advance the offset.
    mVertexSize += DataFormat::GetNumBytesPerStruct(type);
}

void VertexFormat::GetAttribute(int32_t i, VASemantic& semantic, DFType& type,
    uint32_t& unit, uint32_t& offset) const
{
    LogAssert(
        0 <= i && i < mNumAttributes,
        "Invalid index " + std::to_string(i) + ".");

    Attribute const& attribute = mAttributes[i];
    semantic = attribute.semantic;
    type = attribute.type;
    unit = attribute.unit;
    offset = attribute.offset;
}

int32_t VertexFormat::GetIndex(VASemantic semantic, uint32_t unit) const
{
    for (int32_t i = 0; i < mNumAttributes; ++i)
    {
        Attribute const& attribute = mAttributes[i];
        if (attribute.semantic == semantic && attribute.unit == unit)
        {
            return i;
        }
    }

    return -1;
}

DFType VertexFormat::GetType(int32_t i) const
{
    LogAssert(
        0 <= i && i < mNumAttributes,
        "Invalid index " + std::to_string(i) + ".");

    return mAttributes[i].type;
}

uint32_t VertexFormat::GetOffset(int32_t i) const
{
    LogAssert(
        0 <= i && i < mNumAttributes,
        "Invalid index " + std::to_string(i) + ".");

    return mAttributes[i].offset;
}
