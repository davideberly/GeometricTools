// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/VertexBuffer.h>
#include <Graphics/GL46/GL46.h>
#include <array>
#include <cstdint>

namespace gte
{
    class GL46InputLayout
    {
    public:
        // Construction and destruction.
        ~GL46InputLayout();
        GL46InputLayout(GLuint programHandle, GLuint vbufferHandle, VertexBuffer const* vbuffer);

        // Support for drawing geometric primitives.
        void Enable();
        void Disable();

    private:
        struct Attribute
        {
            Attribute()
                :
                semantic(VASemantic::NONE),
                numChannels(0),
                channelType(0),
                normalize(0),
                location(0),
                offset(0),
                stride(0)
            {
            }

            VASemantic semantic;
            GLint numChannels;
            GLint channelType;
            GLboolean normalize;
            GLint location;
            GLintptr offset;
            GLsizei stride;
        };

        GLuint mVBufferHandle;
        GLuint mVArrayHandle;
        int32_t mNumAttributes;
        std::array<Attribute, VAConstant::MAX_ATTRIBUTES> mAttributes;

        // Conversions from GTEngine values to GL46 values.
        static GLenum const msChannelType[];
    };
}


