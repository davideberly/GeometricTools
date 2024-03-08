// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// An ordered edge has (V[0], V[1]) = (v0, v1).  An unordered edge has
// (V[0], V[1]) = (min(V[0],V[1]), max(V[0],V[1])).

#include <Mathematics/FeatureKey.h>
#include <cstdint>
#include <type_traits>

namespace gte
{
    template <bool Ordered>
    class EdgeKey : public FeatureKey<2, Ordered>
    {
    public:
        // Initialize to invalid indices.
        EdgeKey()
        {
            this->V = { -1, -1 };
        }

        // This constructor is specialized based on Ordered.
        explicit EdgeKey(int32_t v0, int32_t v1)
        {
            Initialize(v0, v1);
        }

    private:
        template <bool Dummy = Ordered>
        typename std::enable_if<Dummy, void>::type
        Initialize(int32_t v0, int32_t v1)
        {
            this->V[0] = v0;
            this->V[1] = v1;
        }

        template <bool Dummy = Ordered>
        typename std::enable_if<!Dummy, void>::type
        Initialize(int32_t v0, int32_t v1)
        {
            if (v0 < v1)
            {
                // v0 is minimum
                this->V[0] = v0;
                this->V[1] = v1;
            }
            else
            {
                // v1 is minimum
                this->V[0] = v1;
                this->V[1] = v0;
            }
        }
    };
}
