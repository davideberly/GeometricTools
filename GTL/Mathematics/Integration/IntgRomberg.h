// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.12.16

#pragma once

// An implementation of Romberg integration.  The trapezoid rule is used to
// generate initial estimates, but then Richardson extrapolation is used to
// improve the estimates. This method is preferred over trapezoid rule. The
// order must be positive.

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <GTL/Utility/Exceptions.h>
#include <array>
#include <cstddef>
#include <functional>
#include <vector>

namespace gtl
{
    template <typename T>
    class IntgRomberg
    {
    public:
        static T Integrate(size_t order, T const& a, T const& b,
            std::function<T(T)> const& integrand)
        {
            GTL_ARGUMENT_ASSERT(
                order > 0,
                "The order must be positive.");

            std::vector<std::array<T, 2>> rom(order);
            T h = b - a;
            rom[0][0] = C_<T>(1, 2) * h * (integrand(a) + integrand(b));
            for (size_t i0 = 2, p0 = 1; i0 <= order; ++i0, p0 *= 2, h *= C_<T>(1, 2))
            {
                // Approximations via the trapezoid rule.
                T sum = C_<T>(0);
                for (size_t i1 = 1; i1 <= p0; ++i1)
                {
                    sum += integrand(a + h * (static_cast<T>(i1) - C_<T>(1, 2)));
                }

                // Richardson extrapolation.
                rom[0][1] = C_<T>(1, 2) * (rom[0][0] + h * sum);
                T p2 = C_<T>(4);
                for (size_t i2 = 1; i2 < i0; ++i2, p2 *= C_<T>(4))
                {
                    rom[i2][1] = (p2 * rom[i2 - 1][1] - rom[i2 - 1][0]) / (p2 - C_<T>(1));
                }

                for (size_t i1 = 0; i1 < i0; ++i1)
                {
                    rom[i1][0] = rom[i1][1];
                }
            }

            T result = rom[order - 1][0];
            return result;
        }

    private:
        friend class UnitTestIntgRomberg;
    };
}
