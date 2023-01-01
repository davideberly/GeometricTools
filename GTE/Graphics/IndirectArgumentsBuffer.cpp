// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/IndirectArgumentsBuffer.h>
using namespace gte;

IndirectArgumentsBuffer::IndirectArgumentsBuffer(uint32_t numElements, bool createStorage)
    :
    Buffer(numElements, 4, createStorage)
{
    mType = GT_INDIRECT_ARGUMENTS_BUFFER;
}
