// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.09.22

#pragma once

#include <Mathematics/MarchingCubes.h>
#include <Mathematics/Image3.h>
#include <Mathematics/UniqueVerticesSimplices.h>
#include <Mathematics/Vector3.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace gte
{
    template <typename T>
    class SurfaceExtractorMC : public MarchingCubes
    {
    public:
        // Construction and destruction.
        virtual ~SurfaceExtractorMC()
        {
        }

        SurfaceExtractorMC(Image3<T> const& image)
            :
            mImage(image)
        {
        }

        // Object copies are not allowed.
        SurfaceExtractorMC() = delete;
        SurfaceExtractorMC(SurfaceExtractorMC const&) = delete;
        SurfaceExtractorMC const& operator=(SurfaceExtractorMC const&) = delete;

        struct Mesh
        {
            // All members are set to zeros.
            Mesh()
            {
                std::array<T, 3> zero = { (T)0, (T)0, (T)0 };
                std::fill(vertices.begin(), vertices.end(), zero);
            }

            Topology topology;
            std::array<Vector3<T>, MAX_VERTICES> vertices;
        };

        // Extract the triangle mesh approximating F = level for a single
        // voxel whose origin corner is (x,y,z). The image has dimensions
        // d0, d1, and d2, and the origin corner satisfies the conditions
        // 0 <= x < d0-1, 0 <= y < d1-1, and 0 <= z < d2-1. The input
        // function values must be stored as
        //  F[0] = image(x  ,y  ,z), F[4] = image(x  ,y  ,z+1),
        //  F[1] = image(x+1,y  ,z), F[5] = image(x+1,y  ,z+1),
        //  F[2] = image(x  ,y+1,z), F[6] = image(x  ,y+1,z+1),
        //  F[3] = image(x+1,y+1,z), F[7] = image(x+1,y+1,z+1)
        // In local coordinates where the corners are (0,0,0), (1,0,0),
        // (0,1,0), (1,1,0), (0,0,1), (1,0,1), (0,1,1), and (1,1,1),
        // Thus, F[k] = imageLocal(k & 1, (k & 2) >> 1, (k & 4) >> 2).
        // The caller of this function must add in the (x,y,z) origin corner
        // to the mesh.vertices[] to obtain the global coordinates of the
        // vertices.
        // 
        // The return value is 'true' iff the F[] values are all not equal
        // to 'level'. If at least one of F[] is 'level', the returned
        // 'mesh' has no vertices and no triangles. If you want this behavior,
        // set 'perturb' to zero.
        //
        // If you want to avoid the case when F[i] = level for some i, set
        // 'perturb' to a small nonzero number whose absolute value is smaller
        // than the minimum absolute value of the differences between voxel
        // values and 'level'.
        bool Extract(T const& level, T const& perturb, std::array<T, 8> const& F,
            Mesh& mesh) const
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            std::array<T, 8> localF{};

            int32_t entry = 0;
            for (int32_t i = 0, mask = 1; i < 8; ++i, mask <<= 1)
            {
                localF[i] = F[i] - level;
                if (localF[i] < zero)
                {
                    entry |= mask;
                }
                else if (localF[i] > zero)
                {
                    continue;
                }

                // If 'perturb' is zero, the function will report that no
                // geometry is generated for this voxel. If 'perturb' is
                // not zero, the comparison to zero still needs to be made
                // after the perturbation in case floating-point rounding
                // errors cause localF[i] still to be zero.
                localF[i] += perturb;
                if (localF[i] == zero)
                {
                    return false;
                }
            }

            mesh.topology = GetTable(entry);

            for (int32_t i = 0; i < mesh.topology.numVertices; ++i)
            {
                int32_t j0 = mesh.topology.vpair[i][0];
                int32_t j1 = mesh.topology.vpair[i][1];

                // The vertex can be computed with 3D-only computations as
                //   V = (F[j0] * k1 - F[j1] * k0) / (F[j1] - F[j0])
                // but floating-point rounding errors can cause at least one
                // of the integer-valued components of V not to be 0 or 1 as
                // the case may be. Such errors in turn can lead to 2 nearly
                // identical vertices in the mesh, and MakeUnique will not be
                // able to characterize them as the same vertex. The
                // componentwise computations avoid these floating-point
                // rounding errors. It is guaranteed that j0 < j1, so multiple
                // voxels sharing the same edge will generate the same vertex.
                auto& vertex = mesh.vertices[i];
                std::array<int32_t, 3> k0 = { j0 & 1, (j0 & 2) >> 1, (j0 & 4) >> 2 };
                std::array<int32_t, 3> k1 = { j1 & 1, (j1 & 2) >> 1, (j1 & 4) >> 2 };
                for (int32_t index = 0; index < 3; ++index)
                {
                    if (k0[index] == 0)
                    {
                        if (k1[index] == 0)
                        {
                            vertex[index] = zero;
                        }
                        else // k1[index] = 1
                        {
                            vertex[index] = F[j0] / (F[j0] - F[j1]);
                        }
                    }
                    else  // k0[index] = 1
                    {
                        if (k1[index] == 0)
                        {
                            vertex[index] = F[j1] / (F[j1] - F[j0]);
                        }
                        else // k1[index] = 1
                        {
                            vertex[index] = one;
                        }
                    }
                }
            }
            return true;
        }

        // Extract the triangle mesh approximating F = level for all the
        // voxels in a 3D image. The input image must be stored in a
        // 1-dimensional array with lexicographical order. If the image
        // has dimensions d0, d1, and d2, then voxel location (x, y, z)
        // contains value image[x + d0 * (y + d1 * z)]. The output 'indices'
        // consists indices.size()/3 triangles, each a triple of indices into
        // 'vertices'
        //
        // The triangle table lookups depend on voxel values never being
        // exactly equal to 'level'. Set 'perturb' to zero so that any
        // voxel cube with at least one corner value equal to 'level' is
        // ignored in the final mesh; that is, such a voxel does not generate
        // any triangles in the final mesh. If you want triangles from such
        // voxels, set 'perturb' to a small nonzero number whose absolute
        // value is smaller than the minimum absolute value of the differences
        // between voxel values and 'level'.
        void Extract(T const& level, T const& perturb,
            std::vector<Vector3<T>>& vertices, std::vector<int32_t>& indices) const
        {
            vertices.clear();
            indices.clear();

            for (int32_t z0 = 0, z1 = 1; z1 < mImage.GetDimension(2); z0 = z1++)
            {
                for (int32_t y0 = 0, y1 = 1; y1 < mImage.GetDimension(1); y0 = y1++)
                {
                    for (int32_t x0 = 0, x1 = 1; x1 < mImage.GetDimension(0); x0 = x1++)
                    {
                        std::array<T, 8> F{};
                        F[0] = mImage(x0, y0, z0);
                        F[1] = mImage(x1, y0, z0);
                        F[2] = mImage(x0, y1, z0);
                        F[3] = mImage(x1, y1, z0);
                        F[4] = mImage(x0, y0, z1);
                        F[5] = mImage(x1, y0, z1);
                        F[6] = mImage(x0, y1, z1);
                        F[7] = mImage(x1, y1, z1);

                        Mesh mesh{};

                        if (Extract(level, perturb, F, mesh))
                        {
                            int32_t vbase = static_cast<int32_t>(vertices.size());
                            for (int32_t i = 0; i < mesh.topology.numVertices; ++i)
                            {
                                Vector3<T> position = mesh.vertices[i];
                                position[0] += static_cast<T>(x0);
                                position[1] += static_cast<T>(y0);
                                position[2] += static_cast<T>(z0);
                                vertices.push_back(position);
                            }

                            for (int32_t i = 0; i < mesh.topology.numTriangles; ++i)
                            {
                                for (int32_t j = 0; j < 3; ++j)
                                {
                                    indices.push_back(vbase + mesh.topology.itriple[i][j]);
                                }
                            }
                        }
                    }
                }
            }
        }

        // The extraction has duplicate vertices on edges shared by voxels.
        // This function will eliminate the duplication.
        void MakeUnique(std::vector<Vector3<T>>& vertices, std::vector<int32_t>& indices) const
        {
            std::vector<Vector3<T>> outVertices{};
            std::vector<int32_t> outIndices{};
            UniqueVerticesSimplices<Vector3<T>, int32_t, 3> uvt{};
            uvt.RemoveDuplicateVertices(vertices, indices, outVertices, outIndices);
            vertices = std::move(outVertices);
            indices = std::move(outIndices);
        }

        // The extraction does not use any topological information about the
        // level surface.  The triangles can be a mixture of clockwise-ordered
        // and counterclockwise-ordered.  This function is an attempt to give
        // the triangles a consistent ordering by selecting a normal in
        // approximately the same direction as the average gradient at the
        // vertices (when sameDir is true), or in the opposite direction (when
        // sameDir is false).  This might not always produce a consistent
        // order, but is fast.  A consistent order can be computed if you
        // build a table of vertex, edge, and face adjacencies, but the
        // resulting data structure is somewhat expensive to process to
        // reorient triangles.
        void OrientTriangles(std::vector<Vector3<T>> const& vertices, std::vector<int32_t>& indices, bool sameDir) const
        {
            T const zero = static_cast<T>(0);
            int32_t const numTriangles = static_cast<int32_t>(indices.size() / 3);
            int32_t* triangle = indices.data();
            for (int32_t t = 0; t < numTriangles; ++t, triangle += 3)
            {
                // Get triangle vertices.
                Vector3<T> v0 = vertices[triangle[0]];
                Vector3<T> v1 = vertices[triangle[1]];
                Vector3<T> v2 = vertices[triangle[2]];

                // Construct triangle normal based on current orientation.
                Vector3<T> edge1 = v1 - v0;
                Vector3<T> edge2 = v2 - v0;
                Vector3<T> normal = Cross(edge1, edge2);

                // Get the image gradient at the vertices.
                Vector3<T> gradient0 = GetGradient(v0);
                Vector3<T> gradient1 = GetGradient(v1);
                Vector3<T> gradient2 = GetGradient(v2);

                // Compute the average gradient.
                Vector3<T> gradientAvr = (gradient0 + gradient1 + gradient2) / (T)3;

                // Compute the dot product of normal and average gradient.
                T dot = Dot(gradientAvr, normal);

                // Choose triangle orientation based on gradient direction.
                if (sameDir)
                {
                    if (dot < zero)
                    {
                        // Wrong orientation, reorder it.
                        std::swap(triangle[1], triangle[2]);
                    }
                }
                else
                {
                    if (dot > zero)
                    {
                        // Wrong orientation, reorder it.
                        std::swap(triangle[1], triangle[2]);
                    }
                }
            }
        }

        // Compute vertex normals for the mesh.
        void ComputeNormals(std::vector<Vector3<T>> const& vertices, std::vector<int32_t> const& indices,
            std::vector<Vector3<T>>& normals) const
        {
            // Maintain a running sum of triangle normals at each vertex.
            int32_t const numVertices = static_cast<int32_t>(vertices.size());
            normals.resize(numVertices);
            Vector3<T> zero = Vector3<T>::Zero();
            std::fill(normals.begin(), normals.end(), zero);

            int32_t const numTriangles = static_cast<int32_t>(indices.size() / 3);
            int32_t const* current = indices.data();
            for (int32_t i = 0; i < numTriangles; ++i)
            {
                int32_t i0 = *current++;
                int32_t i1 = *current++;
                int32_t i2 = *current++;
                Vector3<T> v0 = vertices[i0];
                Vector3<T> v1 = vertices[i1];
                Vector3<T> v2 = vertices[i2];

                // Construct triangle normal.
                Vector3<T> edge1 = v1 - v0;
                Vector3<T> edge2 = v2 - v0;
                Vector3<T> normal = Cross(edge1, edge2);

                // Maintain the sum of normals at each vertex.
                normals[i0] += normal;
                normals[i1] += normal;
                normals[i2] += normal;
            }

            // The normal vector storage was used to accumulate the sum of
            // triangle normals. Now these vectors must be rescaled to be
            // unit length.
            for (auto& normal : normals)
            {
                Normalize(normal);
            }
        }

    protected:
        Vector3<T> GetGradient(Vector3<T> position) const
        {
            int32_t x = static_cast<int32_t>(std::floor(position[0]));
            if (x < 0 || x >= mImage.GetDimension(0) - 1)
            {
                return Vector3<T>::Zero();
            }

            int32_t y = static_cast<int32_t>(std::floor(position[1]));
            if (y < 0 || y >= mImage.GetDimension(1) - 1)
            {
                return Vector3<T>::Zero();
            }

            int32_t z = static_cast<int32_t>(std::floor(position[2]));
            if (z < 0 || z >= mImage.GetDimension(2) - 1)
            {
                return Vector3<T>::Zero();
            }

            T const one = static_cast<T>(1);
            position[0] -= static_cast<T>(x);
            position[1] -= static_cast<T>(y);
            position[2] -= static_cast<T>(z);
            T oneMX = one - position[0];
            T oneMY = one - position[1];
            T oneMZ = one - position[2];

            // Get image values at corners of voxel.
            std::array<size_t, 8> corners{};
            mImage.GetCorners(x, y, z, corners);
            T f000 = mImage[corners[0]];
            T f100 = mImage[corners[1]];
            T f010 = mImage[corners[2]];
            T f110 = mImage[corners[3]];
            T f001 = mImage[corners[4]];
            T f101 = mImage[corners[5]];
            T f011 = mImage[corners[6]];
            T f111 = mImage[corners[7]];

            Vector3<T> gradient{};

            T tmp0 = oneMY * (f100 - f000) + position[1] * (f110 - f010);
            T tmp1 = oneMY * (f101 - f001) + position[1] * (f111 - f011);
            gradient[0] = oneMZ * tmp0 + position[2] * tmp1;

            tmp0 = oneMX * (f010 - f000) + position[0] * (f110 - f100);
            tmp1 = oneMX * (f011 - f001) + position[0] * (f111 - f101);
            gradient[1] = oneMZ * tmp0 + position[2] * tmp1;

            tmp0 = oneMX * (f001 - f000) + position[0] * (f101 - f100);
            tmp1 = oneMX * (f011 - f010) + position[0] * (f111 - f110);
            gradient[2] = oneMY * tmp0 + position[1] * tmp1;

            return gradient;
        }

        Image3<T> const& mImage;
    };
}
