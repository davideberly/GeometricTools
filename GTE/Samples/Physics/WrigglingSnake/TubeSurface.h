// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.11

#pragma once

#include <Mathematics/ParametricCurve.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>
#include <Graphics/Visual.h>
#include <functional>
#include <memory>
#include <vector>

namespace gte
{
    class TubeSurface
    {
    public:
        // Create a tube surface with the specified parameters. The centerline
        // of the tube is defined by the 'medial' curve. The radius of the
        // tube is computed by the 'radial' function using the t-parameter of
        // the medial curve. Circular cross sections are computed at a sample
        // of t-values.
        //
        // The 'vformat' must have position bound as shown. The normals and
        // texture coordinates are optional but must be bound as shown.
        //   Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0)
        //   Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0)
        //   Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0)
        //
        // If upVector is not the zero vector, it will be used as 'up' in the
        // frame calculations. If it is the zero vector, the Frenet frame will
        // be used. If you want texture coordinates, the tcoordMin and
        // tcoordMax values specify the rectangular domain of texture
        // coordinates for the rectangular-topology surface. If 'vformat' does
        // not have the texture-coordinate semantic, tcoordMin and tcoordMax
        // are ignored.
        TubeSurface(
            std::shared_ptr<ParametricCurve<3, float>> const& medial,
            std::shared_ptr<std::function<float(float)>> const& radial,
            Vector3<float> const& upVector, size_t numMedialSamples,
            size_t numSliceSamples, VertexFormat const& vformat,
            Vector2<float> const& tcoordMin, Vector2<float> const& tcoordMax,
            bool closed, bool sampleByArcLength, bool insideView, bool dynamicUpdate);

        inline std::shared_ptr<Visual> const& GetSurface() const
        {
            return mSurface;
        }

        // Generate vertices for the end slices. These are useful when you
        // build an open tube and want to attach meshes at the ends to close
        // the tube. The output array has size S+1 where S is the number of
        // slice samples. Function GetTMinSlice is used to access the slice
        // corresponding to the medial curve evaluated at its domain minimum,
        // tmin. Function GetTMaxSlice accesses the slice for the domain
        // maximum, tmax. If the curve is closed, the slices are the same.
        void GetTMinSlice(std::vector<Vector3<float>>& slice);
        void GetTMaxSlice(std::vector<Vector3<float>>& slice);

        // If the medial curve is modified, for example if it is control point
        // based and the control points are modified, then you should call
        // this update function to recompute the tube surface geometry.
        void UpdateSurface();

    protected:
        // Tessellation support.
        inline size_t Index(size_t s, size_t m)
        {
            return s + (mNumSliceSamples + 1) * m;
        }

        void ComputeSinCos();
        void ComputeVertices();
        void ComputeNormals();
        void ComputeUVs();
        void ComputeIndices(uint32_t* indices, bool insideView);

        inline Vector3<float>& Position(size_t i)
        {
            return *reinterpret_cast<Vector3<float>*>(mPosData + i * mVertexSize);
        }

        inline Vector3<float>& Normal(size_t i)
        {
            return *reinterpret_cast<Vector3<float>*>(mNorData + i * mVertexSize);
        }

        inline Vector2<float>& TCoord(size_t i)
        {
            return *reinterpret_cast<Vector2<float>*>(mTcdData + i * mVertexSize);
        }

        // Constructor inputs.
        std::shared_ptr<ParametricCurve<3, float>> mMedial;
        std::shared_ptr<std::function<float(float)>> mRadial;
        Vector3<float> mUpVector;
        size_t mNumMedialSamples, mNumSliceSamples;
        VertexFormat mVFormat;
        Vector2<float> mTCoordMin, mTCoordMax;
        bool mClosed, mSampleByArcLength, mInsideView, mDynamicUpdate;

        // Support for computing slice vertices.
        std::vector<float> mSin, mCos;

        // Accessors to the vertex channels.
        uint32_t mNumVertices, mVertexSize;
        char* mPosData;
        char* mNorData;
        char* mTcdData;

        // The computed surface as a graphics object.
        std::shared_ptr<Visual> mSurface;
    };
}
