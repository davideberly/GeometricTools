// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Convert points and transformations between two coordinate systems.
// The mathematics involves a change of basis. See the document
//   https://www.geometrictools.com/Documentation/ConvertingBetweenCoordinateSystems.pdf
// for the details. Typical usage for 3D conversion is shown next.
//
// Linear change of basis. The columns of U are the basis vectors for the
// source coordinate system. A vector X = { x0, x1, x2 } in the source
// coordinate system is represented by
//   X = x0 * (1,0,0) + x1 * (0,1,0) + x2 * (0,0,1)
// The Cartesian coordinates for the point are the combination of these
// terms,
//   X = (x0, x1, x2)
// The columns of V are the basis vectors for the target coordinate system.
// A vector Y = { y0, y1, y2 } in the target coordinate system is
// represented by
//   Y = y0 * (1,0,0) + y1 * (0,0,1) + y2 * (0,1,0)
// The Cartesian coordinates for the vector are the combination of these
// terms,
//   Y = (y0, y2, y1)
// The call Y = convert.UToV(X) computes y0, y1 and y2 so that the Cartesian
// coordinates for X and for Y are the same. For example,
//   X = { 1.0, 2.0, 3.0 }
//     = 1.0 * (1,0,0) + 2.0 * (0,1,0) + 3.0 * (0,0,1)
//     = (1, 2, 3)
//   Y = { 1.0, 3.0, 2.0 }
//     = 1.0 * (1,0,0) + 3.0 * (0,0,1) + 2.0 * (0,1,0)
//     = (1, 2, 3)
// X and Y represent the same vector (equal Cartesian coordinates) but have
// different representations in the source and target coordinates.
//
 // Affine change of basis. The first three columns of U are the basis
 // vectors for the source coordinate system and must have last components
 // set to 0. The last column is the origin for that system and must have
 // last component set to 1. A point X = { x0, x1, x2, 1 } in the source
 // coordinate system is represented by
 //   X = x0*(-1,0,0,0) + x1*(0,0,1,0) + x2*(0,-1,0,0) + 1*(1,2,3,1)
 // The Cartesian coordinates for the point are the combination of these
 // terms,
 //   X = (-x0 + 1, -x2 + 2, x1 + 3, 1)
 // The first three columns of V are the basis vectors for the target
 // coordinate system and must have last components set to 0. The last
 // column is the origin for that system and must have last component set
 // to 1. A point Y = { y0, y1, y2, 1 } in the target coordinate system is
 // represented by
 //   Y = y0*(0,1,0,0) + y1*(-1,0,0,0) + y2*(0,0,1,0) + 1*(4,5,6,1)
 // The Cartesian coordinates for the point are the combination of these
 // terms,
 //   Y = (-y1 + 4, y0 + 5, y2 + 6, 1)
 // The call Y = convert.UToV(X) computes y0, y1 and y2 so that the Cartesian
 // coordinates for X and for Y are the same. For example,
 //   X = { -1.0, 4.0, -3.0, 1.0 }
 //     = -1.0*(-1,0,0,0) + 4.0*(0,0,1,0) - 3.0*(0,-1,0,0) + 1.0*(1,2,3,1)
 //     = (2, 5, 7, 1)
 //   Y = { 0.0, 2.0, 1.0, 1.0 }
 //     = 0.0*(0,1,0,0) + 2.0*(-1,0,0,0) + 1.0*(0,0,1,0) + 1.0*(4,5,6,1)
 //     = (2, 5, 7, 1)
 // X and Y represent the same point (equal Cartesian coordinates) but have
 // different representations in the source and target affine coordinates.

#include <GTL/Mathematics/MatrixAnalysis/GaussianElimination.h>

namespace gtl
{
    template <typename T, size_t N>
    class ConvertCoordinates
    {
    public:
        using value_type = T;

        // Construction of the change of basis matrix. The implementation
        // supports both linear change of basis and affine change of basis.
        ConvertCoordinates()
            :
            mIsVectorOnRightU(true),
            mIsVectorOnRightV(true),
            mIsRightHandedU(true),
            mIsRightHandedV(true)
        {
            MakeIdentity(mC);
            MakeIdentity(mInverseC);
        }

        // Compute a change of basis between two coordinate systems. The
        // return value is 'true' iff U and V are invertible. The
        // matrix-vector multiplication conventions affect the conversion of
        // matrix transformations. The Boolean inputs indicate how you want
        // the matrices to be interpreted when applied as transformations of
        // a vector.
        bool operator()(
            Matrix<T, N, N> const& U, bool vectorOnRightU,
            Matrix<T, N, N> const& V, bool vectorOnRightV)
        {
            // Initialize in case of early exit.
            MakeIdentity(mC);
            MakeIdentity(mInverseC);
            mIsVectorOnRightU = true;
            mIsVectorOnRightV = true;
            mIsRightHandedU = true;
            mIsRightHandedV = true;

            Matrix<T, N, N> inverseU{};
            T determinantU = C_<T>(0);
            bool invertibleU = GaussianElimination<T>::GetInverseAndDeterminant(
                N, U.data(), inverseU.data(), determinantU);
            if (!invertibleU)
            {
                return false;
            }

            Matrix<T, N, N> inverseV{};
            T determinantV = C_<T>(0);
            bool invertibleV = GaussianElimination<T>::GetInverseAndDeterminant(
                N, V.data(), inverseV.data(), determinantV);
            if (!invertibleV)
            {
                return false;
            }

            mC = inverseU * V;
            mInverseC = inverseV * U;
            mIsVectorOnRightU = vectorOnRightU;
            mIsVectorOnRightV = vectorOnRightV;
            mIsRightHandedU = (determinantU > C_<T>(0));
            mIsRightHandedV = (determinantV > C_<T>(0));
            return true;
        }

        // Member access.
        inline Matrix<T, N, N> const& GetC() const
        {
            return mC;
        }

        inline Matrix<T, N, N> const& GetInverseC() const
        {
            return mInverseC;
        }

        inline bool IsVectorOnRightU() const
        {
            return mIsVectorOnRightU;
        }

        inline bool IsVectorOnRightV() const
        {
            return mIsVectorOnRightV;
        }

        inline bool IsRightHandedU() const
        {
            return mIsRightHandedU;
        }

        inline bool IsRightHandedV() const
        {
            return mIsRightHandedV;
        }

        // Convert points between coordinate systems. The names of the
        // systems are U and V to make it clear which inputs of operator()
        // they are associated with. The X vector stores coordinates for the
        // U-system and the Y vector stores coordinates for the V-system.

        // Y = C^{-1}*X
        inline Vector<T, N> UToV(Vector<T, N> const& X) const
        {
            return mInverseC * X;
        }

        // X = C*Y
        inline Vector<T, N> VToU(Vector<T, N> const& Y) const
        {
            return mC * Y;
        }

        // Convert transformations between coordinate systems. The outputs
        // are computed according to the tables shown before the function
        // declarations. The superscript T denotes the transpose operator.
        // vectorOnRightU = true:  transformation is X' = A*X
        // vectorOnRightU = false: transformation is (X')^T = X^T*A
        // vectorOnRightV = true:  transformation is Y' = B*Y
        // vectorOnRightV = false: transformation is (Y')^T = Y^T*B

        // vectorOnRightU  | vectorOnRightV  | output
        // ----------------+-----------------+---------------------
        // true            | true            | C^{-1} * A * C
        // true            | false           | (C^{-1} * A * C)^T 
        // false           | true            | C^{-1} * A^T * C
        // false           | false           | (C^{-1} * A^T * C)^T
        Matrix<T, N, N> UToV(Matrix<T, N, N> const& A) const
        {
            Matrix<T, N, N> product{};

            if (mIsVectorOnRightU)
            {
                product = mInverseC * A * mC;
                if (mIsVectorOnRightV)
                {
                    return product;
                }
                else
                {
                    return Transpose(product);
                }
            }
            else
            {
                product = mInverseC * MultiplyATB(A, mC);
                if (mIsVectorOnRightV)
                {
                    return product;
                }
                else
                {
                    return Transpose(product);
                }
            }
        }

        // vectorOnRightU  | vectorOnRightV  | output
        // ----------------+-----------------+---------------------
        // true            | true            | C * B * C^{-1}
        // true            | false           | C * B^T * C^{-1}
        // false           | true            | (C * B * C^{-1})^T
        // false           | false           | (C * B^T * C^{-1})^T
        Matrix<T, N, N> VToU(Matrix<T, N, N> const& B) const
        {
            // vectorOnRightU  | vectorOnRightV  | output
            // ----------------+-----------------+---------------------
            // true            | true            | C * B * C^{-1}
            // true            | false           | C * B^T * C^{-1}
            // false           | true            | (C * B * C^{-1})^T
            // false           | false           | (C * B^T * C^{-1})^T
            Matrix<T, N, N> product{};

            if (mIsVectorOnRightV)
            {
                product = mC * B * mInverseC;
                if (mIsVectorOnRightU)
                {
                    return product;
                }
                else
                {
                    return Transpose(product);
                }
            }
            else
            {
                product = mC * MultiplyATB(B, mInverseC);
                if (mIsVectorOnRightU)
                {
                    return product;
                }
                else
                {
                    return Transpose(product);
                }
            }
        }

    private:
        // C = U^{-1}*V, C^{-1} = V^{-1}*U
        Matrix<T, N, N> mC, mInverseC;
        bool mIsVectorOnRightU, mIsVectorOnRightV;
        bool mIsRightHandedU, mIsRightHandedV;

        friend class UnitTestConvertCoordinates;
    };
}
