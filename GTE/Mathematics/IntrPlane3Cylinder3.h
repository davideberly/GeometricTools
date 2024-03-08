// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The intersection queries between a plane and a cylinder (finite or
// infinite) are described in
// https://www.geometrictools.com/Documentation/IntersectionCylinderPlane.pdf
//
// The plane is Dot(N, X - P) = 0, where P is a point on the plane and N is a
// nonzero vector that is not necessarily unit length.
// 
// The cylinder is (X - C)^T * (I - W * W^T) * (X - C) = r^2, where C is the
// center, W is the axis direction and r > 0 is the radius. The cylinder has
// height h. In the intersection queries, an infinite cylinder is specified
// by setting h = -1. Read the aforementioned PDF for details about this
// choice.

#include <Mathematics/Cylinder3.h>
#include <Mathematics/Ellipse3.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Matrix.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/IntrPlane3Plane3.h>
#include <array>
#include <cmath>

namespace gte
{
    template <typename T>
    class TIQuery<T, Plane3<T>, Cylinder3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        // For an infinite cylinder, call cylinder.MakeInfiniteCylinder().
        // Internally, the height is set to -1. This avoids the problem
        // of setting height to std::numeric_limits<T>::max() or
        // std::numeric_limits<T>::infinity() that are designed for
        // floating-point types but that do not work for exact rational types.
        //
        // For a finite cylinder, set cylinder.height > 0.
        Result operator()(Plane3<T> const& plane, Cylinder3<T> const& cylinder)
        {
            Result result{};

            // Convenient names.
            auto const& P = plane.origin;
            auto const& N = plane.normal;
            auto const& C = cylinder.axis.origin;
            auto const& W = cylinder.axis.direction;
            auto const& r = cylinder.radius;
            auto const& h = cylinder.height;

            if (cylinder.IsInfinite())
            {
                if (Dot(N, W) != static_cast<T>(0))
                {
                    // The cylinder direction and plane are not parallel.
                    result.intersect = true;
                }
                else
                {
                    // The cylinder direction and plane are parallel.
                    T dotNCmP = Dot(N, C - P);
                    result.intersect = (std::fabs(dotNCmP) <= r);
                }
            }
            else // cylinder is finite
            {
                T dotNCmP = Dot(N, C - P);
                T dotNW = Dot(N, W);
                Vector3<T> crossNW = Cross(N, W);
                T lhs = std::fabs(dotNCmP);
                T rhs = r * Length(crossNW) + static_cast<T>(0.5) * h * std::fabs(dotNW);
                result.intersect = (lhs <= rhs);
            }

            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Plane3<T>, Cylinder3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                type(Type::NO_INTERSECTION),
                line{},
                ellipse{},
                trimLine{}
            {
            }

            // The type of intersection.
            enum class Type
            {
                // The cylinder and plane are separated.
                NO_INTERSECTION,

                // The plane is tangent to the cylinder direction.
                SINGLE_LINE,

                // The cylinder direction is parallel to the plane and the
                // plane cuts through the cylinder in two lines.
                PARALLEL_LINES,

                // The cylinder direction is perpendicular to the plane.
                CIRCLE,

                // The cylinder direction is not parallel to the plane. When
                // the direction is perpendicular to the plane, the
                // intersection is a circle which is an ellipse with equal
                // extents.
                ELLIPSE
            };

            // The result members are set according to type.
            // 
            // type = NONE
            //   intersect = false
            //   line[0,1] and ellipse have all zero members
            //
            // type = SINGLE_LINE
            //   intersect = true
            //   line[0] is valid
            //   line[1] and ellipse have all zero members
            //
            // type = PARALLEL_LINES
            //   intersect = true
            //   line[0] and line[1] are valid
            //   ellipse has all zero members
            // 
            // type = CIRCLE
            //   intersect = true
            //   ellipse is valid (with extent[0] = extent[1])
            //   line[0,1] have all zero members
            //
            // type = ELLIPSE
            //   intersect = true
            //   ellipse is valid
            //   line[0,1] have all zero members
            bool intersect;
            Type type;
            std::array<Line3<T>, 2> line;
            Ellipse3<T> ellipse;

            // Trim lines when the cylinder is finite. They are computed when
            // the plane and infinite cylinder intersect. If there is no
            // intersection, the trim lines have all zero members.
            std::array<Line3<T>, 2> trimLine;
        };

        Result operator()(Plane3<T> const& plane, Cylinder3<T> const& cylinder)
        {
            Result result{};

            TIQuery<T, Plane3<T>, Cylinder3<T>> tiQuery{};
            auto tiOutput = tiQuery(plane, cylinder);
            if (tiOutput.intersect)
            {
                T const zero = static_cast<T>(0);
                T dotNW = Dot(plane.normal, cylinder.axis.direction);
                if (dotNW != zero)
                {
                    // The cylinder direction is not parallel to the plane.
                    // The intersection is an ellipse or circle.
                    GetEllipseOfIntersection(plane, cylinder, result);
                    GetTrimLines(plane, cylinder, result.trimLine);
                }
                else
                {
                    // The cylinder direction is parallel to the plane. There
                    // are no trim lines for this geometric configuration.
                    GetLinesOfIntersection(plane, cylinder, result);
                }
            }

            return result;
        }

    private:
        // The cylinder is infinite and its direction is not parallel to the
        // plane.
        static void GetEllipseOfIntersection(Plane3<T> const& plane,
            Cylinder3<T> const& cylinder, Result& result)
        {
            // Convenient names.
            auto const& P = plane.origin;
            auto const& N = plane.normal;
            auto const& C = cylinder.axis.origin;
            auto const& W = cylinder.axis.direction;
            auto const& r = cylinder.radius;

            // Compute a right-handed orthonormal basis {N,A,B}. The
            // plane is spanned by A and B.
            std::array<Vector3<T>, 3> basis{};
            basis[0] = N;
            Vector3<T> const& A = basis[1];
            Vector3<T> const& B = basis[2];
            ComputeOrthogonalComplement(1, basis.data());

            // Compute the projection matrix M = I - W * W^T.
            Matrix3x3<T> M{};
            M.MakeIdentity();
            M = M - OuterProduct(W, W);

            // Compute the coefficients of the quadratic equation
            // c00 + c10*x + c01*y + c20*x^2 + c11*x*y + c02*y^2 = 0.
            T const two = static_cast<T>(2);
            Vector3<T> PmC = P - C;
            Vector3<T> MtPmC = M * PmC;
            Vector3<T> MtA = M * A;
            Vector3<T> MtB = M * B;
            std::array<T, 6> coefficients
            {
                Dot(PmC, MtPmC) - r * r,
                two * Dot(A, MtPmC),
                two * Dot(B, MtPmC),
                Dot(A, MtA),
                two * Dot(A, MtB),
                Dot(B, MtB)
            };

            // Compute the 2D ellipse parameters in plane coordinates.
            Ellipse2<T> ellipse2{};
            (void)ellipse2.FromCoefficients(coefficients);

            // Lift the 2D ellipse/circle to the 3D ellipse/circle.
            result.intersect = true;
            result.type = (ellipse2.extent[0] != ellipse2.extent[1] ?
                Result::Type::ELLIPSE : Result::Type::CIRCLE);
            result.ellipse.center = plane.origin + ellipse2.center[0] * A +
                ellipse2.center[1] * B;
            result.ellipse.normal = plane.normal;
            result.ellipse.axis[0] = ellipse2.axis[0][0] * A +
                ellipse2.axis[0][1] * B;
            result.ellipse.axis[1] = ellipse2.axis[1][0] * A +
                ellipse2.axis[1][1] * B;
            result.ellipse.extent = ellipse2.extent;
        }

        // The cylinder is infinite and its direction is parallel to the
        // plane.
        static void GetLinesOfIntersection(Plane3<T> const& plane,
            Cylinder3<T> const& cylinder, Result& result)
        {
            // Convenient names.
            auto const& P = plane.origin;
            auto const& N = plane.normal;
            auto const& C = cylinder.axis.origin;
            auto const& W = cylinder.axis.direction;
            auto const& r = cylinder.radius;

            T const zero = static_cast<T>(0);
            Vector3<T> CmP = C - P;
            T dotNCmP = Dot(N, CmP);
            T ellSqr = r * r - dotNCmP * dotNCmP;  // r^2 - d^2
            if (ellSqr > zero)
            {
                // The plane cuts through the cylinder in two lines.
                result.intersect = true;
                result.type = Result::Type::PARALLEL_LINES;
                Vector3<T> projC = C - dotNCmP * N;
                Vector3<T> crsNW = Cross(N, W);
                T ell = std::sqrt(ellSqr);
                result.line[0].origin = projC - ell * crsNW;
                result.line[0].direction = W;
                result.line[1].origin = projC + ell * crsNW;
                result.line[1].direction = W;
            }
            else if (ellSqr < zero)
            {
                // The cylinder does not intersect the plane.
                result.intersect = false;
                result.type = Result::Type::NO_INTERSECTION;
            }
            else  // ellSqr = 0
            {
                // The plane is tangent to the cylinder.
                result.intersect = true;
                result.type = Result::Type::SINGLE_LINE;
                result.line[0].origin = C - dotNCmP * N;
                result.line[0].direction = W;
            }
        }

        static void GetTrimLines(Plane3<T> const& plane, Cylinder3<T> const& cylinder,
            std::array<Line3<T>, 2>& trimLine)
        {
            // Compute the cylinder end planes.
            auto const& C = cylinder.axis.origin;
            auto const& D = cylinder.axis.direction;
            auto const& h = cylinder.height;
            Vector3<T> offset = (static_cast<T>(0.5) * h) * D;

            FIQuery<T, Plane3<T>, Plane3<T>> ppQuery{};

            Plane3<T> endPlaneNeg(D, C - offset);
            auto ppResult = ppQuery(plane, endPlaneNeg);
            trimLine[0] = ppResult.line;

            Plane3<T> endPlanePos(D, C + offset);
            ppResult = ppQuery(plane, endPlanePos);
            trimLine[1] = ppResult.line;
        }
    };
}
