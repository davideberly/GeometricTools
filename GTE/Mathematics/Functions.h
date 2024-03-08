// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// These functions are convenient for some applications. The classes
// BSNumber, BSRational and IEEEBinary16 have implementations that
// (for now) use typecasting to call the 'float' or 'double' versions.

#include <Mathematics/Constants.h>
#include <cmath>
#include <cstdint>

namespace gte
{
    inline float atandivpi(float x)
    {
        return std::atan(x) * static_cast<float>(GTE_C_INV_PI);
    }

    inline float atan2divpi(float y, float x)
    {
        return std::atan2(y, x) * static_cast<float>(GTE_C_INV_PI);
    }

    inline float clamp(float x, float xmin, float xmax)
    {
        return (x <= xmin ? xmin : (x >= xmax ? xmax : x));
    }

    inline float cospi(float x)
    {
        return std::cos(x * static_cast<float>(GTE_C_PI));
    }

    inline float exp10(float x)
    {
        return std::exp(x * static_cast<float>(GTE_C_LN_10));
    }

    inline float invsqrt(float x)
    {
        return 1.0f / std::sqrt(x);
    }

    inline int32_t isign(float x)
    {
        return (x > 0.0f ? 1 : (x < 0.0f ? -1 : 0));
    }

    inline float saturate(float x)
    {
        return (x <= 0.0f ? 0.0f : (x >= 1.0f ? 1.0f : x));
    }

    inline float sign(float x)
    {
        return (x > 0.0f ? 1.0f : (x < 0.0f ? -1.0f : 0.0f));
    }

    inline float sinpi(float x)
    {
        return std::sin(x * static_cast<float>(GTE_C_PI));
    }

    inline float sqr(float x)
    {
        return x * x;
    }

    // Compute x * y + z as a single operation. If the fused-multiply-add
    // (fma) instruction is supported by your floating-point hardware, the
    // std::fma function is called. If your hardware does not support the
    // fma instruction and the compiler maps it to a software implementation,
    // you can define GTE_DISCARD_FMA to avoid the computational cost in
    // software. If the compiler ignores the instruction and computes the
    // expression with 2 floating-point operations, it does not matter
    // whether you define GTE_DISCARD_FMA. TODO: Add a preprocessor-selected
    // software implementation for fma using the ideas of BSNumber with
    // integer type UIntegerFP32<N> for N sufficiently large.
    inline float FMA(float u, float v, float w)
    {
#if defined(GTE_DISCARD_FMA)
        return u * v + w;
#else
        return std::fma(u, v, w);
#endif
    }

    // Robust sum of products (SOP) u * v + w * z. The robustness occurs only
    // when std::fma is exposed (GTE_DISCARD_FMA is not defined).
    inline float RobustSOP(float u, float v, float w, float z)
    {
#if defined(GTE_DISCARD_FMA)
        return u * v + w * z;
#else
        float productWZ = w * z;
        float roundingError = std::fma(w, z, -productWZ);
        float result = std::fma(u, v, productWZ) + roundingError;
        return result;
#endif
    }

    // Robust difference of products (DOP) u * v - w * z. The robustness
    // occurs only when std::fma is exposed (GTE_DISCARD_FMA is not defined).
    inline float RobustDOP(float u, float v, float w, float z)
    {
#if defined(GTE_DISCARD_FMA)
        return u * v - w * z.
#else
        float productWZ = w * z;
        float roundingError = std::fma(-w, z, productWZ);
        float result = std::fma(u, v, -productWZ) + roundingError;
        return result;
#endif
    }


    inline double atandivpi(double x)
    {
        return std::atan(x) * GTE_C_INV_PI;
    }

    inline double atan2divpi(double y, double x)
    {
        return std::atan2(y, x) * GTE_C_INV_PI;
    }

    inline double clamp(double x, double xmin, double xmax)
    {
        return (x <= xmin ? xmin : (x >= xmax ? xmax : x));
    }

    inline double cospi(double x)
    {
        return std::cos(x * GTE_C_PI);
    }

    inline double exp10(double x)
    {
        return std::exp(x * GTE_C_LN_10);
    }

    inline double invsqrt(double x)
    {
        return 1.0 / std::sqrt(x);
    }

    inline double sign(double x)
    {
        return (x > 0.0 ? 1.0 : (x < 0.0 ? -1.0 : 0.0));
    }

    inline int32_t isign(double x)
    {
        return (x > 0.0 ? 1 : (x < 0.0 ? -1 : 0));
    }

    inline double saturate(double x)
    {
        return (x <= 0.0 ? 0.0 : (x >= 1.0 ? 1.0 : x));
    }

    inline double sinpi(double x)
    {
        return std::sin(x * GTE_C_PI);
    }

    inline double sqr(double x)
    {
        return x * x;
    }

    // Compute x * y + z as a single operation. If the fused-multiply-add
    // (fma) instruction is supported by your floating-point hardware, the
    // std::fma function is called. If your hardware does not support the
    // fma instruction and the compiler maps it to a software implementation,
    // you can define GTE_DISCARD_FMA to avoid the computational cost in
    // software. If the compiler ignores the instruction and computes the
    // expression with 2 floating-point operations, it does not matter
    // whether you define GTE_DISCARD_FMA. TODO: Add a preprocessor-selected
    // software implementation for fma using the ideas of BSNumber with
    // integer type UIntegerFP32<N> for N sufficiently large.
    inline double FMA(double u, double v, double w)
    {
#if defined(GTE_DISCARD_FMA)
        return u * v + w;
#else
        return std::fma(u, v, w);
#endif
    }

    // Robust sum of products (SOP) u * v + w * z. The robustness occurs only
    // when std::fma is exposed (GTE_DISCARD_FMA is not defined).
    inline double RobustSOP(double u, double v, double w, double z)
    {
#if defined(GTE_DISCARD_FMA)
        return u * v + w * z;
#else
        double productWZ = w * z;
        double roundingError = std::fma(w, z, -productWZ);
        double result = std::fma(u, v, productWZ) + roundingError;
        return result;
#endif
    }

    // Robust difference of products (DOP) u * v - w * z. The robustness
    // occurs only when std::fma is exposed (GTE_DISCARD_FMA is not defined).
    inline double RobustDOP(double u, double v, double w, double z)
    {
#if defined(GTE_DISCARD_FMA)
        return u * v - w * z.
#else
        double productWZ = w * z;
        double roundingError = std::fma(-w, z, productWZ);
        double result = std::fma(u, v, -productWZ) + roundingError;
        return result;
#endif
    }
}
