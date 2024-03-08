// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// A class for solving the Linear Complementarity Problem (LCP)
// w = q + M * z, w^T * z = 0, w >= 0, z >= 0.  The vectors q, w, and z are
// n-tuples and the matrix M is n-by-n.  The inputs to Solve(...) are q and M.
// The outputs are w and z, which are valid when the returned bool is true but
// are invalid when the returned bool is false.
//
// The comments at the end of this file explain what the preprocessor symbol
// means regarding the LCP solver implementation.  If the algorithm fails to
// converge within the specified maximum number of iterations, consider
// increasing the number and calling Solve(...) again.

// Expose the following preprocessor symbol if you want the code to throw an
// exception the algorithm fails to converge.  You can choose to trap the
// exception and handle it as you please.  If you do not expose the
// preprocessor symbol, you can pass a Result object and check whether the
// algorithm failed to converge.  Again, you can handle this as you please.
//
//#define GTE_THROW_ON_LCPSOLVER_ERRORS

#include <Mathematics/Logger.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    // Support templates for number of dimensions known at compile time or
    // known only at run time.
    template <typename T, int32_t... Dimensions>
    class LCPSolver {};


    template <typename T>
    class LCPSolverShared
    {
    protected:
        // Abstract base class construction.  A virtual destructor is not
        // provided because there are no required side effects when destroying
        // objects from the derived classes.  The member mMaxIterations is set
        // by this call to the default value n*n.
        LCPSolverShared(int32_t n)
            :
            mNumIterations(0),
            mVarBasic(nullptr),
            mVarNonbasic(nullptr),
            mNumCols(0),
            mAugmented(nullptr),
            mQMin(nullptr),
            mMinRatio(nullptr),
            mRatio(nullptr),
            mPoly(nullptr),
            mZero((T)0),
            mOne((T)1)
        {
            if (n > 0)
            {
                mDimension = n;
                mMaxIterations = n * n;
            }
            else
            {
                mDimension = 0;
                mMaxIterations = 0;
            }
        }

        // Use this constructor when you need a specific representation of
        // zero and of one to be used when manipulating the polynomials of the
        // base class. In particular, this is needed to select the correct
        // zero and correct one for QFNumber objects.
        LCPSolverShared(int32_t n, T const& zero, T const& one)
            :
            mNumIterations(0),
            mVarBasic(nullptr),
            mVarNonbasic(nullptr),
            mNumCols(0),
            mAugmented(nullptr),
            mQMin(nullptr),
            mMinRatio(nullptr),
            mRatio(nullptr),
            mPoly(nullptr),
            mZero(zero),
            mOne(one)
        {
            if (n > 0)
            {
                mDimension = n;
                mMaxIterations = n * n;
            }
            else
            {
                mDimension = 0;
                mMaxIterations = 0;
            }
        }

    public:
        // Theoretically, when there is a solution the algorithm must converge
        // in a finite number of iterations.  The number of iterations depends
        // on the problem at hand, but we need to guard against an infinite
        // loop by limiting the number.  The implementation uses a maximum
        // number of n*n (chosen arbitrarily).  You can set the number
        // yourself, perhaps when a call to Solve fails--increase the number
        // of iterations and call and solve again.
        inline void SetMaxIterations(int32_t maxIterations)
        {
            mMaxIterations = (maxIterations > 0 ? maxIterations : mDimension * mDimension);
        }

        inline int32_t GetMaxIterations() const
        {
            return mMaxIterations;
        }

        // Access the actual number of iterations used in a call to Solve.
        inline int32_t GetNumIterations() const
        {
            return mNumIterations;
        }

        enum class Result
        {
            HAS_TRIVIAL_SOLUTION,
            HAS_NONTRIVIAL_SOLUTION,
            NO_SOLUTION,
            FAILED_TO_CONVERGE,
            INVALID_INPUT
        };

    protected:
        // Bookkeeping of variables during the iterations of the solver.  The
        // name is either 'w' or 'z' and is used for human-readable debugging
        // help.  The 'index' is that for the original variables w[index] or
        // z[index].  The 'complementary' index is the location of the
        // complementary variable in mVarBasic[] or in mVarNonbasic[].  The
        // 'tuple' is a pointer to &w[0] or &z[0], the choice based on name of
        // 'w' or 'z', and is used to fill in the solution values (the
        // variables are permuted during the pivoting algorithm).
        struct Variable
        {
            Variable()
                :
                name(0),
                index(0),
                complementary(0),
                tuple(nullptr)
            {
            }

            char name;
            int32_t index;
            int32_t complementary;
            T* tuple;
        };

        // The augmented problem is w = q + M*z + z[n]*U = 0, where U is an
        // n-tuple of 1-values.  We manipulate the augmented matrix
        // [M | U | p(t)] where p(t) is a column vector of polynomials of at
        // most degree n.  If p[r](t) is the polynomial for row r, then
        // p[r](0) = q[r].  These are perturbations of q[r] designed so that
        // the algorithm avoids degeneracies (a q-term becomes zero during the
        // iterations).  The basic variables are w[0] through w[n-1] and the
        // nonbasic variables are z[0] through z[n].  The returned z consists
        // only of z[0] through z[n-1].

        // The derived classes ensure that the pointers point to the correct
        // of elements for each array.  The matrix M must be stored in
        // row-major order.
        bool Solve(T const* q, T const* M, T* w, T* z, Result* result)
        {
            // Perturb the q[r] constants to be polynomials of degree r+1
            // represented as an array of n+1 coefficients.  The coefficient
            // with index r+1 is 1 and the coefficients with indices larger
            // than r+1 are 0.
            for (int32_t r = 0; r < mDimension; ++r)
            {
                mPoly[r] = &Augmented(r, mDimension + 1);
                MakeZero(mPoly[r]);
                mPoly[r][0] = q[r];
                mPoly[r][r + 1] = mOne;
            }

            // Determine whether there is the trivial solution w = z = 0.
            Copy(mPoly[0], mQMin);
            int32_t basic = 0;
            for (int32_t r = 1; r < mDimension; ++r)
            {
                if (LessThan(mPoly[r], mQMin))
                {
                    Copy(mPoly[r], mQMin);
                    basic = r;
                }
            }

            if (!LessThanZero(mQMin))
            {
                for (int32_t r = 0; r < mDimension; ++r)
                {
                    w[r] = q[r];
                    z[r] = mZero;
                }

                if (result)
                {
                    *result = Result::HAS_TRIVIAL_SOLUTION;
                }
                return true;
            }

            // Initialize the remainder of the augmented matrix with M and U.
            for (int32_t r = 0; r < mDimension; ++r)
            {
                for (int32_t c = 0; c < mDimension; ++c)
                {
                    Augmented(r, c) = M[c + mDimension * r];
                }
                Augmented(r, mDimension) = mOne;
            }

            // Keep track of when the variables enter and exit the dictionary,
            // including where complementary variables are relocated.
            for (int32_t i = 0; i <= mDimension; ++i)
            {
                mVarBasic[i].name = 'w';
                mVarBasic[i].index = i;
                mVarBasic[i].complementary = i;
                mVarBasic[i].tuple = w;
                mVarNonbasic[i].name = 'z';
                mVarNonbasic[i].index = i;
                mVarNonbasic[i].complementary = i;
                mVarNonbasic[i].tuple = z;
            }

            // The augmented variable z[n] is the initial driving variable for
            // pivoting.  The equation 'basic' is the one to solve for z[n]
            // and pivoting with w[basic].  The last column of M remains all
            // 1-values for this initial step, so no algebraic computations
            // occur for M[r][n].
            int32_t driving = mDimension;
            for (int32_t r = 0; r < mDimension; ++r)
            {
                if (r != basic)
                {
                    for (int32_t c = 0; c < mNumCols; ++c)
                    {
                        if (c != mDimension)
                        {
                            Augmented(r, c) -= Augmented(basic, c);
                        }
                    }
                }
            }

            for (int32_t c = 0; c < mNumCols; ++c)
            {
                if (c != mDimension)
                {
                    Augmented(basic, c) = -Augmented(basic, c);
                }
            }

            mNumIterations = 0;
            for (int32_t i = 0; i < mMaxIterations; ++i, ++mNumIterations)
            {
                // The basic variable of equation 'basic' exited the
                // dictionary, so/ its complementary (nonbasic) variable must
                // become the next driving variable in order for it to enter
                // the dictionary.
                int32_t nextDriving = mVarBasic[basic].complementary;
                mVarNonbasic[nextDriving].complementary = driving;
                std::swap(mVarBasic[basic], mVarNonbasic[driving]);
                if (mVarNonbasic[driving].index == mDimension)
                {
                    // The algorithm has converged.
                    for (int32_t r = 0; r < mDimension; ++r)
                    {
                        mVarBasic[r].tuple[mVarBasic[r].index] = mPoly[r][0];
                    }
                    for (int32_t c = 0; c <= mDimension; ++c)
                    {
                        int32_t index = mVarNonbasic[c].index;
                        if (index < mDimension)
                        {
                            mVarNonbasic[c].tuple[index] = mZero;
                        }
                    }
                    if (result)
                    {
                        *result = Result::HAS_NONTRIVIAL_SOLUTION;
                    }
                    return true;
                }

                // Determine the 'basic' equation for which the ratio
                // -q[r]/M(r,driving) is minimized among all equations r with
                // M(r,driving) < 0.
                driving = nextDriving;
                basic = -1;
                for (int32_t r = 0; r < mDimension; ++r)
                {
                    if (Augmented(r, driving) < mZero)
                    {
                        T factor = -mOne / Augmented(r, driving);
                        Multiply(mPoly[r], factor, mRatio);
                        if (basic == -1 || LessThan(mRatio, mMinRatio))
                        {
                            Copy(mRatio, mMinRatio);
                            basic = r;
                        }
                    }
                }

                if (basic == -1)
                {
                    // The coefficients of z[driving] in all the equations are
                    // nonnegative, so the z[driving] variable cannot leave
                    // the dictionary.  There is no solution to the LCP.
                    for (int32_t r = 0; r < mDimension; ++r)
                    {
                        w[r] = mZero;
                        z[r] = mZero;
                    }

                    if (result)
                    {
                        *result = Result::NO_SOLUTION;
                    }
                    return false;
                }

                // Solve the basic equation so that z[driving] enters the
                // dictionary and w[basic] exits the dictionary.
                T invDenom = mOne / Augmented(basic, driving);
                for (int32_t r = 0; r < mDimension; ++r)
                {
                    if (r != basic && Augmented(r, driving) != mZero)
                    {
                        T multiplier = Augmented(r, driving) * invDenom;
                        for (int32_t c = 0; c < mNumCols; ++c)
                        {
                            if (c != driving)
                            {
                                Augmented(r, c) -= Augmented(basic, c) * multiplier;
                            }
                            else
                            {
                                Augmented(r, driving) = multiplier;
                            }
                        }
                    }
                }

                for (int32_t c = 0; c < mNumCols; ++c)
                {
                    if (c != driving)
                    {
                        Augmented(basic, c) = -Augmented(basic, c) * invDenom;
                    }
                    else
                    {
                        Augmented(basic, driving) = invDenom;
                    }
                }
            }

            // Numerical round-off errors can cause the Lemke algorithm not to
            // converge.  In particular, the code above has a test
            //   if (mAugmented[r][driving] < (T)0) { ... }
            // to determine the 'basic' equation with which to pivot.  It is
            // possible that theoretically mAugmented[r][driving]is zero but
            // rounding errors cause it to be slightly negative.  If
            // theoretically all mAugmented[r][driving] >= 0, there is no
            // solution to the LCP.  With the rounding errors, if the
            // algorithm fails to converge within the specified number of
            // iterations, NO_SOLUTION is returned, which is hopefully the
            // correct result.  It is also possible that the rounding errors
            // lead to a NO_SOLUTION (returned from inside the loop) when in
            // fact there is a solution. When the LCP solver is used by
            // intersection testing algorithms, the hope is that
            // misclassifications occur only when the two objects are nearly
            // in tangential contact.
            //
            // To determine whether the rounding errors are the problem, you
            // can execute the query using exact arithmetic with the following
            // type used for 'T' (replacing 'float' or 'double') of
            // BSRational<UIntegerAP32> Rational.
            //
            // That said, if the algorithm fails to converge and you believe
            // that the rounding errors are not causing this, please file a
            // bug report and provide the input data to the solver.

#if defined(GTE_THROW_ON_LCPSOLVER_ERRORS)
            LogError("LCPSolverShared::Solve failed to converge.");
#endif
            if (result)
            {
                *result = Result::FAILED_TO_CONVERGE;
            }
            return false;
        }

        // Access mAugmented as a 2-dimensional array.
        inline T const& Augmented(int32_t row, int32_t col) const
        {
            return mAugmented[col + mNumCols * row];
        }

        inline T& Augmented(int32_t row, int32_t col)
        {
            return mAugmented[col + mNumCols * row];
        }

        // Support for polynomials with n+1 coefficients and degree no larger
        // than n.
        void MakeZero(T* poly)
        {
            for (int32_t i = 0; i <= mDimension; ++i)
            {
                poly[i] = mZero;
            }
        }

        void Copy(T const* poly0, T* poly1)
        {
            for (int32_t i = 0; i <= mDimension; ++i)
            {
                poly1[i] = poly0[i];
            }
        }

        bool LessThan(T const* poly0, T const* poly1)
        {
            for (int32_t i = 0; i <= mDimension; ++i)
            {
                if (poly0[i] < poly1[i])
                {
                    return true;
                }

                if (poly0[i] > poly1[i])
                {
                    return false;
                }
            }

            return false;
        }

        bool LessThanZero(T const* poly)
        {
            for (int32_t i = 0; i <= mDimension; ++i)
            {
                if (poly[i] < mZero)
                {
                    return true;
                }

                if (poly[i] > mZero)
                {
                    return false;
                }
            }

            return false;
        }

        void Multiply(T const* poly, T scalar, T* product)
        {
            for (int32_t i = 0; i <= mDimension; ++i)
            {
                product[i] = poly[i] * scalar;
            }
        }

        int32_t mDimension;
        int32_t mMaxIterations;
        int32_t mNumIterations;

        // These pointers are set by the derived-class constructors to arrays
        // that have the correct number of elements.  The arrays mVarBasic,
        // mVarNonbasic, mQMin, mMinRatio, and mRatio each have n+1 elements.
        // The mAugmented array has n rows and 2*(n+1) columns stored in
        // row-major order in a 1-dimensional array.  The array of pointers
        // mPoly has n elements.
        Variable* mVarBasic;
        Variable* mVarNonbasic;
        int32_t mNumCols;
        T* mAugmented;
        T* mQMin;
        T* mMinRatio;
        T* mRatio;
        T** mPoly;
        T mZero, mOne;
    };


    template <typename T, int32_t n>
    class LCPSolver<T, n> : public LCPSolverShared<T>
    {
    public:
        // Construction.  The member mMaxIterations is set by this call to the
        // default value n*n.
        LCPSolver()
            :
            LCPSolverShared<T>(n)
        {
            this->mVarBasic = mArrayVarBasic.data();
            this->mVarNonbasic = mArrayVarNonbasic.data();
            this->mNumCols = 2 * (n + 1);
            this->mAugmented = mArrayAugmented.data();
            this->mQMin = mArrayQMin.data();
            this->mMinRatio = mArrayMinRatio.data();
            this->mRatio = mArrayRatio.data();
            this->mPoly = mArrayPoly.data();
        }

        // Use this constructor when you need a specific representation of
        // zero and of one to be used when manipulating the polynomials of the
        // base class. In particular, this is needed to select the correct
        // zero and correct one for QFNumber objects.
        LCPSolver(T const& zero, T const& one)
            :
            LCPSolverShared<T>(n, zero, one)
        {
            this->mVarBasic = mArrayVarBasic.data();
            this->mVarNonbasic = mArrayVarNonbasic.data();
            this->mNumCols = 2 * (n + 1);
            this->mAugmented = mArrayAugmented.data();
            this->mQMin = mArrayQMin.data();
            this->mMinRatio = mArrayMinRatio.data();
            this->mRatio = mArrayRatio.data();
            this->mPoly = mArrayPoly.data();
        }

        // If you want to know specifically why 'true' or 'false' was
        // returned, pass the address of a Result variable as the last
        // parameter.
        bool Solve(std::array<T, n> const& q, std::array<std::array<T, n>, n> const& M,
            std::array<T, n>& w, std::array<T, n>& z,
            typename LCPSolverShared<T>::Result* result = nullptr)
        {
            return LCPSolverShared<T>::Solve(q.data(), M.front().data(), w.data(), z.data(), result);
        }

    private:
        std::array<typename LCPSolverShared<T>::Variable, n + 1> mArrayVarBasic;
        std::array<typename LCPSolverShared<T>::Variable, n + 1> mArrayVarNonbasic;
        std::array<T, 2 * (n + 1)* n> mArrayAugmented;
        std::array<T, n + 1> mArrayQMin;
        std::array<T, n + 1> mArrayMinRatio;
        std::array<T, n + 1> mArrayRatio;
        std::array<T*, n> mArrayPoly;
    };


    template <typename T>
    class LCPSolver<T> : public LCPSolverShared<T>
    {
    public:
        // Construction.  The member mMaxIterations is set by this call to the
        // default value n*n.
        LCPSolver(int32_t n)
            :
            LCPSolverShared<T>(n)
        {
            if (n > 0)
            {
                size_t np1 = static_cast<size_t>(n) + 1;
                mVectorVarBasic.resize(np1);
                mVectorVarNonbasic.resize(np1);
                mVectorAugmented.resize(2 * np1 * n);
                mVectorQMin.resize(np1);
                mVectorMinRatio.resize(np1);
                mVectorRatio.resize(np1);
                mVectorPoly.resize(n);

                this->mVarBasic = mVectorVarBasic.data();
                this->mVarNonbasic = mVectorVarNonbasic.data();
                this->mNumCols = 2 * (n + 1);
                this->mAugmented = mVectorAugmented.data();
                this->mQMin = mVectorQMin.data();
                this->mMinRatio = mVectorMinRatio.data();
                this->mRatio = mVectorRatio.data();
                this->mPoly = mVectorPoly.data();
            }
        }

        // The input q must have n elements and the input M must be an n-by-n
        // matrix stored in row-major order.  The outputs w and z have n
        // elements.  If you want to know specifically why 'true' or 'false'
        // was returned, pass the address of a Result variable as the last
        // parameter.
        bool Solve(std::vector<T> const& q, std::vector<T> const& M,
            std::vector<T>& w, std::vector<T>& z,
            typename LCPSolverShared<T>::Result* result = nullptr)
        {
            if (this->mDimension > static_cast<int32_t>(q.size())
                || this->mDimension * this->mDimension > static_cast<int32_t>(M.size()))
            {
                if (result)
                {
                    *result = LCPSolverShared<T>::Result::INVALID_INPUT;
                }
                return false;
            }

            if (this->mDimension > static_cast<int32_t>(w.size()))
            {
                w.resize(this->mDimension);
            }

            if (this->mDimension > static_cast<int32_t>(z.size()))
            {
                z.resize(this->mDimension);
            }

            return LCPSolverShared<T>::Solve(q.data(), M.data(), w.data(), z.data(), result);
        }

    private:
        std::vector<typename LCPSolverShared<T>::Variable> mVectorVarBasic;
        std::vector<typename LCPSolverShared<T>::Variable> mVectorVarNonbasic;
        std::vector<T> mVectorAugmented;
        std::vector<T> mVectorQMin;
        std::vector<T> mVectorMinRatio;
        std::vector<T> mVectorRatio;
        std::vector<T*> mVectorPoly;
    };
}
