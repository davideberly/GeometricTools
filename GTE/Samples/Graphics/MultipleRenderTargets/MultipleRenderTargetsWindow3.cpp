// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.12.14

#include "MultipleRenderTargetsWindow3.h"
#include <Applications/WICFileIO.h>

// The readback of depth data is done in a graphics-API-dependent manner.
// TODO: Hide these details via an interface to avoid the conditional
// compilation.
#if defined(GTE_USE_DIRECTX)
#include <Graphics/DX11/DX11TextureDS.h>
#endif

MultipleRenderTargetsWindow3::MultipleRenderTargetsWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mActiveOverlay(0)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 10.0f, 0.001f, 0.001f,
        { 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();

    CreateOverlays();
}

void MultipleRenderTargetsWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    // Render the square to offscreen memory.
    mEngine->Enable(mDrawTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);
    mEngine->Disable(mDrawTarget);

    // PSMain of MultipleRenderTarget.hlsl has written linearized depth to the
    // depth buffer.  It is not possible to attach a depth-stencil texture as
    // a shader input; you cannot create a shader resource view for it.  The
    // best you can do is read it back from the GPU and copy it to a texture
    // that is a shader input.  NOTE: If you really want to use depth as a
    // shader input, pass the 'perspectiveDepth' to the pixel shader as is
    // done in MultipleRenderTarget.hlsl and write it to a render target, not
    // to the depth-stencil texture.  You can then attach that render target
    // as a shader input.  This avoids the expensive read-back-and-copy step
    // here.
    std::shared_ptr<TextureDS> dsTexture = mDrawTarget->GetDSTexture();

    // Simple code for reading the depth texture from GPU to CPU, repackaging
    // it as a regular texture, and uploading from CPU to GPU is as follows.
    // On the AMD 7970, the initial display runs at 720 fps for a 512x512
    // application window.
    //
    // mEngine->CopyGpuToCpu(dsTexture);
    // std::memcpy(mLinearDepth->GetData(), dsTexture->GetData(),
    //     dsTexture->GetNumBytes());
    // mEngine->CopyCpuToGpu(mLinearDepth);
    //
    // This block of code does
    //   gpu -> srcStaging -> cpu(lineardepth) -> dstStaging -> gpu
    // The block of code below skips the cpu memory,
    //   gpu -> srcStaging -> dstStaging -> gpu
    //
    // We currently do not have wrappers CopyGpuToStaging, CopyStagingToGpu,
    // or CopyStagingToStaging.  CopyGpuToGpu can use CopySubresourceRegion
    // or CopyRegion as long as the resources are compatible.  But in the
    // situation here, the depth format DF_D24_UNORM_S8_UINT is not compatible
    // with DF_R32_UINT according to the error messages produced by the D3D11
    // debug layer when calling CopyResource, even though the textures are the
    // dimensions and have the same number of bytes.  In fact, CopyResource
    // does not have a return value that indicates the failure to copy, so
    // how does one trap the error?  On the AMD 7970, the initial display
    // runs at 840 fps for a 512x512 application window.
#if defined(GTE_USE_OPENGL)
    mEngine->CopyGpuToCpu(dsTexture);
    std::memcpy(mLinearDepth->GetData(), dsTexture->GetData(), dsTexture->GetNumBytes());
    mEngine->CopyCpuToGpu(mLinearDepth);
#endif
#if defined(GTE_USE_DIRECTX)
    DX11Engine* engine = static_cast<DX11Engine*>(mEngine.get());
    ID3D11DeviceContext* context = engine->GetImmediate();
    DX11TextureDS* srcTexture = (DX11TextureDS*)engine->Bind(dsTexture);
    ID3D11Resource* srcResource = srcTexture->GetDXResource();
    ID3D11Resource* srcStaging = srcTexture->GetStagingResource();
    DX11Texture2* dstTexture = (DX11Texture2*)engine->Bind(mLinearDepth);
    ID3D11Resource* dstResource = dstTexture->GetDXResource();
    ID3D11Resource* dstStaging = dstTexture->GetStagingResource();
    context->CopySubresourceRegion(srcStaging, 0, 0, 0, 0, srcResource, 0, nullptr);
    D3D11_MAPPED_SUBRESOURCE srcSub, dstSub;
    context->Map(srcStaging, 0, D3D11_MAP_READ, 0, &srcSub);
    context->Map(dstStaging, 0, D3D11_MAP_WRITE, 0, &dstSub);
    std::memcpy(dstSub.pData, srcSub.pData, dsTexture->GetNumBytes());
    context->Unmap(srcStaging, 0);
    context->Unmap(dstStaging, 0);
    context->CopySubresourceRegion(dstResource, 0, 0, 0, 0, dstStaging, 0, nullptr);
#endif

    if (mActiveOverlay == 0)
    {
        // mSquare was rendered to the render target attached to mOverlay[0].
        // Draw the overlay, which contains the stone-textured 3D rendering.
        mEngine->Draw(mOverlay[0]);
    }
    else if (1 <= mActiveOverlay && mActiveOverlay <= 4)
    {
        mEngine->Draw(mOverlay[mActiveOverlay]);
    }
    else if (mActiveOverlay == 5)
    {
        // The output depth for the rendering of mSquare is set to linearized
        // depth, not the default perspective depth.  The depth texture of the
        // draw target is of the form 0xSSDDDDDD, which means the high-order 8
        // bits are stencil values and the low-order 24 bits are depth values.
        // The depth texture was read from GPU to CPU as a 32-bit integer and
        // copied to mLinearDepth, a regular 2D texture with format R32_INT.
        // This texture is attached as an input to the gsOverlay1PShader pixel
        // shader and used as the pixel color output, which is the texture
        // attached to mOverlay[1].  The linearized depth as R32_INT has
        // values between 0 and 0xFFFFFF (16777215) but is normalized to
        // [0,1] on output.  The background is white because the depth buffer
        // was cleared to 1.0f, causing linearized depth to be 0xFFFFFF, and
        // normalized output to be 1.0f.  Rotate the square so it is not
        // parallel to the view direction and  move the camera backward
        // (press down arrow) so that the square is clipped by the far plane.
        // You will see the linearized depth become gray-to-white close to the
        // far plane, indicating the depth is varying from 0.0f (close to
        // near) to 1.0f (close to far).
        mEngine->Draw(mOverlay[5]);
    }
    else // mActiveOverlay == 6
    {
        // mOverlay[1] causes the gsOverlay1PShader shader to be executed,
        // which leads to writing the UAV colorTexture that is attached to
        // mOverlay[2].  We then draw that color texture using mOverlay[2].
        // This verifies that indeed the UAV can be written by a pixel
        // shader.
        mEngine->Draw(mOverlay[5]);
        mEngine->Draw(mOverlay[6]);
    }

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MultipleRenderTargetsWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '0':
        // Display mSquare with the stone texture.
        mActiveOverlay = 0;
        return true;
    case '1':
        // Display miplevel 1 of the color output of rendering the square.
        mActiveOverlay = 1;
        return true;
    case '2':
        // Display miplevel 2 of the color output of rendering the square.
        mActiveOverlay = 2;
        return true;
    case '3':
        // Display miplevel 3 of the color output of rendering the square.
        mActiveOverlay = 3;
        return true;
    case '4':
        // Display miplevel 4 of the color output of rendering the square.
        mActiveOverlay = 4;
        return true;
    case '5':
        // Display mSquare with the linearized depth, shown as a monochrome
        // image (depth stored in r, g, and b).
        mActiveOverlay = 5;
        return true;
    case '6':
        // Display the colorTexture UAV that is written in the pixel shader
        // that is defined at the beginning of this file.
        mActiveOverlay = 6;
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool MultipleRenderTargetsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Graphics/MultipleRenderTargets/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("MultipleRenderTargets.vs"),
        mEngine->GetShaderName("MultipleRenderTargets.ps"),
        "StoneWall.png"
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

bool MultipleRenderTargetsWindow3::CreateScene()
{
    // Create a visual effect that populates the draw target.
    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("MultipleRenderTargets.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("MultipleRenderTargets.ps"));
    auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!program)
    {
        return false;
    }

    auto cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVertexShader()->Set("PVWMatrix", cbuffer);

    auto const& pshader = program->GetPixelShader();
    auto farNearRatio = std::make_shared<ConstantBuffer>(sizeof(float), false);
    pshader->Set("FarNearRatio", farNearRatio);
    farNearRatio->SetMember("farNearRatio", mCamera->GetDMax() / mCamera->GetDMin());

    std::string path = mEnvironment.GetPath("StoneWall.png");
    auto baseTexture = WICFileIO::Load(path, true);
    baseTexture->AutogenerateMipmaps();
    auto baseSampler = std::make_shared<SamplerState>();
    baseSampler->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_L;
    baseSampler->mode[0] = SamplerState::Mode::CLAMP;
    baseSampler->mode[1] = SamplerState::Mode::CLAMP;
    pshader->Set("baseTexture", baseTexture, "baseSampler", baseSampler);

    auto effect = std::make_shared<VisualEffect>(program);

    // Create a vertex buffer for a two-triangle square.  The PNG is stored
    // in left-handed coordinates.  The texture coordinates are chosen to
    // reflect the texture in the y-direction.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    Vertex* vertex = vbuffer->Get<Vertex>();
#if defined(GTE_USE_OPENGL)
    vertex[1].position = { -1.0f, -1.0f, 0.0f };
    vertex[1].tcoord = { 0.0f, 1.0f };
    vertex[0].position = { 1.0f, -1.0f, 0.0f };
    vertex[0].tcoord = { 1.0f, 1.0f };
    vertex[3].position = { -1.0f, 1.0f, 0.0f };
    vertex[3].tcoord = { 0.0f, 0.0f };
    vertex[2].position = { 1.0f, 1.0f, 0.0f };
    vertex[2].tcoord = { 1.0f, 0.0f };
#else
    vertex[0].position = { -1.0f, -1.0f, 0.0f };
    vertex[0].tcoord = { 0.0f, 1.0f };
    vertex[1].position = { 1.0f, -1.0f, 0.0f };
    vertex[1].tcoord = { 1.0f, 1.0f };
    vertex[2].position = { -1.0f, 1.0f, 0.0f };
    vertex[2].tcoord = { 0.0f, 0.0f };
    vertex[3].position = { 1.0f, 1.0f, 0.0f };
    vertex[3].tcoord = { 1.0f, 0.0f };
#endif

    // Create an indexless buffer for a triangle mesh with two triangles.
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRISTRIP, 2);

    // Create the geometric object for drawing and enable automatic updates
    // of pvw-matrices and w-matrices.
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mTrackBall.Attach(mSquare);
    mTrackBall.Update();
    mPVWMatrices.Subscribe(mSquare->worldTransform, cbuffer);
    return true;
}

void MultipleRenderTargetsWindow3::CreateOverlays()
{
    // Create the draw target with 2 render targets and 1 depth-stencil
    // texture.  Each of these is used as a texture for an overlay.
    mDrawTarget = std::make_shared<DrawTarget>(2, DF_R32G32B32A32_FLOAT,
        mXSize, mYSize, true, true, DF_D32_FLOAT, true);
    mDrawTarget->AutogenerateRTMipmaps();
    mDrawTarget->GetRTTexture(0)->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mDrawTarget->GetDSTexture()->SetCopy(Resource::Copy::STAGING_TO_CPU);
    mEngine->Bind(mDrawTarget);

    // Display mSquare that was rendered to a draw target with mipmaps
    // enabled.  The depth texture output is linearized depth, not perspective
    // depth.  The mipmap selection is the standard algorithm used in the
    // HLSL Texture2D.Sample function.
    mOverlay[0] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::Filter::MIN_L_MAG_L_MIP_L,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, true);
    mOverlay[0]->SetTexture(mDrawTarget->GetRTTexture(0));

    // Display mSquare using only miplevel i and using nearest-neighbor
    // sampling.
    int32_t api = mProgramFactory->GetAPI();
    std::shared_ptr<Shader> pshader;
    auto nearestSampler = std::make_shared<SamplerState>();
    nearestSampler->filter = SamplerState::Filter::MIN_P_MAG_P_MIP_P;
    nearestSampler->mode[0] = SamplerState::Mode::CLAMP;
    nearestSampler->mode[1] = SamplerState::Mode::CLAMP;
    for (int32_t i = 1; i < 5; ++i)
    {
        mOverlay[i] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
            mYSize, mXSize, mYSize, *msOverlayPSSource[i][api]);
        pshader = mOverlay[i]->GetProgram()->GetPixelShader();
        pshader->Set("inTexture", mDrawTarget->GetRTTexture(0), "inSampler", nearestSampler);
    }

    // Display mSquare using linearized depth.
    mLinearDepth = std::make_shared<Texture2>(DF_R32_FLOAT, mXSize, mYSize);
    mLinearDepth->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mLinearDepth->SetCopy(Resource::Copy::CPU_TO_STAGING);
    mOverlay[5] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, *msOverlayPSSource[0][api]);
    std::shared_ptr<SamplerState> linearSampler = std::make_shared<SamplerState>();
    linearSampler->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_L;
    linearSampler->mode[0] = SamplerState::Mode::CLAMP;
    linearSampler->mode[1] = SamplerState::Mode::CLAMP;
    pshader = mOverlay[5]->GetProgram()->GetPixelShader();
    pshader->Set("positionTexture", mDrawTarget->GetRTTexture(1), "positionSampler", linearSampler);
    pshader->Set("depthTexture", mLinearDepth);
    pshader->Set("colorTexture", mDrawTarget->GetRTTexture(0));

    // Display the UAV color texture that is written by mOverlay[5].
    mOverlay[6] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::Filter::MIN_L_MAG_L_MIP_L,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, true);
    mOverlay[6]->SetTexture(mDrawTarget->GetRTTexture(0));
}


std::string const MultipleRenderTargetsWindow3::msGLSLOverlayPSSource[5] =
{
    "layout (r32f) uniform readonly image2D depthTexture;\n"
    "layout (rgba32f) uniform writeonly image2D colorTexture;\n"
    "uniform sampler2D positionSampler;\n"
    "\n"
    "layout(location = 0) in vec2 vertexTCoord;\n"
    "layout(location = 0) out vec4 pixelColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec4 pos = texture(positionSampler, vertexTCoord);\n"
    "    float depth = imageLoad(depthTexture, ivec2(pos.xy)).x;\n"
    "    pixelColor = vec4(depth, depth, depth, 1.0f);\n"
    "    imageStore(colorTexture, ivec2(pos.xy), vec4(0.4f, 0.5f, 0.6f, 1.0f));\n"
    "}\n",

    "uniform sampler2D inSampler;\n"
    "\n"
    "layout(location = 0) in vec2 vertexTCoord;\n"
    "layout(location = 0) out vec4 color;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    color = textureLod(inSampler, vertexTCoord, 1.0f);\n"
    "}\n",

    "uniform sampler2D inSampler;\n"
    "\n"
    "layout(location = 0) in vec2 vertexTCoord;\n"
    "layout(location = 0) out vec4 color;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    color = textureLod(inSampler, vertexTCoord, 2.0f);\n"
    "}\n",

    "uniform sampler2D inSampler;\n"
    "\n"
    "layout(location = 0) in vec2 vertexTCoord;\n"
    "layout(location = 0) out vec4 color;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    color = textureLod(inSampler, vertexTCoord, 3.0f);\n"
    "}\n",

    "uniform sampler2D inSampler;\n"
    "\n"
    "layout(location = 0) in vec2 vertexTCoord;\n"
    "layout(location = 0) out vec4 color;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    color = textureLod(inSampler, vertexTCoord, 4.0f);\n"
    "}\n"
};

std::string const MultipleRenderTargetsWindow3::msHLSLOverlayPSSource[5] =
{
    "Texture2D<float> depthTexture;\n"
    "Texture2D<float4> positionTexture;\n"
    "SamplerState positionSampler;\n"
    "RWTexture2D<float4> colorTexture;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 pixelColor : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    float4 pos = positionTexture.Sample(positionSampler, input.vertexTCoord);\n"
    "    float depth = depthTexture[(int2)pos.xy];\n"
    "    output.pixelColor = float4(depth, depth, depth, 1.0f);\n"
    "    colorTexture[(int2)pos.xy] = float4(0.4f, 0.5f, 0.6f, 1.0f);\n"
    "    return output;\n"
    "}\n",

    "Texture2D<float4> inTexture;\n"
    "SamplerState inSampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 color : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    output.color = inTexture.SampleLevel(inSampler, input.vertexTCoord, 1.0f);\n"
    "    return output;\n"
    "}\n",

    "Texture2D<float4> inTexture;\n"
    "SamplerState inSampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 color : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    output.color = inTexture.SampleLevel(inSampler, input.vertexTCoord, 2.0f);\n"
    "    return output;\n"
    "}\n",

    "Texture2D<float4> inTexture;\n"
    "SamplerState inSampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 color : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    output.color = inTexture.SampleLevel(inSampler, input.vertexTCoord, 3.0f);\n"
    "    return output;\n"
    "}\n",

    "Texture2D<float4> inTexture;\n"
    "SamplerState inSampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 color : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    output.color = inTexture.SampleLevel(inSampler, input.vertexTCoord, 4.0f);\n"
    "    return output;\n"
    "}\n"
};

ProgramSources const MultipleRenderTargetsWindow3::msOverlayPSSource[5] =
{
    {
        &msGLSLOverlayPSSource[0],
        &msHLSLOverlayPSSource[0]
    },
    {
        &msGLSLOverlayPSSource[1],
        &msHLSLOverlayPSSource[1]
    },
    {
        &msGLSLOverlayPSSource[2],
        &msHLSLOverlayPSSource[2]
    },
    {
        &msGLSLOverlayPSSource[3],
        &msHLSLOverlayPSSource[3]
    },
    {
        &msGLSLOverlayPSSource[4],
        & msHLSLOverlayPSSource[4]
    }
};
