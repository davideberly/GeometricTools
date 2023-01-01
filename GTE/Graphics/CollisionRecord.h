// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.20

#pragma once

#include <Mathematics/IntrTriangle3Triangle3.h>
#include <Graphics/BoundTree.h>
#include <functional>

// Class Mesh must have the following functions in its interface.
//    size_t GetNumVertices() const;
//    Vector3<float> GetPosition(size_t i) const;
//    size_t GetNumTriangles() const;
//    bool GetTriangle(size_t t, std::array<int32_t, 3>& indices) const;
//    bool GetModelTriangle(size_t t, Triangle3<float>& modelTriangle) const;
//    bool GetWorldTriangle(size_t t, Triangle3<float>& worldTriangle) const;
//    Matrix4x4<float> const& GetWorldTransform() const;
// A wrapper of this type for Visual objects representing triangle meshes
// is Graphics/CollisionMesh.{h,cpp}.
//
// Class Bound must have the following functions in its interface.
//    Bound();
//    void ComputeFromData(uint32_t numElements, uint32_t stride,
//      char const* data);
//    void TransformBy(Matrix4x4<float> const& hmatrix,
//        Bound& bound) const;
//    bool TestIntersection(Bound const& bound) const;
//    bool TestIntersection(Bound const& bound, float tmax,
//        Vector3<float> const& velocity0,
//        Vector3<float> const& velocity1) const;
// A wrapper of this type for bounding spheres is Graphics/BoundingSphere.h.

namespace gte
{
    template <class Mesh, class Bound>
    class CollisionRecord
    {
    public:
        // The callback for test-intersection queries. The int32_t inputs are
        // indices into the array of triangles in the meshes of the queries.
        // The float input is the contact time, which is 0 for stationary
        // objects and nonnegative for moving objects.
        using TICallback = std::function<void(
            CollisionRecord&, int32_t,
            CollisionRecord&, int32_t,
            float)>;

        // The callback for find-intersection queries. The int32_t inputs are
        // indices into the array of triangles in the meshes of the queries.
        // The float input is the contact time, which is 0 for stationary
        // objects and nonnegative for moving objects. The std::vector input
        // contains the contact points.
        using FICallback = std::function<void(
            CollisionRecord&, int32_t,
            CollisionRecord&, int32_t,
            float,
            std::vector<Vector3<float>> const&)>;

        // Construction and destruction. The input 'tree' must be dynamically
        // allocated and created using 'storeInteriorTris = true'.
        CollisionRecord(
            std::shared_ptr<BoundTree<Mesh, Bound>> const& tree,
            Vector3<float> const& velocity,
            std::shared_ptr<TICallback> const& tiCallback,
            std::shared_ptr<FICallback> const& fiCallback)
            :
            mTree(tree),
            mVelocity(velocity),
            mTICallback(tiCallback),
            mFICallback(fiCallback)
        {
        }

        ~CollisionRecord() = default;

        // Member access.
        inline std::shared_ptr<Mesh> const& GetMesh() const
        {
            return mTree->GetMesh();
        }

        inline Vector3<float> const& GetVelocity() const
        {
            return mVelocity;
        }

        inline std::shared_ptr<TICallback> const& GetTICallback() const
        {
            return mTICallback;
        }

        inline std::shared_ptr<FICallback> const& GetFICallback() const
        {
            return mFICallback;
        }

        // Intersection queries. See the comments in class CollisionGroup
        // about the information that is available to the application via
        // the callback function.
        void TestIntersection(CollisionRecord& record)
        {
            // Convenience variables.
            auto& tree0 = mTree;
            auto& tree1 = record.mTree;
            auto const& mesh0 = tree0->GetMesh();
            auto const& mesh1 = tree1->GetMesh();
            auto const& worldBound0 = tree0->GetWorldBound();
            auto const& worldBound1 = tree1->GetWorldBound();

            tree0->UpdateWorldBound();
            tree1->UpdateWorldBound();

            if (worldBound0.TestIntersection(worldBound1))
            {
                std::shared_ptr<BoundTree<Mesh, Bound>> root{};

                if (tree0->IsInteriorNode())
                {
                    root = mTree;

                    // Compare Tree0.L to Tree1.
                    mTree = root->GetLChild();
                    TestIntersection(record);

                    // Compare Tree0.R to Tree1.
                    mTree = root->GetRChild();
                    TestIntersection(record);

                    mTree = root;
                }
                else if (tree1->IsInteriorNode())
                {
                    root = record.mTree;

                    // Compare Tree0 to Tree1.L.
                    record.mTree = root->GetLChild();
                    TestIntersection(record);

                    // Compare Tree0 to Tree1.R.
                    record.mTree = root->GetRChild();
                    TestIntersection(record);

                    record.mTree = root;
                }
                else
                {
                    // The traversal is at a leaf in each tree.
                    int32_t numTriangles0 = tree0->GetNumTriangles();
                    for (int32_t i0 = 0; i0 < numTriangles0; ++i0)
                    {
                        // Get the world space triangle.
                        int32_t t0 = tree0->GetTriangle(i0);
                        Triangle3<float> tri0{};
                        mesh0->GetWorldTriangle(t0, tri0);

                        int32_t numTriangles1 = tree1->GetNumTriangles();
                        for (int32_t i1 = 0; i1 < numTriangles1; ++i1)
                        {
                            // Get the world space triangle.
                            int32_t t1 = tree1->GetTriangle(i1);
                            Triangle3<float> tri1{};
                            mesh1->GetWorldTriangle(t1, tri1);

                            TIQuery<float, Triangle3<float>, Triangle3<float>> calc{};
                            auto const& result = calc(tri0, tri1);
                            if (result.intersect)
                            {
                                if (mTICallback)
                                {
                                    (*mTICallback)(
                                        *this, t0, record, t1, 0.0f);
                                }

                                if (record.mTICallback)
                                {
                                    (*record.mTICallback)(
                                        record, t1, *this, t0, 0.0f);
                                }
                            }
                        }
                    }
                }
            }
        }

        void FindIntersection(CollisionRecord& record)
        {
            // Convenience variables.
            auto& tree0 = mTree;
            auto& tree1 = record.mTree;
            auto const& mesh0 = tree0->GetMesh();
            auto const& mesh1 = tree1->GetMesh();
            auto const& worldBound0 = tree0->GetWorldBound();
            auto const& worldBound1 = tree1->GetWorldBound();

            tree0->UpdateWorldBound();
            tree1->UpdateWorldBound();

            if (worldBound0.TestIntersection(worldBound1))
            {
                std::shared_ptr<BoundTree<Mesh, Bound>> root{};

                if (tree0->IsInteriorNode())
                {
                    root = mTree;

                    // Compare Tree0.L to Tree1.
                    mTree = root->GetLChild();
                    FindIntersection(record);

                    // Compare Tree0.R to Tree1.
                    mTree = root->GetRChild();
                    FindIntersection(record);

                    mTree = root;
                }
                else if (tree1->IsInteriorNode())
                {
                    root = record.mTree;

                    // Compare Tree0 to Tree1.L.
                    record.mTree = root->GetLChild();
                    FindIntersection(record);

                    // Compare Tree0 to Tree1.R.
                    record.mTree = root->GetRChild();
                    FindIntersection(record);

                    record.mTree = root;
                }
                else
                {
                    // The traversal is at a leaf in each tree.
                    int32_t numTriangles0 = tree0->GetNumTriangles();
                    for (int32_t i0 = 0; i0 < numTriangles0; ++i0)
                    {
                        // Get the world space triangle.
                        int32_t t0 = tree0->GetTriangle(i0);
                        Triangle3<float> tri0{};
                        mesh0->GetWorldTriangle(t0, tri0);

                        int32_t numTriangles1 = tree1->GetNumTriangles();
                        for (int32_t i1 = 0; i1 < numTriangles1; ++i1)
                        {
                            // Get the world space triangle.
                            int32_t t1 = tree1->GetTriangle(i1);
                            Triangle3<float> tri1{};
                            mesh1->GetWorldTriangle(t1, tri1);

                            FIQuery<float, Triangle3<float>, Triangle3<float>> calc{};
                            auto const& result = calc(tri0, tri1);
                            if (result.intersect)
                            {
                                if (mFICallback)
                                {
                                    (*mFICallback)(
                                        *this, t0, record, t1,
                                        0.0f, result.intersection);
                                }

                                if (record.mFICallback)
                                {
                                    (*record.mFICallback)(
                                        record, t1, *this, t0,
                                        0.0f, result.intersection);
                                }
                            }
                        }
                    }
                }
            }
        }

        void TestIntersection(float tMax, CollisionRecord& record)
        {
            // Convenience variables.
            auto& tree0 = mTree;
            auto& tree1 = record.mTree;
            auto const& mesh0 = tree0->GetMesh();
            auto const& mesh1 = tree1->GetMesh();
            auto const& worldBound0 = tree0->GetWorldBound();
            auto const& worldBound1 = tree1->GetWorldBound();
            auto const& velocity0 = mVelocity;
            auto const& velocity1 = record.mVelocity;

            tree0->UpdateWorldBound();
            tree1->UpdateWorldBound();

            if (worldBound0.TestIntersection(worldBound1, tMax, velocity0, velocity1))
            {
                std::shared_ptr<BoundTree<Mesh, Bound>> root{};

                if (tree0->IsInteriorNode())
                {
                    root = mTree;

                    // Compare Tree0.L to Tree1.
                    mTree = root->GetLChild();
                    TestIntersection(tMax, record);

                    // Compare Tree0.R to Tree1.
                    mTree = root->GetRChild();
                    TestIntersection(tMax, record);

                    mTree = root;
                }
                else if (tree1->IsInteriorNode())
                {
                    root = record.mTree;

                    // Compare Tree0 to Tree1.L.
                    record.mTree = root->GetLChild();
                    TestIntersection(tMax, record);

                    // Compare Tree0 to Tree1.R.
                    record.mTree = root->GetRChild();
                    TestIntersection(tMax, record);

                    record.mTree = root;
                }
                else
                {
                    // The traversal is at a leaf in each tree.
                    int32_t numTriangles0 = tree0->GetNumTriangles();
                    for (int32_t i0 = 0; i0 < numTriangles0; ++i0)
                    {
                        // Get the world space triangle.
                        int32_t t0 = tree0->GetTriangle(i0);
                        Triangle3<float> tri0{};
                        mesh0->GetWorldTriangle(t0, tri0);

                        int32_t numTriangles1 = tree1->GetNumTriangles();
                        for (int32_t i1 = 0; i1 < numTriangles1; ++i1)
                        {
                            // Get the world space triangle.
                            int32_t t1 = tree1->GetTriangle(i1);
                            Triangle3<float> tri1{};
                            mesh1->GetWorldTriangle(t1, tri1);

                            TIQuery<float, Triangle3<float>, Triangle3<float>> calc{};
                            auto const& result = calc(tMax, tri0, velocity0, tri1, velocity1);
                            if (result.intersect)
                            {
                                if (mTICallback)
                                {
                                    (*mTICallback)(
                                        *this, t0, record, t1,
                                        result.contactTime);
                                }

                                if (record.mTICallback)
                                {
                                    (*record.mTICallback)(
                                        record, t1, *this, t0,
                                        result.contactTime);
                                }
                            }
                        }
                    }
                }
            }
        }

        void FindIntersection(float tMax, CollisionRecord& record)
        {
            // Convenience variables.
            auto& tree0 = mTree;
            auto& tree1 = record.mTree;
            auto const& mesh0 = tree0->GetMesh();
            auto const& mesh1 = tree1->GetMesh();
            auto const& worldBound0 = tree0->GetWorldBound();
            auto const& worldBound1 = tree1->GetWorldBound();
            auto const& velocity0 = mVelocity;
            auto const& velocity1 = record.mVelocity;

            tree0->UpdateWorldBound();
            tree1->UpdateWorldBound();

            if (worldBound0.TestIntersection(worldBound1, tMax, velocity0, velocity1))
            {
                std::shared_ptr<BoundTree<Mesh, Bound>> root{};

                if (tree0->IsInteriorNode())
                {
                    root = mTree;

                    // Compare Tree0.L to Tree1.
                    mTree = root->GetLChild();
                    FindIntersection(tMax, record);

                    // Compare Tree0.R to Tree1.
                    mTree = root->GetRChild();
                    FindIntersection(tMax, record);

                    mTree = root;
                }
                else if (tree1->IsInteriorNode())
                {
                    root = record.mTree;

                    // Compare Tree0 to Tree1.L.
                    record.mTree = root->GetLChild();
                    FindIntersection(tMax, record);

                    // compare Tree0 to Tree1.R
                    record.mTree = root->GetRChild();
                    FindIntersection(tMax, record);

                    record.mTree = root;
                }
                else
                {
                    // The traversal is at a leaf in each tree.
                    int32_t numTriangles0 = tree0->GetNumTriangles();
                    for (int32_t i0 = 0; i0 < numTriangles0; ++i0)
                    {
                        // Get the world space triangle.
                        int32_t t0 = tree0->GetTriangle(i0);
                        Triangle3<float> tri0{};
                        mesh0->GetWorldTriangle(t0, tri0);

                        int32_t numTriangles1 = tree1->GetNumTriangles();
                        for (int32_t i1 = 0; i1 < numTriangles1; ++i1)
                        {
                            // Get the world space triangle.
                            int32_t t1 = tree1->GetTriangle(i1);
                            Triangle3<float> tri1{};
                            mesh1->GetWorldTriangle(t1, tri1);

                            FIQuery<float, Triangle3<float>, Triangle3<float>> calc{};
                            auto const& result = calc(tMax, tri0, velocity0, tri1, velocity1);
                            if (result.intersect)
                            {
                                if (mFICallback)
                                {
                                    (*mFICallback)(
                                        *this, t0, record, t1,
                                        result.contactTime, result.intersection);
                                }

                                if (record.mFICallback)
                                {
                                    (*record.mFICallback)(
                                        record, t1, *this, t0,
                                        result.contactTime, result.intersection);
                                }
                            }
                        }
                    }
                }
            }
        }

    private:
        std::shared_ptr<BoundTree<Mesh, Bound>> mTree;
        Vector3<float> mVelocity;
        std::shared_ptr<TICallback> mTICallback;
        std::shared_ptr<FICallback> mFICallback;
    };
}
