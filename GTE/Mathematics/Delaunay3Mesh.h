// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.02.01

#pragma once

#include <Mathematics/Delaunay3.h>

namespace gte
{
    template <typename InputType, typename ComputeType, typename RationalType>
    class Delaunay3Mesh
    {
    public:
        // Construction.
        Delaunay3Mesh(Delaunay3<InputType, ComputeType> const& delaunay)
            :
            mDelaunay(&delaunay)
        {
        }

        // Mesh information.
        inline int32_t GetNumVertices() const
        {
            return mDelaunay->GetNumVertices();
        }

        inline int32_t GetNumTetrahedra() const
        {
            return mDelaunay->GetNumTetrahedra();
        }

        inline Vector3<InputType> const* GetVertices() const
        {
            return mDelaunay->GetVertices();
        }

        inline int32_t const* GetIndices() const
        {
            return &mDelaunay->GetIndices()[0];
        }

        inline int32_t const* GetAdjacencies() const
        {
            return &mDelaunay->GetAdjacencies()[0];
        }

        // Containment queries.

        int32_t GetContainingTetrahedron(Vector3<InputType> const& P) const
        {
            typename Delaunay3<InputType, ComputeType>::SearchInfo info;
            return mDelaunay->GetContainingTetrahedron(P, info);
        }

        bool GetVertices(int32_t t, std::array<Vector3<InputType>, 4>& vertices) const
        {
            if (mDelaunay->GetDimension() == 3)
            {
                std::array<int32_t, 4> indices;
                if (mDelaunay->GetIndices(t, indices))
                {
                    PrimalQuery3<ComputeType> const& query = mDelaunay->GetQuery();
                    Vector3<ComputeType> const* ctVertices = query.GetVertices();
                    for (int32_t i = 0; i < 4; ++i)
                    {
                        Vector3<ComputeType> const& V = ctVertices[indices[i]];
                        for (int32_t j = 0; j < 3; ++j)
                        {
                            vertices[i][j] = (InputType)V[j];
                        }
                    }
                    return true;
                }
            }
            return false;
        }

        bool GetIndices(int32_t t, std::array<int32_t, 4>& indices) const
        {
            return mDelaunay->GetIndices(t, indices);
        }

        bool GetAdjacencies(int32_t t, std::array<int32_t, 4>& adjacencies) const
        {
            return mDelaunay->GetAdjacencies(t, adjacencies);
        }

        bool GetBarycentrics(int32_t t, Vector3<InputType> const& P, std::array<InputType, 4>& bary) const
        {
            std::array<int32_t, 4> indices;
            if (mDelaunay->GetIndices(t, indices))
            {
                PrimalQuery3<ComputeType> const& query = mDelaunay->GetQuery();
                Vector3<ComputeType> const* vertices = query.GetVertices();
                Vector3<RationalType> rtP{ P[0], P[1], P[2] };
                std::array<Vector3<RationalType>, 4> rtV;
                for (int32_t i = 0; i < 4; ++i)
                {
                    Vector3<ComputeType> const& V = vertices[indices[i]];
                    for (int32_t j = 0; j < 3; ++j)
                    {
                        rtV[i][j] = (RationalType)V[j];
                    }
                };

                std::array<RationalType, 4> rtBary{};
                if (ComputeBarycentrics(rtP, rtV[0], rtV[1], rtV[2], rtV[3], rtBary))
                {
                    for (int32_t i = 0; i < 4; ++i)
                    {
                        bary[i] = (InputType)rtBary[i];
                    }
                    return true;
                }
            }
            return false;
        }

    private:
        Delaunay3<InputType, ComputeType> const* mDelaunay;
    };
}
