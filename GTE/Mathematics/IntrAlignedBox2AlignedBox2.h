// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the box to be a solid.
//
// The aligned-aligned queries use simple min-max comparisions.  The
// interesection of aligned boxes is an aligned box, possibly degenerate,
// where min[d] == max[d] for at least one dimension d.

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/AlignedBox.h>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, AlignedBox2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(AlignedBox2<T> const& box0, AlignedBox2<T> const& box1)
        {
            Result result{};
            for (int32_t i = 0; i < 2; i++)
            {
                if (box0.max[i] < box1.min[i] || box0.min[i] > box1.max[i])
                {
                    result.intersect = false;
                    return result;
                }
            }
            result.intersect = true;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, AlignedBox2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                box{}
            {
            }

            bool intersect;
            AlignedBox2<T> box;
        };

        Result operator()(AlignedBox2<T> const& box0, AlignedBox2<T> const& box1)
        {
            Result result{};
            for (int32_t i = 0; i < 2; i++)
            {
                if (box0.max[i] < box1.min[i] || box0.min[i] > box1.max[i])
                {
                    result.intersect = false;
                    return result;
                }
            }

            for (int32_t i = 0; i < 2; i++)
            {
                if (box0.max[i] <= box1.max[i])
                {
                    result.box.max[i] = box0.max[i];
                }
                else
                {
                    result.box.max[i] = box1.max[i];
                }

                if (box0.min[i] <= box1.min[i])
                {
                    result.box.min[i] = box1.min[i];
                }
                else
                {
                    result.box.min[i] = box0.min[i];
                }
            }
            result.intersect = true;
            return result;
        }
    };
}
