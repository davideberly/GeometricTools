// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/Font.h>

namespace gte
{
    class FontArialW400H12 : public Font
    {
    public:
        virtual ~FontArialW400H12() = default;
        FontArialW400H12(std::shared_ptr<ProgramFactory> const& factory, int32_t maxMessageLength);

    private:
        static int32_t msWidth;
        static int32_t msHeight;
        static uint8_t msTexels[];
        static float msCharacterData[];
    };
}

