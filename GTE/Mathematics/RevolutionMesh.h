// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/Constants.h>
#include <Mathematics/Mesh.h>
#include <Mathematics/ParametricCurve.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace gte
{
    template <typename Real>
    class RevolutionMesh : public Mesh<Real>
    {
    public:
        // The axis of revolution is the z-axis.  The curve of revolution is
        // p(t) = (x(t),z(t)), where t in [tmin,tmax], x(t) > 0 for t in
        // (tmin,tmax), x(tmin) >= 0, and x(tmax) >= 0.  The values tmin and
        // tmax are those for the curve object passed to the constructor.  The
        // curve must be non-self-intersecting, except possibly at its
        // endpoints.  The curve is closed when p(tmin) = p(tmax), in which
        // case the surface of revolution has torus topology.  The curve is
        // open when p(tmin) != p(tmax).  For an open curve, define
        // x0 = x(tmin) and x1 = x(tmax).  The surface has cylinder topology
        // when x0 > 0 and x1 > 0, disk topology when exactly one of x0 or x1
        // is zero, or sphere topology when x0 and x1 are both zero.  However,
        // to simplify the design, the mesh is always built using cylinder
        // topology.  The row samples correspond to curve points and the
        // column samples correspond to the points on the circles of
        // revolution.
        RevolutionMesh(MeshDescription const& description,
            std::shared_ptr<ParametricCurve<2, Real>> const& curve,
            bool sampleByArcLength = false)
            :
            Mesh<Real>(description,
                { MeshTopology::CYLINDER, MeshTopology::TORUS, MeshTopology::DISK, MeshTopology::SPHERE }),
            mCurve(curve),
            mSampleByArcLength(sampleByArcLength)
        {
            if (!this->mDescription.constructed)
            {
                // The logger system will report these errors in the Mesh
                // constructor.
                mCurve = nullptr;
                return;
            }

            LogAssert(mCurve != nullptr, "A nonnull revolution curve is required.");

            // The four supported topologies all wrap around in the column
            // direction.
            mCosAngle.resize(static_cast<size_t>(this->mDescription.numCols) + 1);
            mSinAngle.resize(static_cast<size_t>(this->mDescription.numCols) + 1);
            Real invRadialSamples = (Real)1 / (Real)this->mDescription.numCols;
            for (uint32_t c = 0; c < this->mDescription.numCols; ++c)
            {
                Real angle = c * invRadialSamples * (Real)GTE_C_TWO_PI;
                mCosAngle[c] = std::cos(angle);
                mSinAngle[c] = std::sin(angle);
            }
            mCosAngle[this->mDescription.numCols] = mCosAngle[0];
            mSinAngle[this->mDescription.numCols] = mSinAngle[0];

            CreateSampler();

            if (!this->mTCoords)
            {
                mDefaultTCoords.resize(this->mDescription.numVertices);
                this->mTCoords = mDefaultTCoords.data();
                this->mTCoordStride = sizeof(Vector2<Real>);

                this->mDescription.allowUpdateFrame = this->mDescription.wantDynamicTangentSpaceUpdate;
                if (this->mDescription.allowUpdateFrame)
                {
                    if (!this->mDescription.hasTangentSpaceVectors)
                    {
                        this->mDescription.allowUpdateFrame = false;
                    }

                    if (!this->mNormals)
                    {
                        this->mDescription.allowUpdateFrame = false;
                    }
                }
            }

            this->ComputeIndices();
            InitializeTCoords();
            UpdatePositions();
            if (this->mDescription.allowUpdateFrame)
            {
                this->UpdateFrame();
            }
            else if (this->mNormals)
            {
                this->UpdateNormals();
            }
        }

        // Member access.
        inline std::shared_ptr<ParametricCurve<2, Real>> const& GetCurve() const
        {
            return mCurve;
        }

        inline bool IsSampleByArcLength() const
        {
            return mSampleByArcLength;
        }

    private:
        void CreateSampler()
        {
            if (this->mDescription.topology == MeshTopology::CYLINDER
                || this->mDescription.topology == MeshTopology::TORUS)
            {
                mSamples.resize(static_cast<size_t>(this->mDescription.rMax) + 1);
            }
            else if (this->mDescription.topology == MeshTopology::DISK)
            {
                mSamples.resize(static_cast<size_t>(this->mDescription.rMax) + 2);
            }
            else if (this->mDescription.topology == MeshTopology::SPHERE)
            {
                mSamples.resize(static_cast<size_t>(this->mDescription.rMax) + 3);
            }

            Real invDenom = (Real)1 / ((Real)mSamples.size() - (Real)1);
            if (mSampleByArcLength)
            {
                Real factor = mCurve->GetTotalLength() * invDenom;
                mTSampler = [this, factor](uint32_t i)
                {
                    return mCurve->GetTime(i * factor);
                };
            }
            else
            {
                Real factor = (mCurve->GetTMax() - mCurve->GetTMin()) * invDenom;
                mTSampler = [this, factor](uint32_t i)
                {
                    return mCurve->GetTMin() + i * factor;
                };
            }
        }

        void InitializeTCoords()
        {
            Vector2<Real>tcoord;

            switch (this->mDescription.topology)
            {
            case MeshTopology::CYLINDER:
            {
                for (uint32_t r = 0, i = 0; r < this->mDescription.numRows; ++r)
                {
                    tcoord[1] = (Real)r / (Real)(this->mDescription.numRows - 1);
                    for (uint32_t c = 0; c <= this->mDescription.numCols; ++c, ++i)
                    {
                        tcoord[0] = (Real)c / (Real)this->mDescription.numCols;
                        this->TCoord(i) = tcoord;
                    }
                }
                break;
            }
            case MeshTopology::TORUS:
            {
                for (uint32_t r = 0, i = 0; r <= this->mDescription.numRows; ++r)
                {
                    tcoord[1] = (Real)r / (Real)this->mDescription.numRows;
                    for (uint32_t c = 0; c <= this->mDescription.numCols; ++c, ++i)
                    {
                        tcoord[0] = (Real)c / (Real)this->mDescription.numCols;
                        this->TCoord(i) = tcoord;
                    }
                }
                break;
            }
            case MeshTopology::DISK:
            {
                Vector2<Real> origin{ (Real)0.5, (Real)0.5 };
                uint32_t i = 0;
                for (uint32_t r = 0; r < this->mDescription.numRows; ++r)
                {
                    Real radius = ((Real)r + (Real)1) / ((Real)2 * (Real)this->mDescription.numRows);
                    radius = std::min(radius, (Real)0.5);
                    for (uint32_t c = 0; c <= this->mDescription.numCols; ++c, ++i)
                    {
                        Real angle = (Real)GTE_C_TWO_PI * (Real)c / (Real)this->mDescription.numCols;
                        this->TCoord(i) = { radius * std::cos(angle), radius * std::sin(angle) };
                    }
                }
                this->TCoord(i) = origin;
                break;
            }
            case MeshTopology::SPHERE:
            {
                uint32_t i = 0;
                for (uint32_t r = 0; r < this->mDescription.numRows; ++r)
                {
                    tcoord[1] = (Real)r / (Real)(this->mDescription.numRows - 1);
                    for (uint32_t c = 0; c <= this->mDescription.numCols; ++c, ++i)
                    {
                        tcoord[0] = (Real)c / (Real)this->mDescription.numCols;
                        this->TCoord(i) = tcoord;
                    }
                }
                this->TCoord(i++) = { (Real)0.5, (Real)0 };
                this->TCoord(i) = { (Real)0.5, (Real)1 };
                break;
            }
            default:
                // Invalid topology is reported by the Mesh constructor, so there is
                // no need to log a message here.
                break;
            }
        }

        virtual void UpdatePositions() override
        {
            uint32_t const numSamples = static_cast<uint32_t>(mSamples.size());
            for (uint32_t i = 0; i < numSamples; ++i)
            {
                Real t = mTSampler(i);
                Vector2<Real> position = mCurve->GetPosition(t);
                mSamples[i][0] = position[0];
                mSamples[i][1] = (Real)0;
                mSamples[i][2] = position[1];
            }

            switch (this->mDescription.topology)
            {
            case MeshTopology::CYLINDER:
                UpdateCylinderPositions();
                break;
            case MeshTopology::TORUS:
                UpdateTorusPositions();
                break;
            case MeshTopology::DISK:
                UpdateDiskPositions();
                break;
            case MeshTopology::SPHERE:
                UpdateSpherePositions();
                break;
            default:
                break;
            }
        }

        void UpdateCylinderPositions()
        {
            for (uint32_t r = 0, i = 0; r <= this->mDescription.rMax; ++r)
            {
                Real radius = mSamples[r][0];
                for (uint32_t c = 0; c <= this->mDescription.cMax; ++c, ++i)
                {
                    this->Position(i) = { radius * mCosAngle[c], radius * mSinAngle[c], mSamples[r][2] };
                }
            }
        }

        void UpdateTorusPositions()
        {
            for (uint32_t r = 0, i = 0; r <= this->mDescription.rMax; ++r)
            {
                Real radius = mSamples[r][0];
                for (uint32_t c = 0; c <= this->mDescription.cMax; ++c, ++i)
                {
                    this->Position(i) = { radius * mCosAngle[c], radius * mSinAngle[c], mSamples[r][2] };
                }
            }
        }

        void UpdateDiskPositions()
        {
            for (uint32_t r = 0, rp1 = 1, i = 0; r <= this->mDescription.rMax; ++r, ++rp1)
            {
                Real radius = mSamples[rp1][0];
                for (uint32_t c = 0; c <= this->mDescription.cMax; ++c, ++i)
                {
                    this->Position(i) = { radius * mCosAngle[c], radius * mSinAngle[c], mSamples[rp1][2] };
                }
            }

            this->Position(this->mDescription.numVertices - 1) = { (Real)0, (Real)0, mSamples.front()[2] };
        }

        void UpdateSpherePositions()
        {
            for (uint32_t r = 0, rp1 = 1, i = 0; r <= this->mDescription.rMax; ++r, ++rp1)
            {
                Real radius = mSamples[rp1][0];
                for (uint32_t c = 0; c <= this->mDescription.cMax; ++c, ++i)
                {
                    this->Position(i) = { radius * mCosAngle[c], radius * mSinAngle[c], mSamples[rp1][2] };
                }
            }

            this->Position(this->mDescription.numVertices - 2) = { (Real)0, (Real)0, mSamples.front()[2] };
            this->Position(this->mDescription.numVertices - 1) = { (Real)0, (Real)0, mSamples.back()[2] };
        }

        std::shared_ptr<ParametricCurve<2, Real>> mCurve;
        bool mSampleByArcLength;
        std::vector<Real> mCosAngle, mSinAngle;
        std::function<Real(uint32_t)> mTSampler;
        std::vector<Vector3<Real>> mSamples;

        // If the client does not request texture coordinates, they will be
        // computed internally for use in evaluation of the surface geometry.
        std::vector<Vector2<Real>> mDefaultTCoords;
    };
}
