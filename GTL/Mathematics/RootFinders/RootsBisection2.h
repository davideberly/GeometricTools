// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.20

#pragma once

// Estimate a root to continuous functions F(x,y) and G(x,y) defined on a
// rectangle [xMin,xMax]x[yMin,yMax]. The requirements are that for each
// y' in [yMin,yMax], A(x) = F(x,y') satisfies A(xMin) * A(xMax) < 0,
// which guarantees A(x) has a root. Also, for each x' in [xMin,xMax],
// B(y) = G(x',y) satisfies B(yMin) * B(yMax) < 0, which guarantees B(y)
// has a root. Bisection is performed in the x-direction for A(x). Let
// x' be the root. Bisection is then performed in the y-direction for
// B(y). Let y' be the root. The function value is A(x') = F(x',y').
// This effectively is a bisection of C(x) = F(x,h(x)) along the curve
// where G(x,h(x)) = 0.

#include <GTL/Mathematics/RootFinders/RootsBisection1.h>
#include <cstddef>
#include <functional>

namespace gtl
{
    template <typename T>
    class RootsBisection2
    {
    public:
        // Use this constructor when T is a floating-point type.
        template <typename Numeric = T, IsFPType<Numeric> = 0>
        RootsBisection2(size_t xMaxIterations, size_t yMaxIterations, size_t = 0)
            :
            mXBisector(xMaxIterations),
            mYBisector(yMaxIterations),
            mXRoot(C_<T>(0)),
            mYRoot(C_<T>(0)),
            mFRoot(C_<T>(0)),
            mGRoot(C_<T>(0))
        {
        }

        // Use this constructor when T is an arbitrary-precision type.
        template <typename Numeric = T, IsAPType<Numeric> = 0>
        RootsBisection2(size_t xMaxIterations, size_t yMaxIterations, size_t precision)
            :
            mXBisector(xMaxIterations, precision),
            mYBisector(yMaxIterations, precision),
            mXRoot(C_<T>(0)),
            mYRoot(C_<T>(0)),
            mFRoot(C_<T>(0)),
            mGRoot(C_<T>(0))
        {
        }

        bool operator()(
            std::function<T(T const&, T const&)> const& F,
            std::function<T(T const&, T const&)> const& G, 
            T const& xMin, T const& xMax, T const& yMin, T const& yMax,
            T& xRoot, T& yRoot, T& fRoot, T& gRoot)
        {
            // XFunction(x) = F(x,y), where G(x,y) = 0.
            auto XFunction = [this, &F, &G, &yMin, &yMax](T const& x)
            {
                // YFunction(y) = G(x,y)
                auto YFunction = [this, &G, &x](T const& y)
                {
                    return G(x, y);
                };

                // Bisect in the y-variable to find the root of YFunction(y).
                // If numYIterations is 0, the interval is not guaranteed to
                // bound a root.
                bool hasYRoot = mYBisector(YFunction, yMin, yMax, mYRoot, mGRoot);
                (void)hasYRoot;
                return F(x, mYRoot);
            };

            // Bisect in the x-variable to find the root of XFunction(x).
            // If numYIterations is 0, the interval is not guaranteed to
            // bound a root.
            bool hasXRoot = mXBisector(XFunction, xMin, xMax, mXRoot, mFRoot);
            xRoot = mXRoot;
            yRoot = mYRoot;
            fRoot = mFRoot;
            gRoot = mGRoot;
            return hasXRoot;
        }

    private:
        RootsBisection1<T> mXBisector, mYBisector;
        T mXRoot, mYRoot, mFRoot, mGRoot;

        friend class UnitTestRootsBisection2;
    };
}
