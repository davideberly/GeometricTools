// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// A torus with origin (0,0,0), outer radius r0 and inner radius r1 (with
// (r0 >= r1) is defined implicitly as follows. The point P0 = (x,y,z) is on
// the torus. Its projection onto the xy-plane is P1 = (x,y,0). The circular
// cross section of the torus that contains the projection has radius r0 and
// center P2 = r0*(x,y,0)/sqrt(x^2+y^2). The points triangle <P0,P1,P2> is a
// right triangle with right angle at P1. The hypotenuse <P0,P2> has length
// r1, leg <P1,P2> has length z and leg <P0,P1> has length
// |r0 - sqrt(x^2+y^2)|. The Pythagorean theorem says
// z^2 + |r0 - sqrt(x^2+y^2)|^2 = r1^2. This can be algebraically
// manipulated to
//   (x^2 + y^2 + z^2 + r0^2 - r1^2)^2 - 4 * r0^2 * (x^2 + y^2) = 0
//
// A parametric form is
//   x = (r0 + r1 * cos(v)) * cos(u)
//   y = (r0 + r1 * cos(v)) * sin(u)
//   z = r1 * sin(v)
// for u in [0,2*pi) and v in [0,2*pi).
//
// Generally, let the torus center be C with plane of symmetry containing C
// and having directions D0 and D1. The axis of symmetry is the line
// containing C and having direction N (the plane normal). The radius from
// the center of the torus is r0 and the radius of the tube of the torus is
// r1. A point P may be written as P = C + x*D0 + y*D1 + z*N, where matrix
// [D0 D1 N] is orthonormal and has determinant 1. Thus, x = Dot(D0,P-C),
// y = Dot(D1,P-C) and z = Dot(N,P-C).  The implicit form is
//      [|P-C|^2 + r0^2 - r1^2]^2 - 4*r0^2*[|P-C|^2 - (Dot(N,P-C))^2] = 0
// Observe that D0 and D1 are not present in the equation, which is to be
// expected by the symmetry. The parametric form is
//      P(u,v) = C + (r0 + r1*cos(v))*(cos(u)*D0 + sin(u)*D1) + r1*sin(v)*N
// for u in [0,2*pi) and v in [0,2*pi).
//
// In the class Torus3, the members are 'center' C, 'direction0' D0,
// 'direction1' D1, 'normal' N, 'radius0' r0 and 'radius1' r1.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gtl
{
    template <typename T>
    class Torus3
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all member to zero.
        Torus3()
            :
            center{},
            direction0{},
            direction1{},
            normal{},
            radius0(C_<T>(0)),
            radius1(C_<T>(0))
        {
        }

        Torus3(Vector3<T> const& inCenter, Vector3<T> const& inDirection0,
            Vector3<T> const& inDirection1, Vector3<T> const& inNormal,
            T const& inRadius0, T const& inRadius1)
            :
            center(inCenter),
            direction0(inDirection0),
            direction1(inDirection1),
            normal(inNormal),
            radius0(inRadius0),
            radius1(inRadius1)
        {
        }

        // Evaluation of the surface. The function supports derivative
        // calculation through order 2; that is, order <= 2 is required.
        // If you want only the position, pass in order of 0. If you want
        // the position and first-order derivatives, pass in order of 1,
        // and so on. The output 'values' are ordered as: position X;
        // first-order derivatives dX/du, dX/dv; second-order derivatives
        // d2X/du2, d2X/dudv, d2X/dv2. The input array 'jet' has enough
        // storage for the maximum order 2.
        bool Evaluate(T const& u, T const& v, size_t order,
            std::array<Vector3<T>, 6>& jet) const
        {
            // Compute position.
            T csu = std::cos(u);
            T snu = std::sin(u);
            T csv = std::cos(v);
            T snv = std::sin(v);
            T r1csv = radius1 * csv;
            T r1snv = radius1 * snv;
            T r0pr1csv = radius0 + r1csv;
            Vector3<T> combo0 = csu * direction0 + snu * direction1;
            Vector3<T> r0pr1csvcombo0 = r0pr1csv * combo0;
            Vector3<T> r1snvnormal = r1snv * normal;
            jet[0] = center + r0pr1csvcombo0 + r1snvnormal;

            if (order >= 1)
            {
                // Compute first-order derivatives.
                Vector3<T> combo1 = -snu * direction0 + csu * direction1;
                jet[1] = r0pr1csv * combo1;
                jet[2] = -r1snv * combo0 + r1csv * normal;

                if (order == 2)
                {
                    // Compute second-order derivatives.
                    jet[3] = -r0pr1csvcombo0;
                    jet[4] = -r1snv * combo1;
                    jet[5] = -r1csv * combo0 - r1snvnormal;
                }
                else
                {
                    return false;
                }
            }

            return true;
        }

        // Get the (u,v) parameters for the specified position.
        void GetParameters(Vector3<T> const& X, T& u, T& v) const
        {
            Vector3<T> delta = X - center;

            // (r0 + r1*cos(v))*cos(u)
            T dot0 = Dot(direction0, delta);

            // (r0 + r1*cos(v))*sin(u)
            T dot1 = Dot(direction1, delta);

            // r1*sin(v)
            T dot2 = Dot(normal, delta);

            // r1*cos(v)
            T r1csv = std::sqrt(dot0 * dot0 + dot1 * dot1) - radius0;

            u = std::atan2(dot1, dot0);
            v = std::atan2(dot2, r1csv);
        }

        Vector3<T> center, direction0, direction1, normal;
        T radius0, radius1;

    private:
        friend class UnitTestTorus3;
    };

    // Comparisons to support sorted containers.
    template <typename T>
    bool operator==(Torus3<T> const& torus0, Torus3<T> const& torus1)
    {
        return torus0.center == torus1.center
            && torus0.direction0 == torus1.direction0
            && torus0.direction1 == torus1.direction1
            && torus0.normal == torus1.normal
            && torus0.radius0 == torus1.radius0
            && torus0.radius1 == torus1.radius1;
    }

    template <typename T>
    bool operator!=(Torus3<T> const& torus0, Torus3<T> const& torus1)
    {
        return !operator==(torus0, torus1);
    }

    template <typename T>
    bool operator<(Torus3<T> const& torus0, Torus3<T> const& torus1)
    {
        if (torus0.center < torus1.center)
        {
            return true;
        }

        if (torus0.center > torus1.center)
        {
            return false;
        }

        if (torus0.direction0 < torus1.direction0)
        {
            return true;
        }

        if (torus0.direction0 > torus1.direction0)
        {
            return false;
        }

        if (torus0.direction1 < torus1.direction1)
        {
            return true;
        }

        if (torus0.direction1 > torus1.direction1)
        {
            return false;
        }

        if (torus0.normal < torus1.normal)
        {
            return true;
        }

        if (torus0.normal > torus1.normal)
        {
            return false;
        }

        if (torus0.radius0 < torus1.radius0)
        {
            return true;
        }

        if (torus0.radius0 > torus1.radius0)
        {
            return false;
        }

        return torus0.radius1 < torus1.radius1;
    }

    template <typename T>
    bool operator<=(Torus3<T> const& torus0, Torus3<T> const& torus1)
    {
        return !operator<(torus1, torus0);
    }

    template <typename T>
    bool operator>(Torus3<T> const& torus0, Torus3<T> const& torus1)
    {
        return operator<(torus1, torus0);
    }

    template <typename T>
    bool operator>=(Torus3<T> const& torus0, Torus3<T> const& torus1)
    {
        return !operator<(torus0, torus1);
    }
}
