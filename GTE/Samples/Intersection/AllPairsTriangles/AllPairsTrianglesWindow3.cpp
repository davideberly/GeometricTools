// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "AllPairsTrianglesWindow3.h"
#include <Graphics/MeshFactory.h>

AllPairsTrianglesWindow3::AllPairsTrianglesWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    int32_t const count = 48;
    if (!SetEnvironment()
        || !CreateCylinder(count, count, 1.0f, 8.0f)    // 4416 triangles
        || !CreateTorus(count, count, 2.0f, 0.5f))      // 4608 triangles
    {
        parameters.created = false;
        return;
    }

#if !defined(USE_CPU_FIND_INTERSECTIONS)
    if (!CreateShaders())
    {
        parameters.created = false;
        return;
    }
#endif

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.1f, 0.01f,
        { 8.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mPVWMatrices.Update();
}

void AllPairsTrianglesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    UpdateTransforms();
    FindIntersections();

    mEngine->ClearBuffers();
#if defined(USE_CPU_FIND_INTERSECTIONS)
    mEngine->Draw(mCylinder);
    mEngine->Draw(mTorus);
#else
    mEngine->Draw(mCylinderID);
    mEngine->Draw(mTorusID);
#endif
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool AllPairsTrianglesWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() != mWireState)
        {
            mEngine->SetRasterizerState(mWireState);
        }
        else
        {
            mEngine->SetDefaultRasterizerState();
        }
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool AllPairsTrianglesWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Intersection/AllPairsTriangles/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("DrawUsingVertexID.vs"),
        mEngine->GetShaderName("DrawUsingVertexID.ps"),
        mEngine->GetShaderName("InitializeColors.cs"),
        mEngine->GetShaderName("TriangleIntersection.cs"),
        mEngine->GetShaderName("VertexColorIndexed.vs"),
        mEngine->GetShaderName("VertexColorIndexed.ps")
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

bool AllPairsTrianglesWindow3::CreateCylinder(uint32_t numAxisSamples,
    uint32_t numRadialSamples, float radius, float height)
{
    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("VertexColorIndexed.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("VertexColorIndexed.ps"));
    auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!program)
    {
        return false;
    }

    // Create a cylinder as an indexed triangle mesh.  The positions are used
    // to create a cylinder as a non-indexed collection of triangles.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto cylinder = mf.CreateCylinderClosed(numAxisSamples, numRadialSamples, radius, height);
    auto const& vbuffer = cylinder->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vector3<float>>();
    auto const& ibuffer = cylinder->GetIndexBuffer();
    uint32_t numIndices = ibuffer->GetNumElements();
    auto* indices = ibuffer->Get<uint32_t>();

    // Create a cylinder as a non-indexed collection of triangles.  The vertex
    // colors are generated in the shaders by a color index that is in
    // {0,1,2,3}.  The vertex format is (x,y,z,colorIndex).
    VertexFormat meshVFormat;
    meshVFormat.Bind(VASemantic::POSITION, DF_R32G32B32A32_FLOAT, 0);
    auto meshVBuffer = std::make_shared<VertexBuffer>(meshVFormat, numIndices);
    meshVBuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto* meshVertices = meshVBuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numIndices; ++i)
    {
        meshVertices[i].position = vertices[indices[i]];
        meshVertices[i].colorIndex = 0.0f;
    }

    mNumCylinderTriangles = numIndices / 3;
    auto meshIBuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, mNumCylinderTriangles);

    mCylinderPVWMatrix = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVertexShader()->Set("PVWMatrix", mCylinderPVWMatrix);
    mCylinderEffect = std::make_shared<VisualEffect>(program);

    mCylinder = std::make_shared<Visual>(meshVBuffer, meshIBuffer, mCylinderEffect);
    return true;
}

bool AllPairsTrianglesWindow3::CreateTorus(uint32_t numCircleSamples,
    uint32_t numRadialSamples, float outerRadius, float innerRadius)
{
    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("VertexColorIndexed.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("VertexColorIndexed.ps"));
    auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!program)
    {
        return false;
    }

    // Create a torus as an indexed triangle mesh.  The positions are used
    // to create a torus as a non-indexed collection of triangles.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto torus = mf.CreateTorus(numCircleSamples, numRadialSamples, outerRadius, innerRadius);
    auto const& vbuffer = torus->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vector3<float>>();
    auto const& ibuffer = torus->GetIndexBuffer();
    uint32_t numIndices = ibuffer->GetNumElements();
    auto* indices = ibuffer->Get<uint32_t>();

    // Create a torus as a non-indexed collection of triangles.  The vertex
    // colors are generated in the shaders by a color index that is in
    // {0,1,2,3}.  The vertex format is (x,y,z,colorIndex).
    VertexFormat meshVFormat;
    meshVFormat.Bind(VASemantic::POSITION, DF_R32G32B32A32_FLOAT, 0);
    auto meshVBuffer = std::make_shared<VertexBuffer>(meshVFormat, numIndices);
    meshVBuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto* meshVertices = meshVBuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numIndices; ++i)
    {
        meshVertices[i].position = vertices[indices[i]];
        meshVertices[i].colorIndex = 1.0f;
    }

    mNumTorusTriangles = numIndices / 3;
    auto meshIBuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, mNumTorusTriangles);

    mTorusPVWMatrix = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVertexShader()->Set("PVWMatrix", mTorusPVWMatrix);
    mTorusEffect = std::make_shared<VisualEffect>(program);

    mTorus = std::make_shared<Visual>(meshVBuffer, meshIBuffer, mTorusEffect);
    return true;
}

#if !defined(USE_CPU_FIND_INTERSECTIONS)

bool AllPairsTrianglesWindow3::CreateShaders()
{
    // Create the compute programs.
    uint32_t const numThreads = 8;
    mNumXGroups = mNumCylinderTriangles / numThreads;
    mNumYGroups = mNumTorusTriangles / numThreads;
    mProgramFactory->defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", numThreads);
    std::string csPath = mEnvironment.GetPath(mEngine->GetShaderName("InitializeColors.cs"));
    mInitializeColor = mProgramFactory->CreateFromFile(csPath);
    if (!mInitializeColor)
    {
        return false;
    }
    csPath = mEnvironment.GetPath(mEngine->GetShaderName("TriangleIntersection.cs"));
    mTriangleIntersection = mProgramFactory->CreateFromFile(csPath);
    if (!mTriangleIntersection)
    {
        return false;
    }
    mProgramFactory->defines.Clear();

    // Create the visual programs.
    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("DrawUsingVertexID.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("DrawUsingVertexID.ps"));
    auto cylinderProgram = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!cylinderProgram)
    {
        return false;
    }
    mCylinderIDEffect = std::make_shared<VisualEffect>(cylinderProgram);

    vsPath = mEnvironment.GetPath(mEngine->GetShaderName("DrawUsingVertexID.vs"));
    psPath = mEnvironment.GetPath(mEngine->GetShaderName("DrawUsingVertexID.ps"));
    auto torusProgram = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!torusProgram)
    {
        return false;
    }
    mTorusIDEffect = std::make_shared<VisualEffect>(torusProgram);

    uint32_t const numIndices0 = 3 * mNumCylinderTriangles;
    uint32_t const numIndices1 = 3 * mNumTorusTriangles;

    // Create resources for the compute programs and attach them to the
    // shaders.
    mColor0Buffer = std::make_shared<StructuredBuffer>(numIndices0, sizeof(uint32_t));
    mColor0Buffer->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mColor0Buffer->SetCopy(Resource::Copy::STAGING_TO_CPU);

    mColor1Buffer = std::make_shared<StructuredBuffer>(numIndices1, sizeof(uint32_t));
    mColor1Buffer->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mColor1Buffer->SetCopy(Resource::Copy::STAGING_TO_CPU);

    mTIParameters = std::make_shared<ConstantBuffer>(sizeof(TIParameters), true);
    TIParameters& data = *mTIParameters->Get<TIParameters>();
    data.wMatrix0 = Matrix4x4<float>::Identity();
    data.wMatrix1 = Matrix4x4<float>::Identity();
    data.numTriangles0 = mNumCylinderTriangles;
    data.numTriangles1 = mNumTorusTriangles;

    mVertices0 = std::make_shared<StructuredBuffer>(numIndices0, sizeof(Vector4<float>));
    auto* data0 = mVertices0->Get<Vector4<float>>();
    auto* meshVertices0 = mCylinder->GetVertexBuffer()->Get<Vertex>();
    for (uint32_t i = 0; i < numIndices0; ++i)
    {
        data0[i] = HLift(meshVertices0[i].position, 1.0f);
    }

    mVertices1 = std::make_shared<StructuredBuffer>(numIndices1, sizeof(Vector4<float>));
    auto* data1 = mVertices1->Get<Vector4<float>>();
    auto* meshVertices1 = mTorus->GetVertexBuffer()->Get<Vertex>();
    for (uint32_t i = 0; i < numIndices1; ++i)
    {
        data1[i] = HLift(meshVertices1[i].position, 1.0f);
    }

    std::shared_ptr<Shader> cshader = mInitializeColor->GetComputeShader();
    cshader->Set("color0", mColor0Buffer);
    cshader->Set("color1", mColor1Buffer);

    cshader = mTriangleIntersection->GetComputeShader();
    cshader->Set("Parameters", mTIParameters);
    cshader->Set("vertices0", mVertices0);
    cshader->Set("vertices1", mVertices1);
    cshader->Set("color0", mColor0Buffer);
    cshader->Set("color1", mColor1Buffer);

    // Create resources for the cylinder visual program, attach them to the
    // shaders, and create the geometric primitive.
    auto cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    std::shared_ptr<Shader> vshader = cylinderProgram->GetVertexShader();
    vshader->Set("PVWMatrix", cbuffer);
    vshader->Set("positions", mVertices0);
    vshader->Set("colorIndices", mColor0Buffer);
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32A32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, mVertices0);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numIndices0 / 3);
    mCylinderID = std::make_shared<Visual>(vbuffer, ibuffer, mCylinderIDEffect);

    // Create resources for the torus visual program, attach them to the
    // shaders, and create the geometric primitive.
    cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    vshader = torusProgram->GetVertexShader();
    vshader->Set("PVWMatrix", cbuffer);
    vshader->Set("positions", mVertices1);
    vshader->Set("colorIndices", mColor1Buffer);
    vbuffer = std::make_shared<VertexBuffer>(vformat, mVertices1);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numIndices1 / 3);
    mTorusID = std::make_shared<Visual>(vbuffer, ibuffer, mTorusIDEffect);
    return true;
}

#endif

void AllPairsTrianglesWindow3::UpdateTransforms()
{
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float> wMatrix = mTrackBall.GetOrientation();
    Matrix4x4<float> pvwMatrix = DoTransform(pvMatrix, wMatrix);
    mCylinderPVWMatrix->SetMember("pvwMatrix", pvMatrix);
    mTorusPVWMatrix->SetMember("pvwMatrix", pvwMatrix);
    mEngine->Update(mCylinderPVWMatrix);
    mEngine->Update(mTorusPVWMatrix);

#if !defined(USE_CPU_FIND_INTERSECTIONS)
    TIParameters& data = *mTIParameters->Get<TIParameters>();
    data.wMatrix0 = Matrix4x4<float>::Identity();
    data.wMatrix1 = wMatrix;
    mEngine->Update(mTIParameters);

    std::shared_ptr<ConstantBuffer> cbuffer;
    cbuffer = mCylinderIDEffect->GetVertexShader()->Get<ConstantBuffer>("PVWMatrix");
    *cbuffer->Get<Matrix4x4<float>>() = pvMatrix;
    mEngine->Update(cbuffer);
    cbuffer = mTorusIDEffect->GetVertexShader()->Get<ConstantBuffer>("PVWMatrix");
    *cbuffer->Get<Matrix4x4<float>>() = pvwMatrix;
    mEngine->Update(cbuffer);
#endif
}

void AllPairsTrianglesWindow3::FindIntersections()
{
#if defined(USE_CPU_FIND_INTERSECTIONS)
    auto const& buffer0 = mCylinder->GetVertexBuffer();
    uint32_t numVertices0 = buffer0->GetNumElements();
    uint32_t numTriangles0 = numVertices0 / 3;
    auto* vertices0 = buffer0->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices0; ++i)
    {
        vertices0[i].colorIndex = 0.0f;
    }

    auto const& buffer1 = mTorus->GetVertexBuffer();
    uint32_t numVertices1 = buffer1->GetNumElements();
    uint32_t numTriangles1 = numVertices1 / 3;
    auto* vertices1 = buffer1->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices1; ++i)
    {
        vertices1[i].colorIndex = 1.0f;
    }

    Matrix4x4<float> wMatrix = mTrackBall.GetOrientation();
    TriangleIntersection intersects;
    Vector3<float> cylinder[3], torus[3];
    for (uint32_t t0 = 0; t0 < numTriangles0; ++t0)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            cylinder[j] = vertices0[3 * t0 + j].position;
        }

        for (uint32_t t1 = 0; t1 < numTriangles1; ++t1)
        {
            for (int32_t j = 0; j < 3; ++j)
            {
                Vector4<float> pos = HLift(vertices1[3 * t1 + j].position, 1.0f);
                pos = DoTransform(wMatrix, pos);
                torus[j] = HProject(pos);
            }

            if (intersects(cylinder, torus))
            {
                for (int32_t j = 0; j < 3; ++j)
                {
                    vertices0[3 * t0 + j].colorIndex = 2.0f;
                    vertices1[3 * t1 + j].colorIndex = 3.0f;
                }
            }
        }
    }

    mEngine->Update(buffer0);
    mEngine->Update(buffer1);
#else
    mEngine->Execute(mInitializeColor, mNumXGroups, mNumYGroups, 1);
    mEngine->Execute(mTriangleIntersection, mNumXGroups, mNumYGroups, 1);
#endif
}
