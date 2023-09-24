// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/Texture3Effect.h>
#include <Mathematics/MarchingCubes.h>
using namespace gte;

//#define USE_DRAW_DIRECT

class SurfaceExtractionWindow3 : public Window3
{
public:
    SurfaceExtractionWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    enum
    {
        XBOUND = 64,
        YBOUND = 64,
        ZBOUND = 64,
        NUM_VOXELS = XBOUND * YBOUND * ZBOUND,
        NUM_GAUSSIANS = 32,
        XTHREADS = 4,
        YTHREADS = 4,
        ZTHREADS = 4,
        XGROUPS = XBOUND / XTHREADS,
        YGROUPS = YBOUND / YTHREADS,
        ZGROUPS = ZBOUND / ZTHREADS
    };

    bool SetEnvironment();
    bool CreateScene();
    void CreateSharedResources();
    void UpdateConstants();

    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    // Resources shared by direct and indirect drawing of voxels.
    MarchingCubes<int32_t> mMarchingCubes;
    std::shared_ptr<StructuredBuffer> mLookup;
    std::shared_ptr<StructuredBuffer> mImage;
    std::shared_ptr<ConstantBuffer> mParametersBuffer;
    float* mLevel;
    Transform<float> mTranslate;
    std::shared_ptr<Texture3> mColorTexture;
    std::shared_ptr<SamplerState> mColorSampler;

#if defined(USE_DRAW_DIRECT)
    // Resources specific to direct drawing.
    void CreateMesh();
    bool CreateDirectResources();

    struct DirectVoxel
    {
        // GLSL will store the first 3 'int32_t' members in a 4-tuple, so the
        // 'unused0' member is padding.
        int32_t configuration;
        int32_t numVertices;
        int32_t numTriangles;
        int32_t unused0;

        Vector4<float> vertices[12];

        // GLSL will store the array in a 16-element chunk of memory, so the
        // 'unused1' member is padding.
        // 'unused0' member is padding.
        int32_t indices[15];
        int32_t unused1;
    };

    struct Vertex
    {
        Vector3<float> position;
        Vector3<float> tcoord;
    };

    std::shared_ptr<StructuredBuffer> mDirectVoxels;
    std::shared_ptr<ComputeProgram> mDirectExtractProgram;
    std::shared_ptr<Texture3Effect> mDirectDrawEffect;
    std::shared_ptr<Visual> mDirectMesh;
#else
    // Resources specific to indirect drawing.
    bool CreateIndirectResources();

    struct IndirectVoxel
    {
        uint32_t index;
        uint32_t configuration;
    };

    std::shared_ptr<Visual> mVoxelMesh;
    std::shared_ptr<StructuredBuffer> mIndirectVoxels;
    std::shared_ptr<ComputeProgram> mIndirectExtractProgram;
    std::shared_ptr<VisualEffect> mIndirectDrawEffect;
    std::shared_ptr<ConstantBuffer> mIndirectPVWMatrixBuffer;
    Matrix4x4<float>* mIndirectPVWMatrix;
#endif
};
