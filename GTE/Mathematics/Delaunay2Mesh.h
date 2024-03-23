// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.03.12

#pragma once

#include <Mathematics/Delaunay2.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T, typename...>
    class Delaunay2Mesh {};
}

namespace gte
{
    // The InputType is 'float' or 'double'. The ComputeType can be a
    // floating-point type or BSNumber<*> type, because it does not require
    // divisions. The RationalType requires division, so you can use
    // BSRational<*>.

    template <typename InputType, typename ComputeType, typename RationalType>
    class // [[deprecated("Use Delaunay2Mesh<T> instead.")]]
        Delaunay2Mesh<InputType, ComputeType, RationalType>
    {
    public:
        // Construction.
        Delaunay2Mesh(Delaunay2<InputType, ComputeType> const& delaunay)
            :
            mDelaunay(&delaunay)
        {
            LogAssert(
                mDelaunay->GetDimension() == 2,
                "Invalid Delaunay dimension.");
        }

        // Mesh information.
        inline int32_t GetNumVertices() const
        {
            return mDelaunay->GetNumVertices();
        }

        inline int32_t GetNumTriangles() const
        {
            return mDelaunay->GetNumTriangles();
        }

        inline Vector2<InputType> const* GetVertices() const
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

        inline int32_t GetInvalidIndex() const
        {
            return mDelaunay->negOne;
        }

        // Containment queries.
        int32_t GetContainingTriangle(Vector2<InputType> const& P) const
        {
            // VS 2019 16.8.1 generates LNT1006 "Local variable is not
            // initialized." Incorrect, because the default constructor
            // initializes all the members.
            typename Delaunay2<InputType, ComputeType>::SearchInfo info{};
            return mDelaunay->GetContainingTriangle(P, info);
        }

        bool GetVertices(int32_t t, std::array<Vector2<InputType>, 3>& vertices) const
        {
            std::array<int32_t, 3> indices = { 0, 0, 0 };
            if (mDelaunay->GetIndices(t, indices))
            {
                PrimalQuery2<ComputeType> const& query = mDelaunay->GetQuery();
                Vector2<ComputeType> const* ctVertices = query.GetVertices();
                for (int32_t i = 0; i < 3; ++i)
                {
                    Vector2<ComputeType> const& V = ctVertices[indices[i]];
                    for (int32_t j = 0; j < 2; ++j)
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

        bool GetIndices(int32_t t, std::array<int32_t, 3>& indices) const
        {
            return mDelaunay->GetIndices(t, indices);
        }

        bool GetAdjacencies(int32_t t, std::array<int32_t, 3>& adjacencies) const
        {
            return mDelaunay->GetAdjacencies(t, adjacencies);
        }

        bool GetBarycentrics(int32_t t, Vector2<InputType> const& P, std::array<InputType, 3>& bary) const
        {
            std::array<int32_t, 3> indices = { 0, 0, 0 };
            if (mDelaunay->GetIndices(t, indices))
            {
                PrimalQuery2<ComputeType> const& query = mDelaunay->GetQuery();
                Vector2<ComputeType> const* vertices = query.GetVertices();
                Vector2<RationalType> rtP{ P[0], P[1] };
                std::array<Vector2<RationalType>, 3> rtV;
                for (int32_t i = 0; i < 3; ++i)
                {
                    Vector2<ComputeType> const& V = vertices[indices[i]];
                    for (int32_t j = 0; j < 2; ++j)
                    {
                        rtV[i][j] = (RationalType)V[j];
                    }
                };

                std::array<RationalType, 3> rtBary{};
                if (ComputeBarycentrics(rtP, rtV[0], rtV[1], rtV[2], rtBary))
                {
                    for (int32_t i = 0; i < 3; ++i)
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
        Delaunay2<InputType, ComputeType> const* mDelaunay;
    };
}

namespace gte
{
    // The input type T is 'float' or 'double'.

    template <typename T>
    class Delaunay2Mesh<T>
    {
    public:
        // Construction.
        Delaunay2Mesh(Delaunay2<T> const& delaunay)
            :
            mDelaunay(&delaunay)
        {
            static_assert(std::is_floating_point<T>::value,
                "The input type must be 'float' or 'double'.");

            LogAssert(
                mDelaunay->GetDimension() == 2,
                "Invalid Delaunay dimension.");
        }

        // Mesh information.
        inline size_t GetNumVertices() const
        {
            return mDelaunay->GetNumVertices();
        }

        inline size_t GetNumTriangles() const
        {
            return mDelaunay->GetNumTriangles();
        }

        inline Vector2<T> const* GetVertices() const
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
        size_t GetContainingTriangle(Vector2<T> const& P) const
        {
            // VS 2019 16.8.1 generates LNT1006 "Local variable is not
            // initialized." Incorrect, because the default constructor
            // initializes all the members.
            typename Delaunay2<T>::SearchInfo info{};
            return mDelaunay->GetContainingTriangle(P, info);
        }

        inline size_t GetInvalidIndex() const
        {
            return mDelaunay->negOne;
        }

        bool GetVertices(size_t t, std::array<Vector2<T>, 3>& vertices) const
        {
            std::array<int32_t, 3> indices = { 0, 0, 0 };
            if (mDelaunay->GetIndices(t, indices))
            {
                auto const* delaunayVertices = mDelaunay->GetVertices();
                for (size_t i = 0; i < 3; ++i)
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

        bool GetIndices(size_t t, std::array<int32_t, 3>& indices) const
        {
            return mDelaunay->GetIndices(t, indices);
        }

        bool GetAdjacencies(size_t t, std::array<int32_t, 3>& adjacencies) const
        {
            return mDelaunay->GetAdjacencies(t, adjacencies);
        }

        bool GetBarycentrics(size_t t, Vector2<T> const& P, std::array<T, 3>& bary) const
        {
            std::array<int32_t, 3> indices = { 0, 0, 0 };
            if (mDelaunay->GetIndices(t, indices))
            {
                auto const* delaunayVertices = mDelaunay->GetVertices();
                std::array<Vector2<Rational>, 3> rtV{};
                for (size_t i = 0; i < 3; ++i)
                {
                    auto const& V = delaunayVertices[indices[i]];
                    for (int32_t j = 0; j < 2; ++j)
                    {
                        rtV[i][j] = static_cast<Rational>(V[j]);
                    }
                };

                Vector2<Rational> rtP{ P[0], P[1] };
                std::array<Rational, 3> rtBary{};
                if (ComputeBarycentrics(rtP, rtV[0], rtV[1], rtV[2], rtBary))
                {
                    for (size_t i = 0; i < 3; ++i)
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
        Delaunay2<T> const* mDelaunay;
    };
}
