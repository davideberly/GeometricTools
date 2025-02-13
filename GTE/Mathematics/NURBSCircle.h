// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.02.05

#pragma once

// The algorithm for representing a circle as a NURBS curve or a sphere as a
// NURBS surface is described in
//   https://www.geometrictools.com/Documentation/NURBSCircleSphere.pdf
// The implementations are related to the documents as shown next.
//   NURBSQuarterCircleDegree2 implements equation (9)
//   NURBSQuarterCircleDegree4 implements equation (10)
//   NURBSHalfCircleDegree3 implements equation (12)
//   NURBSFullCircleDegree3 implements Section 2.3
//   NURBSCircularArcDegree2 implements Section 2.4

#include <Mathematics/NURBSCurve.h>
#include <Mathematics/Arc2.h>
#include <cmath>

namespace gte
{
    template <typename T>
    class NURBSQuarterCircleDegree2 : public NURBSCurve<2, T>
    {
    public:
        // Construction. The quarter circle is x^2 + y^2 = 1 for x >= 0
        // and y >= 0. The direction of traversal is counterclockwise as
        // u increase from 0 to 1.
        NURBSQuarterCircleDegree2()
            :
            NURBSCurve<2, T>(BasisFunctionInput<T>(3, 2), nullptr, nullptr)
        {
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);
            T const sqrt2 = std::sqrt(two);

            this->mWeights[0] = sqrt2;
            this->mWeights[1] = one;
            this->mWeights[2] = sqrt2;

            this->mControls[0] = { one, one };
            this->mControls[1] = { one, one };
            this->mControls[2] = { one, one };
        }
    };

    template <typename T>
    class NURBSQuarterCircleDegree4 : public NURBSCurve<2, T>
    {
    public:
        // Construction. The quarter circle is x^2 + y^2 = 1 for x >= 0
        // and y >= 0. The direction of traversal is counterclockwise as
        // u increases from 0 to 1.
        NURBSQuarterCircleDegree4()
            :
            NURBSCurve<2, T>(BasisFunctionInput<T>(5, 4), nullptr, nullptr)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);
            T const three = static_cast<T>(3);
            T const eight = static_cast<T>(8);
            T const half = static_cast<T>(0.5);
            T const sqrt2 = std::sqrt(two);

            this->mWeights[0] = one;
            this->mWeights[1] = one;
            this->mWeights[2] = two * sqrt2 / three;
            this->mWeights[3] = one;
            this->mWeights[4] = one;

            T const x1 = one;
            T const y1 = half / sqrt2;
            T const x2 = one - sqrt2 / eight;
            this->mControls[0] = { one, zero };
            this->mControls[1] = { x1, y1 };
            this->mControls[2] = { x2, x2 };
            this->mControls[3] = { y1, x1 };
            this->mControls[4] = { zero, one };
        }
    };

    template <typename T>
    class NURBSHalfCircleDegree3 : public NURBSCurve<2, T>
    {
    public:
        // Construction. The half circle is x^2 + y^2 = 1 for x >= 0. The
        // direction of traversal is counterclockwise as u increases from
        // 0 to 1.
        NURBSHalfCircleDegree3()
            :
            NURBSCurve<2, T>(BasisFunctionInput<T>(4, 3), nullptr, nullptr)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);
            T const three = static_cast<T>(3);
            T const negOne = static_cast<T>(-1);
            T const oneThird = one / three;

            this->mWeights[0] = one;
            this->mWeights[1] = oneThird;
            this->mWeights[2] = oneThird;
            this->mWeights[3] = one;

            this->mControls[0] = { one, zero };
            this->mControls[1] = { one, two };
            this->mControls[2] = { negOne, two };
            this->mControls[3] = { negOne, zero };
        }
    };

    template <typename T>
    class NURBSFullCircleDegree3 : public NURBSCurve<2, T>
    {
    public:
        // Construction. The full circle is x^2 + y^2 = 1. The direction of
        // traversal is counterclockwise as u increases from 0 to 1.
        NURBSFullCircleDegree3()
            :
            NURBSCurve<2, T>(CreateBasisFunctionInput(), nullptr, nullptr)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);
            T const three = static_cast<T>(3);
            T const negOne = static_cast<T>(-1);
            T const negTwo = static_cast<T>(-2);
            T const oneThird = one / three;

            this->mWeights[0] = one;
            this->mWeights[1] = oneThird;
            this->mWeights[2] = oneThird;
            this->mWeights[3] = one;
            this->mWeights[4] = oneThird;
            this->mWeights[5] = oneThird;
            this->mWeights[6] = one;

            this->mControls[0] = { one, zero };
            this->mControls[1] = { one, two };
            this->mControls[2] = { negOne, two };
            this->mControls[3] = { negOne, zero };
            this->mControls[4] = { negOne, negTwo };
            this->mControls[5] = { one, negTwo };
            this->mControls[6] = { one, zero };
        }

    private:
        static BasisFunctionInput<T> CreateBasisFunctionInput()
        {
            // We need knots (0,0,0,0,1/2,1/2,1/2,1,1,1,1).
            BasisFunctionInput<T> input{};
            input.numControls = 7;
            input.degree = 3;
            input.uniform = true;
            input.periodic = false;
            input.numUniqueKnots = 3;
            input.uniqueKnots.resize(input.numUniqueKnots);
            input.uniqueKnots[0] = { static_cast<T>(0), 4 };
            input.uniqueKnots[1] = { static_cast<T>(0.5), 3 };
            input.uniqueKnots[2] = { static_cast<T>(1), 4 };
            return input;
        }
    };

    template <typename T>
    class NURBSCircularArcDegree2 : public NURBSCurve<2, T>
    {
    public:
        NURBSCircularArcDegree2(Arc2<T> const& arc)
            :
            NURBSCurve<2, T>(BasisFunctionInput<T>(3, 2), nullptr, nullptr)
        {
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);

            Vector2<T> P0 = (arc.end[0] - arc.center) / arc.radius;
            Vector2<T> P2 = (arc.end[1] - arc.center) / arc.radius;
            Vector2<T> P1 = Perp(P2 - P0) / DotPerp(P0, P2);

            this->mWeights[0] = std::sqrt(two * (Dot(P1, P1) - one) / (one - Dot(P0, P2)));
            this->mWeights[1] = one;
            this->mWeights[2] = this->mWeights[0];

            this->mControls[0] = arc.center + arc.radius * P0;
            this->mControls[1] = arc.center + arc.radius * P1;
            this->mControls[2] = arc.center + arc.radius * P2;
        }
    };
}
