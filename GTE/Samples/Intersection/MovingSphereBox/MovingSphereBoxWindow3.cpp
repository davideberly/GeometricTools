// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "MovingSphereBoxWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>

MovingSphereBoxWindow3::MovingSphereBoxWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mAlpha(0.5f),
    mNumSamples0(128),
    mNumSamples1(64),
    mSample0(0),
    mSample1(0),
    mDX(0.1f),
    mDY(0.1f),
    mDZ(0.1f),
    mDrawSphereVisual(true)
{
    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mEngine->SetRasterizerState(mNoCullState);

    CreateScene();

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 24.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void MovingSphereBoxWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    // This is not the correct drawing order, but it is close enough for
    // demonstrating the moving sphere-box intersection query.
    mEngine->SetBlendState(mBlendState);

    if (mDrawSphereVisual)
    {
        mEngine->Draw(mSphereVisual);
    }
    mEngine->Draw(mVelocityVisual);
    if (mSphereContactVisual->culling != CullingMode::ALWAYS)
    {
        mEngine->Draw(mPointContactVisual);
        mEngine->Draw(mSphereContactVisual);
    }

    mEngine->Draw(mBoxVisual);
    for (int32_t i = 0; i < 8; ++i)
    {
        mEngine->Draw(mVertexVisual[i]);
    }
    for (int32_t i = 0; i < 12; ++i)
    {
        mEngine->Draw(mEdgeVisual[i]);
    }
    for (int32_t i = 0; i < 6; ++i)
    {
        mEngine->Draw(mFaceVisual[i]);
    }

    mEngine->SetDefaultBlendState();

    std::array<float, 4> const black{ 0.0f, 0.0f, 0.0f, 1.0f };
    mEngine->Draw(8, mYSize - 8, black, mTimer.GetFPS());
    mEngine->Draw(8, 24, black, mMessage);
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MovingSphereBoxWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullState == mEngine->GetRasterizerState())
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;

    // Manually launch the intersection query.
    case 'e':
    case 'E':
        UpdateSphereCenter();
        return true;

    // Modify theta in [0,2*pi].
    case 'a':
        mSample0 = (mSample0 + mNumSamples0 - 1) % mNumSamples0;
        UpdateSphereVelocity();
        return true;
    case 'A':
        mSample0 = (mSample0 + 1) % mNumSamples0;
        UpdateSphereVelocity();
        return true;

    // Modify phi in [0,pi].
    case 'b':
        mSample1 = (mSample1 + mNumSamples1 - 1) % mNumSamples1;
        UpdateSphereVelocity();
        return true;
    case 'B':
        mSample1 = (mSample1 + 1) % mNumSamples1;
        UpdateSphereVelocity();
        return true;

    // Translate the sphere.
    case 'x':
        mSphere.center[0] -= mDX;
        UpdateSphereCenter();
        return true;
    case 'X':
        mSphere.center[0] += mDX;
        UpdateSphereCenter();
        return true;
    case 'y':
        mSphere.center[1] -= mDY;
        UpdateSphereCenter();
        return true;
    case 'Y':
        mSphere.center[1] += mDY;
        UpdateSphereCenter();
        return true;
    case 'z':
        mSphere.center[2] -= mDZ;
        UpdateSphereCenter();
        return true;
    case 'Z':
        mSphere.center[2] += mDZ;
        UpdateSphereCenter();
        return true;

    // Toggle the drawing of the moving sphere.
    case 's':
    case 'S':
        mDrawSphereVisual = !mDrawSphereVisual;
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void MovingSphereBoxWindow3::CreateScene()
{
    mBoxRoot = std::make_shared<Node>();
    mTrackBall.Attach(mBoxRoot);

#if defined(APP_USE_OBB)
    mBox.center = { 1.0f, 0.0f, 0.0f };
    mBox.axis[0] = { 1.0f, 1.0f, 1.0f };
    Normalize(mBox.axis[0]);
    mBox.axis[1] = { 1.0f, -1.0f, 0.0f };
    Normalize(mBox.axis[1]);
    mBox.axis[2] = Cross(mBox.axis[0], mBox.axis[1]);
    mBox.extent = { 3.0f, 2.0f, 1.0f };
    Matrix3x3<float> rotate;
    rotate.SetCol(0, mBox.axis[0]);
    rotate.SetCol(1, mBox.axis[1]);
    rotate.SetCol(2, mBox.axis[2]);
    mBoxRoot->localTransform.SetTranslation(mBox.center);
    mBoxRoot->localTransform.SetRotation(rotate);
#else
    mBox.min = { -3.0f, -2.0f, -1.0f };
    mBox.max = { +3.0f, +2.0f, +1.0f };
#endif
    mSphere.center = { 5.0f, 5.0f, 5.0f };
    mSphere.radius = 1.0f;
    mBoxVelocity = { 0.0f, 0.0f, 0.0f };

    CreateRoundedBoxVertices();
    CreateRoundedBoxEdges();
    CreateRoundedBoxFaces();
    CreateBox();
    CreateSpheres();
    CreateMotionCylinder();
    UpdateSphereVelocity();
}

void MovingSphereBoxWindow3::CreateRoundedBoxVertices()
{
    // Create the octants of a sphere by using NURBS.
    float sqrt2 = std::sqrt(2.0f), sqrt3 = std::sqrt(3.0f);
    float a0 = (sqrt3 - 1.0f) / sqrt3;
    float a1 = (sqrt3 + 1.0f) / (2.0f * sqrt3);
    float a2 = 1.0f - (5.0f - sqrt2) * (7.0f - sqrt3) / 46.0f;
    float b0 = 4.0f * sqrt3 * (sqrt3 - 1.0f);
    float b1 = 3.0f * sqrt2;
    float b2 = 4.0f;
    float b3 = sqrt2 * (3.0f + 2.0f * sqrt2 - sqrt3) / sqrt3;

    std::array<std::array<Vector3<float>, 5>, 5> control{};
    std::array<std::array<float, 5>, 5> weight{};
    std::array<std::array<std::function<float(float, float, float)>, 5>, 5> bernstein{};

    control[0][0] = { 0.0f, 0.0f, 1.0f };   // P004
    control[0][1] = { 0.0f, a0,   1.0f };   // P013
    control[0][2] = { 0.0f, a1,   a1 };     // P022
    control[0][3] = { 0.0f, 1.0f, a0 };     // P031
    control[0][4] = { 0.0f, 1.0f, 0.0f };   // P040

    control[1][0] = { a0,   0.0f, 1.0f };   // P103
    control[1][1] = { a2,   a2,   1.0f };   // P112
    control[1][2] = { a2,   1.0f, a2 };     // P121
    control[1][3] = { a0,   1.0f, 0.0f };   // P130
    control[1][4] = { 0.0f, 0.0f, 0.0f };   // unused

    control[2][0] = { a1,   0.0f, a1 };     // P202
    control[2][1] = { 1.0f, a2,   a2 };     // P211
    control[2][2] = { a1,   a1,   0.0f };   // P220
    control[2][3] = { 0.0f, 0.0f, 0.0f };   // unused
    control[2][4] = { 0.0f, 0.0f, 0.0f };   // unused

    control[3][0] = { 1.0f, 0.0f, a0 };     // P301
    control[3][1] = { 1.0f, a0,   0.0f };   // P310
    control[3][2] = { 0.0f, 0.0f, 0.0f };   // unused
    control[3][3] = { 0.0f, 0.0f, 0.0f };   // unused
    control[3][4] = { 0.0f, 0.0f, 0.0f };   // unused

    control[4][0] = { 1.0f, 0.0f, 0.0f };   // P400
    control[4][1] = { 0.0f, 0.0f, 0.0f };   // unused
    control[4][2] = { 0.0f, 0.0f, 0.0f };   // unused
    control[4][3] = { 0.0f, 0.0f, 0.0f };   // unused
    control[4][4] = { 0.0f, 0.0f, 0.0f };   // unused

    weight[0][0] = b0;    // w004
    weight[0][1] = b1;    // w013
    weight[0][2] = b2;    // w022
    weight[0][3] = b1;    // w031
    weight[0][4] = b0;    // w040

    weight[1][0] = b1;    // w103
    weight[1][1] = b3;    // w112
    weight[1][2] = b3;    // w121
    weight[1][3] = b1;    // w130
    weight[1][4] = 0.0f;  // unused

    weight[2][0] = b2;    // w202
    weight[2][1] = b3;    // w211
    weight[2][2] = b2;    // w220
    weight[2][3] = 0.0f;  // unused
    weight[2][4] = 0.0f;  // unused

    weight[3][0] = b1;    // w301
    weight[3][1] = b1;    // w310
    weight[3][2] = 0.0f;  // unused
    weight[3][3] = 0.0f;  // unused
    weight[3][4] = 0.0f;  // unused

    weight[4][0] = b0;    // w400
    weight[4][1] = 0.0f;  // unused
    weight[4][2] = 0.0f;  // unused
    weight[4][3] = 0.0f;  // unused
    weight[4][4] = 0.0f;  // unused

    bernstein[0][0] = [](float, float, float w) { return w * w * w * w; };
    bernstein[0][1] = [](float, float v, float w) { return 4.0f * v * w * w * w; };
    bernstein[0][2] = [](float, float v, float w) { return 6.0f * v * v * w * w; };
    bernstein[0][3] = [](float, float v, float w) { return 4.0f * v * v * v * w; };
    bernstein[0][4] = [](float, float v, float) { return v * v * v * v; };
    bernstein[1][0] = [](float u, float, float w) { return 4.0f * u * w * w * w; };
    bernstein[1][1] = [](float u, float v, float w) { return 12.0f * u * v * w * w; };
    bernstein[1][2] = [](float u, float v, float w) { return 12.0f * u * v * v * w; };
    bernstein[1][3] = [](float u, float v, float) { return 4.0f * u * v * v * v; };
    bernstein[2][0] = [](float u, float, float w) { return 6.0f * u * u * w * w; };
    bernstein[2][1] = [](float u, float v, float w) { return 12.0f * u * u * v * w; };
    bernstein[2][2] = [](float u, float v, float) { return 6.0f * u * u * v * v; };
    bernstein[3][0] = [](float u, float, float w) { return 4.0f * u * u * u * w; };
    bernstein[3][1] = [](float u, float v, float) { return 4.0f * u * u * u * v; };
    bernstein[4][0] = [](float u, float, float) { return u * u * u * u; };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, DENSITY * DENSITY);
    auto vertices = vbuffer->Get<Vector3<float>>();
    std::memset(vbuffer->GetData(), 0, vbuffer->GetNumBytes());
    for (int32_t iv = 0; iv <= DENSITY - 1; ++iv)
    {
        float v = (float)iv / (float)(DENSITY - 1);
        for (int32_t iu = 0; iu + iv <= DENSITY - 1; ++iu)
        {
            float u = (float)iu / (float)(DENSITY - 1);
            float w = 1.0f - u - v;

            Vector3<float> numer{ 0.0f, 0.0f, 0.0f };
            float denom = 0.0f;
            for (int32_t j1 = 0; j1 <= 4; ++j1)
            {
                for (int32_t j0 = 0; j0 + j1 <= 4; ++j0)
                {
                    float product = weight[j1][j0] * bernstein[j1][j0](u, v, w);
                    numer += product * control[j1][j0];
                    denom += product;
                }
            }

            vertices[iu + DENSITY * iv] = mSphere.radius * numer / denom;
        }
    }

    std::vector<int32_t> indices;
    for (int32_t iv = 0; iv <= DENSITY - 2; ++iv)
    {
        // two triangles per square
        int32_t iu, j0, j1, j2, j3;
        for (iu = 0; iu + iv <= DENSITY - 3; ++iu)
        {
            j0 = iu + DENSITY * iv;
            j1 = j0 + 1;
            j2 = j0 + DENSITY;
            j3 = j2 + 1;
            indices.push_back(j0);
            indices.push_back(j1);
            indices.push_back(j2);
            indices.push_back(j1);
            indices.push_back(j3);
            indices.push_back(j2);
        }

        // last triangle in row is singleton
        j0 = iu + DENSITY * iv;
        j1 = j0 + 1;
        j2 = j0 + DENSITY;
        indices.push_back(j0);
        indices.push_back(j1);
        indices.push_back(j2);
    }

    uint32_t numTriangles = (uint32_t)(indices.size() / 3);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(int32_t));
    std::memcpy(ibuffer->GetData(), indices.data(), indices.size() * sizeof(int32_t));

    Vector4<float> color[8] =
    {
        Vector4<float>{ 0.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 0.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 0.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 0.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 0.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 0.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 0.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 0.0f, 1.0f, 0.0f, mAlpha }
    };

#if defined(APP_USE_OBB)
    Vector3<float> center[8] =
    {
        Vector3<float>{ -mBox.extent[0], -mBox.extent[1], -mBox.extent[2] },
        Vector3<float>{ +mBox.extent[0], -mBox.extent[1], -mBox.extent[2] },
        Vector3<float>{ -mBox.extent[0], +mBox.extent[1], -mBox.extent[2] },
        Vector3<float>{ +mBox.extent[0], +mBox.extent[1], -mBox.extent[2] },
        Vector3<float>{ -mBox.extent[0], -mBox.extent[1], +mBox.extent[2] },
        Vector3<float>{ +mBox.extent[0], -mBox.extent[1], +mBox.extent[2] },
        Vector3<float>{ -mBox.extent[0], +mBox.extent[1], +mBox.extent[2] },
        Vector3<float>{ +mBox.extent[0], +mBox.extent[1], +mBox.extent[2] }
    };
#else
    Vector3<float> center[8] =
    {
        Vector3<float>{ mBox.min[0], mBox.min[1], mBox.min[2] },
        Vector3<float>{ mBox.max[0], mBox.min[1], mBox.min[2] },
        Vector3<float>{ mBox.min[0], mBox.max[1], mBox.min[2] },
        Vector3<float>{ mBox.max[0], mBox.max[1], mBox.min[2] },
        Vector3<float>{ mBox.min[0], mBox.min[1], mBox.max[2] },
        Vector3<float>{ mBox.max[0], mBox.min[1], mBox.max[2] },
        Vector3<float>{ mBox.min[0], mBox.max[1], mBox.max[2] },
        Vector3<float>{ mBox.max[0], mBox.max[1], mBox.max[2] }
    };
#endif

    float sqrtHalf = std::sqrt(0.5f);
    Quaternion<float> orient[8] =
    {
        Quaternion<float>{ sqrtHalf, 0.0f, -sqrtHalf, 0.0f },
        Quaternion<float>{ 0.5f, -0.5f, 0.5f, -0.5f },
        Quaternion<float>{ 0.0f, 1.0f, 0.0f, 0.0f },
        Quaternion<float>{ 0.0f, sqrtHalf, 0.0f, sqrtHalf },
        Quaternion<float>{ 0.0f, 0.0f, 1.0f, 0.0f },
        Quaternion<float>{ 0.0f, 0.0f, -sqrtHalf, sqrtHalf },
        Quaternion<float>{ 0.0f, 0.0f, sqrtHalf, sqrtHalf },
        Quaternion<float>{ 0.0f, 0.0f, 0.0f, 1.0f }
    };

    mVNormal[0] = { -1.0f, -1.0f, -1.0f, 0.0f };
    mVNormal[1] = { +1.0f, -1.0f, -1.0f, 0.0f };
    mVNormal[2] = { -1.0f, +1.0f, -1.0f, 0.0f };
    mVNormal[3] = { +1.0f, +1.0f, -1.0f, 0.0f };
    mVNormal[4] = { -1.0f, -1.0f, +1.0f, 0.0f };
    mVNormal[5] = { +1.0f, -1.0f, +1.0f, 0.0f };
    mVNormal[6] = { -1.0f, +1.0f, +1.0f, 0.0f };
    mVNormal[7] = { +1.0f, +1.0f, +1.0f, 0.0f };

    for (int32_t i = 0; i < 8; ++i)
    {
        auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color[i]);
        mVertexVisual[i] = std::make_shared<Visual>(vbuffer, ibuffer, effect);
        mVertexVisual[i]->localTransform.SetTranslation(center[i]);
        mVertexVisual[i]->localTransform.SetRotation(orient[i]);
        mPVWMatrices.Subscribe(mVertexVisual[i]->worldTransform, effect->GetPVWMatrixConstant());

        mBoxRoot->AttachChild(mVertexVisual[i]);
    }
}

void MovingSphereBoxWindow3::CreateRoundedBoxEdges()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto visual = mf.CreateRectangle(DENSITY, DENSITY, 1.0f, 1.0f);
    auto const& vbuffer = visual->GetVertexBuffer();
    auto const& ibuffer = visual->GetIndexBuffer();
    auto vertices = vbuffer->Get<Vector3<float>>();
    for (int32_t row = 0; row < DENSITY; ++row)
    {
        float z = -1.0f + 2.0f * (float)row / (float)(DENSITY - 1);
        for (int32_t col = 0; col < DENSITY; ++col)
        {
            float angle = (float)GTE_C_HALF_PI * (float)col / (float)(DENSITY - 1);
            float cs = std::cos(angle), sn = std::sin(angle);
            *vertices++ = Vector3<float>{ mSphere.radius * cs, mSphere.radius * sn, z };
        }
    }

    Vector4<float> color[12] =
    {
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha },
        Vector4<float>{ 1.0f, 0.5f, 0.0f, mAlpha }
    };

#if defined(APP_USE_OBB)
    Vector3<float> center[12] =
    {
        Vector3<float>{ -mBox.extent[0], -mBox.extent[1], 0.0f },
        Vector3<float>{ +mBox.extent[0], -mBox.extent[1], 0.0f },
        Vector3<float>{ -mBox.extent[0], +mBox.extent[1], 0.0f },
        Vector3<float>{ +mBox.extent[0], +mBox.extent[1], 0.0f },
        Vector3<float>{ -mBox.extent[0], 0.0f, -mBox.extent[2] },
        Vector3<float>{ +mBox.extent[0], 0.0f, -mBox.extent[2] },
        Vector3<float>{ -mBox.extent[0], 0.0f, +mBox.extent[2] },
        Vector3<float>{ +mBox.extent[0], 0.0f, +mBox.extent[2] },
        Vector3<float>{ 0.0f, -mBox.extent[1], -mBox.extent[2] },
        Vector3<float>{ 0.0f, +mBox.extent[1], -mBox.extent[2] },
        Vector3<float>{ 0.0f, -mBox.extent[1], +mBox.extent[2] },
        Vector3<float>{ 0.0f, +mBox.extent[1], +mBox.extent[2] }
    };

    Vector3<float> scale[12] =
    {
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[2] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[2] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[2] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[2] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[1] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[1] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[1] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[1] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[0] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[0] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[0] },
        Vector3<float>{ 1.0f, 1.0f, mBox.extent[0] }
    };
#else
    Vector3<float> center[12] =
    {
        Vector3<float>{ -mBox.max[0], -mBox.max[1], 0.0f },
        Vector3<float>{ +mBox.max[0], -mBox.max[1], 0.0f },
        Vector3<float>{ -mBox.max[0], +mBox.max[1], 0.0f },
        Vector3<float>{ +mBox.max[0], +mBox.max[1], 0.0f },
        Vector3<float>{ -mBox.max[0], 0.0f, -mBox.max[2] },
        Vector3<float>{ +mBox.max[0], 0.0f, -mBox.max[2] },
        Vector3<float>{ -mBox.max[0], 0.0f, +mBox.max[2] },
        Vector3<float>{ +mBox.max[0], 0.0f, +mBox.max[2] },
        Vector3<float>{ 0.0f, -mBox.max[1], -mBox.max[2] },
        Vector3<float>{ 0.0f, +mBox.max[1], -mBox.max[2] },
        Vector3<float>{ 0.0f, -mBox.max[1], +mBox.max[2] },
        Vector3<float>{ 0.0f, +mBox.max[1], +mBox.max[2] }
    };

    Vector3<float> scale[12] =
    {
        Vector3<float>{ 1.0f, 1.0f, mBox.max[2] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[2] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[2] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[2] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[1] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[1] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[1] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[1] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[0] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[0] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[0] },
        Vector3<float>{ 1.0f, 1.0f, mBox.max[0] }
    };
#endif

    float sqrtHalf = std::sqrt(0.5f);
    Quaternion<float> orient[12] =
    {
        Quaternion<float>{ 0.0f, 0.0f, 1.0f, 0.0f },
        Quaternion<float>{ 0.0f, 0.0f, -sqrtHalf, sqrtHalf },
        Quaternion<float>{ 0.0f, 0.0f, sqrtHalf, sqrtHalf },
        Quaternion<float>{ 0.0f, 0.0f, 0.0f, 1.0f },

        Quaternion<float>{ -0.5f, 0.5f, 0.5f, 0.5f },
        Quaternion<float>{ -sqrtHalf, 0.0f, 0.0f, sqrtHalf },
        Quaternion<float>{ 0.5f, -0.5f, 0.5f, 0.5f },
        Quaternion<float>{ sqrtHalf, 0.0f, 0.0f, sqrtHalf },

        Quaternion<float>{ 0.5f, -0.5f, 0.5f, -0.5f },
        Quaternion<float>{ 0.0f, sqrtHalf, 0.0f, sqrtHalf },
        Quaternion<float>{ 0.5f, -0.5f, -0.5f, 0.5f },
        Quaternion<float>{ 0.0f, -sqrtHalf, 0.0f, sqrtHalf }
    };

    mENormal[ 0] = { -1.0f, -1.0f, 0.0f, 0.0f };
    mENormal[ 1] = { +1.0f, -1.0f, 0.0f, 0.0f };
    mENormal[ 2] = { -1.0f, +1.0f, 0.0f, 0.0f };
    mENormal[ 3] = { +1.0f, +1.0f, 0.0f, 0.0f };
    mENormal[ 4] = { -1.0f, 0.0f, -1.0f, 0.0f };
    mENormal[ 5] = { +1.0f, 0.0f, -1.0f, 0.0f };
    mENormal[ 6] = { -1.0f, 0.0f, +1.0f, 0.0f };
    mENormal[ 7] = { +1.0f, 0.0f, +1.0f, 0.0f };
    mENormal[ 8] = { 0.0f, -1.0f, -1.0f, 0.0f };
    mENormal[ 9] = { 0.0f, -1.0f, +1.0f, 0.0f };
    mENormal[10] = { 0.0f, +1.0f, -1.0f, 0.0f };
    mENormal[11] = { 0.0f, +1.0f, +1.0f, 0.0f };

    for (int32_t i = 0; i < 12; ++i)
    {
        auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color[i]);
        mEdgeVisual[i] = std::make_shared<Visual>(vbuffer, ibuffer, effect);
        mEdgeVisual[i]->localTransform.SetTranslation(center[i]);
        mEdgeVisual[i]->localTransform.SetRotation(orient[i]);
        mEdgeVisual[i]->localTransform.SetScale(scale[i]);
        mPVWMatrices.Subscribe(mEdgeVisual[i]->worldTransform, effect->GetPVWMatrixConstant());

        mBoxRoot->AttachChild(mEdgeVisual[i]);
    }
}

void MovingSphereBoxWindow3::CreateRoundedBoxFaces()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto visual = mf.CreateRectangle(DENSITY, DENSITY, 1.0f, 1.0f);
    auto const& vbuffer = visual->GetVertexBuffer();
    auto const& ibuffer = visual->GetIndexBuffer();

    Vector4<float> color[6] =
    {
        Vector4<float>{ 0.5f, 0.0f, 0.5f, mAlpha },
        Vector4<float>{ 0.5f, 0.0f, 0.5f, mAlpha },
        Vector4<float>{ 0.5f, 0.0f, 0.5f, mAlpha },
        Vector4<float>{ 0.5f, 0.0f, 0.5f, mAlpha },
        Vector4<float>{ 0.5f, 0.0f, 0.5f, mAlpha },
        Vector4<float>{ 0.5f, 0.0f, 0.5f, mAlpha }
    };

#if defined(APP_USE_OBB)
    Vector3<float> center[6] =
    {
        Vector3<float>{ 0.0f, 0.0f, -mBox.extent[2] - mSphere.radius },
        Vector3<float>{ 0.0f, 0.0f, +mBox.extent[2] + mSphere.radius },
        Vector3<float>{ 0.0f, -mBox.extent[1] - mSphere.radius, 0.0f },
        Vector3<float>{ 0.0f, +mBox.extent[1] + mSphere.radius, 0.0f },
        Vector3<float>{ -mBox.extent[0] - mSphere.radius, 0.0f, 0.0f },
        Vector3<float>{ +mBox.extent[0] + mSphere.radius, 0.0f, 0.0f }
    };

    Vector3<float> scale[6] =
    {
        Vector3<float>{ mBox.extent[0], mBox.extent[1], 1.0f },
        Vector3<float>{ mBox.extent[0], mBox.extent[1], 1.0f },
        Vector3<float>{ mBox.extent[0], 1.0f, mBox.extent[2] },
        Vector3<float>{ mBox.extent[0], 1.0f, mBox.extent[2] },
        Vector3<float>{ 1.0f, mBox.extent[1], mBox.extent[2] },
        Vector3<float>{ 1.0f, mBox.extent[1], mBox.extent[2] }
    };
#else
    Vector3<float> center[6] =
    {
        Vector3<float>{ 0.0f, 0.0f, -mBox.max[2] - mSphere.radius },
        Vector3<float>{ 0.0f, 0.0f, +mBox.max[2] + mSphere.radius },
        Vector3<float>{ 0.0f, -mBox.max[1] - mSphere.radius, 0.0f },
        Vector3<float>{ 0.0f, +mBox.max[1] + mSphere.radius, 0.0f },
        Vector3<float>{ -mBox.max[0] - mSphere.radius, 0.0f, 0.0f },
        Vector3<float>{ +mBox.max[0] + mSphere.radius, 0.0f, 0.0f }
    };

    Vector3<float> scale[6] =
    {
        Vector3<float>{ mBox.max[0], mBox.max[1], 1.0f },
        Vector3<float>{ mBox.max[0], mBox.max[1], 1.0f },
        Vector3<float>{ mBox.max[0], 1.0f, mBox.max[2] },
        Vector3<float>{ mBox.max[0], 1.0f, mBox.max[2] },
        Vector3<float>{ 1.0f, mBox.max[1], mBox.max[2] },
        Vector3<float>{ 1.0f, mBox.max[1], mBox.max[2] }
    };
#endif

    AxisAngle<3, float> aa;
    aa.axis = { 0.0f, 1.0f, 0.0f };
    aa.angle = { (float)-GTE_C_HALF_PI };
    Quaternion<float> q = Rotation<3, float>(aa);
    (void)q;

    float sqrtHalf = std::sqrt(0.5f);
    Quaternion<float> orient[6] =
    {
        Quaternion<float>{ 1.0f, 0.0f, 0.0f, 0.0f },  // done
        Quaternion<float>{ 0.0f, 0.0f, 0.0f, 1.0f },  // done
        Quaternion<float>{ sqrtHalf, 0.0f, 0.0f, sqrtHalf },
        Quaternion<float>{ -sqrtHalf, 0.0f, 0.0f, sqrtHalf },
        Quaternion<float>{ 0.0f, -sqrtHalf, 0.0f, sqrtHalf },
        Quaternion<float>{ 0.0f, sqrtHalf, 0.0f, sqrtHalf }
    };

    mFNormal[0] = { 0.0f, 0.0f, -1.0f, 0.0f };
    mFNormal[1] = { 0.0f, 0.0f, +1.0f, 0.0f };
    mFNormal[2] = { 0.0f, -1.0f, 0.0f, 0.0f };
    mFNormal[3] = { 0.0f, +1.0f, 0.0f, 0.0f };
    mFNormal[4] = { -1.0f, 0.0f, 0.0f, 0.0f };
    mFNormal[5] = { +1.0f, 0.0f, 0.0f, 0.0f };

    for (int32_t i = 0; i < 6; ++i)
    {
        auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color[i]);
        mFaceVisual[i] = std::make_shared<Visual>(vbuffer, ibuffer, effect);
        mFaceVisual[i]->localTransform.SetTranslation(center[i]);
        mFaceVisual[i]->localTransform.SetRotation(orient[i]);
        mFaceVisual[i]->localTransform.SetScale(scale[i]);
        mPVWMatrices.Subscribe(mFaceVisual[i]->worldTransform, effect->GetPVWMatrixConstant());

        mBoxRoot->AttachChild(mFaceVisual[i]);
    }
}

void MovingSphereBoxWindow3::CreateBox()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
#if defined(APP_USE_OBB)
    mBoxVisual = mf.CreateBox(mBox.extent[0], mBox.extent[1], mBox.extent[2]);
#else
    Vector3<float> extent = 0.5f * (mBox.max - mBox.min);
    mBoxVisual = mf.CreateBox(extent[0], extent[1], extent[2]);
#endif
    Vector4<float> color{ 0.5f, 0.5f, 0.5f, mAlpha };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mBoxVisual->SetEffect(effect);
    mPVWMatrices.Subscribe(mBoxVisual->worldTransform, effect->GetPVWMatrixConstant());

    mBoxRoot->AttachChild(mBoxVisual);
}

void MovingSphereBoxWindow3::CreateSpheres()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mSphereVisual = mf.CreateSphere(16, 16, mSphere.radius);
    Vector4<float> color{ 0.75f, 0.75f, 0.75f, mAlpha };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mSphereVisual->SetEffect(effect);
    mSphereVisual->localTransform.SetTranslation(mSphere.center);
    mPVWMatrices.Subscribe(mSphereVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mSphereVisual);

    mSphereContactVisual = mf.CreateSphere(16, 16, mSphere.radius);
    color = { 0.25f, 0.25f, 0.25f, mAlpha };
    mSphereContactVisual->culling = CullingMode::ALWAYS;
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mSphereContactVisual->SetEffect(effect);
    mSphereContactVisual->localTransform.SetTranslation(mSphere.center);
    mPVWMatrices.Subscribe(mSphereContactVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mSphereContactVisual);

    mPointContactVisual = mf.CreateSphere(8, 8, mSphere.radius / 8.0f);
    color = { 1.0f, 0.0f, 0.0f, mAlpha };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mPointContactVisual->SetEffect(effect);
    mPointContactVisual->localTransform.SetTranslation(mSphere.center);
    mPVWMatrices.Subscribe(mPointContactVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mPointContactVisual);
}

void MovingSphereBoxWindow3::CreateMotionCylinder()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    auto vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { 0.0f, 0.0f, 0.0f };
    vertices[1] = { 0.0f, 0.0f, 1000.0f };
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);
    Vector4<float> color{ 0.0f, 1.0f, 0.0f, mAlpha };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mVelocityVisual = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mVelocityVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mVelocityVisual);
}

void MovingSphereBoxWindow3::UpdateSphereVelocity()
{
    float angle0 = static_cast<float>(mSample0 * GTE_C_TWO_PI / mNumSamples0);
    float angle1 = static_cast<float>(mSample1 * GTE_C_PI / mNumSamples1);
    float cs0 = std::cos(angle0), sn0 = std::sin(angle0);
    float cs1 = std::cos(angle1), sn1 = std::sin(angle1);
    mSphereVelocity = { cs0 * sn1, sn0 * sn1, cs1 };

    std::array<Vector3<float>, 3> basis{};
    basis[0] = mSphereVelocity;
    ComputeOrthogonalComplement(1, basis.data());
    Matrix3x3<float> rotate{};
    rotate.SetCol(0, basis[1]);
    rotate.SetCol(1, basis[2]);
    rotate.SetCol(2, basis[0]);
    mVelocityVisual->localTransform.SetRotation(rotate);
    mVelocityVisual->localTransform.SetTranslation(mSphere.center);
    mVelocityVisual->Update();

    auto result = mQuery(mBox, mBoxVelocity, mSphere, mSphereVelocity);
    bool intersect = (result.intersectionType != 0);
    if (intersect)
    {
        mSphereContactVisual->culling = CullingMode::DYNAMIC;
        mSphereContactVisual->localTransform.SetTranslation(
            mSphere.center + result.contactTime * mSphereVelocity);
        mSphereContactVisual->Update();
        mPointContactVisual->localTransform.SetTranslation(result.contactPoint);
        mPointContactVisual->Update();

        // Transform the contact point to box coordinates for debugging.
#if defined(APP_USE_OBB)
        Vector3<float> temp = result.contactPoint - mBox.center;
        Vector3<float> P{ Dot(temp, mBox.axis[0]), Dot(temp, mBox.axis[1]), Dot(temp, mBox.axis[2]) };
#else
        Vector3<float> P = result.contactPoint;
#endif
        mMessage = "(" +
            std::to_string(P[0]) + ", " +
            std::to_string(P[1]) + ", " +
            std::to_string(P[2]) + ")";
    }
    else
    {
        mSphereContactVisual->culling = CullingMode::ALWAYS;
        mMessage = "";
    }

    mPVWMatrices.Update();
    mTrackBall.Update();
}

void MovingSphereBoxWindow3::UpdateSphereCenter()
{
    mSphereVisual->localTransform.SetTranslation(mSphere.center);
    mSphereVisual->Update();
    UpdateSphereVelocity();
}
