// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The TVector template parameter allows you to create solvers with
// Vector<N,Real> when the dimension N is known at compile time or
// GVector<Real> when the dimension N is known at run time.  Both classes
// have 'int32_t GetSize() const' that allow OdeSolver-derived classes to query
// for the dimension.

#include <Mathematics/OdeSolver.h>
#include <functional>

namespace gte
{
    template <typename Real, typename TVector>
    class OdeEuler : public OdeSolver<Real, TVector>
    {
    public:
        // Construction and destruction.
        virtual ~OdeEuler() = default;

        OdeEuler(Real tDelta, std::function<TVector(Real, TVector const&)> const& F)
            :
            OdeSolver<Real, TVector>(tDelta, F)
        {
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x).  You may
        // allow xIn and xOut to be the same object.
        virtual void Update(Real tIn, TVector const& xIn, Real& tOut, TVector& xOut) override
        {
            TVector fVector = this->mFunction(tIn, xIn);
            tOut = tIn + this->mTDelta;
            xOut = xIn + this->mTDelta * fVector;
        }
    };
}
