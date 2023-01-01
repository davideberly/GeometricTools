// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.25

#include "FitConeByEllipseAndPointsWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/ApprCone3EllipseAndPoints.h>
#include <random>

std::array<std::string, 4> FitConeByEllipseAndPointsWindow3::msFile =
{
    "CircleAndVertex.txt",
    "OneCircleOneEllipse.txt",
    "TwoEllipses.txt",
    "TwoPartialEllipses.txt"
};

FitConeByEllipseAndPointsWindow3::FitConeByEllipseAndPointsWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mFileSelection(0),
    mBlendState(),
    mNoCullState(),
    mNoCullWireState(),
    mPoints{},
    mPointMesh{},
    mBoxMesh{},
    mEllipseMesh{},
    mConeMesh{},
    mDrawPointMesh(false),
    mDrawBoxMesh(false),
    mDrawEllipseMesh(true),
    mDrawConeMesh(true)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);

    InitializeCamera(60.0f, GetAspectRatio(), 0.001f, 100.0f, 0.0001f, 0.0001f,
        { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
}

void FitConeByEllipseAndPointsWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mDrawPointMesh)
    {
        mEngine->Draw(mPointMesh);
    }

    if (mDrawBoxMesh)
    {
        for (auto const& mesh : mBoxMesh)
        {
            mEngine->Draw(mesh);
        }
    }

    if (mDrawEllipseMesh)
    {
        mEngine->Draw(mEllipseMesh[0]);
        mEngine->Draw(mEllipseMesh[1]);
    }

    if (mDrawConeMesh)
    {
        mEngine->SetBlendState(mBlendState);
        mEngine->Draw(mConeMesh);
        mEngine->SetDefaultBlendState();
    }

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool FitConeByEllipseAndPointsWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '0':
        if (mFileSelection != 0)
        {
            mFileSelection = 0;
            DeleteScene();
            CreateScene();
        }
        return true;

    case '1':
        if (mFileSelection != 1)
        {
            mFileSelection = 1;
            DeleteScene();
            CreateScene();
        }
        return true;

    case '2':
        if (mFileSelection != 2)
        {
            mFileSelection = 2;
            DeleteScene();
            CreateScene();
        }
        return true;

    case '3':
        if (mFileSelection != 3)
        {
            mFileSelection = 3;
            DeleteScene();
            CreateScene();
        }
        return true;

    case 'p':
    case 'P':
        mDrawPointMesh = !mDrawPointMesh;
        return true;

    case 'b':
    case 'B':
        mDrawBoxMesh = !mDrawBoxMesh;
        return true;

    case 'e':
    case 'E':
        mDrawEllipseMesh = !mDrawEllipseMesh;
        return true;

    case 'c':
    case 'C':
        mDrawConeMesh = !mDrawConeMesh;
        return true;

    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullWireState)
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool FitConeByEllipseAndPointsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Mathematics/FitConeByEllipseAndPoints/Data/");

    for (auto const& input : msFile)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

void FitConeByEllipseAndPointsWindow3::CreateScene()
{
    std::string path = mEnvironment.GetPath(msFile[mFileSelection]);
    std::ifstream input(path);

    // Subtract out the averages of the points so that you can rotate the
    // scene using the virtual trackball (move by left-mouse-click-and-drag).
    Vector3<double> average{ 0.0, 0.0, 0.0 };
    for (;;)
    {
        Vector3<double> point{};
        input >> point[0] >> point[1] >> point[2];
        if (input.eof())
        {
            break;
        }

        mPoints.push_back(point);
        average += point;
    }
    input.close();

    average /= static_cast<double>(mPoints.size());
    for (auto& p : mPoints)
    {
        p -= average;
    }

    // Extract ellipses from the points.
    ApprCone3ExtractEllipses<double> extractor{};
    std::vector<Ellipse3<double>> ellipses{};
    extractor.Extract(mPoints, 1e-06, 1e-06, ellipses);

    // Fit a cone to an ellipse and the points.
    ApprCone3EllipseAndPoints<double> fitter{};
    auto cone = fitter.Fit(ellipses[0], mPoints);

    // Access the bounding boxes for visualization.
    auto const& boxes = extractor.GetBoxes();

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    uint32_t numVertices = static_cast<uint32_t>(mPoints.size());
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto* vertices = vbuffer->Get<Vector3<float>>();
    for (size_t i = 0; i < mPoints.size(); ++i)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(mPoints[i][j]);
        }
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IPType::IP_POLYPOINT, numVertices);

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f });

    mPointMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mPointMesh);
    mTrackBall.Attach(mPointMesh);

    std::default_random_engine dre{};
    std::uniform_real_distribution<float> urd(0.25f, 0.75f);

    MeshFactory mf{};
    mf.SetVertexFormat(vformat);

    mBoxMesh.reserve(boxes.size());
    for (size_t i = 0; i < boxes.size(); ++i)
    {
        auto const& box = boxes[i];

        auto mesh = mf.CreateBox(
            static_cast<float>(box.extent[0]),
            static_cast<float>(box.extent[1]),
            static_cast<float>(box.extent[2]));

        Vector4<float> color{ urd(dre), urd(dre), urd(dre), 1.0f };
        auto colorEffect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
        mesh->SetEffect(colorEffect);

        Vector3<float> translate
        {
            static_cast<float>(box.center[0]),
            static_cast<float>(box.center[1]),
            static_cast<float>(box.center[2])
        };
        mesh->localTransform.SetTranslation(translate);

        Matrix3x3<float> rotate{};
        for (int32_t j = 0; j < 3; ++j)
        {
            Vector3<float> axis
            {
                static_cast<float>(box.axis[j][0]),
                static_cast<float>(box.axis[j][1]),
                static_cast<float>(box.axis[j][2])
            };
            rotate.SetCol(j, axis);
        }
        mesh->localTransform.SetRotation(rotate);

        mPVWMatrices.Subscribe(mesh);
        mTrackBall.Attach(mesh);
        mBoxMesh.push_back(mesh);
    }

    numVertices = 256;
    auto const& E0 = ellipses[0];
    vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        double t = GTE_C_TWO_PI * static_cast<double>(i) / 256.0;
        double cs = std::cos(t), sn = std::sin(t);
        Vector3<double> epoint = E0.center +
            E0.extent[0] * cs * E0.axis[0] +
            E0.extent[1] * sn * E0.axis[1];

        for (int32_t j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(epoint[j]);
        }
    }
    ibuffer = std::make_shared<IndexBuffer>(IPType::IP_POLYPOINT, numVertices);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 1.0f, 0.0f, 1.0f });
    mEllipseMesh[0] = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mEllipseMesh[0]);
    mTrackBall.Attach(mEllipseMesh[0]);

    auto const& E1 = ellipses[1];
    vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        double t = GTE_C_TWO_PI * static_cast<double>(i) / 256.0;
        double cs = std::cos(t), sn = std::sin(t);
        Vector3<double> epoint = E1.center +
            E1.extent[0] * cs * E1.axis[0] +
            E1.extent[1] * sn * E1.axis[1];

        for (int32_t j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(epoint[j]);
        }
    }
    ibuffer = std::make_shared<IndexBuffer>(IPType::IP_POLYPOINT, numVertices);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 1.0f, 0.0f, 0.0f, 1.0f });
    mEllipseMesh[1] = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mEllipseMesh[1]);
    mTrackBall.Attach(mEllipseMesh[1]);

    float const coneHeight = 2.0f;
    float const coneRadius = coneHeight * (float)cone.tanAngle;
    mConeMesh = mf.CreateDisk(64, 64, coneRadius);
    vbuffer = mConeMesh->GetVertexBuffer();
    numVertices = vbuffer->GetNumElements();
    vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        float radial = Length(vertices[i]);
        vertices[i][2] = coneHeight * radial / coneRadius;
    }

    std::array<Vector3<float>, 3> basis{};
    basis[0] = { (float)cone.ray.direction[0], (float)cone.ray.direction[1],
        (float)cone.ray.direction[2] };
    ComputeOrthogonalComplement(1, basis.data());
    Matrix3x3<float> rotate{};
    rotate.SetCol(0, basis[1]);
    rotate.SetCol(1, basis[2]);
    rotate.SetCol(2, basis[0]);
    Vector3<float> translate = { (float)cone.ray.origin[0],
        (float)cone.ray.origin[1], (float)cone.ray.origin[2] };
    mConeMesh->localTransform.SetRotation(rotate);
    mConeMesh->localTransform.SetTranslation(translate);

    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 0.5f });
    mConeMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mConeMesh);
    mTrackBall.Attach(mConeMesh);

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void FitConeByEllipseAndPointsWindow3::DeleteScene()
{
    mPoints.clear();
    mPointMesh.reset();

    for (auto& mesh : mBoxMesh)
    {
        mesh.reset();
    }

    for (auto& mesh : mEllipseMesh)
    {
        mesh.reset();
    }

    mConeMesh.reset();
}
