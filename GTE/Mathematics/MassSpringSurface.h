// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/ParticleSystem.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <int32_t N, typename Real>
    class MassSpringSurface : public ParticleSystem<N, Real>
    {
    public:
        // Construction and destruction.  This class represents an RxC array
        // of masses lying on a surface and connected by an array of springs.
        // The masses are indexed by mass[r][c] for 0 <= r < R and 0 <= c < C.
        // The mass at interior position X[r][c] is connected by springs to
        // the masses at positions X[r-1][c], X[r+1][c], X[r][c-1], and
        // X[r][c+1].  Boundary masses have springs connecting them to the
        // obvious neighbors ("edge" mass has 3 neighbors, "corner" mass has
        // 2 neighbors).  The masses are arranged in row-major order:
        // position[c+C*r] = X[r][c] for 0 <= r < R and 0 <= c < C.  The
        // other arrays are stored similarly.
        virtual ~MassSpringSurface() = default;

        MassSpringSurface(int32_t numRows, int32_t numCols, Real step)
            :
            ParticleSystem<N, Real>(numRows* numCols, step),
            mNumRows(numRows),
            mNumCols(numCols),
            mConstantR(static_cast<size_t>(numRows) * static_cast<size_t>(numCols)),
            mLengthR(static_cast<size_t>(numRows)* static_cast<size_t>(numCols)),
            mConstantC(static_cast<size_t>(numRows)* static_cast<size_t>(numCols)),
            mLengthC(static_cast<size_t>(numRows)* static_cast<size_t>(numCols))
        {
            std::fill(mConstantR.begin(), mConstantR.end(), (Real)0);
            std::fill(mLengthR.begin(), mLengthR.end(), (Real)0);
            std::fill(mConstantC.begin(), mConstantC.end(), (Real)0);
            std::fill(mLengthC.begin(), mLengthC.end(), (Real)0);
        }

        // Member access.
        inline int32_t GetNumRows() const
        {
            return mNumRows;
        }

        inline int32_t GetNumCols() const
        {
            return mNumCols;
        }

        inline void SetMass(int32_t r, int32_t c, Real mass)
        {
            ParticleSystem<N, Real>::SetMass(GetIndex(r, c), mass);
        }

        inline void SetPosition(int32_t r, int32_t c, Vector<N, Real> const& position)
        {
            ParticleSystem<N, Real>::SetPosition(GetIndex(r, c), position);
        }

        inline void SetVelocity(int32_t r, int32_t c, Vector<N, Real> const& velocity)
        {
            ParticleSystem<N, Real>::SetVelocity(GetIndex(r, c), velocity);
        }

        inline Real const& GetMass(int32_t r, int32_t c) const
        {
            return ParticleSystem<N, Real>::GetMass(GetIndex(r, c));
        }

        inline Vector<N, Real> const& GetPosition(int32_t r, int32_t c) const
        {
            return ParticleSystem<N, Real>::GetPosition(GetIndex(r, c));
        }

        inline Vector<N, Real> const& GetVelocity(int32_t r, int32_t c) const
        {
            return ParticleSystem<N, Real>::GetVelocity(GetIndex(r, c));
        }

        // The interior mass at (r,c) has springs to the left, right, bottom,
        // and top.  Edge masses have only three neighbors and corner masses
        // have only two neighbors.  The mass at (r,c) provides access to the
        // springs connecting to locations (r,c+1) and (r+1,c).  Edge and
        // corner masses provide access to only a subset of these.  The caller
        // is responsible for ensuring the validity of the (r,c) inputs.

        // to (r+1,c)
        inline void SetConstantR(int32_t r, int32_t c, Real constant)
        {
            mConstantR[GetIndex(r, c)] = constant;
        }

        // to (r+1,c)
        inline void SetLengthR(int32_t r, int32_t c, Real length)
        {
            mLengthR[GetIndex(r, c)] = length;
        }

        // to (r,c+1)
        inline void SetConstantC(int32_t r, int32_t c, Real constant)
        {
            mConstantC[GetIndex(r, c)] = constant;
        }

        // to (r,c+1)
        inline void SetLengthC(int32_t r, int32_t c, Real length)
        {
            mLengthC[GetIndex(r, c)] = length;
        }

        inline Real const& GetConstantR(int32_t r, int32_t c) const
        {
            return mConstantR[GetIndex(r, c)];
        }

        inline Real const& GetLengthR(int32_t r, int32_t c) const
        {
            return mLengthR[GetIndex(r, c)];
        }

        inline Real const& GetConstantC(int32_t r, int32_t c) const
        {
            return mConstantC[GetIndex(r, c)];
        }

        inline Real const& GetLengthC(int32_t r, int32_t c) const
        {
            return mLengthC[GetIndex(r, c)];
        }

        // The default external force is zero.  Derive a class from this one
        // to provide nonzero external forces such as gravity, wind, friction,
        // and so on.  This function is called by Acceleration(...) to compute
        // the impulse F/m generated by the external force F.
        virtual Vector<N, Real> ExternalAcceleration(int32_t, Real,
            std::vector<Vector<N, Real>> const&,
            std::vector<Vector<N, Real>> const&)
        {
            return Vector<N, Real>::Zero();
        }

    protected:
        // Callback for acceleration (ODE solver uses x" = F/m) applied to
        // particle i.  The positions and velocities are not necessarily
        // mPosition and mVelocity, because the ODE solver evaluates the
        // impulse function at intermediate positions.
        virtual Vector<N, Real> Acceleration(int32_t i, Real time,
            std::vector<Vector<N, Real>> const& position,
            std::vector<Vector<N, Real>> const& velocity)
        {
            // Compute spring forces on position X[i].  The positions are not
            // necessarily mPosition, because the RK4 solver in ParticleSystem
            // evaluates the acceleration function at intermediate positions.
            // The edge and corner points of the surface of masses must be
            // handled separately, because each has fewer than four springs
            // attached to it.

            Vector<N, Real> acceleration = ExternalAcceleration(i, time, position, velocity);
            Vector<N, Real> diff, force;
            Real ratio;

            int32_t r, c, prev, next;
            GetCoordinates(i, r, c);

            if (r > 0)
            {
                prev = i - mNumCols;  // index to previous row-neighbor
                diff = position[prev] - position[i];
                ratio = GetLengthR(r - 1, c) / Length(diff);
                force = GetConstantR(r - 1, c) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            if (r < mNumRows - 1)
            {
                next = i + mNumCols;  // index to next row-neighbor
                diff = position[next] - position[i];
                ratio = GetLengthR(r, c) / Length(diff);
                force = GetConstantR(r, c) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            if (c > 0)
            {
                prev = i - 1;  // index to previous col-neighbor
                diff = position[prev] - position[i];
                ratio = GetLengthC(r, c - 1) / Length(diff);
                force = GetConstantC(r, c - 1) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            if (c < mNumCols - 1)
            {
                next = i + 1;  // index to next col-neighbor
                diff = position[next] - position[i];
                ratio = GetLengthC(r, c) / Length(diff);
                force = GetConstantC(r, c) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            return acceleration;
        }

        inline int32_t GetIndex(int32_t r, int32_t c) const
        {
            return c + mNumCols * r;
        }

        void GetCoordinates(int32_t i, int32_t& r, int32_t& c) const
        {
            c = i % mNumCols;
            r = i / mNumCols;
        }

        int32_t mNumRows, mNumCols;
        std::vector<Real> mConstantR, mLengthR;
        std::vector<Real> mConstantC, mLengthC;
    };
}
