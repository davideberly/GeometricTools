// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Delaunay2Mesh.h>
#include <Mathematics/IntpQuadraticNonuniform2.h>
#include <memory>

// Interpolation of a scalar-valued function defined on a sphere.  Although
// the sphere lives in 3D, the interpolation is a 2D method whose input
// points are angles (theta,phi) from spherical coordinates.  The domains of
// the angles are -pi <= theta <= pi and 0 <= phi <= pi.

namespace gte
{
    template <typename InputType, typename ComputeType, typename RationalType>
    class IntpSphere2
    {
    public:
        // Construction and destruction.  For complete spherical coverage,
        // include the two antipodal (theta,phi) points (-pi,0,F(-pi,0)) and
        // (-pi,pi,F(-pi,pi)) in the input data.  These correspond to the
        // sphere poles x = 0, y = 0, and |z| = 1.
        ~IntpSphere2() = default;

        IntpSphere2(int numPoints, InputType const* theta, InputType const* phi, InputType const* F)
            :
            mMesh(mDelaunay)
        {
            // Copy the input data.  The larger arrays are used to support
            // wrap-around in the Delaunay triangulation for the interpolator.
            int totalPoints = 3 * numPoints;
            mWrapAngles.resize(totalPoints);
            mWrapF.resize(totalPoints);
            for (int i = 0; i < numPoints; ++i)
            {
                mWrapAngles[i][0] = theta[i];
                mWrapAngles[i][1] = phi[i];
                mWrapF[i] = F[i];
            }

            // Use periodicity to get wrap-around in the Delaunay
            // triangulation.
            int i0 = 0, i1 = numPoints, i2 = 2 * numPoints;
            for (/**/; i0 < numPoints; ++i0, ++i1, ++i2)
            {
                mWrapAngles[i1][0] = mWrapAngles[i0][0] + (InputType)GTE_C_TWO_PI;
                mWrapAngles[i2][0] = mWrapAngles[i0][0] - (InputType)GTE_C_TWO_PI;
                mWrapAngles[i1][1] = mWrapAngles[i0][1];
                mWrapAngles[i2][1] = mWrapAngles[i0][1];
                mWrapF[i1] = mWrapF[i0];
                mWrapF[i2] = mWrapF[i0];
            }

            mDelaunay(totalPoints, &mWrapAngles[0], (ComputeType)0);
            mInterp = std::make_unique<IntpQuadraticNonuniform2<InputType, TriangleMesh>>(
                mMesh, &mWrapF[0], (InputType)1);
        }

        // Spherical coordinates are
        //   x = cos(theta)*sin(phi)
        //   y = sin(theta)*sin(phi)
        //   z = cos(phi)
        // for -pi <= theta <= pi, 0 <= phi <= pi.  The application can use
        // this function to convert unit length vectors (x,y,z) to (theta,phi).
        static void GetSphericalCoordinates(InputType x, InputType y, InputType z,
            InputType& theta, InputType& phi)
        {
            // Assumes (x,y,z) is unit length.  Returns -pi <= theta <= pi and
            // 0 <= phiAngle <= pi.

            if (z < (InputType)1)
            {
                if (z > -(InputType)1)
                {
                    theta = std::atan2(y, x);
                    phi = std::acos(z);
                }
                else
                {
                    theta = -(InputType)GTE_C_PI;
                    phi = (InputType)GTE_C_PI;
                }
            }
            else
            {
                theta = -(InputType)GTE_C_PI;
                phi = (InputType)0;
            }
        }

        // The return value is 'true' if and only if the input point is in the
        // convex hull of the input (theta,pi) array, in which case the
        // interpolation is valid.
        bool operator()(InputType theta, InputType phi, InputType& F) const
        {
            Vector2<InputType> angles{ theta, phi };
            InputType thetaDeriv, phiDeriv;
            return (*mInterp)(angles, F, thetaDeriv, phiDeriv);
        }

    private:
        typedef Delaunay2Mesh<InputType, ComputeType, RationalType> TriangleMesh;

        std::vector<Vector2<InputType>> mWrapAngles;
        Delaunay2<InputType, ComputeType> mDelaunay;
        TriangleMesh mMesh;
        std::vector<InputType> mWrapF;
        std::unique_ptr<IntpQuadraticNonuniform2<InputType, TriangleMesh>> mInterp;
    };
}
