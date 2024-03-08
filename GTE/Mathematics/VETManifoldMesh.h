// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The VETManifoldMesh class represents an edge-triangle manifold mesh but
// additionally stores vertex adjacency information. It is general purpose,
// allowing insertion and removal of triangles at any time. Howver, the
// performance is limited because of the use of C++ container classes
// (unordered sets and maps). If your application requires a
// vertex-edge-triangle manifold mesh for which no triangles will be
// removed, a better choice is StaticVETManifoldMesh.

#include <Mathematics/ETManifoldMesh.h>
#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace gte
{
    class VETManifoldMesh : public ETManifoldMesh
    {
    public:
        // Vertex data types.
        class Vertex;
        typedef std::unique_ptr<Vertex>(*VCreator)(int32_t);
        using VMap = std::unordered_map<int32_t, std::unique_ptr<Vertex>>;

        // Vertex object.
        class Vertex
        {
        public:
            virtual ~Vertex() = default;

            Vertex(int32_t vIndex)
                :
                V(vIndex),
                VAdjacent{},
                EAdjacent{},
                TAdjacent{}
            {
            }

            // The index into the vertex pool of the mesh.
            int32_t V;

            // Adjacent objects.
            std::unordered_set<int32_t> VAdjacent;
            std::unordered_set<Edge*> EAdjacent;
            std::unordered_set<Triangle*> TAdjacent;
        };


        // Construction and destruction.
        virtual ~VETManifoldMesh() = default;

        VETManifoldMesh(VCreator vCreator = nullptr, ECreator eCreator = nullptr, TCreator tCreator = nullptr)
            :
            ETManifoldMesh(eCreator, tCreator),
            mVCreator(vCreator ? vCreator : CreateVertex),
            mVMap{}
        {
        }

        // Support for a deep copy of the mesh. The mVMap, mEMap and mTMap
        // objects have dynamically allocated memory for vertices, edges and
        // triangles. A shallow copy of the pointers to this memory is
        // problematic. Allowing sharing, say, via std::shared_ptr, is an
        // option but not really the intent of copying the mesh graph.
        VETManifoldMesh(VETManifoldMesh const& mesh)
            :
            VETManifoldMesh()
        {
            *this = mesh;
        }

        VETManifoldMesh& operator=(VETManifoldMesh const& mesh)
        {
            Clear();
            mVCreator = mesh.mVCreator;
            ETManifoldMesh::operator=(mesh);
            return *this;
        }

        // Member access.
        inline VMap const& GetVertices() const
        {
            return mVMap;
        }

        // If <v0,v1,v2> is not in the mesh, a Triangle object is created and
        // returned; otherwise, <v0,v1,v2> is in the mesh and nullptr is
        // returned. If the insertion leads to a nonmanifold mesh, the call
        // fails with a nullptr returned.
        virtual Triangle* Insert(int32_t v0, int32_t v1, int32_t v2) override
        {
            Triangle* tri = ETManifoldMesh::Insert(v0, v1, v2);
            if (!tri)
            {
                return nullptr;
            }

            for (int32_t i = 0; i < 3; ++i)
            {
                int32_t vIndex = tri->V[i];
                auto vItem = mVMap.find(vIndex);
                Vertex* vertex = nullptr;
                if (vItem == mVMap.end())
                {
                    std::unique_ptr<Vertex> newVertex = mVCreator(vIndex);
                    vertex = newVertex.get();
                    mVMap[vIndex] = std::move(newVertex);
                }
                else
                {
                    vertex = vItem->second.get();
                }

                vertex->TAdjacent.insert(tri);

                for (int32_t j = 0; j < 3; ++j)
                {
                    auto edge = tri->E[j];
                    LogAssert(edge != nullptr, "Unexpected condition.");
                    if (edge->V[0] == vIndex)
                    {
                        vertex->VAdjacent.insert(edge->V[1]);
                        vertex->EAdjacent.insert(edge);
                    }
                    else if (edge->V[1] == vIndex)
                    {
                        vertex->VAdjacent.insert(edge->V[0]);
                        vertex->EAdjacent.insert(edge);
                    }
                }
            }

            return tri;
        }

        // If <v0,v1,v2> is in the mesh, it is removed and 'true' is returned;
        // otherwise, <v0,v1,v2> is not in the mesh and 'false' is returned.
        virtual bool Remove(int32_t v0, int32_t v1, int32_t v2) override
        {
            auto tItem = mTMap.find(TriangleKey<true>(v0, v1, v2));
            if (tItem == mTMap.end())
            {
                return false;
            }

            Triangle* tri = tItem->second.get();
            for (int32_t i = 0; i < 3; ++i)
            {
                int32_t vIndex = tri->V[i];
                auto vItem = mVMap.find(vIndex);
                LogAssert(vItem != mVMap.end(), "Unexpected condition.");
                Vertex* vertex = vItem->second.get();
                for (int32_t j = 0; j < 3; ++j)
                {
                    auto edge = tri->E[j];
                    LogAssert(edge != nullptr, "Unexpected condition.");
                    if (edge->T[0] && !edge->T[1])
                    {
                        if (edge->V[0] == vIndex)
                        {
                            vertex->VAdjacent.erase(edge->V[1]);
                            vertex->EAdjacent.erase(edge);
                        }
                        else if (edge->V[1] == vIndex)
                        {
                            vertex->VAdjacent.erase(edge->V[0]);
                            vertex->EAdjacent.erase(edge);
                        }
                    }
                }

                vertex->TAdjacent.erase(tri);

                if (vertex->TAdjacent.size() == 0)
                {
                    LogAssert(
                        vertex->VAdjacent.size() == 0 && vertex->EAdjacent.size() == 0,
                        "Unexpected condition.");

                    mVMap.erase(vItem);
                }
            }

            return ETManifoldMesh::Remove(v0, v1, v2);
        }

        // Destroy the vertices, edges and triangles to obtain an empty mesh.
        virtual void Clear() override
        {
            mVMap.clear();
            ETManifoldMesh::Clear();
        }

    protected:
        // The vertex data and default vertex creation.
        static std::unique_ptr<Vertex> CreateVertex(int32_t vIndex)
        {
            return std::make_unique<Vertex>(vIndex);
        }

        VCreator mVCreator;
        VMap mVMap;
    };
}
