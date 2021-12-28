// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.12.28

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/HashCombine.h>
#include <Mathematics/TetrahedronKey.h>
#include <Mathematics/TriangleKey.h>
#include <map>
#include <memory>
#include <unordered_map>

namespace gte
{
    class TSManifoldMesh
    {
    public:
        // Triangle data types.
        class Triangle;
        typedef std::unique_ptr<Triangle>(*TCreator)(int, int, int);
        typedef std::unordered_map<TriangleKey<false>, std::unique_ptr<Triangle>,
            TriangleKey<false>, TriangleKey<false>> TMap;

        // Tetrahedron data types.
        class Tetrahedron;
        typedef std::unique_ptr<Tetrahedron>(*SCreator)(int, int, int, int);
        typedef std::unordered_map<TetrahedronKey<true>, std::unique_ptr<Tetrahedron>,
            TetrahedronKey<true>, TetrahedronKey<true>> SMap;

        // Triangle object.
        class Triangle
        {
        public:
            virtual ~Triangle() = default;

            Triangle(int v0, int v1, int v2)
                :
                V{ v0, v1, v2 },
                T{ nullptr, nullptr }
            {
            }

            // Vertices of the face.
            std::array<int, 3> V;

            // Tetrahedra sharing the face.
            std::array<Tetrahedron*, 2> T;
        };

        // Tetrahedron object.
        class Tetrahedron
        {
        public:
            virtual ~Tetrahedron() = default;

            Tetrahedron(int v0, int v1, int v2, int v3)
                :
                V{ v0, v1, v2, v3 },
                T{ nullptr, nullptr, nullptr, nullptr },
                S{ nullptr, nullptr, nullptr, nullptr }
            {
            }

            // Vertices, listed in an order so that each face vertices in
            // counterclockwise order when viewed from outside the
            // tetrahedron.
            std::array<int, 4> V;

            // Adjacent faces.  T[i] points to the triangle face
            // opposite V[i].
            //   T[0] points to face (V[1],V[2],V[3])
            //   T[1] points to face (V[0],V[3],V[2])
            //   T[2] points to face (V[0],V[1],V[3])
            //   T[3] points to face (V[0],V[2],V[1])
            std::array<Triangle*, 4> T;

            // Adjacent tetrahedra.  S[i] points to the adjacent tetrahedron
            // sharing face T[i].
            std::array<Tetrahedron*, 4> S;
        };


        // Construction and destruction.
        virtual ~TSManifoldMesh() = default;

        TSManifoldMesh(TCreator tCreator = nullptr, SCreator sCreator = nullptr)
            :
            mTCreator(tCreator ? tCreator : CreateTriangle),
            mSCreator(sCreator ? sCreator : CreateTetrahedron),
            mThrowOnNonmanifoldInsertion(true)
        {
        }

        // Support for a deep copy of the mesh.  The mTMap and mSMap objects
        // have dynamically allocated memory for triangles and tetrahedra.  A
        // shallow/ copy of the pointers to this memory is problematic.
        // Allowing sharing, say, via std::shared_ptr, is an option but not
        // really the intent of copying the mesh graph.
        TSManifoldMesh(TSManifoldMesh const& mesh)
        {
            *this = mesh;
        }

        TSManifoldMesh& operator=(TSManifoldMesh const& mesh)
        {
            Clear();

            mTCreator = mesh.mTCreator;
            mSCreator = mesh.mSCreator;
            mThrowOnNonmanifoldInsertion = mesh.mThrowOnNonmanifoldInsertion;
            for (auto const& element : mesh.mSMap)
            {
                // The typecast avoids warnings about not storing the return
                // value in a named variable. The return value is discarded.
                (void)Insert(element.first.V[0], element.first.V[1],
                    element.first.V[2], element.first.V[3]);
            }

            return *this;
        }

        // Member access.
        inline TMap const& GetTriangles() const
        {
            return mTMap;
        }

        inline SMap const& GetTetrahedra() const
        {
            return mSMap;
        }

        // If the insertion of a tetrahedron fails because the mesh would
        // become nonmanifold, the default behavior is to throw an exception.
        // You can disable this behavior and continue gracefully without an
        // exception.
        bool ThrowOnNonmanifoldInsertion(bool doException)
        {
            std::swap(doException, mThrowOnNonmanifoldInsertion);
            return doException;  // return the previous state
        }

        // If <v0,v1,v2,v3> is not in the mesh, a Tetrahedron object is
        // created and returned; otherwise, <v0,v1,v2,v3> is in the mesh and
        // nullptr is returned.  If the insertion leads to a nonmanifold mesh,
        // the call fails with a nullptr returned.
        Tetrahedron* Insert(int v0, int v1, int v2, int v3)
        {
            TetrahedronKey<true> skey(v0, v1, v2, v3);
            if (mSMap.find(skey) != mSMap.end())
            {
                // The tetrahedron already exists.  Return a null pointer as
                // a signal to the caller that the insertion failed.
                return nullptr;
            }

            // Add the new tetrahedron.
            std::unique_ptr<Tetrahedron> newTetra = mSCreator(v0, v1, v2, v3);
            Tetrahedron* tetra = newTetra.get();

            // Add the faces to the mesh if they do not already exist.
            for (int i = 0; i < 4; ++i)
            {
                auto const& opposite = TetrahedronKey<true>::GetOppositeFace()[i];
                TriangleKey<false> tkey(tetra->V[opposite[0]], tetra->V[opposite[1]], tetra->V[opposite[2]]);
                Triangle* face;
                auto titer = mTMap.find(tkey);
                if (titer == mTMap.end())
                {
                    // This is the first time the face is encountered.
                    std::unique_ptr<Triangle> newFace = mTCreator(tetra->V[opposite[0]],
                        tetra->V[opposite[1]], tetra->V[opposite[2]]);
                    face = newFace.get();
                    mTMap[tkey] = std::move(newFace);

                    // Update the face and tetrahedron.
                    face->T[0] = tetra;
                    tetra->T[i] = face;
                }
                else
                {
                    // This is the second time the face is encountered.
                    face = titer->second.get();
                    LogAssert(face != nullptr, "Unexpected condition.");

                    // Update the face.
                    if (face->T[1])
                    {
                        if (mThrowOnNonmanifoldInsertion)
                        {
                            LogError("Attempt to create nonmanifold mesh.");
                        }
                        else
                        {
                            return nullptr;
                        }
                    }
                    face->T[1] = tetra;

                    // Update the adjacent tetrahedra.
                    auto adjacent = face->T[0];
                    LogAssert(adjacent != nullptr, "Unexpected condition.");
                    for (int j = 0; j < 4; ++j)
                    {
                        if (adjacent->T[j] == face)
                        {
                            adjacent->S[j] = tetra;
                            break;
                        }
                    }

                    // Update the tetrahedron.
                    tetra->T[i] = face;
                    tetra->S[i] = adjacent;
                }
            }

            mSMap[skey] = std::move(newTetra);
            return tetra;
        }

        // If <v0,v1,v2,v3> is in the mesh, it is removed and 'true' is
        // returned; otherwise, <v0,v1,v2,v3> is not in the mesh and 'false'
        // is returned.
        bool Remove(int v0, int v1, int v2, int v3)
        {
            TetrahedronKey<true> skey(v0, v1, v2, v3);
            auto siter = mSMap.find(skey);
            if (siter == mSMap.end())
            {
                // The tetrahedron does not exist.
                return false;
            }

            // Get the tetrahedron.
            Tetrahedron* tetra = siter->second.get();

            // Remove the faces and update adjacent tetrahedra if necessary.
            for (int i = 0; i < 4; ++i)
            {
                // Inform the faces the tetrahedron is being deleted.
                auto face = tetra->T[i];
                LogAssert(face != nullptr, "Unexpected condition.");

                if (face->T[0] == tetra)
                {
                    // One-tetrahedron faces always have pointer at index
                    // zero.
                    face->T[0] = face->T[1];
                    face->T[1] = nullptr;
                }
                else if (face->T[1] == tetra)
                {
                    face->T[1] = nullptr;
                }
                else
                {
                    LogError("Unexpected condition.");
                }

                // Remove the face if you have the last reference to it.
                if (!face->T[0] && !face->T[1])
                {
                    TriangleKey<false> tkey(face->V[0], face->V[1], face->V[2]);
                    mTMap.erase(tkey);
                }

                // Inform adjacent tetrahedra the tetrahedron is being
                // deleted.
                auto adjacent = tetra->S[i];
                if (adjacent)
                {
                    for (int j = 0; j < 4; ++j)
                    {
                        if (adjacent->S[j] == tetra)
                        {
                            adjacent->S[j] = nullptr;
                            break;
                        }
                    }
                }
            }

            mSMap.erase(skey);
            return true;
        }

        // Destroy the triangles and tetrahedra to obtain an empty mesh.
        virtual void Clear()
        {
            mTMap.clear();
            mSMap.clear();
        }

        // A manifold mesh is closed if each face is shared twice.
        bool IsClosed() const
        {
            for (auto const& element : mTMap)
            {
                Triangle* tri = element.second.get();
                if (!tri->T[0] || !tri->T[1])
                {
                    return false;
                }
            }
            return true;
        }

    protected:
        // The triangle data and default triangle creation.
        static std::unique_ptr<Triangle> CreateTriangle(int v0, int v1, int v2)
        {
            return std::make_unique<Triangle>(v0, v1, v2);
        }

        TCreator mTCreator;
        TMap mTMap;

        // The tetrahedron data and default tetrahedron creation.
        static std::unique_ptr<Tetrahedron> CreateTetrahedron(int v0, int v1, int v2, int v3)
        {
            return std::make_unique<Tetrahedron>(v0, v1, v2, v3);
        }

        SCreator mSCreator;
        SMap mSMap;
        bool mThrowOnNonmanifoldInsertion;  // default: true
    };
}
