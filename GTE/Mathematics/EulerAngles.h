// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The Euler angle data structure for representing rotations.  See the
// document
//   https://www.geometrictools.com/Documentation/EulerAngles.pdf

#include <Mathematics/Vector.h>
#include <array>
#include <cstdint>

namespace gte
{
    // Factorization into Euler angles is not necessarily unique. Let the
    // integer indices for the axes be (N0,N1,N2), which must be in the set
    //   {(0,1,2),(0,2,1),(1,0,2),(1,2,0),(2,0,1),(2,1,0),
    //    (0,1,0),(0,2,0),(1,0,1),(1,2,1),(2,0,2),(2,1,2)}
    // Let the corresponding angles be (angleN0,angleN1,angleN2). If the
    // result is NOT_UNIQUE_SUM, then the multiple solutions occur because
    // angleN2+angleN0 is constant. If the result is NOT_UNIQUE_DIF, then
    // the multiple solutions occur because angleN2-angleN0 is constant.
    // In either type of nonuniqueness, the function returns angleN0=0.
    enum class EulerResult
    {
        // The solution is invalid (incorrect axis indices).
        INVALID,

        // The solution is unique.
        UNIQUE,

        // The solution is not unique.  A sum of angles is constant.
        NOT_UNIQUE_SUM,

        // The solution is not unique.  A difference of angles is constant.
        NOT_UNIQUE_DIF
    };

    template <typename Real>
    class EulerAngles
    {
    public:

        EulerAngles()
            :
            axis{0, 0, 0},
            angle{ (Real)0, (Real)0, (Real)0 },
            result(EulerResult::INVALID)
        {
        }

        EulerAngles(int32_t i0, int32_t i1, int32_t i2, Real a0, Real a1, Real a2)
            :
            axis{ i0, i1, i2 },
            angle{ a0, a1, a2 },
            result(EulerResult::UNIQUE)
        {
        }

        std::array<int32_t, 3> axis;
        std::array<Real, 3> angle;

        // This member is set during conversions from rotation matrices,
        // quaternions, or axis-angles.
        EulerResult result;
    };
}
