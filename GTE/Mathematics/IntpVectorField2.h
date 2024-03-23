// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.03.12

#pragma once

// Given points (x0[i],y0[i]) which are mapped to (x1[i],y1[i]) for
// 0 <= i < N, interpolate positions (xIn,yIn) to (xOut,yOut).

#include <Mathematics/Delaunay2Mesh.h>
#include <Mathematics/IntpQuadraticNonuniform2.h>
#include <cstdint>
#include <memory>
#include <vector>

namespace gte
{
    template <typename T, typename...>
    class IntpVectorField2 {};
}

namespace gte
{
    // The InputType is 'float' or 'double'. The ComputeType can be a
    // floating-point type or BSNumber<*> type, because it does not require
    // divisions. The RationalType requires division, so you can use
    // BSRational<*>.

    template <typename InputType, typename ComputeType, typename RationalType>
    class // [[deprecated("Use IntpVectorField2<T> instead.")]]
        IntpVectorField2<InputType, ComputeType, RationalType>
    {
    public:
        // Construction and destruction.
        ~IntpVectorField2() = default;

        IntpVectorField2(int32_t numPoints, Vector2<InputType> const* domain,
            Vector2<InputType> const* range)
            :
            mDelaunay{},
            mMesh(mDelaunay),
            mXRange{},
            mYRange{},
            mXInterp{},
            mYInterp{}
        {
            static_assert(std::is_floating_point<InputType>::value,
                "The input type must be 'float' or 'double'.");

            // Repackage the output vectors into individual components.  This
            // is required because of the format that the quadratic
            // interpolator expects for its input data.
            mXRange.resize(numPoints);
            mYRange.resize(numPoints);
            for (int32_t i = 0; i < numPoints; ++i)
            {
                mXRange[i] = range[i][0];
                mYRange[i] = range[i][1];
            }

            // Common triangulator for the interpolators.
            mDelaunay(numPoints, &domain[0], (ComputeType)0);

            // Create interpolator for x-coordinate of vector field.
            mXInterp = std::make_unique<Interpolator>(mMesh, mXRange.data(), static_cast<InputType>(1));

            // Create interpolator for y-coordinate of vector field, but share the
            // already created triangulation for the x-interpolator.
            mYInterp = std::make_unique<Interpolator>(mMesh, mYRange.data(), static_cast<InputType>(1));
        }

        // The return value is 'true' if and only if (xIn,yIn) is in the
        // convex hull of the input domain points, in which case the
        // interpolation is valid.
        bool operator()(Vector2<InputType> const& input, Vector2<InputType>& output) const
        {
            InputType xDeriv{}, yDeriv{};
            return (*mXInterp)(input, output[0], xDeriv, yDeriv)
                && (*mYInterp)(input, output[1], xDeriv, yDeriv);
        }

    protected:
        using TriangleMesh = Delaunay2Mesh<InputType, ComputeType, RationalType>;
        using Interpolator = IntpQuadraticNonuniform2<InputType, TriangleMesh>;

        Delaunay2<InputType, ComputeType> mDelaunay;
        TriangleMesh mMesh;
        std::vector<InputType> mXRange;
        std::vector<InputType> mYRange;
        std::unique_ptr<Interpolator> mXInterp;
        std::unique_ptr<Interpolator> mYInterp;
    };
}

namespace gte
{
    template <typename T>
    class IntpVectorField2<T>
    {
    public:
        // Construction and destruction.
        ~IntpVectorField2() = default;

        IntpVectorField2(int32_t numPoints, Vector2<T> const* domain, Vector2<T> const* range)
            :
            mMesh(mDelaunay)
        {
            static_assert(std::is_floating_point<T>::value,
                "The input type must be 'float' or 'double'.");

            // Repackage the output vectors into individual components.  This
            // is required because of the format that the quadratic
            // interpolator expects for its input data.
            mXRange.resize(numPoints);
            mYRange.resize(numPoints);
            for (int32_t i = 0; i < numPoints; ++i)
            {
                mXRange[i] = range[i][0];
                mYRange[i] = range[i][1];
            }

            // Common triangulator for the interpolators.
            mDelaunay(numPoints, domain);

            // Create interpolator for x-coordinate of vector field.
            mXInterp = std::make_unique<Interpolator>(mMesh, mXRange.data(), static_cast<T>(1));

            // Create interpolator for y-coordinate of vector field, but share the
            // already created triangulation for the x-interpolator.
            mYInterp = std::make_unique<Interpolator>(mMesh, mYRange.data(), static_cast<T>(1));
        }

        // The return value is 'true' if and only if (xIn,yIn) is in the
        // convex hull of the input domain points, in which case the
        // interpolation is valid.
        bool operator()(Vector2<T> const& input, Vector2<T>& output) const
        {
            T xDeriv{}, yDeriv{};
            return (*mXInterp)(input, output[0], xDeriv, yDeriv)
                && (*mYInterp)(input, output[1], xDeriv, yDeriv);
        }

    protected:
        using TriangleMesh = Delaunay2Mesh<T>;
        using Interpolator = IntpQuadraticNonuniform2<T, TriangleMesh>;

        Delaunay2<T> mDelaunay;
        TriangleMesh mMesh;
        std::vector<T> mXRange;
        std::vector<T> mYRange;
        std::unique_ptr<Interpolator> mXInterp;
        std::unique_ptr<Interpolator> mYInterp;
    };
}
