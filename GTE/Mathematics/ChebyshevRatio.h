// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

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

#include <Mathematics/Constants.h>
#include <Mathematics/Logger.h>
#include <array>
#include <cmath>

namespace gte
{
    // The angle must be in [0,pi).
    template <typename T>
    T ChebyshevRatio(T t, T angle)
    {
        T const zero = static_cast<T>(0);
        if (angle > zero)
        {
            if (angle < static_cast<T>(GTE_C_PI))
            {
                // The angle A is in (0,pi).
                return std::sin(t * angle) / std::sin(angle);
            }
        }
        else if (angle == zero)
        {
            // The angle A is 0. Using l'Hospital's rule,
            // lim_{A->0} sin(t*A)/sin(A) = lim_{A->0} t*cos(t*A)/cos(A) = t.
            return t;
        }

        // The angle A is not in [0,pi).
        LogError("Invalid angle.");
    }

    // The angle extracted from cosAngle is in [0,pi).
    template <typename T>
    T ChebyshevRatioUsingCosAngle(T t, T cosAngle)
    {
        T const one = static_cast<T>(1);
        if (cosAngle < one)
        {
            if (cosAngle > -one)
            {
                // The angle A is in (0,pi).
                T angle = std::acos(cosAngle);
                return std::sin(t * angle) / std::sin(angle);
            }
            else
            {
                // The angle A is pi.
                LogError("Invalid angle.");
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
        T const zero = static_cast<T>(0);
        T const one = static_cast<T>(1);
        if (angle > zero)
        {
            if (angle < static_cast<T>(GTE_C_PI))
            {
                // The angle A is in (0,pi).
                T sinAngle = std::sin(angle);
                std::array<T, 2> f
                {
                    std::sin((one - t) * angle) / sinAngle,
                    std::sin(t * angle) / sinAngle
                };
                return f;
            }
        }
        else if (angle == zero)
        {
            // The angle A is 0. Using l'Hospital's rule,
            // lim_{A->0} sin(t*A)/sin(A) = lim_{A->0} t*cos(t*A)/cos(A) = t.
            std::array<T, 2> f = { one - t, t };
            return f;
        }

        // The angle A is not in [0,pi).
        LogError("Invalid angle.");
    }

    // The angle extracted from cosAngle is in [0,pi). Although it is possible
    // to compute invSin = 1/sin(angle) and perform two multiplications for
    // f[0] and f[1], the resulting ratios typically do not match those from
    // ChebyshevRatioAngle. Therefore, two divisions are performed in this
    // function to ensure the resulting ratios are the same.
    template <typename T>
    std::array<T, 2> ChebyshevRatiosUsingCosAngle(T t, T cosAngle)
    {
        T const one = static_cast<T>(1);
        if (cosAngle < one)
        {
            if (cosAngle > -one)
            {
                // The angle A is in (0,pi).
                T angle = std::acos(cosAngle);
                T sinAngle = std::sin(angle);
                std::array<T, 2> f
                {
                    std::sin((one - t) * angle) / sinAngle,
                    std::sin(t * angle) / sinAngle
                };
                return f;
            }
            else
            {
                // The angle A is pi.
                LogError("Invalid angle.");
            }
        }
        else
        {
            // The angle A is 0. Using l'Hospital's rule,
            // lim_{A->0} sin(t*A)/sin(A) = lim_{A->0} t*cos(t*A)/cos(A) = t.
            std::array<T, 2> f = { one - t, t };
            return f;
        }
    }
}
