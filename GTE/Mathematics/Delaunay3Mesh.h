// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.03.12

#pragma once

#include <Mathematics/Delaunay3.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <typename T, typename...>
    class Delaunay3Mesh {};
}

namespace gte
{
    template <typename InputType, typename ComputeType, typename RationalType>
    class// [[deprecated("Use Delaunay3Mesh<T> instead.")]]
        Delaunay3Mesh<InputType, ComputeType, RationalType>
    {
    public:
        // Construction.
        Delaunay3Mesh(Delaunay3<InputType, ComputeType> const& delaunay)
            :
            mDelaunay(&delaunay)
        {
            LogAssert(
                mDelaunay->GetDimension() == 3,
                "Invalid Delaunay dimension.");
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
            typename Delaunay3<InputType, ComputeType>::SearchInfo info{};
            return mDelaunay->GetContainingTetrahedron(P, info);
        }

        bool GetVertices(int32_t t, std::array<Vector3<InputType>, 4>& vertices) const
        {
            std::array<int32_t, 4> indices{};
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
            else
            {
                for (auto& vertex : vertices)
                {
                    vertex.MakeZero();
                }
                return false;
            }
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
            std::array<int32_t, 4> indices{};
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

            for (auto& b : bary)
            {
                b = (InputType)0;
            }
            return false;
        }

    private:
        Delaunay3<InputType, ComputeType> const* mDelaunay;
    };
}

namespace gte
{
    template <typename T>
    class Delaunay3Mesh<T>
    {
    public:
        // Construction.
        Delaunay3Mesh(Delaunay3<T> const& delaunay)
            :
            mDelaunay(&delaunay)
        {
            static_assert(std::is_floating_point<T>::value,
                "The input type must be 'float' or 'double'.");

            LogAssert(
                mDelaunay->GetDimension() == 3,
                "Invalid Delaunay dimension.");
        }

        // Mesh information.
        inline size_t GetNumVertices() const
        {
            return mDelaunay->GetNumVertices();
        }

        inline size_t GetNumTetrahedra() const
        {
            return mDelaunay->GetNumTetrahedra();
        }

        inline Vector3<T> const* GetVertices() const
        {
            return mDelaunay->GetVertices();
        }

        inline int32_t const* GetIndices() const
        {
            return mDelaunay->GetIndices().data();
        }

        inline int32_t const* GetAdjacencies() const
        {
            return mDelaunay->GetAdjacencies().data();
        }

        // Containment queries.
        size_t GetContainingTetrahedron(Vector3<T> const& P) const
        {
            // VS 2019 16.8.1 generates LNT1006 "Local variable is not
            // initialized." Incorrect, because the default constructor
            // initializes all the members.
            typename Delaunay3<T>::SearchInfo info{};
            return mDelaunay->GetContainingTetrahedron(P, info);
        }

        inline size_t GetInvalidIndex() const
        {
            return mDelaunay->negOne;
        }

        bool GetVertices(size_t t, std::array<Vector3<T>, 4>& vertices) const
        {
            std::array<int32_t, 4> indices = { 0, 0, 0, 0 };
            if (mDelaunay->GetIndices(t, indices))
            {
                auto const* delaunayVertices = mDelaunay->GetVertices();
                for (size_t i = 0; i < 4; ++i)
                {
                    vertices[i] = delaunayVertices[indices[i]];
                }
                return true;
            }
            else
            {
                for (auto& vertex : vertices)
                {
                    vertex.MakeZero();
                }
                return false;
            }
        }

        bool GetIndices(size_t t, std::array<int32_t, 4>& indices) const
        {
            return mDelaunay->GetIndices(t, indices);
        }

        bool GetAdjacencies(size_t t, std::array<int32_t, 4>& adjacencies) const
        {
            return mDelaunay->GetAdjacencies(t, adjacencies);
        }

        bool GetBarycentrics(size_t t, Vector3<T> const& P, std::array<T, 4>& bary) const
        {
            std::array<int32_t, 4> indices = { 0, 0, 0, 0 };
            if (mDelaunay->GetIndices(t, indices))
            {
                auto const* delaunayVertices = mDelaunay->GetVertices();
                std::array<Vector3<Rational>, 4> rtV{};
                for (size_t i = 0; i < 4; ++i)
                {
                    auto const& V = delaunayVertices[indices[i]];
                    for (int32_t j = 0; j < 3; ++j)
                    {
                        rtV[i][j] = static_cast<Rational>(V[j]);
                    }
                };

                Vector3<Rational> rtP{ P[0], P[1], P[2]};
                std::array<Rational, 4> rtBary{};
                if (ComputeBarycentrics(rtP, rtV[0], rtV[1], rtV[2], rtV[3], rtBary))
                {
                    for (size_t i = 0; i < 4; ++i)
                    {
                        bary[i] = static_cast<T>(rtBary[i]);
                    }
                    return true;
                }
            }

            for (auto& b : bary)
            {
                b = static_cast<T>(0);
            }
            return false;
        }

    private:
        using Rational = BSRational<UIntegerAP32>;
        Delaunay3<T> const* mDelaunay;
    };
}
