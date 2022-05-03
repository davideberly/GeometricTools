// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// The Chebyshev ratio is f(t,A) = sin(t*A)/sin(A) for t in [0,1] and A in
// [0,pi). The implementation ChebyshevRatio computes this function. The
// implementation ChebyshevRatios computes the pair {f(1-t,A), f(t,A)}, which
// is useful for spherical linear interpolation.
//
// TODO: The evaluation for A near 0 or pi needs to be more robust. For A near
// 0, sin(t*A)/sin(A) has a removable singularity. Use the idea in RAEFGC for
// an approximation to remove the singularity. For A near pi, the singularity
// is not removable, so some approximation must be used such as those found in
// ChebyshevRatioEstimate.h.

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <GTL/Utility/Exceptions.h>
#include <array>
#include <cmath>

namespace gtl
{
    // The angle must be in [0,pi).
    template <typename T>
    T ChebyshevRatio(T t, T angle)
    {
        if (angle > C_<T>(0))
        {
            if (angle < C_PI<T>)
            {
                // The angle A is in (0,pi).
                return std::sin(t * angle) / std::sin(angle);
            }
        }
        else if (angle == C_<T>(0))
        {
            // The angle A is 0. Using l'Hospital's rule,
            // lim_{A->0} sin(t*A)/sin(A) = lim_{A->0} t*cos(t*A)/cos(A) = t.
            return t;
        }

        // The angle A is not in [0,pi).
        GTL_DOMAIN_ERROR(
            "Invalid angle.");
    }

    // The angle extracted from cosAngle is in [0,pi).
    template <typename T>
    T ChebyshevRatioUsingCosAngle(T t, T cosAngle)
    {
        if (cosAngle < C_<T>(1))
        {
            if (cosAngle > -C_<T>(1))
            {
                // The angle A is in (0,pi).
                T angle = std::acos(cosAngle);
                return std::sin(t * angle) / std::sin(angle);
            }
            else
            {
                // The angle A is pi.
                GTL_DOMAIN_ERROR(
                    "Invalid angle.");
            }
        }
        else
        {
            // The angle A is 0. Using l'Hospital's rule,
            // lim_{A->0} sin(t*A)/sin(A) = lim_{A->0} t*cos(t*A)/cos(A) = t.
            return t;
        }
    }

    // The angle must be in [0,pi). Although it is possible to compute
    // invSin = 1/sin(angle) and perform two multiplications for f[0] and
    // f[1], the resulting ratios typically do not match those from
    // ChebyshevRatioAngle. Therefore, two divisions are performed in this
    // function to ensure the resulting ratios are the same.
    template <typename T>
    std::array<T, 2> ChebyshevRatios(T t, T angle)
    {
        if (angle > C_<T>(0))
        {
            if (angle < C_PI<T>)
            {
                // The angle A is in (0,pi).
                T sinAngle = std::sin(angle);
                std::array<T, 2> f
                {
                    std::sin((C_<T>(1) - t) * angle) / sinAngle,
                    std::sin(t * angle) / sinAngle
                };
                return f;
            }
        }
        else if (angle == C_<T>(0))
        {
            // The angle A is 0. Using l'Hospital's rule,
            // lim_{A->0} sin(t*A)/sin(A) = lim_{A->0} t*cos(t*A)/cos(A) = t.
            std::array<T, 2> f = { C_<T>(1) - t, t };
            return f;
        }

        // The angle A is not in [0,pi).
        GTL_DOMAIN_ERROR(
            "Invalid angle.");
    }

    // The angle extracted from cosAngle is in [0,pi). Although it is possible
    // to compute invSin = 1/sin(angle) and perform two multiplications for
    // f[0] and f[1], the resulting ratios typically do not match those from
    // ChebyshevRatioAngle. Therefore, two divisions are performed in this
    // function to ensure the resulting ratios are the same.
    template <typename T>
    std::array<T, 2> ChebyshevRatiosUsingCosAngle(T t, T cosAngle)
    {
        if (cosAngle < C_<T>(1))
        {
            if (cosAngle > -C_<T>(1))
            {
                // The angle A is in (0,pi).
                T angle = std::acos(cosAngle);
                T sinAngle = std::sin(angle);
                std::array<T, 2> f
                {
                    std::sin((C_<T>(1) - t) * angle) / sinAngle,
                    std::sin(t * angle) / sinAngle
                };
                return f;
            }
            else
            {
                // The angle A is pi.
                GTL_DOMAIN_ERROR(
                    "Invalid angle.");
            }
        }
        else
        {
            // The angle A is 0. Using l'Hospital's rule,
            // lim_{A->0} sin(t*A)/sin(A) = lim_{A->0} t*cos(t*A)/cos(A) = t.
            std::array<T, 2> f = { C_<T>(1) - t, t };
            return f;
        }
    }
}
