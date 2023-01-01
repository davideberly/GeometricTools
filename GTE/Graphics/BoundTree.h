// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.20

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/ApprOrthogonalLine3.h>
#include <Mathematics/Matrix4x4.h>
#include <Mathematics/Triangle.h>
#include <memory>
#include <numeric>

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
    template <typename Mesh, typename Bound>
    class BoundTree
    {
    public:
        // Construction and destruction.
        BoundTree(std::shared_ptr<Mesh> const& mesh, int32_t maxTrisPerLeaf = 1,
            bool storeInteriorTris = false)
            :
            mMesh(mesh),
            mModelBound{},
            mWorldBound{},
            mLChild{},
            mRChild{},
            mTriangles{}
        {
            if (maxTrisPerLeaf == 0)
            {
                // This is a signal to defer construction, a behavior needed
                // in the BuildTree function.
                return;
            }

            // Centroids of triangles are used for splitting a mesh. The
            // centroids are projected onto a splitting axis and sorted. The
            // split is based on the median of the projections, which leads
            // to a balanced tree.
            size_t const numTriangles = mMesh->GetNumTriangles();
            std::vector<Vector3<float>> centroids(numTriangles);
            for (size_t t = 0; t < numTriangles; ++t)
            {
                Triangle3<float> triangle{};
                mMesh->GetModelTriangle(t, triangle);
                centroids[t] = (triangle.v[0] + triangle.v[1] + triangle.v[2]) / 3.0f;
            }

            // Initialize binary-tree arrays for storing triangle indices.
            // These are used to store the indices when the mesh is split.
            std::vector<int32_t> inSplit(numTriangles);
            std::vector<int32_t> outSplit(numTriangles);
            std::iota(inSplit.begin(), inSplit.end(), 0);

            BuildTree(maxTrisPerLeaf, storeInteriorTris, centroids, 0,
                numTriangles - 1, inSplit, outSplit);
        }

        ~BoundTree() = default;

        // Tree topology.
        inline std::shared_ptr<BoundTree> const& GetLChild() const
        {
            return mLChild;
        }

        inline std::shared_ptr<BoundTree> const& GetRChild() const
        {
            return mRChild;
        }

        inline bool IsInteriorNode() const
        {
            return mLChild || mRChild;
        }

        inline bool IsLeafNode() const
        {
            return !mLChild && !mRChild;
        }

        // Member access.
        inline std::shared_ptr<Mesh> const& GetMesh() const
        {
            return mMesh;
        }

        inline const Bound& GetWorldBound() const
        {
            return mWorldBound;
        }

        inline int32_t GetNumTriangles() const
        {
            return static_cast<int32_t>(mTriangles.size());
        }

        inline int32_t GetTriangle(int32_t i) const
        {
            return mTriangles[static_cast<size_t>(i)];
        }

        inline std::vector<int32_t> const& GetTriangles() const
        {
            return mTriangles;
        }

        // The Mesh world transform is assumed to change dynamically.
        void UpdateWorldBound()
        {
            mModelBound.TransformBy(mMesh->GetWorldTransform(), mWorldBound);
        }

    private:
        void BuildTree(size_t maxTrisPerLeaf, bool storeInteriorTris,
            std::vector<Vector3<float>> const& centroids, size_t i0,
            size_t i1, std::vector<int32_t>& inSplit,
            std::vector<int32_t>& outSplit)
        {
            LogAssert(
                i0 <= i1,
                "Invalid index ordering.");

            Vector3<float> origin{}, direction{};
            CreateModelBound(i0, i1, inSplit, origin, direction);

            if (i1 - i0 < maxTrisPerLeaf)
            {
                // At a leaf node.
                size_t numTriangles = i1 - i0 + 1;
                mTriangles.resize(numTriangles);
                for (size_t t0 = 0, t1 = i0; t0 < numTriangles; ++t0, ++t1)
                {
                    mTriangles[t0] = inSplit[t1];
                }

                mLChild.reset();
                mRChild.reset();
            }
            else
            {
                // At an interior node.
                if (storeInteriorTris)
                {
                    size_t numTriangles = i1 - i0 + 1;
                    mTriangles.resize(numTriangles);
                    for (size_t t0 = 0, t1 = i0; t0 < numTriangles; ++t0, ++t1)
                    {
                        mTriangles[t0] = inSplit[t1];
                    }
                }
                else
                {
                    mTriangles.clear();
                }

                size_t j0{}, j1{};
                SplitTriangles(centroids, i0, i1, inSplit, j0, j1, outSplit,
                    origin, direction);

                mLChild = std::make_shared<BoundTree<Mesh, Bound>>(mMesh, 0, false);
                mLChild->BuildTree(maxTrisPerLeaf, storeInteriorTris,
                    centroids, i0, j0, outSplit, inSplit);

                mRChild = std::make_shared<BoundTree<Mesh, Bound>>(mMesh, 0, false);
                mRChild->BuildTree(maxTrisPerLeaf, storeInteriorTris,
                    centroids, j1, i1, outSplit, inSplit);
            }
        }

        // Compute the model bound for the subset of triangles.  Return a
        // line used for splitting the projections of the triangle centroids.
        void CreateModelBound(size_t i0, size_t i1, std::vector<int32_t>& inSplit,
            Vector3<float>& origin, Vector3<float>& direction)
        {
            // Tag vertices that are used in the submesh.
            size_t const numVertices = mMesh->GetNumVertices();
            std::vector<bool> valid(numVertices, false);
            for (size_t i = i0; i <= i1; ++i)
            {
                std::array<int32_t, 3> vertex{};
                mMesh->GetTriangle(inSplit[i], vertex);
                for (size_t j = 0; j < 3; ++j)
                {
                    valid[vertex[j]] = true;
                }
            }

            // Create a contiguous set of vertices in the submesh.
            std::vector<Vector3<float>> meshVertices{};
            for (size_t i = 0; i < numVertices; ++i)
            {
                if (valid[i])
                {
                    meshVertices.push_back(mMesh->GetPosition(i));
                }
            }

            // Compute the bound for the submesh.
            uint32_t numSubvertices = static_cast<uint32_t>(meshVertices.size());
            uint32_t stride = static_cast<uint32_t>(sizeof(Vector3<float>));
            mModelBound.ComputeFromData(numSubvertices, stride,
                reinterpret_cast<char const*>(meshVertices.data()));

            // Compute a splitting line for the submesh.
            ApprOrthogonalLine3<float> fitter{};
            fitter.Fit(meshVertices);
            auto const& line = fitter.GetParameters();
            origin = line.origin;
            direction = line.direction;
        }

        static void SplitTriangles(std::vector<Vector3<float>> const& centroids,
            size_t i0, size_t i1, std::vector<int32_t>& inSplit,
            size_t& j0, size_t& j1, std::vector<int32_t>& outSplit,
            Vector3<float>& origin, Vector3<float>& direction)
        {
            // Project onto specified line.
            size_t const quantity = i1 - i0 + 1;
            std::vector<ProjectionInfo> info(quantity);
            for (size_t i = i0, k = 0; i <= i1; ++i, ++k)
            {
                int32_t t = inSplit[i];
                Vector3<float> diff = centroids[static_cast<size_t>(t)] - origin;
                info[k].triangle = t;
                info[k].projection = Dot(direction, diff);
            }

            // Find median of projections by sorting.
            std::sort(info.begin(), info.end());
            size_t median = (quantity - 1) / 2;
            std::nth_element(info.begin(), info.begin() + median, info.end());

            // Partition the triangles by the median.
            size_t k = 0;
            for (j0 = i0 - 1; k <= median; ++k)
            {
                outSplit[++j0] = info[k].triangle;
            }
            for (j1 = i1 + 1; k < quantity; ++k)
            {
                outSplit[--j1] = info[k].triangle;
            }

        }

        // For sorting centroid projections on axes.
        class ProjectionInfo
        {
        public:
            ProjectionInfo()
                :
                triangle(0),
                projection(0.0f)
            {
            }

            bool operator<(ProjectionInfo const& other) const
            {
                return projection < other.projection;
            }

            int32_t triangle;
            float projection;
        };

        // Mesh and bounds.
        std::shared_ptr<Mesh> mMesh;
        Bound mModelBound;
        Bound mWorldBound;

        // Binary tree representation.
        std::shared_ptr<BoundTree> mLChild;
        std::shared_ptr<BoundTree> mRChild;

        // If storeInteriorTris is set to 'false' in the constructor, the interior
        // nodes set the triangle quantity to zero and the array to null.  Leaf
        // nodes set the quantity to the number of triangles at that node (1, if
        // maxTrianglesPerLeaf was set to 1) and allocate an array of triangle
        // indices that are relative to the input mesh of the top level
        // constructor.
        //
        // If storeInteriorTris is set to 'true', the interior nodes also save
        // the triangle quantity and array of triangle indices for the mesh that
        // the node represents.
        std::vector<int32_t> mTriangles;
    };
}
