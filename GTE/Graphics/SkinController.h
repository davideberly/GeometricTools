// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Controller.h>
#include <Graphics/VertexBuffer.h>
#include <Mathematics/Vector4.h>
#include <cstdint>

namespace gte
{
    class Node;

    class SkinController : public Controller
    {
    public:
        // Construction and destruction.  The numbers of vertices and bones
        // are fixed for the lifetime of the object.  The controlled object
        // must have a vertex buffer with 'numVertices' elements, with 3D
        // (x,y,z) or 4D (x,y,z,1) positions, and the bind of positions is in
        // unit 0.  The post-update function is used to allow a graphics
        // engine object to copy the modified vertex buffer to graphics memory.
        virtual ~SkinController() = default;
        SkinController(int32_t numVertices, int32_t numBones, BufferUpdater const& postUpdate);

        // Member access.  After calling the constructor, you must set the
        // data using these functions.  The bone array uses weak pointers to
        // avoid reference-count cycles in the scene graph.
        inline int32_t GetNumVertices() const
        {
            return mNumVertices;
        }

        inline int32_t GetNumBones() const
        {
            return mNumBones;
        }

        inline std::vector<std::weak_ptr<Node>>& GetBones()
        {
            return mBones;
        }

        inline std::vector<float>& GetWeights()
        {
            return mWeights;
        }

        inline std::vector<Vector4<float>>& GetOffsets()
        {
            return mOffsets;
        }

        // The animation update.  The application time is in milliseconds.
        virtual bool Update(double applicationTime) override;

    protected:
        // On the first call to Update(...), the position channel and stride
        // are extracted from mObject's vertex buffer.  This is a deferred
        // construction, because we do not know mObject when SkinController
        // is constructed.
        void OnFirstUpdate();

        int32_t mNumVertices;
        int32_t mNumBones;

        // bones[numBones]
        std::vector<std::weak_ptr<Node>> mBones;

        // weight[numVertices * numBones], index = bone + numBones * vertex
        std::vector<float> mWeights;

        // offfset[numVertices * numBones], index = bone + numBones * vertex
        std::vector<Vector4<float>> mOffsets;

        BufferUpdater mPostUpdate;
        char* mPosition;
        uint32_t mStride;
        bool mFirstUpdate, mCanUpdate;
    };
}
