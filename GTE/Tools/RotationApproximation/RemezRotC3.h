// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include "RemezConstrained.h"

namespace gte
{
    class RemezRotC3 : public RemezConstrained
    {
    public:
        RemezRotC3();

    protected:
        virtual double F(double t) const override;

        virtual double FDer(double t) const override;

        virtual void ComputeACoefficients() override;

        virtual void ComputeBCoefficients() override;

        virtual Interval GetIntervalF(double t, Interval const& iCos,
            Interval const& iSin) const override;

        virtual Interval GetIntervalG(double t, Interval const& iCos,
            Interval const& iSin) const override;

        virtual void GetFirstSignETerm(size_t j, Rational const& factor,
            Rational& term) const override;

        virtual void GetSecondSignETerm(size_t j, Rational const& tSqr,
            Rational& factor, Rational& term) const override;

        virtual void GetFirstSignDTerm(size_t j, Rational const& factor,
            Rational& term) const override;

        virtual void GetSecondSignDTerm(size_t j, Rational const& tSqr,
            Rational& factor, Rational& term) const override;
    };
}

