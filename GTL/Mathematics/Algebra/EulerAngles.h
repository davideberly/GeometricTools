// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Representation of a 3x3 rotation matrix by Euler angles. Such a
// representation is not necessarily unique. Let the integer indices for the
// axes be (N0,N1,N2), which must be in the set
//   {(0,1,2),(0,2,1),(1,0,2),(1,2,0),(2,0,1),(2,1,0),
//    (0,1,0),(0,2,0),(1,0,1),(1,2,1),(2,0,2),(2,1,2)}
// Let the corresponding angles be (angleN0,angleN1,angleN2). If the result is
// nonUniqueSum, then the multiple solutions occur because angleN2+angleN0 is
// constant. If the result is nonUniqueDifference, then the multiple solutions
// occur because angleN2-angleN0 is constant. With either type of
// nonuniqueness, the function returns angleN0=0.

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <array>

namespace gtl
{
    template <typename T>
    class EulerAngles
    {
    public:
        using value_type = T;

        // The solution is invalid (incorrect axis indices).
        static size_t constexpr invalid = 0;

        // The solution is unique.
        static size_t constexpr unique = 1;

        // The solution is not unique. A sum of angles is constant.
        static size_t constexpr nonUniqueSum = 2;

        // The solution is not unique. A difference of angles is constant.
        static size_t constexpr nonUniqueDifference = 3;

        // The default constructor produces an invalid object because the
        // axes are all the same.
        EulerAngles()
            :
            axis{ 0, 0, 0 },
            angle{ C_<T>(0), C_<T>(0), C_<T>(0) },
            result(invalid)
        {
        }

        // Create an object for which the factorization order is specified.
        // The caller can adjust angles as needed. Conversion to Euler angles
        // from other rotation representations can use the specified order.
        EulerAngles(size_t i0, size_t i1, size_t i2)
            :
            axis{ i0, i1, i2 },
            angle{ C_<T>(0), C_<T>(0), C_<T>(0) },
            result(unique)
        {
        }

        EulerAngles(size_t i0, size_t i1, size_t i2, T const& a0, T const& a1, T const& a2)
            :
            axis{ i0, i1, i2 },
            angle{ a0, a1, a2 },
            result(unique)
        {
        }

		std::array<size_t, 3> axis;
		std::array<T, 3> angle;

        // This member is set during conversions from rotation matrices,
        // quaternions, or axis-angles.
        size_t result;

    private:
        friend class UnitTestEulerAngles;
    };
}
