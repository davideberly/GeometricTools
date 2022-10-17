// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// The algorithm for representing a circle as a NURBS curve or a sphere as a
// NURBS surface is described in
//   https://www.geometrictools.com/Documentation/NURBSCircleSphere.pdf
// The implementations are related to the documents as shown next.
//   NURBSQuarterCircleDegree2 implements equation (9)
//   NURBSQuarterCircleDegree4 implements equation (10)
//   NURBSHalfCircleDegree3 implements equation (12)
//   NURBSFullCircleDegree3 implements Section 2.3

#include <GTL/Mathematics/Curves/NURBSCurve.h>

namespace gtl
{
    template <typename T>
    class NURBSQuarterCircleDegree2 : public NURBSCurve<T, 2>
    {
    public:
        // Construction. The quarter circle is x^2 + y^2 = 1 for x >= 0
        // and y >= 0. The direction of traversal is counterclockwise as
        // u increase from 0 to 1.
        NURBSQuarterCircleDegree2()
            :
            NURBSCurve<T, 2>(typename BasisFunction<T>::Input(3, 2), nullptr, nullptr)
        {
            this->mWeights[0] = C_SQRT_2<T>;
            this->mWeights[1] = C_<T>(1);
            this->mWeights[2] = C_SQRT_2<T>;

            this->mControls[0] = { C_<T>(1), C_<T>(0) };
            this->mControls[1] = { C_<T>(1), C_<T>(1) };
            this->mControls[2] = { C_<T>(0), C_<T>(1) };
        }
    };

    template <typename T>
    class NURBSQuarterCircleDegree4 : public NURBSCurve<T, 2>
    {
    public:
        // Construction. The quarter circle is x^2 + y^2 = 1 for x >= 0
        // and y >= 0. The direction of traversal is counterclockwise as
        // u increases from 0 to 1.
        NURBSQuarterCircleDegree4()
            :
            NURBSCurve<T, 2>(typename BasisFunction<T>::Input(5, 4), nullptr, nullptr)
        {
            this->mWeights[0] = C_<T>(1);
            this->mWeights[1] = C_<T>(1);
            this->mWeights[2] = C_<T>(2, 3) * C_SQRT_2<T>;
            this->mWeights[3] = C_<T>(1);
            this->mWeights[4] = C_<T>(1);

            T const x1 = C_<T>(1);
            T const y1 = C_<T>(1, 2) * C_INV_SQRT_2<T>;
            T const x2 = C_<T>(1) - C_SQRT_2<T> * C_<T>(1, 8);
            this->mControls[0] = { C_<T>(1), C_<T>(0) };
            this->mControls[1] = { x1, y1 };
            this->mControls[2] = { x2, x2 };
            this->mControls[3] = { y1, x1 };
            this->mControls[4] = { C_<T>(0), C_<T>(1) };
        }
    };

    template <typename T>
    class NURBSHalfCircleDegree3 : public NURBSCurve<T, 2>
    {
    public:
        // Construction. The half circle is x^2 + y^2 = 1 for x >= 0. The
        // direction of traversal is counterclockwise as u increases from
        // 0 to 1.
        NURBSHalfCircleDegree3()
            :
            NURBSCurve<T, 2>(typename BasisFunction<T>::Input(4, 3), nullptr, nullptr)
        {
            this->mWeights[0] = C_<T>(1);
            this->mWeights[1] = C_<T>(1, 3);
            this->mWeights[2] = C_<T>(1, 3);
            this->mWeights[3] = C_<T>(1);

            this->mControls[0] = { C_<T>(1), C_<T>(0) };
            this->mControls[1] = { C_<T>(1), C_<T>(2) };
            this->mControls[2] = { -C_<T>(1), C_<T>(2) };
            this->mControls[3] = { -C_<T>(1), C_<T>(0) };
        }
    };

    template <typename T>
    class NURBSFullCircleDegree3 : public NURBSCurve<T, 2>
    {
    public:
        // Construction. The full circle is x^2 + y^2 = 1. The direction of
        // traversal is counterclockwise as u increases from 0 to 1.
        NURBSFullCircleDegree3()
            :
            NURBSCurve<T, 2>(CreateBasisFunctionInput(), nullptr, nullptr)
        {
            this->mWeights[0] = C_<T>(1);
            this->mWeights[1] = C_<T>(1, 3);
            this->mWeights[2] = C_<T>(1, 3);
            this->mWeights[3] = C_<T>(1);
            this->mWeights[4] = C_<T>(1, 3);
            this->mWeights[5] = C_<T>(1, 3);
            this->mWeights[6] = C_<T>(1);

            this->mControls[0] = { C_<T>(1), C_<T>(0) };
            this->mControls[1] = { C_<T>(1), C_<T>(2) };
            this->mControls[2] = { -C_<T>(1), C_<T>(2) };
            this->mControls[3] = { -C_<T>(1), C_<T>(0) };
            this->mControls[4] = { -C_<T>(1), -C_<T>(2) };
            this->mControls[5] = { C_<T>(1), -C_<T>(2) };
            this->mControls[6] = { C_<T>(1), C_<T>(0) };
        }

    private:
        static typename BasisFunction<T>::Input CreateBasisFunctionInput()
        {
            using Input = typename BasisFunction<T>::Input;
            using UniqueKnot = typename BasisFunction<T>::UniqueKnot;

            Input input{};
            input.numControls = 7;
            input.degree = 3;
            input.uniform = true;
            input.periodic = false;
            input.uniqueKnots.resize(3);
            input.uniqueKnots[0] = UniqueKnot(C_<T>(0), 4);
            input.uniqueKnots[1] = UniqueKnot(C_<T>(1, 2), 3);
            input.uniqueKnots[2] = UniqueKnot(C_<T>(1), 4);
            return input;
        }
    };
}
