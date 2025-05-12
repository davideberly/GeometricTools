// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/StructuredBuffer.h>
using namespace gte;

StructuredBuffer::StructuredBuffer(uint32_t numElements, size_t elementSize, bool createStorage)
    :
    Buffer(numElements, elementSize, createStorage),
    mCounterType(CounterType::NONE),
    mKeepInternalCount(false)
{
    mType = GT_STRUCTURED_BUFFER;
}

