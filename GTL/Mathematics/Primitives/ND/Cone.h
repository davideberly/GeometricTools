// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// An infinite cone is defined by a vertex V, a unit-length direction D and an
// angle A with 0 < A < pi/2. A point X is on the cone when
//   Dot(D, X - V) = |X - V| * cos(A)
// A solid cone includes points on the cone and in the region that contains
// the cone ray V + h * D for h >= 0. It is defined by
//   Dot(D, X - V) >= |X - V| * cos(A)
// The height of any point Y in space relative to the cone is defined by
// h = Dot(D, Y - V), which is the signed length of the projection of X - V
// onto the cone axis. Observe that we have restricted the cone definition to
// an acute angle A, so |X - V| * cos(A) >= 0; therefore, points on or inside
// the cone have nonnegative heights: Dot(D, X - V) >= 0. I will refer to the
// infinite solid cone as the "positive cone," which means that the non-vertex
// points inside the cone have positive heights. Although rare in computer
// graphics, one might also want to consider the "negative cone," which is
// defined by
//   -Dot(D, X - V) <= -|X - V| * cos(A)
// The non-vertex points inside this cone have negative heights.
//
// For many of the geometric queries involving cones, we can avoid the square
// root computation implied by |X - V|. The positive cone is defined by
//   Dot(D, X - V)^2 >= |X - V|^2 * cos(A)^2,
// which is a quadratic inequality, but the squaring of the terms leads to an
// inequality that includes points X in the negative cone. When using the
// quadratic inequality for the positive cone, we need to include also the
// constraint Dot(D, X - V) >= 0.
//
// I define four different types of cones. They all involve V, D and A. The
// differences are based on restrictions to the heights of the cone points.
// The height range is defined to be the interval of possible heights, say,
// [hmin,hmax] with 0 <= hmin < hmax <= infinity.
//     1. infinite cone: hmin = 0, hmax = infinity
//     2. infinite truncated cone:  hmin > 0, hmax = infinity
//     3. finite cone:  hmin >= 0, hmax < infinity
//     4. frustum of a cone:  hmin > 0, hmax < infinity
// The infinite truncated cone is truncated for h-minimum; the radius of the
// disk at h-minimum is rmin = hmin * tan(A). The finite cone is truncated for
// h-maximum; the radius of the disk at h-maximum is rmax = hmax * tan(A).
// The frustum of a cone is truncated both for h-minimum and h-maximum.
//
// A technical problem when creating a data structure to represent a cone is
// deciding how to represent infinity in the height range. When the template
// type T is 'float' or 'double', we could represent it as
// std::numeric_limits<T>::infinity(). The geometric queries must be
// structured properly to conform to the semantics associated with the
// floating-point infinity. We could also use the largest finite
// floating-point number, std::numeric_limits<T>::max(). Either choice is
// problematic when instead T is an arbitrary precision type that does not
// have a representation for infinity; this is the case for the types
// BSNumber<U> and BSRational<U>, where U is UIntegerAP32 or UIntegerFP32<N>.
//
// The introduction of representations of infinities for the arbitrary
// precision types would require modifying the arithmetic operations to test
// whether the number is finite or infinite. This leads to a greater
// computational cost for all queries, even when those queries do not require
// manipulating infinities. In the case of a cone, the height manipulations
// are nearly always for comparisons of heights. I choose to represent
// infinity by setting the maxHeight member to -1. The member functions
// IsFinite() and IsInfinite() compare maxHeight to -1 and report the correct
// state.
//
// My choice of representation has the main consequence that comparisons
// between heights requires extra logic. This can make geometric queries
// cumbersome to implement. For example, the point-in-cone test using the
// quadratic inequality is shown in the pseudocode
//   Vector point = <some point>;
//   Cone cone = <some cone>;
//   Vector delta = point - cone.V;
//   T h = Dot(cone.D, delta);
//   bool pointInCone =
//       cone.hmin <= h &&
//       h <= cone.hmax &&
//       h * h >= Dot(delta, delta) * cone.cosAngleSqr;
// In the event the cone is infinite and we choose cone.hmax = -1 to
// represent this, the test 'h <= cone.hmax' must be revised,
//   bool pointInCone =
//       cone.hmin <= h &&
//       (cone.hmax == -1 ? true : (h <= cone.hmax)) &&
//       h * h >= Dot(delta, delta) * cone.cosAngleSqr;
// To encapsulate the comparisons against height extremes, use the member
// function HeightInRange(h); that is
//   bool pointInCone =
//       cone.HeightInRange(h) &&
//       h * h >= Dot(delta, delta) * cone.cosAngleSqr;
// The modification is not that complicated here, but consider a more
// sophisticated query such as determining the interval of intersection
// of two height intervals [h0,h1] and [cone.hmin,cone.hmax]. The file
// GteIntrIntervals.h provides implementations for computing the
// intersection of two intervals, where either or both intervals are
// semi-infinite.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <GTL/Utility/Exceptions.h>
#include <cmath>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Cone
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Cone()
            :
            vertex{},
            direction{},
            angle(C_<T>(0)),
            cosAngle(C_<T>(0)),
            sinAngle(C_<T>(0)),
            tanAngle(C_<T>(0)),
            cosAngleSqr(C_<T>(0)),
            sinAngleSqr(C_<T>(0)),
            invSinAngle(C_<T>(0)),
            mMinHeight(C_<T>(0)),
            mMaxHeight(C_<T>(0))
        {
        }

        // Create an infinite cone with the specified vertex, axis direction,
        // angle and with minimum height 0 and maximum height infinity.
        Cone(Vector<T, N> const& inVertex, Vector<T, N> const& inDirection,
            T const& inAngle)
            :
            vertex(inVertex),
            direction(inDirection),
            angle(C_<T>(0)),
            cosAngle(C_<T>(0)),
            sinAngle(C_<T>(0)),
            tanAngle(C_<T>(0)),
            cosAngleSqr(C_<T>(0)),
            sinAngleSqr(C_<T>(0)),
            invSinAngle(C_<T>(0)),
            mMinHeight(C_<T>(0)),
            mMaxHeight(C_<T>(0))
        {
            SetAngle(inAngle);
            MakeInfiniteCone();
        }

        // Create an infinite truncated cone with the specified vertex, axis
        // direction, angle and positive minimum height. The maximum height
        // is infinity. If you specify a minimum height of 0, you get the 
        // equivalent of calling the constructor for an infinite cone.
        Cone(Vector<T, N> const& inVertex, Vector<T, N> const& inDirection,
            T const& inAngle, T const& inMinHeight)
            :
            vertex(inVertex),
            direction(inDirection),
            angle(C_<T>(0)),
            cosAngle(C_<T>(0)),
            sinAngle(C_<T>(0)),
            tanAngle(C_<T>(0)),
            cosAngleSqr(C_<T>(0)),
            sinAngleSqr(C_<T>(0)),
            invSinAngle(C_<T>(0)),
            mMinHeight(C_<T>(0)),
            mMaxHeight(C_<T>(0))
        {
            SetAngle(inAngle);
            MakeInfiniteTruncatedCone(inMinHeight);
        }

        // Create a finite cone or a frustum of a cone with all parameters
        // specified. If you specify a minimum height of 0, you get a finite
        // cone. If you specify a positive minimum height, you get a frustum
        // of a cone.
        Cone(Vector<T, N> const& inVertex, Vector<T, N> const& inDirection,
            T const& inAngle, T const& inMinHeight, T const& inMaxHeight)
            :
            vertex(inVertex),
            direction(inDirection),
            angle(C_<T>(0)),
            cosAngle(C_<T>(0)),
            sinAngle(C_<T>(0)),
            tanAngle(C_<T>(0)),
            cosAngleSqr(C_<T>(0)),
            sinAngleSqr(C_<T>(0)),
            invSinAngle(C_<T>(0)),
            mMinHeight(C_<T>(0)),
            mMaxHeight(C_<T>(0))
        {
            SetAngle(inAngle);
            MakeConeFrustum(inMinHeight, inMaxHeight);
        }

        // The angle must be in (0,pi/2). The function sets 'angle' and
        // computes 'cosAngle', 'sinAngle', 'tanAngle', 'cosAngleSqr',
        // 'sinAngleSqr' and 'invSinAngle'.
        void SetAngle(T const& inAngle)
        {
            GTL_DOMAIN_ASSERT(
                C_<T>(0) < inAngle && inAngle < C_PI_DIV_2<T>,
                "The angle must be in (0,pi/2).");

            angle = inAngle;
            cosAngle = std::cos(angle);
            sinAngle = std::sin(angle);
            tanAngle = std::tan(angle);
            cosAngleSqr = cosAngle * cosAngle;
            sinAngleSqr = sinAngle * sinAngle;
            invSinAngle = C_<T>(1) / sinAngle;
        }

        // Set the heights to obtain one of the four types of cones. Be aware
        // that an infinite cone has maxHeight set to -1. Be careful not to
        // use maxHeight without understanding this interpretation.
        inline T Infinity() const
        {
            return -C_<T>(1);
        }

        void MakeInfiniteCone()
        {
            mMinHeight = C_<T>(0);
            mMaxHeight = Infinity();
        }

        void MakeInfiniteTruncatedCone(T const& inMinHeight)
        {
            GTL_DOMAIN_ASSERT(
                inMinHeight >= C_<T>(0),
                "The minimum height must be nonnegative.");

            mMinHeight = inMinHeight;
            mMaxHeight = Infinity();
        }

        void MakeFiniteCone(T const& inMaxHeight)
        {
            GTL_DOMAIN_ASSERT(
                inMaxHeight > C_<T>(0),
                "The maximum height must be nonnegative.");

            mMinHeight = C_<T>(0);
            mMaxHeight = inMaxHeight;
        }

        void MakeConeFrustum(T const& inMinHeight, T const& inMaxHeight)
        {
            GTL_DOMAIN_ASSERT(
                inMinHeight >= C_<T>(0) && inMaxHeight > inMinHeight,
                "The minimum height must be nonnegative and smaller than the maximum height.");

            mMinHeight = inMinHeight;
            mMaxHeight = inMaxHeight;
        }

        // Get the height extremes. For an infinite cone, maxHeight is set to
        // -1. For a finite cone, maxHeight is set to a positive number. Be
        // careful not to use maxHeight without understanding this
        // interpretation.
        inline T const& GetMinHeight() const
        {
            return mMinHeight;
        }

        inline T const& GetMaxHeight() const
        {
            return mMaxHeight;
        }

        inline bool HeightInRange(T const& h) const
        {
            return mMinHeight <= h && (mMaxHeight != Infinity() ? h <= mMaxHeight : true);
        }

        inline bool HeightLessThanMin(T const& h) const
        {
            return h < mMinHeight;
        }

        inline bool HeightGreaterThanMax(T const& h) const
        {
            return (mMaxHeight != Infinity() ? h > mMaxHeight : false);
        }

        inline bool IsFinite() const
        {
            return mMaxHeight != Infinity();
        }

        inline bool IsInfinite() const
        {
            return mMaxHeight == Infinity();
        }

        // The cone axis direction must be unit length.
        Vector<T, N> vertex, direction;

        // The angle must be in (0,pi/2). The other members are derived from
        // angle to avoid calling trigonometric functions in geometric queries
        // (for speed). You may set the angle and compute these by calling
        // SetAngle(inAngle).
        T angle;
        T cosAngle, sinAngle, tanAngle;
        T cosAngleSqr, sinAngleSqr, invSinAngle;

    private:
        // The heights must satisfy 0 <= minHeight < maxHeight <= infinity.
        // For an infinite cone, maxHeight is set to -1. For a finite cone,
        // maxHeight is set to a positive number. Be careful not to use
        // maxHeight without understanding this interpretation.
        T mMinHeight, mMaxHeight;

    private:
        friend class UnitTestCone;
    };

    // Comparisons to support sorted containers. These are based only on
    // 'ray', 'angle', 'minHeight' and 'maxHeight'.
    template <typename T, size_t N>
    bool operator==(Cone<T, N> const& cone0, Cone<T, N> const& cone1)
    {
        return cone0.vertex == cone1.vertex
            && cone0.direction == cone1.direction
            && cone0.angle == cone1.angle
            && cone0.GetMinHeight() == cone1.GetMinHeight()
            && cone0.GetMaxHeight() == cone1.GetMaxHeight();
    }

    template <typename T, size_t N>
    bool operator!=(Cone<T, N> const& cone0, Cone<T, N> const& cone1)
    {
        return !operator==(cone0, cone1);
    }

    template <typename T, size_t N>
    bool operator<(Cone<T, N> const& cone0, Cone<T, N> const& cone1)
    {
        if (cone0.vertex < cone1.vertex)
        {
            return true;
        }

        if (cone0.vertex > cone1.vertex)
        {
            return false;
        }

        if (cone0.direction < cone1.direction)
        {
            return true;
        }

        if (cone0.direction > cone1.direction)
        {
            return false;
        }

        if (cone0.angle < cone1.angle)
        {
            return true;
        }

        if (cone0.angle > cone1.angle)
        {
            return false;
        }

        if (cone0.GetMinHeight() < cone1.GetMinHeight())
        {
            return true;
        }

        if (cone0.GetMinHeight() > cone1.GetMinHeight())
        {
            return false;
        }

        return cone0.GetMaxHeight() < cone1.GetMaxHeight();
    }

    template <typename T, size_t N>
    bool operator<=(Cone<T, N> const& cone0, Cone<T, N> const& cone1)
    {
        return !operator<(cone1, cone0);
    }

    template <typename T, size_t N>
    bool operator>(Cone<T, N> const& cone0, Cone<T, N> const& cone1)
    {
        return operator<(cone1, cone0);
    }

    template <typename T, size_t N>
    bool operator>=(Cone<T, N> const& cone0, Cone<T, N> const& cone1)
    {
        return !operator<(cone0, cone1);
    }

    // Template alias for convenience.
    template <typename T> using Cone3 = Cone<T, 3>;
}
