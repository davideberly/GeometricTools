// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "FitCylinderWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/ApprCylinder3.h>
#include <iostream>

// Expose only one of these.
#define USE_MESH_POINTS
//#define USE_CYLINDER_RING
//#define USE_CYLINDER_SKEW

// Expose this if you want the fitter to use the eigenvector corresponding to
// the largest eigenvalue of the covariance matrix as the cylinder axis
// direction.  Otherwise, a hemisphere of directions are searched for the
// one that produces the minimum error.
//#define USE_COVARIANCE_W_DIRECTION

// When the hemisphere is searched, we can do this in a single thread,
// which is slow.  Or we can search using multiple threads.  Expose this
// define if you want a multithreaded search.
#define USE_MULTIPLE_THREADS

FitCylinderWindow3::FitCylinderWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });

    CreateScene();

    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
        { -30.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void FitCylinderWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mPoints);
    mEngine->SetRasterizerState(mNoCullWireState);
    mEngine->Draw(mCylinder);
    mEngine->SetDefaultRasterizerState();
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool FitCylinderWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Mathematics/FitCylinder/Data/");

    if (mEnvironment.GetPath("mesh.txt") == "")
    {
        LogError("Cannot find file mesh.txt.");
        return false;
    }

    return true;
}

void FitCylinderWindow3::CreateScene()
{
    std::vector<Vector3<double>> positions;

#ifdef USE_MESH_POINTS
    std::ifstream input(mEnvironment.GetPath("mesh.txt"));
    uint32_t const numPoints = 10765;
    for (uint32_t i = 0; i < numPoints; ++i)
    {
        Vector3<double> data{};
        input >> data[0];
        input >> data[1];
        input >> data[2];
        positions.push_back(data);
    }
    input.close();
#endif

#ifdef USE_CYLINDER_RING
    for (uint32_t j = 0; j < 64; ++j)
    {
        double theta = GTE_C_TWO_PI * j / 64.0;
        double cstheta = std::cos(theta);
        double sntheta = std::sin(theta);
        for (uint32_t i = 0; i <= 64; ++i)
        {
            double t = -2.0 + 4.0 * i / 64.0;
            Vector3<double> sample{ cstheta, sntheta, t };
            positions.push_back(sample);
        }
    }
#endif

#ifdef USE_CYLINDER_SKEW
    double const b = 0.25;
    for (uint32_t j = 0; j < 64; ++j)
    {
        double theta = GTE_C_TWO_PI * j / 64.0;
        double cstheta = std::cos(theta);
        double sntheta = std::sin(theta);
        for (uint32_t i = 0; i <= 64; ++i)
        {
            double t = -b + cstheta + 2.0 * b * i / 64.0;
            Vector3<double> sample{ cstheta, sntheta, t };
            positions.push_back(sample);
        }
    }
#endif

#ifdef USE_COVARIANCE_W_DIRECTION
    // Use the eigenvector associated with the largest eigenvalue of W.
    ApprCylinder3<double> fitter(2);
#else
#ifdef USE_MULTIPLE_THREADS
    // Use all hardware threads available (subject to OS scheduling).
    uint32_t numThreads = std::thread::hardware_concurrency();
    ApprCylinder3<double> fitter(numThreads, 1024, 512);
#else
    // Execute the algorithm on the main thread.
    ApprCylinder3<double> fitter(0, 1024, 512);
#endif
#endif
    uint32_t numVertices = static_cast<uint32_t>(positions.size());
    Cylinder3<double> cylinder;
    double minError = fitter(numVertices, positions.data(), cylinder);
    std::cout << "min error = " << minError << std::endl;
    std::cout << "center = "
        << cylinder.axis.origin[0] << " "
        << cylinder.axis.origin[1] << " "
        << cylinder.axis.origin[2] << std::endl;
    std::cout << "direction = "
        << cylinder.axis.direction[0] << " "
        << cylinder.axis.direction[1] << " "
        << cylinder.axis.direction[2] << std::endl;
    std::cout << "radius = " << cylinder.radius << std::endl;
    std::cout << "height = " << cylinder.height << std::endl;

    // Create point cloud for display.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto* vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(positions[i][j]);
        }
    }
    
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, numVertices);

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{1.0f, 0.0f, 1.0f, 1.0f});

    mPoints = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mPoints->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mPoints);

    Vector3<float> translate{};
    for (uint32_t j = 0; j < 3; ++j)
    {
        translate[j] = static_cast<float>(-cylinder.axis.origin[j]);
    }
    mPoints->localTransform.SetTranslation(translate);

    std::array<Vector3<float>, 3> basis{};
    for (uint32_t j = 0; j < 3; ++j)
    {
        basis[0][j] = static_cast<float>(cylinder.axis.direction[j]);
    }
    ComputeOrthogonalComplement(1, basis.data());
#if defined(GTE_USE_MAT_VEC)
    Matrix4x4<float> rotate
    {
        basis[1][0], basis[2][0], basis[0][0], 0.0f,
        basis[1][1], basis[2][1], basis[0][1], 0.0f,
        basis[1][2], basis[2][2], basis[0][2], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
#else
    Matrix4x4<float> rotate
    {
        basis[1][0], basis[1][1], basis[1][2], 0.0f,
        basis[2][0], basis[2][1], basis[2][2], 0.0f,
        basis[0][0], basis[0][1], basis[0][2], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
#endif

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetIndexFormat(true);
    float radius = static_cast<float>(cylinder.radius);
    float height = static_cast<float>(cylinder.height);
#ifdef USE_MESH_POINTS
    mCylinder = mf.CreateCylinderOpen(8, 32, radius, height);
#else
    mCylinder = mf.CreateCylinderOpen(8, 32, radius, height);
#endif
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{0.0f, 0.0f, 1.0f, 1.0f});
    mCylinder->SetEffect(effect);
    mCylinder->localTransform.SetRotation(rotate);
    mPVWMatrices.Subscribe(mCylinder->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mCylinder);
}
