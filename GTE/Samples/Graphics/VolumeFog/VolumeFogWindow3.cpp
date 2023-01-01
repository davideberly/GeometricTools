// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.22

#include "VolumeFogWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/VolumeFogEffect.h>
#include <random>

VolumeFogWindow3::VolumeFogWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoDepthStencilState = std::make_shared<DepthStencilState>();
    mNoDepthStencilState->depthEnable = false;
    mNoDepthStencilState->stencilEnable = false;

    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
        { 0.0f, -9.0f, 1.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
}

void VolumeFogWindow3::OnIdle()
{
    mTimer.Measure();

    bool fogNeedsUpdate = mCameraRig.Move();
    if (fogNeedsUpdate)
    {
        mPVWMatrices.Update();
        UpdateFog();
    }

    mEngine->ClearBuffers();

    mEngine->SetDepthStencilState(mNoDepthStencilState);
    mEngine->Draw(mOverlay);
    mEngine->SetDefaultDepthStencilState();

    mEngine->Draw(mMesh);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool VolumeFogWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "BlueSky.png",
        "HeightField.png"
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

void VolumeFogWindow3::CreateScene()
{
    CreateBackground();
    CreateMesh();
}

void VolumeFogWindow3::CreateBackground()
{
    std::string path = mEnvironment.GetPath("BlueSky.png");
    auto skyTexture = WICFileIO::Load(path, false);
    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize, mYSize,
        mXSize, mYSize, SamplerState::Filter::MIN_P_MAG_P_MIP_P,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, true);
    mOverlay->SetTexture(skyTexture);
}

void VolumeFogWindow3::CreateMesh()
{
    std::default_random_engine dre{};
    std::uniform_real_distribution<float> urd(0.0f, 1.0f);

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf{};
    mf.SetVertexFormat(vformat);

    mMesh = mf.CreateRectangle(64, 64, 8.0f, 8.0f);
    auto const& vbuffer = mMesh->GetVertexBuffer();
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    auto* vertices = vbuffer->Get<Vertex>();
    uint32_t numVertices = vbuffer->GetNumElements();

    // Set the heights based on a precomputed height field. Also create a
    // texture image to go with the height field.
    std::string path = mEnvironment.GetPath("HeightField.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto* texels = texture->Get<uint8_t>();
    Vector4<float> white{ 1.0f, 1.0f, 1.0f, 0.0f };
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        uint8_t value = *texels;
        float height = 3.0f * value / 255.0f + 0.05f * (2.0f * urd(dre) - 1.0f);
        vertices[i].position[2] = height;

        // The fog color is white. The alpha channel is fillled in by the
        // UpdateFog() function.
        vertices[i].color = white;

        // The texture has blends of red and green.
        *texels++ = static_cast<uint8_t>(32.0f * (urd(dre) + 1.0f));
        *texels++ = 3 * (128 - value / 2) / 4;
        *texels++ = 0;
        *texels++ = 255;
    }

    auto effect = std::make_shared<VolumeFogEffect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP);
    mMesh->SetEffect(effect);

    mPVWMatrices.Subscribe(mMesh);
    mTrackBall.Attach(mMesh);
    mTrackBall.Update();
    mPVWMatrices.Update();

    UpdateFog();
}

void VolumeFogWindow3::UpdateFog()
{
    // Compute the fog factors based on the intersection of a slab bounded
    // by z = zBot and z = zTop with rays emanating from the camera location
    // to each vertex in the height field.
    float constexpr zBot = 0.0f;  // slab bottom
    float constexpr zTop = 0.5f;  // slab top
    float constexpr fogConstant = 8.0f; // fog = length/(length+fogconstant)
    float tBot{}, tTop{}, invDirZ{}, length{};

    // Having the camera below the height field is not natural, so we just
    // leave the fog values the way they are.
    auto const& camPosition = mCamera->GetPosition();
    if (camPosition[2] <= zBot)
    {
        return;
    }

    auto const& vbuffer = mMesh->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vertex>();
    uint32_t numVertices = vbuffer->GetNumElements();
    float constexpr tolerance = 1e-06f;
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        // The ray is E+t*D, where D = V-E with E the eye point and V the
        // vertex. The ray reaches the vertex at t = 1.
        Vector4<float> heightPosition = HLift(vertices[i].position, 1.0f);

        Vector4<float> direction = heightPosition - camPosition;
        float tVmE = Normalize(direction);
        float alpha{};

        if (camPosition[2] >= zTop)
        {
            if (direction[2] >= -tolerance)
            {
                // The ray never intersects the slab, so no fog at vertex.
                alpha = 0.0f;
            }
            else
            {
                // The ray intersects the slab. Compute the length of the
                // intersection and map it to a fog factor.
                invDirZ = 1.0f / direction[2];
                tTop = (zTop - camPosition[2]) * invDirZ;
                if (tTop < tVmE)
                {
                    // The eye is above the slab; the vertex is in or below.
                    tBot = (zBot - camPosition[2]) * invDirZ;
                    if (tBot < tVmE)
                    {
                        // The vertex is below the slab. Clamp to bottom.
                        length = tBot - tTop;
                        alpha = length / (length + fogConstant);
                    }
                    else
                    {
                        // The vertex is inside the slab.
                        length = tVmE - tTop;
                        alpha = length / (length + fogConstant);
                    }
                }
                else
                {
                    // The eye and vertex are above the slab, so no fog.
                    alpha = 0.0f;
                }
            }
        }
        else  // position[2] in (z0,z1)
        {
            if (direction[2] >= tolerance)
            {
                // The ray intersects the top of the slab.
                invDirZ = 1.0f / direction[2];
                tTop = (zTop - camPosition[2]) * invDirZ;
                if (tTop < tVmE)
                {
                    // The vertex is above the slab.
                    alpha = tTop / (tTop + fogConstant);
                }
                else
                {
                    // The vertex is on or inside the slab.
                    alpha = tVmE / (tVmE + fogConstant);
                }
            }
            else if (direction[2] <= -tolerance)
            {
                // The ray intersects the bottom of the slab.
                invDirZ = 1.0f / direction[2];
                tBot = (zBot - camPosition[2]) * invDirZ;
                if (tBot < tVmE)
                {
                    // The vertex is below the slab.
                    alpha = tBot / (tBot + fogConstant);
                }
                else
                {
                    // The vertex is on or inside the slab.
                    alpha = tVmE / (tVmE + fogConstant);
                }
            }
            else
            {
                // The ray is parallel to the slab, so both eye and vertex
                // are inside the slab.
                alpha = tVmE / (tVmE + fogConstant);
            }
        }

        vertices[i].color[3] = alpha;
    }

    mEngine->Update(vbuffer);
}
