#pragma once

#include <array>
#include <functional>
#include <limits>

namespace gte
{

template <typename Real>
class DualNumber
{
public:
    // construction and destruction
    ~DualNumber();
    DualNumber();
    DualNumber(DualNumber const& n);
    DualNumber(Real x0);
    DualNumber(Real x0, Real x1);

    // assignment
    DualNumber& operator=(DualNumber const& n);

    // member access
    inline Real& operator[](int i);
    inline Real const& operator[](int i) const;

    // comparisons for sorted containers and geometric ordering
    inline bool operator==(DualNumber const& n) const;
    inline bool operator!=(DualNumber const& n) const;
    inline bool operator< (DualNumber const& n) const;
    inline bool operator<=(DualNumber const& n) const;
    inline bool operator> (DualNumber const& n) const;
    inline bool operator>=(DualNumber const& n) const;

private:
    // (x0,x1) representing x0 + x1*e, where e != 0 but e*e = 0
    std::array<Real, 2> mTuple;
};

// unary operators
template <typename Real> DualNumber<Real> operator+(DualNumber<Real> const& n);
template <typename Real> DualNumber<Real> operator-(DualNumber<Real> const& n);

// binary arithmetic operators
template <typename Real> DualNumber<Real> operator+(DualNumber<Real> const& n0, DualNumber<Real> const& n1);
template <typename Real> DualNumber<Real> operator-(DualNumber<Real> const& n0, DualNumber<Real> const& n1);
template <typename Real> DualNumber<Real> operator*(DualNumber<Real> const& n0, DualNumber<Real> const& n1);
template <typename Real> DualNumber<Real> operator/(DualNumber<Real> const& n0, DualNumber<Real> const& n1);
template <typename Real> DualNumber<Real>& operator+=(DualNumber<Real>& n0, DualNumber<Real> const& n1);
template <typename Real> DualNumber<Real>& operator-=(DualNumber<Real>& n0, DualNumber<Real> const& n1);
template <typename Real> DualNumber<Real>& operator*=(DualNumber<Real>& n0, DualNumber<Real> const& n1);
template <typename Real> DualNumber<Real>& operator/=(DualNumber<Real>& n0, DualNumber<Real> const& n1);

// standard mathematical functions
template <typename Real> DualNumber<Real> sin(DualNumber<Real> const& n);
template <typename Real> DualNumber<Real> cos(DualNumber<Real> const& n);
template <typename Real> DualNumber<Real> exp(DualNumber<Real> const& n);

template <typename Real, typename Functor>
class AutoDifferentiator
{
public:
    AutoDifferentiator(Functor const& functor) : mFunctor(functor) {}
    AutoDifferentiator(AutoDifferentiator const&) = delete;
    AutoDifferentiator& operator=(AutoDifferentiator const&) = delete;

    void operator()(Real x, Real& F, Real& DF) const
    {
        DualNumber<Real> y = mFunctor(DualNumber<Real>(x, (Real)1));
        F = y[0];
        DF = y[1];
    }

private:
    Functor const& mFunctor;
};

//----------------------------------------------------------------------------
template <typename Real>
DualNumber<Real>::~DualNumber()
{
}

template <typename Real>
DualNumber<Real>::DualNumber()
    :
    mTuple{ (Real)0, (Real)0 }
{
}

template <typename Real>
DualNumber<Real>::DualNumber(DualNumber const& n)
    :
    mTuple(n.mTuple)
{
}

template <typename Real>
DualNumber<Real>::DualNumber(Real x0)
    :
    mTuple{ x0, (Real)0 }
{
}

template <typename Real>
DualNumber<Real>::DualNumber(Real x0, Real x1)
    :
    mTuple{ x0, x1 }
{
}

template <typename Real>
DualNumber<Real>& DualNumber<Real>::operator=(DualNumber const& n)
{
    mTuple = n.mTuple;
    return *this;
}

template <typename Real>
inline Real& DualNumber<Real>::operator[](int i)
{
    return mTuple[i];
}

template <typename Real>
inline Real const& DualNumber<Real>::operator[](int i) const
{
    return mTuple[i];
}

template <typename Real>
inline bool DualNumber<Real>::operator==(DualNumber const& n) const
{
    return mTuple == n.mTuple;
}

template <typename Real>
inline bool DualNumber<Real>::operator!=(DualNumber const& n) const
{
    return mTuple != n.mTuple;
}

template <typename Real>
inline bool DualNumber<Real>::operator< (DualNumber const& n) const
{
    return mTuple < n.mTuple;
}

template <typename Real>
inline bool DualNumber<Real>::operator<=(DualNumber const& n) const
{
    return mTuple <= n.mTuple;
}

template <typename Real>
inline bool DualNumber<Real>::operator> (DualNumber const& n) const
{
    return mTuple > n.mTuple;
}

template <typename Real>
inline bool DualNumber<Real>::operator>=(DualNumber const& n) const
{
    return mTuple >= n.mTuple;
}

template <typename Real>
DualNumber<Real> operator+(DualNumber<Real> const& n)
{
    return n;
}

template <typename Real>
DualNumber<Real> operator-(DualNumber<Real> const& n)
{
    DualNumber<Real> negate{ -n[0], -n[1] };
    return negate;
}

template <typename Real>
DualNumber<Real> operator+(DualNumber<Real> const& n0, DualNumber<Real> const& n1)
{
    DualNumber<Real> result = n0;
    return result += n1;
}

template <typename Real>
DualNumber<Real> operator-(DualNumber<Real> const& n0, DualNumber<Real> const& n1)
{
    DualNumber<Real> result = n0;
    return result -= n1;
}

template <typename Real>
DualNumber<Real> operator*(DualNumber<Real> const& n0, DualNumber<Real> const& n1)
{
    DualNumber<Real> result = n0;
    return result *= n1;
}

template <typename Real>
DualNumber<Real> operator/(DualNumber<Real> const& n0, DualNumber<Real> const& n1)
{
    DualNumber<Real> result = n0;
    return result /= n1;
}

template <typename Real>
DualNumber<Real>& operator+=(DualNumber<Real>& n0, DualNumber<Real> const& n1)
{
    n0[0] += n1[0];
    n0[1] += n1[1];
    return n0;
}

template <typename Real>
DualNumber<Real>& operator-=(DualNumber<Real>& n0, DualNumber<Real> const& n1)
{
    n0[0] -= n1[0];
    n0[1] -= n1[1];
    return n0;
}

template <typename Real>
DualNumber<Real>& operator*=(DualNumber<Real>& n0, DualNumber<Real> const& n1)
{
    Real x0 = n0[0];
    n0[0] *= n1[0];
    n0[1] = n0[1] * n1[0] + x0 * n1[1];
    return n0;
}

template <typename Real>
DualNumber<Real>& operator/=(DualNumber<Real>& n0, DualNumber<Real> const& n1)
{
    Real x0 = n0[0];
    n0[0] /= n1[0];
    n0[1] = (n0[1] * n1[0] - x0 * n1[1]) / (n1[0] * n1[0]);
    return n0;
}

template <typename Real>
DualNumber<Real> sin(DualNumber<Real> const& n)
{
    DualNumber<Real> result{ ::sin(n[0]), n[1] * ::cos(n[0]) };
    return result;
}

template <typename Real>
DualNumber<Real> cos(DualNumber<Real> const& n)
{
    DualNumber<Real> result{ ::cos(n[0]), -n[1] * ::sin(n[0]) };
    return result;
}

template <typename Real>
DualNumber<Real> exp(DualNumber<Real> const& n)
{
    Real z = ::exp(n[0]);
    DualNumber<Real> result{ z, n[1] * z };
    return result;
}

}
