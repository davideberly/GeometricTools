// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.12

#include <Mathematics/ParametricCurve.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>
#include <Graphics/Visual.h>
#include <functional>
#include <memory>
#include <vector>

namespace gte
{
    class RevolutionSurface
    {
    public:
        // Create a revolution surface with the specified parameters. The axis
        // of revolution is the z-axis. You can obtain arbitrary axes after
        // the fact by applying translations and rotations to the object. The
        // curve of revolution is (x(t),z(t)) with t in [tmin,tmax] and
        // z(t) > 0. It is also assumed to be non-self-intersecting. When the
        // curve is open, three cases to consider: If z(tmin) and z(tmax) are
        // both positive, the surface topology is that of a cylinder
        // (REV_CYLINDER_TOPOLOGY). If exactly one of z(tmin) or z(tmax) is
        // zero, the surface topology is that of a disk (REV_DISK_TOPOLOGY).
        // If z(tmin) and z(tmax) are both zero, the surface topology is that
        // of a sphere (REV_SPHERE_TOPOLOGY). When the curve of revolution is
        // closed so that (x(tmin),z(tmin)) and (x(tmax),z(tmax)) are the same
        // point, the surface topology is that of a torus. For now, rather
        // than having the surface object determine the type of curve, use
        // the Topology enumerated values to provide this information.
        //
        // The 'vformat' must have position bound as shown. The other channels
        // are filled in by MeshFactory creation.
        //   Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0)

        enum TopologyType
        {
            REV_DISK_TOPOLOGY,
            REV_CYLINDER_TOPOLOGY,
            REV_SPHERE_TOPOLOGY,
            REV_TORUS_TOPOLOGY
        };

        RevolutionSurface(
            std::shared_ptr<ParametricCurve<2, float>> const& curve,
            float xCenter, TopologyType topology, size_t numCurveSamples,
            size_t numRadialSamples, VertexFormat const& vformat,
            bool sampleByArcLength, bool outsideView, bool dynamicUpdate);

        inline std::shared_ptr<Visual> const& GetSurface() const
        {
            return mSurface;
        }

        // Modify vertices and normals when the curve itself changes over
        // time. You are responsible for maintaining the curve topology. For
        // example, if your constructor input was REV_SPHERE_TOPOLOGY, you
        // should not change the curve to produce a non-spherical topology.
        void UpdateSurface();

        // You may modify the curve that generates the surface, which is
        // useful for dynamic effects.
        void SetCurve(std::shared_ptr<ParametricCurve<2, float>> const& curve)
        {
            mCurve = curve;
        }

    protected:
        void UpdateDisk();
        void UpdateCylinder();
        void UpdateSphere();
        void UpdateTorus();
        void ComputeSinCos();

        inline Vector3<float>& Position(size_t i)
        {
            return *reinterpret_cast<Vector3<float>*>(mPosData + i * mVertexSize);
        }

        // Constructor inputs.
        std::shared_ptr<ParametricCurve<2, float>> mCurve;
        float mXCenter;
        TopologyType mTopology;
        size_t mNumCurveSamples, mNumRadialSamples;
        VertexFormat mVFormat;
        bool mSampleByArcLength, mOutsideView, mDynamicUpdate;

        // Support for computing slice vertices.
        std::vector<Vector3<float>> mSamples;
        std::vector<float> mSin, mCos;

        // Accessors to the vertex channels.
        uint32_t mNumVertices, mVertexSize;
        char* mPosData;

        // The computed surface as a graphics object.
        std::shared_ptr<Visual> mSurface;
    };
}
