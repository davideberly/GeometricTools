// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "SurfaceExtractionWindow3.h"
#include <random>

SurfaceExtractionWindow3::SurfaceExtractionWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mLevel(nullptr)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 4.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void SurfaceExtractionWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    UpdateConstants();

    mEngine->ClearBuffers();
    std::array<float, 4> textColor{ 0.0f, 0.0f, 0.0f, 1.0f };

#if defined(USE_DRAW_DIRECT)
    // Extract the voxels using a compute shader.
    mDirectVoxels->SetNumActiveElements(0);
    mEngine->Execute(mDirectExtractProgram, XGROUPS, YGROUPS, ZGROUPS);

    // Copy the vertices and indices from the GPU to the CPU.  Create a
    // triangle mesh from them.
    CreateMesh();

    if (mDirectMesh)
    {
        // Draw the triangle mesh.
        mEngine->Draw(mDirectMesh.get());
    }

    mEngine->Draw(8, mYSize-24, textColor, "direct: level = " + std::to_string(*mLevel));
#else
    // Extract the voxels using a compute shader.
    mIndirectVoxels->SetNumActiveElements(0);
    mEngine->Execute(mIndirectExtractProgram, XGROUPS, YGROUPS, ZGROUPS);

    // Copy from the GPU to the CPU only the number of voxels extracted.
    mEngine->GetNumActiveElements(mIndirectVoxels);
    int numVoxels = mIndirectVoxels->GetNumActiveElements();
    if (numVoxels > 0)
    {
        // Draw the triangle mesh directly from the voxel information
        // that is already on the GPU.
        mVoxelMesh->GetVertexBuffer()->SetNumActiveElements(numVoxels);
        mEngine->Draw(mVoxelMesh);
    }

    mEngine->Draw(8, mYSize - 24, textColor, "indirect: level = " + std::to_string(*mLevel));
#endif

    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool SurfaceExtractionWindow3::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':  // Toggle solid/wireframe.
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullSolidState)
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullSolidState);
        }
        return true;

    case '+':  // Increase the level value for the isosurface.
    case '=':
        *mLevel += 0.01f;
        if (*mLevel > 0.99f)
        {
            *mLevel = 0.99f;
        }
        mEngine->Update(mParametersBuffer);
        return true;

    case '-':  // Decrease the level value for the isosurface.
    case '_':
        *mLevel -= 0.01f;
        if (*mLevel < 0.01f)
        {
            *mLevel = 0.01f;
        }
        mEngine->Update(mParametersBuffer);
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool SurfaceExtractionWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Imagics/SurfaceExtraction/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("ExtractSurface.cs"),
        mEngine->GetShaderName("ExtractSurfaceIndirect.cs"),
        mEngine->GetShaderName("DrawSurfaceIndirect.vs"),
        mEngine->GetShaderName("DrawSurfaceIndirect.gs"),
        mEngine->GetShaderName("DrawSurfaceIndirect.ps")
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

bool SurfaceExtractionWindow3::CreateScene()
{
    CreateSharedResources();
#if defined(USE_DRAW_DIRECT)
    return CreateDirectResources();
#else
    return CreateIndirectResources();
#endif
}

void SurfaceExtractionWindow3::CreateSharedResources()
{
    // Disable culling.
    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->cullMode = RasterizerState::CULL_NONE;
    mNoCullSolidState->fillMode = RasterizerState::FILL_SOLID;
    mEngine->SetRasterizerState(mNoCullSolidState);

    // Enable wireframe (when requested).
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    // Create the Marching Cubes table.
    unsigned int const numElements = 256 * 41;
    unsigned int const numBytes = numElements * sizeof(int);
    mLookup = std::make_shared<StructuredBuffer>(numElements, sizeof(int));
    std::memcpy(mLookup->GetData(), mMarchingCubes.GetTable(), numBytes);

    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> symr(-1.0f, 1.0f);
    std::uniform_real_distribution<float> posr(0.01f, 100.0f);

    // Create an image as a sum of randomly generated Gaussians distributions.
    std::vector<Vector3<float>> mean(NUM_GAUSSIANS);
    std::vector<Matrix3x3<float>> covariance(NUM_GAUSSIANS);
    for (int i = 0; i < NUM_GAUSSIANS; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            mean[i][j] = symr(mte);
            mean[i][j] = symr(mte);
            mean[i][j] = symr(mte);
        }

        Vector3<float> D{ posr(mte), posr(mte), posr(mte) };
        Matrix3x3<float> diagonal;
        MakeDiagonal(D, diagonal);
        Quaternion<float> q(symr(mte), symr(mte), symr(mte), symr(mte));
        Normalize(q);
        Matrix3x3<float> rotate = Rotation<3,float>(q);
        covariance[i] = rotate*diagonal*Transpose(rotate);
    }

    float const dx = 2.0f / XBOUND, dy = 2.0f / YBOUND, dz = 2.0f / ZBOUND;
    mImage = std::make_shared<StructuredBuffer>(NUM_VOXELS, sizeof(float));
    auto* image = mImage->Get<float>();
    Vector3<float> pos;
    float wmin = std::numeric_limits<float>::max(), wmax = 0.0f;
    for (int z = 0; z < ZBOUND; ++z)
    {
        pos[2] = -1.0f + 2.0f * z / ZBOUND;
        for (int y = 0; y < YBOUND; ++y)
        {
            pos[1] = -1.0f + 2.0f * y / YBOUND;
            for (int x = 0; x < XBOUND; ++x)
            {
                pos[0] = -1.0f + 2.0f * x / XBOUND;

                float w = 0.0f;
                for (int i = 0; i < NUM_GAUSSIANS; ++i)
                {
                    Vector3<float> diff = pos - mean[i];
                    float arg = Dot(diff, covariance[i] * diff);
                    w += std::exp(-arg);
                }

                *image++ = w;
                if (w > wmax)
                {
                    wmax = w;
                }
                if (w < wmin)
                {
                    wmin = w;
                }
            }
        }
    }

    // Scale to [0,1].
    float invRange = 1.0f / (wmax - wmin);
    image = mImage->Get<float>();
    for (int i = 0; i < NUM_VOXELS; ++i)
    {
        image[i] = (image[i] - wmin)*invRange;
    }

    mParametersBuffer = std::make_shared<ConstantBuffer>(4 * sizeof(float), true);
    auto* param = mParametersBuffer->Get<float>();
    *param++ = dx;
    *param++ = dy;
    *param++ = dz;
    *param = 0.5f;
    mLevel = param;

    mTranslate.SetTranslation(-1.0f, -1.0f, -1.0f);

    mColorTexture = std::make_shared<Texture3>(DF_R8G8B8A8_UNORM, 2, 2, 2);
    auto* color = mColorTexture->Get<unsigned int>();
    color[0] = 0xFF000000;
    color[1] = 0xFF0000FF;
    color[2] = 0xFF00FF00;
    color[3] = 0xFF00FFFF;
    color[4] = 0xFFFF0000;
    color[5] = 0xFFFF00FF;
    color[6] = 0xFFFFFF00;
    color[7] = 0xFFFFFFFF;
}

#if defined(USE_DRAW_DIRECT)

bool SurfaceExtractionWindow3::CreateDirectResources()
{
    // Create the compute shader.
    mProgramFactory->defines.Set("XBOUND", XBOUND);
    mProgramFactory->defines.Set("YBOUND", YBOUND);
    mProgramFactory->defines.Set("ZBOUND", ZBOUND);
    mProgramFactory->defines.Set("XTHREADS", XTHREADS);
    mProgramFactory->defines.Set("YTHREADS", YTHREADS);
    mProgramFactory->defines.Set("ZTHREADS", ZTHREADS);

    std::string csPath = mEnvironment.GetPath(mEngine->GetShaderName("ExtractSurface.cs"));
    mDirectExtractProgram = mProgramFactory->CreateFromFile(csPath);
    if (!mDirectExtractProgram)
    {
        return false;
    }

    // Create the buffer for voxel output.  Because we will read back the
    // voxels every frame, create a persistent staging buffer for the copy
    // (avoids creating/destroying a staging buffer on each read back).
    mDirectVoxels = std::make_shared<StructuredBuffer>(NUM_VOXELS, sizeof(DirectVoxel));
    mDirectVoxels->MakeAppendConsume();
    mDirectVoxels->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    // Attach resources to the shader.
    auto cshader = mDirectExtractProgram->GetComputeShader();
    cshader->Set("Parameters", mParametersBuffer);
    cshader->Set("lookup", mLookup);
    cshader->Set("image", mImage);
    cshader->Set("voxels", mDirectVoxels);

    // Create a vertex color effect (for now uses only red).
    mDirectDrawEffect = std::make_shared<Texture3Effect>(mProgramFactory,
        mColorTexture, SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP,
        SamplerState::CLAMP, SamplerState::CLAMP);

    // The mDirectMesh will be created each frame by a call to CreateMesh().
    return true;
}

void SurfaceExtractionWindow3::CreateMesh()
{
    mEngine->CopyGpuToCpu(mDirectVoxels);
    DirectVoxel* voxels = mDirectVoxels->Get<DirectVoxel>();
    int numActive = mDirectVoxels->GetNumActiveElements();
    if (numActive <= 0)
    {
        return;
    }

    // Create the mesh.
    std::vector<Vector3<float>> vertices;
    std::vector<int> indices;
    for (int i = 0, vbase = 0; i < numActive; ++i)
    {
        DirectVoxel const& voxel = voxels[i];
        for (int j = 0; j < voxel.numVertices; ++j)
        {
            Vector3<float> vertex;
            vertex[0] = voxel.vertices[j][0];
            vertex[1] = voxel.vertices[j][1];
            vertex[2] = voxel.vertices[j][2];
            vertices.push_back(vertex);
        }

        for (int j = 0; j < 3 * voxel.numTriangles; ++j)
        {
            indices.push_back(vbase + voxel.indices[j]);
        }

        vbase += voxel.numVertices;
    }

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32B32_FLOAT, 0);
    unsigned int numVertices = static_cast<unsigned int>(vertices.size());
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto* v = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i, ++v)
    {
        v->position = vertices[i];
        v->tcoord = 0.5f*vertices[i];
    }

    unsigned int numTriangles = static_cast<int>(indices.size() / 3);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(unsigned int));
    std::memcpy(ibuffer->GetData(), &indices[0], ibuffer->GetNumBytes());

    mDirectMesh = std::make_shared<Visual>(vbuffer, ibuffer, mDirectDrawEffect);
}

#else

bool SurfaceExtractionWindow3::CreateIndirectResources()
{
    // Create the shaders.
    ProgramDefines defines;
    mProgramFactory->defines.Set("XBOUND", XBOUND);
    mProgramFactory->defines.Set("YBOUND", YBOUND);
    mProgramFactory->defines.Set("ZBOUND", ZBOUND);
    mProgramFactory->defines.Set("XTHREADS", XTHREADS);
    mProgramFactory->defines.Set("YTHREADS", YTHREADS);
    mProgramFactory->defines.Set("ZTHREADS", ZTHREADS);

    std::string csPath = mEnvironment.GetPath(mEngine->GetShaderName("ExtractSurfaceIndirect.cs"));
    mIndirectExtractProgram = mProgramFactory->CreateFromFile(csPath);
    if (!mIndirectExtractProgram)
    {
        return false;
    }

#if defined(GTE_USE_OPENGL)
    BufferLayout layoutVoxelsCS;
    auto iepCShader = mIndirectExtractProgram->GetComputeShader();
    iepCShader->GetStructuredBufferLayout("voxels", layoutVoxelsCS);
    for (auto const& layout : layoutVoxelsCS)
    {
        if (layout.name == "index")
        {
            LogAssert(layout.offset == offsetof(IndirectVoxel, index),
                "IndirectVoxel::index in GLSL is at offset = " + std::to_string(layout.offset));
        }
        else if (layout.name == "configuration")
        {
            LogAssert(layout.offset == offsetof(IndirectVoxel, configuration),
                "IndirectVoxel::configuration in GLSL is at offset = " + std::to_string(layout.offset));
        }
    }
    auto const layoutSizeVoxelsCS = iepCShader->GetStructuredBufferSize("voxels");
    LogAssert(layoutSizeVoxelsCS == sizeof(IndirectVoxel),
        "IndirectVoxel in GLSL has size = " + std::to_string(layoutSizeVoxelsCS));
#endif

    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("DrawSurfaceIndirect.vs"));
    std::string gsPath = mEnvironment.GetPath(mEngine->GetShaderName("DrawSurfaceIndirect.gs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("DrawSurfaceIndirect.ps"));
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(vsPath, psPath, gsPath);
    if (!program)
    {
        return false;
    }

    mProgramFactory->defines.Clear();

    // Create the buffer for voxel output and that is used as the actual
    // vertex buffer input.
    mIndirectVoxels = std::make_shared<StructuredBuffer>(NUM_VOXELS, sizeof(IndirectVoxel));
    mIndirectVoxels->MakeAppendConsume();

    // Create the vertex and index buffers for SV_VertexID-based drawing.
    VertexFormat vformat;
    vformat.Bind(VA_NO_SEMANTIC, DF_R32G32_UINT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, mIndirectVoxels);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, NUM_VOXELS);

    // Create and attach resources to the shaders.
    auto cshader = mIndirectExtractProgram->GetComputeShader();
    cshader->Set("Parameters", mParametersBuffer);
    cshader->Set("image", mImage);
    cshader->Set("voxels", mIndirectVoxels);

    program->GetVertexShader()->Set("voxels", mIndirectVoxels);

    mIndirectPVWMatrixBuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    mIndirectPVWMatrix = mIndirectPVWMatrixBuffer->Get<Matrix4x4<float>>();
    *mIndirectPVWMatrix = Matrix4x4<float>::Identity();

    auto gshader = program->GetGeometryShader();
    gshader->Set("Parameters", mParametersBuffer);
    gshader->Set("PVWMatrix", mIndirectPVWMatrixBuffer);
    gshader->Set("lookup", mLookup);
    gshader->Set("image", mImage);

    mColorSampler = std::make_shared<SamplerState>();
    mColorSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    mColorSampler->mode[0] = SamplerState::CLAMP;
    mColorSampler->mode[1] = SamplerState::CLAMP;
    mColorSampler->mode[2] = SamplerState::CLAMP;

    program->GetPixelShader()->Set("colorTexture", mColorTexture, "colorSampler", mColorSampler);

    mIndirectDrawEffect = std::make_shared<VisualEffect>(program);

    mVoxelMesh = std::make_shared<Visual>(vbuffer, ibuffer, mIndirectDrawEffect);
    return true;
}

#endif

void SurfaceExtractionWindow3::UpdateConstants()
{
    // Compute the new world transforms and copy to constant buffers.
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float> rotate = mTrackBall.GetOrientation();
    Matrix4x4<float> translate = mTranslate.GetHMatrix();
    Matrix4x4<float> pvwMatrix = DoTransform(DoTransform(pvMatrix, rotate), translate);

#if defined(USE_DRAW_DIRECT)
    mDirectDrawEffect->SetPVWMatrix(pvwMatrix);
    mEngine->Update(mDirectDrawEffect->GetPVWMatrixConstant());
#else
    *mIndirectPVWMatrix = pvwMatrix;
    mEngine->Update(mIndirectPVWMatrixBuffer);
#endif
}
