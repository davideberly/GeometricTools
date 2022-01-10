#pragma once

#include "Mathematics/GteDCPQuery.h"
#include "Mathematics/GteCylinder3.h"
#include "Mathematics/GteLine.h"
#include "Mathematics/GteVector3.h"

// The queries consider the cylinder to be a solid.

namespace gte
{

template <typename Real>
class DCPQuery<Real, Line3<Real>, Cylinder3<Real>>
{
public:
    struct Result
    {
        Real distance;

        // Classification of the set of closest pairs.  In the discussion,
        // infinity = std::numeric_limits<Real>::max().
        //
        // type = 0:
        //   There is a unique pair of closest points, stored in lineClosest
        //   and cylinderClosest.  The t-value for the closest line point is
        //   stored in parameter[0] and parameter[1].
        //
        // type = 1:
        //   The line is not parallel to the cylinder.  The line intersects
        //   the cylinder in a segment, which is a subset of the line P+t*D
        //   where t in [parameter[0],parameter[1]].  The parameters are both
        //   finite.  A pair of closest points is
        //     lineClosest = cylinderClosest = P + parameter[0]*D
        //
        // type = 2:
        //   The line is parallel to the cylinder and intersects the cylinder
        //   either as a line or a segment.
        //
        //   When a line, parameter[0] = -infinity and parameter[1] =
        //   +infinity.  A pair of closest points is
        //     lineClosest = cylinderClosest = P
        //
        //   When a segment, both parameter[] values are finite.  A pair of
        //   closest points is
        //     lineClosest = cylinderClosest = P + parameter[0]*D
        //
        // type = 3:
        //   The line is parallel to the cylinder and outside the cylinder.
        //   The closest points on the line are the line itself (for an
        //   infinite cylinder) or a segment (for a finite cylinder).  The
        //   closest points on the cylinder are of the form Q+t*D, where Q
        //   is the perpendicular projection of P on the cylinder wall and
        //   where the t-interval is the same as that for the line, namely,
        //   [parameter[0],parameter[1]].
        //
        //   When a line, parameter[0] = -infinity and parameter[1] =
        //   +infinity.  A pair of closest points is
        //     lineClosest = P, cylinderClosest = Q
        //
        //   When a segment, both parameter[] values are finite.  A pair of
        //   closest points is
        //     lineClosest = P + parameter[0]*D,
        //     cylinderClosest = Q + parameter[0]*D

        int type;
        Vector3<Real> lineClosest, cylinderClosest;
        Real parameter[2];
    };

    Result operator()(Line3<Real> const& line,
        Cylinder3<Real> const& cylinder);

private:
    void DoQueryInfiniteCylinder(Vector3<Real> const& P,
        Vector3<Real> const& D, Real radius, Result& result);

    void DoQueryFiniteCylinder(Vector3<Real> const& P,
        Vector3<Real> const& D, Real radius, Real height, Result& result);
};


template <typename Real>
typename DCPQuery<Real, Line3<Real>, Cylinder3<Real>>::Result
DCPQuery<Real, Line3<Real>, Cylinder3<Real>>::operator()(
    Line3<Real> const& line, Cylinder3<Real> const& cylinder)
{
    Result result;

    // Convert the line to the cylinder coordinate system.  In this system,
    // the line believes (0,0,0) is the cylinder axis origin and (0,0,1) is
    // the cylinder axis direction.
    Vector3<Real> basis[3];
    basis[0] = cylinder.axis.direction;
    ComputeOrthogonalComplement(1, basis);

    Vector3<Real> delta = line.origin - cylinder.axis.origin;
    Vector3<Real> P
    {
        Dot(basis[1], delta),
        Dot(basis[2], delta),
        Dot(basis[0], delta)
    };
    Vector3<Real> D
    {
        Dot(basis[1], line.direction),
        Dot(basis[2], line.direction),
        Dot(basis[0], line.direction)
    };

    if (cylinder.height == std::numeric_limits<Real>::max())
    {
        DoQueryInfiniteCylinder(P, D, cylinder.radius, result);
    }
    else
    {
        DoQueryFiniteCylinder(P, D, cylinder.radius, cylinder.height, result);
    }

    // Convert the closest points from the cylinder coordinate system to the
    // original coordinate system.
    result.lineClosest = cylinder.axis.origin +
        result.lineClosest[0] * basis[1] +
        result.lineClosest[1] * basis[2] +
        result.lineClosest[2] * basis[0];

    result.cylinderClosest = cylinder.axis.origin +
        result.cylinderClosest[0] * basis[1] +
        result.cylinderClosest[1] * basis[2] +
        result.cylinderClosest[2] * basis[0];

    return result;
}

template <typename Real>
void DCPQuery<Real, Line3<Real>, Cylinder3<Real>>::DoQueryInfiniteCylinder(
    Vector3<Real> const& P, Vector3<Real> const& D, Real radius,
    Result& result)
{
    Real sqrRadius = radius * radius;
    Real projDSqrLength = D[0] * D[0] + D[1] * D[1];
    if (projDSqrLength > (Real)0)
    {
        // The line is not parallel to the cylinder.  The projection of the
        // line onto the plane z = 0 is (p0 + t*d0, p1 + t*d1, 0).  The
        // squared distance from a projection point to the origin is
        // Q(t) = (p0 + t*d0)^2 + (p1 + t*d1)^2.  The minimum occurs when
        // tbar = -(p0*d0 + p1*d1)/(d0^2 + d1^2) and the minimum squared
        // distance is Q(tbar) = (p0*d1 - p1*d0)^2/(d0^2 + d1^2).
        Real temp = P[0] * D[1] - P[1] * D[0];
        if (temp * temp >= sqrRadius * projDSqrLength)
        {
            // The line is outside the cylinder or tangential to the cylinder.
            // The pair of closest points is unique.
            result.distance = std::abs(temp) / sqrt(projDSqrLength) - radius;
            result.type = 0;

            Real tbar = -(P[0] * D[0] + P[1] * D[1]) / projDSqrLength;
            result.parameter[0] = tbar;
            result.parameter[1] = tbar;
            result.lineClosest = P + tbar * D;

            Real projPLength = sqrt(
                result.lineClosest[0] * result.lineClosest[0] +
                result.lineClosest[1] * result.lineClosest[1]);
            temp = radius / projPLength;
            result.cylinderClosest[0] = result.lineClosest[0] * temp;
            result.cylinderClosest[1] = result.lineClosest[1] * temp;
            result.cylinderClosest[2] = result.lineClosest[2];
        }
        else
        {
            // The line intersects the cylinder in a segment.
            result.distance = (Real)0;
            result.type = 1;

            // The segment has endpoints determined by the t-roots of
            // a2*t^2 + 2*a1*t + a0 = 0.
            Real a2 = projDSqrLength;
            Real a1 = P[0] * D[0] + P[1] * D[1];
            Real a0 = P[0] * P[0] + P[1] * P[1] - sqrRadius;
            Real rootDiscr = sqrt(std::max(a1 * a1 - a0 * a2, (Real)0));
            Real invA2 = ((Real)1) / a2;
            result.parameter[0] = (-a1 - rootDiscr) * invA2;
            result.parameter[1] = (-a1 + rootDiscr) * invA2;
            result.lineClosest = P + result.parameter[0] * D;
            result.cylinderClosest = result.lineClosest;
        }
    }
    else  // D is parallel to (0,0,1)
    {
        // The line is parallel to the cylinder.  There are infinitely many
        // pairs of closest points.
        Real infinity = std::numeric_limits<Real>::max();
        Real sqrDistance = P[0] * P[0] + P[1] * P[1];
        if (sqrDistance <= sqrRadius)
        {
            // The line is inside the cylinder or on the cylinder wall.
            result.distance = (Real)0;
            result.type = 2;
            result.lineClosest = P;
            result.cylinderClosest = P;
        }
        else
        {
            // The line is outside the cylinder.
            Real distance = sqrt(sqrDistance);
            result.distance = distance - radius;
            result.type = 3;
            result.lineClosest = P;
            Real temp = radius / distance;
            result.cylinderClosest[0] = P[0] * temp;
            result.cylinderClosest[1] = P[1] * temp;
            result.cylinderClosest[2] = P[2];
        }

        result.parameter[0] = -infinity;
        result.parameter[1] = +infinity;
    }
}

template <typename Real>
void DCPQuery<Real, Line3<Real>, Cylinder3<Real>>::DoQueryFiniteCylinder(
    Vector3<Real> const&, Vector3<Real> const&, Real, Real, Result&)
{

}


}
