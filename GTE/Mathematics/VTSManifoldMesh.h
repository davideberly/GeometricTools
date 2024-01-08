// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// The VTSManifoldMesh class represents a triangle-tetrahedron manifold mesh
// but additionally stores vertex adjacency information. The 'V' stands for
// vertex, the 'T' stands for triangle (face) and the 'S' stands for simplex
// (tetrahedron). It is general purpose, allowing insertion and removal of
// tetrahedra at any time. However, the performance is limited because of the
// use of C++ container classes (unordered sets and maps). If your application
// requires a static vertex-triangle-simplex manifold mesh for which no
// modifications will occur, a better choice is StaticVTSManifoldMesh.

#include <Mathematics/TSManifoldMesh.h>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace gte
{
    class VTSManifoldMesh : public TSManifoldMesh
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
                TAdjacent{},
                SAdjacent{}
            {
            }

            // The index into the vertex pool of the mesh.
            int32_t V;

            // Adjacent objects.
            std::unordered_set<int32_t> VAdjacent;
            std::unordered_set<Triangle*> TAdjacent;
            std::unordered_set<Tetrahedron*> SAdjacent;
        };


        // Construction and destruction.
        virtual ~VTSManifoldMesh() = default;

        VTSManifoldMesh(VCreator vCreator = nullptr, TCreator tCreator = nullptr, SCreator sCreator = nullptr)
            :
            TSManifoldMesh(tCreator, sCreator),
            mVCreator(vCreator ? vCreator : CreateVertex),
            mVMap{}
        {
        }

        // Support for a deep copy of the mesh. The mVMap, mTMap and mSMap
        // objects have dynamically allocated memory for vertices, triangles
        // and tetrahedra. A shallow copy of the pointers to this memory is
        // problematic. Allowing sharing, say, via std::shared_ptr, is an
        // option but not really the intent of copying the mesh graph.
        VTSManifoldMesh(VTSManifoldMesh const& mesh)
            :
            VTSManifoldMesh()
        {
            *this = mesh;
        }

        VTSManifoldMesh& operator=(VTSManifoldMesh const& mesh)
        {
            Clear();
            mVCreator = mesh.mVCreator;
            TSManifoldMesh::operator=(mesh);
            return *this;
        }

        // Member access.
        inline VMap const& GetVertices() const
        {
            return mVMap;
        }

        // If <v0,v1,v2> is not in the mesh, a Triangle object is created and
        // returned; otherwise, <v0,v1,v2> is in the mesh and nullptr is
        // returned.  If the insertion leads to a nonmanifold mesh, the call
        // fails with a nullptr returned.
        virtual Tetrahedron* Insert(int32_t v0, int32_t v1, int32_t v2, int32_t v3) override
        {
            Tetrahedron* tetra = TSManifoldMesh::Insert(v0, v1, v2, v3);
            if (!tetra)
            {
                return nullptr;
            }

            for (int32_t i = 0; i < 4; ++i)
            {
                int32_t vIndex = tetra->V[i];
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

                vertex->SAdjacent.insert(tetra);

                for (int32_t j = 0; j < 4; ++j)
                {
                    auto tri = tetra->T[j];
                    LogAssert(tri != nullptr, "Unexpected condition.");
                    if (tri->V[0] == vIndex)
                    {
                        vertex->VAdjacent.insert(tri->V[1]);
                        vertex->VAdjacent.insert(tri->V[2]);
                        vertex->TAdjacent.insert(tri);
                    }
                    else if (tri->V[1] == vIndex)
                    {
                        vertex->VAdjacent.insert(tri->V[0]);
                        vertex->VAdjacent.insert(tri->V[2]);
                        vertex->TAdjacent.insert(tri);
                    }
                    else if (tri->V[2] == vIndex)
                    {
                        vertex->VAdjacent.insert(tri->V[0]);
                        vertex->VAdjacent.insert(tri->V[1]);
                        vertex->TAdjacent.insert(tri);
                    }
                }
            }

            return tetra;
        }

        // If <v0,v1,v2> is in the mesh, it is removed and 'true' is returned;
        // otherwise, <v0,v1,v2> is not in the mesh and 'false' is returned.
        virtual bool Remove(int32_t v0, int32_t v1, int32_t v2, int32_t v3) override
        {
            auto sItem = mSMap.find(TetrahedronKey<true>(v0, v1, v2, v3));
            if (sItem == mSMap.end())
            {
                return false;
            }

            Tetrahedron* tetra = sItem->second.get();
            for (int32_t i = 0; i < 4; ++i)
            {
                int32_t vIndex = tetra->V[i];
                auto vItem = mVMap.find(vIndex);
                LogAssert(vItem != mVMap.end(), "Unexpected condition.");
                Vertex* vertex = vItem->second.get();
                for (int32_t j = 0; j < 4; ++j)
                {
                    auto tri = tetra->T[j];
                    LogAssert(tri != nullptr, "Unexpected condition.");
                    if (tri->S[0] && !tri->S[1])
                    {
                        if (tri->V[0] == vIndex)
                        {
                            vertex->VAdjacent.erase(tri->V[1]);
                            vertex->VAdjacent.erase(tri->V[2]);
                            vertex->TAdjacent.erase(tri);
                        }
                        else if (tri->V[1] == vIndex)
                        {
                            vertex->VAdjacent.erase(tri->V[0]);
                            vertex->VAdjacent.erase(tri->V[2]);
                            vertex->TAdjacent.erase(tri);
                        }
                        else if (tri->V[2] == vIndex)
                        {
                            vertex->VAdjacent.erase(tri->V[0]);
                            vertex->VAdjacent.erase(tri->V[1]);
                            vertex->TAdjacent.erase(tri);
                        }
                    }
                }

                vertex->SAdjacent.erase(tetra);

                if (vertex->SAdjacent.size() == 0)
                {
                    LogAssert(
                        vertex->VAdjacent.size() == 0 && vertex->TAdjacent.size() == 0,
                        "Unexpected condition.");

                    mVMap.erase(vItem);
                }
            }

            return TSManifoldMesh::Remove(v0, v1, v2, v3);
        }

        // Destroy the vertices, edges, and triangles to obtain an empty mesh.
        virtual void Clear() override
        {
            mVMap.clear();
            TSManifoldMesh::Clear();
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
