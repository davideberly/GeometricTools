// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "GelatinCubeWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/Texture2Effect.h>
#include <random>

GelatinCubeWindow3::GelatinCubeWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.713725f, 0.807843f, 0.929411f, 1.0f });

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    mDepthReadNoWriteState = std::make_shared<DepthStencilState>();
    mDepthReadNoWriteState->depthEnable = true;
    mDepthReadNoWriteState->writeMask = DepthStencilState::WriteMask::ZERO;

    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->fill = RasterizerState::Fill::SOLID;
    mNoCullSolidState->cull = RasterizerState::Cull::NONE;
    mEngine->SetRasterizerState(mNoCullSolidState);

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mNoCullWireState->cull = RasterizerState::Cull::NONE;

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.01f,
        { 0.0f, -1.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void GelatinCubeWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(GELATIN_CUBE_SINGLE_STEP)
    PhysicsTick();
#endif
    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool GelatinCubeWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullSolidState == mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullSolidState);
        }
        return true;

#if defined(GELATIN_CUBE_SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool GelatinCubeWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Water.png") == "")
    {
        LogError("Cannot find file Water.png");
        return false;
    }

    return true;
}

void GelatinCubeWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();
    CreateSprings();
    CreateCube();
    mTrackBall.Attach(mScene);
    mTrackBall.Update();
}

void GelatinCubeWindow3::CreateCube()
{
    // Create a quadratic volumetric spline using the interior particles as
    // control points.
    BasisFunctionInput<float> input[3] =
    {
        BasisFunctionInput<float>(mModule->GetNumSlices() - 2, 2),
        BasisFunctionInput<float>(mModule->GetNumRows() - 2, 2),
        BasisFunctionInput<float>(mModule->GetNumCols() - 2, 2)
    };

    mVolume = std::make_unique<BSplineVolume<3, float>>(input, nullptr);

    for (int32_t s = 0; s < input[2].numControls; ++s)
    {
        for (int32_t r = 0; r < input[1].numControls; ++r)
        {
            for (int32_t c = 0; c < input[0].numControls; ++c)
            {
                mVolume->SetControl(c, r, s, mModule->GetPosition(s + 1, r + 1, c + 1));
            }
        }
    }

    mNumUSamples = 8;
    mNumVSamples = 8;
    mNumWSamples = 8;

    uint32_t numVertices = 2 * (
        mNumUSamples * mNumVSamples +
        mNumUSamples * mNumWSamples +
        mNumVSamples * mNumWSamples);

    uint32_t numTriangles = 4 * (
        (mNumUSamples - 1) * (mNumVSamples - 1) +
        (mNumUSamples - 1) * (mNumWSamples - 1) +
        (mNumVSamples - 1) * (mNumWSamples - 1));

    // Create the cube mesh.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(uint32_t));
    auto indices = ibuffer->Get<uint32_t>();
    uint32_t vBase = 0;
    CreateFaceIndices(mNumWSamples, mNumVSamples, false, vBase, indices);  // u = 0
    CreateFaceIndices(mNumWSamples, mNumVSamples, true, vBase, indices);  // u = 1
    CreateFaceIndices(mNumWSamples, mNumUSamples, true, vBase, indices);  // v = 0
    CreateFaceIndices(mNumWSamples, mNumUSamples, false, vBase, indices);  // v = 1
    CreateFaceIndices(mNumVSamples, mNumUSamples, false, vBase, indices);  // w = 0
    CreateFaceIndices(mNumVSamples, mNumUSamples, true, vBase, indices);  // w = 1

    mCube = std::make_shared<Visual>(vbuffer, ibuffer);
    mCube->localTransform.SetTranslation(-0.5f, -0.5f, -0.5f);
    UpdateFaces();

    // Load the water texture and modify the alpha channel to 0.5 for some
    // transparency.
    auto texture = WICFileIO::Load(mEnvironment.GetPath("Water.png"), false);
    uint32_t numTexels = texture->GetNumElements();
    auto texels = texture->Get<uint32_t>();
    for (uint32_t i = 0; i < numTexels; ++i)
    {
        texels[i] = (texels[i] & 0x00FFFFFF) | 0x80000000;
    }

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mCube->SetEffect(effect);
    mPVWMatrices.Subscribe(mCube->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mCube);
}

void GelatinCubeWindow3::CreateSprings()
{
    // The inner 4-by-4-by-4 particles are used as the control points of a
    // B-spline volume.  The outer layer of particles are immovable to
    // prevent the cuboid from collapsing into itself.
    int32_t const numSlices = 6, numRows = 6, numCols = 6;

    // Viscous forces applied.  If you set viscosity to zero, the cuboid
    // wiggles indefinitely since there is no dissipation of energy.  If
    // the viscosity is set to a positive value, the oscillations eventually
    // stop.  The length of time to steady state is inversely proportional
    // to the viscosity.
    float const step = 0.001f;
    float const viscosity = 0.01f;
    mModule = std::make_unique<PhysicsModule>(numSlices, numRows, numCols, step, viscosity);

    // The initial cuboid is axis-aligned.  The outer shell is immovable.
    // All other masses are constant.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-0.1f, 0.1f);
    float constexpr fmax = std::numeric_limits<float>::max();
    float sFactor = 1.0f / static_cast<float>(numSlices - 1);
    float rFactor = 1.0f / static_cast<float>(numRows - 1);
    float cFactor = 1.0f / static_cast<float>(numCols - 1);
    for (int32_t s = 0; s < numSlices; ++s)
    {
        for (int32_t r = 0; r < numRows; ++r)
        {
            for (int32_t c = 0; c < numCols; ++c)
            {
                mModule->SetPosition(s, r, c, { c * cFactor, r * rFactor, s * sFactor });

                if (1 <= s && s < numSlices - 1
                    && 1 <= r && r < numRows - 1
                    && 1 <= c && c < numCols - 1)
                {
                    mModule->SetMass(s, r, c, 1.0f);
                    mModule->SetVelocity(s, r, c, { rnd(mte), rnd(mte), rnd(mte) });
                }
                else
                {
                    mModule->SetMass(s, r, c, fmax);
                    mModule->SetVelocity(s, r, c, { 0.0f, 0.0f, 0.0f });
                }
            }
        }
    }

    // Springs are at rest in the initial configuration.
    float const constant = 10.0f;

    for (int32_t s = 0; s < numSlices - 1; ++s)
    {
        for (int32_t r = 0; r < numRows; ++r)
        {
            for (int32_t c = 0; c < numCols; ++c)
            {
                mModule->SetConstantS(s, r, c, constant);
                mModule->SetLengthS(s, r, c, Length(mModule->GetPosition(s + 1, r, c) -
                    mModule->GetPosition(s, r, c)));
            }
        }
    }

    for (int32_t s = 0; s < numSlices; ++s)
    {
        for (int32_t r = 0; r < numRows - 1; ++r)
        {
            for (int32_t c = 0; c < numCols; ++c)
            {
                mModule->SetConstantR(s, r, c, constant);
                mModule->SetLengthR(s, r, c, Length(mModule->GetPosition(s, r + 1, c) -
                    mModule->GetPosition(s, r, c)));
            }
        }
    }

    for (int32_t s = 0; s < numSlices; ++s)
    {
        for (int32_t r = 0; r < numRows; ++r)
        {
            for (int32_t c = 0; c < numCols - 1; ++c)
            {
                mModule->SetConstantC(s, r, c, constant);
                mModule->SetLengthC(s, r, c, Length(mModule->GetPosition(s, r, c + 1) -
                    mModule->GetPosition(s, r, c)));
            }
        }
    }
}

void GelatinCubeWindow3::PhysicsTick()
{
    mModule->Update(static_cast<float>(mMotionTimer.GetSeconds()));

    // Update spline surface.  Remember that the spline maintains its own
    // copy of the control points, so this update is necessary.
    int32_t const numSlices = mModule->GetNumSlices() - 2;
    int32_t const numRows = mModule->GetNumRows() - 2;
    int32_t const numCols = mModule->GetNumCols() - 2;
    for (int32_t s = 0; s < numSlices; ++s)
    {
        for (int32_t r = 0; r < numRows; ++r)
        {
            for (int32_t c = 0; c < numCols; ++c)
            {
                mVolume->SetControl(c, r, s, mModule->GetPosition(s + 1, r + 1, c + 1));
            }
        }
    }

    UpdateFaces();
    mEngine->Update(mCube->GetVertexBuffer());
}

void GelatinCubeWindow3::GraphicsTick()
{
    mEngine->ClearBuffers();

    auto const& previousBlendState = mEngine->GetBlendState();
    mEngine->SetBlendState(mBlendState);
    mEngine->SetDepthStencilState(mDepthReadNoWriteState);
    mEngine->Draw(mCube);
    mEngine->SetDefaultDepthStencilState();
    mEngine->SetBlendState(previousBlendState);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);
}

void GelatinCubeWindow3::CreateFaceVertices(uint32_t numRows, uint32_t numCols,
    float faceValue, uint32_t const permute[3], Vertex* vertices,
    uint32_t& index)
{
    std::array<float, 3> param{};
    param[permute[2]] = faceValue;
    float rowFactor = 1.0f / static_cast<float>(numRows - 1);
    float colFactor = 1.0f / static_cast<float>(numCols - 1);
    std::array<Vector3<float>, 10> values{};
    for (uint32_t row = 0; row < numRows; ++row)
    {
        param[permute[1]] = row * rowFactor;
        for (uint32_t col = 0; col < numCols; ++col, ++index)
        {
            param[permute[0]] = col * colFactor;
            mVolume->Evaluate(param[0], param[1], param[2], 0, values.data());
            vertices[index].position = values[0];
            vertices[index].tcoord = { param[permute[0]], param[permute[1]] };
        }
    }
}

void GelatinCubeWindow3::CreateFaceIndices(uint32_t numRows, uint32_t numCols,
    bool ccw, uint32_t& vBase, uint32_t*& indices)
{
    for (uint32_t row = 0, i = vBase; row + 1 < numRows; ++row)
    {
        uint32_t i0 = i;
        uint32_t i1 = i0 + 1;
        i += numCols;
        uint32_t i2 = i;
        uint32_t i3 = i2 + 1;
        for (uint32_t col = 0; col + 1 < numCols; ++col, indices += 6)
        {
            if (ccw)
            {
                indices[0] = i0;
                indices[1] = i1;
                indices[2] = i2;
                indices[3] = i1;
                indices[4] = i3;
                indices[5] = i2;
            }
            else
            {
                indices[0] = i0;
                indices[1] = i2;
                indices[2] = i1;
                indices[3] = i1;
                indices[4] = i2;
                indices[5] = i3;
            }
            ++i0;
            ++i1;
            ++i2;
            ++i3;
        }
    }

    vBase += numRows * numCols;
}

void GelatinCubeWindow3::UpdateFaces()
{
    std::array<uint32_t, 3> permute{};
    uint32_t index = 0;
    auto vertices = mCube->GetVertexBuffer()->Get<Vertex>();

    // u faces (u = 0, u = 1)
    permute[0] = 1;
    permute[1] = 2;
    permute[2] = 0;
    CreateFaceVertices(mNumWSamples, mNumVSamples, 0.0f, permute.data(), vertices, index);
    CreateFaceVertices(mNumWSamples, mNumVSamples, 1.0f, permute.data(), vertices, index);

    // v faces (v = 0, v = 1)
    permute[0] = 0;
    permute[1] = 2;
    permute[2] = 1;
    CreateFaceVertices(mNumWSamples, mNumUSamples, 0.0f, permute.data(), vertices, index);
    CreateFaceVertices(mNumWSamples, mNumUSamples, 1.0f, permute.data(), vertices, index);

    // w faces (w = 0, w = 1)
    permute[0] = 0;
    permute[1] = 1;
    permute[2] = 2;
    CreateFaceVertices(mNumVSamples, mNumUSamples, 0.0f, permute.data(), vertices, index);
    CreateFaceVertices(mNumVSamples, mNumUSamples, 1.0f, permute.data(), vertices, index);
}
