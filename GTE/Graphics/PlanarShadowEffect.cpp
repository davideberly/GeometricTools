// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/PlanarShadowEffect.h>
using namespace gte;

PlanarShadowEffect::PlanarShadowEffect(
    std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Node> const& shadowCaster,
    std::shared_ptr<LightProjector> const& lightProjector,
    std::vector<std::shared_ptr<Visual>> const& planeVisuals,
    std::vector<Vector4<float>> const& shadowColors)
    :
    mShadowCaster(shadowCaster),
    mLightProjector(lightProjector),
    mPlaneVisuals(planeVisuals),
    mShadowColors(shadowColors),
    mCasterVisuals{},
    mCasterEffects{},
    mSaveVisualEffects{},
    mModelSpaceTriangles{planeVisuals.size()},
    mShadowBlend{},
    mDSPass0{},
    mDSPass1{},
    mAPI(factory->GetAPI())
{
    // Recursively traverse the shadow caster hierarchy and gather all the
    // Visual objects. Each Visual requires a constant-color effect for
    // drawing the shadow it casts. The model-space triangles are also
    // computed during the traversal.
    GatherVisuals(factory, mShadowCaster);
    mSaveVisualEffects.resize(mCasterEffects.size());

    // Verify the planeVisuals satisfy the constraints for the POSITION
    // semantic. Package the first triangle of vertices into the model-space
    // storage.
    GetModelSpaceTriangles();

    mShadowBlend = std::make_shared<BlendState>();
    mShadowBlend->target[0].enable = true;
    mShadowBlend->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mShadowBlend->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mShadowBlend->target[0].opColor = BlendState::Operation::ADD;
    mShadowBlend->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mShadowBlend->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;
    mShadowBlend->target[0].opAlpha = BlendState::Operation::ADD;
    mShadowBlend->target[0].mask = BlendState::ColorWrite::ENABLE_ALL;

    // Enable depth reads and writes. The stencil face.fail value is
    // irrelevant. The face.depthFail value is KEEP so that invisible pixels
    // have stencil value 0. The face.pass value is REPLACE so that visible
    // pixels have stencil value i+1 for plane i. The stencil comparison value
    // is ALWAYS to that stencil writes always occur. The stencil reference
    // i+1 is set on demand for each plane.
    mDSPass0 = std::make_shared<DepthStencilState>();
    mDSPass0->depthEnable = true;
    mDSPass0->writeMask = DepthStencilState::WriteMask::ALL;
    mDSPass0->comparison = DepthStencilState::Comparison::LESS_EQUAL;
    mDSPass0->stencilEnable = true;
    mDSPass0->stencilReadMask = 0xFF;
    mDSPass0->stencilWriteMask = 0xFF;
    mDSPass0->frontFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass0->frontFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass0->frontFace.pass = DepthStencilState::Operation::OP_REPLACE;
    mDSPass0->frontFace.comparison = DepthStencilState::Comparison::ALWAYS;
    mDSPass0->backFace.fail =  DepthStencilState::Operation::OP_KEEP;
    mDSPass0->backFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass0->backFace.pass = DepthStencilState::Operation::OP_REPLACE;
    mDSPass0->backFace.comparison = DepthStencilState::Comparison::ALWAYS;

    // Disable depth reads and writes. The stencil face.fail value is
    // KEEP so that invisible pixels retain stencil value 0. The
    // face.depthFail value is irrelevant. The face.pass value is ZERO
    // so that visible pixels have stencil value reset to 0. The stencil
    // comparison is EQUAL so that only stencil values i+1 from dspass0
    // are processed during dspass1.
    mDSPass1 = std::make_shared<DepthStencilState>();
    mDSPass1->depthEnable = false;
    mDSPass1->writeMask = DepthStencilState::WriteMask::ALL;
    mDSPass1->comparison = DepthStencilState::Comparison::LESS_EQUAL;
    mDSPass1->stencilEnable = true;
    mDSPass1->stencilReadMask = 0xFF;
    mDSPass1->stencilWriteMask = 0xFF;
    mDSPass1->frontFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->frontFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->frontFace.pass = DepthStencilState::Operation::OP_ZERO;
    mDSPass1->frontFace.comparison = DepthStencilState::Comparison::EQUAL;
    mDSPass1->backFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->backFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->backFace.pass = DepthStencilState::Operation::OP_ZERO;
    mDSPass1->backFace.comparison = DepthStencilState::Comparison::EQUAL;
}

void PlanarShadowEffect::Draw(std::shared_ptr<GraphicsEngine> const& engine,
    PVWUpdater& pvwMatrices)
{
    // Save the global state, to be restored later.
    std::shared_ptr<BlendState> saveBState = engine->GetBlendState();
    std::shared_ptr<DepthStencilState> saveDSState = engine->GetDepthStencilState();

    // Get the camera to store post-world transformations.
    std::shared_ptr<Camera> camera = pvwMatrices.GetCamera();

    // Draw the the shadow caster.
    engine->Draw(mCasterVisuals);

    for (size_t i = 0; i < mPlaneVisuals.size(); ++i)
    {
        auto const& plane = mPlaneVisuals[i];
        uint32_t reference = static_cast<uint32_t>(i + 1);

        // Enable default depth state. Enable the stencil state so that the
        // shadow can be clipped by the plane. The stencil values are set
        // whenever the corresponding plane pixels are visible. No blending
        // is used in this pass.
        mDSPass0->reference = reference;
        engine->SetDepthStencilState(mDSPass0);
        engine->SetBlendState(saveBState);
        engine->Draw(plane);
    
        // Disable the depth buffer reading so that no depth-buffer fighting
        // occurs. The drawing of pixels is controlled solely by the stencil
        // value. Blend the shadow color with the pixels drawn on the
        // projection plane.
        mDSPass1->reference = reference;
        engine->SetDepthStencilState(mDSPass1);
        engine->SetBlendState(mShadowBlend);

        // Set the projection matrix relative to the projector (light).
        Matrix4x4<float> projectionMatrix{};
        if (!GetProjectionMatrix(i, projectionMatrix))
        {
            // The shadow should not be cast because the caster is on the far
            // side of the world plane.
            continue;
        }

        camera->SetPreViewMatrix(projectionMatrix);

        // Draw the caster again, but temporarily use a material effect so
        // that the shadow color is blended onto the plane. TODO: This
        // drawing pass should use a visible set relative to the projector so
        // that the objects thare are out of view of the camera (not in the
        // camera's visible set) can cast shadows.
        for (size_t j = 0; j < mCasterVisuals.size(); ++j)
        {
            // Save the currently attached visual effect.
            auto const& visual = mCasterVisuals[j];
            mSaveVisualEffects[j] = visual->GetEffect();

            // Draw the shadow using the current plane's shadow color.
            auto const& cbuffer = mCasterEffects[j]->GetColorConstant();
            auto* color = cbuffer->Get<Vector4<float>>();
            *color = mShadowColors[i];
            engine->Update(cbuffer);

            // Attach the constant color effect.
            pvwMatrices.Unsubscribe(visual);
            visual->SetEffect(mCasterEffects[j]);
            pvwMatrices.Subscribe(visual);
        }

        // Update the PVW matrices for the constant color effects.
        pvwMatrices.Update();

        // Draw the shadows.
        engine->Draw(mCasterVisuals);

        // Restore the original visual effects.
        for (size_t j = 0; j < mCasterVisuals.size(); ++j)
        {
            auto const& visual = mCasterVisuals[j];
            pvwMatrices.Unsubscribe(visual);
            visual->SetEffect(mSaveVisualEffects[j]);
            pvwMatrices.Subscribe(visual);
        }

        camera->SetPreViewMatrix(Matrix4x4<float>::Identity());

        // Update the PVW matrices for the original visual effects.
        pvwMatrices.Update();

        if (mAPI == ProgramFactory::PF_GLSL)
        {
            // Read the comments in PlanarShadowEffect.h about the GL45
            // version not properly setting the stencil reference values
            // i+1 back to 0 during the mDSPass1 drawing. With an explicit
            // (and presumably expensive) clear, the drawing is then correct.
            engine->ClearStencilBuffer();
        }
    }

    // Restore the global state that existed before this function call.
    engine->SetBlendState(saveBState);
    engine->SetDepthStencilState(saveDSState);
}

void PlanarShadowEffect::GatherVisuals(
    std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Spatial> const& spatial)
{
    auto visual = std::dynamic_pointer_cast<Visual>(spatial);
    if (visual)
    {
        Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
        auto effect = std::make_shared<ConstantColorEffect>(factory, black);
        mCasterVisuals.push_back(visual);
        mCasterEffects.push_back(effect);
        return;
    }

    auto node = std::dynamic_pointer_cast<Node>(spatial);
    if (node)
    {
        for (int32_t i = 0; i < node->GetNumChildren(); ++i)
        {
            auto const& child = node->GetChild(i);
            if (child)
            {
                GatherVisuals(factory, child);
            }
        }
    }
}

void PlanarShadowEffect::GetModelSpaceTriangles()
{
    for (size_t i = 0; i < mPlaneVisuals.size(); ++i)
    {
        auto const& visual = mPlaneVisuals[i];
        auto const& vbuffer = visual->GetVertexBuffer();
        auto const& vformat = vbuffer->GetFormat();

        // Verify the vertex format satisfies the constraints.
        int32_t index = vformat.GetIndex(VASemantic::POSITION, 0);
        LogAssert(
            index >= 0,
            "The POSITION semantic must occur with unit 0.");

        DFType posType = vformat.GetType(index);
        LogAssert(
            posType == DF_R32G32B32_FLOAT || posType == DF_R32G32B32A32_FLOAT,
            "The POSITION must be 3-tuple or 4-tuple float-valued.");

        uint32_t offset = vformat.GetOffset(index);
        LogAssert(
            offset == 0,
            "The POSITION must occur first in the vertex format.");

        auto const& ibuffer = visual->GetIndexBuffer();
        IPType primitiveType = ibuffer->GetPrimitiveType();
        LogAssert(
            primitiveType == IPType::IP_TRIMESH,
            "The visual must have TRIMESH topology (for now).");

        // Get the first triangle's vertex indices. Get the model-space
        // vertices from the vertex buffer.
        std::array<uint32_t, 3> v{};
        ibuffer->GetTriangle(0, v[0], v[1], v[2]);
        char const* rawData = vbuffer->GetData();
        size_t const stride = static_cast<size_t>(vformat.GetVertexSize());
        auto& triangle = mModelSpaceTriangles[i];
        for (size_t j = 0; j < 3; ++j)
        {
            auto vertices = reinterpret_cast<Vector3<float> const*>(
                rawData + v[j] * stride);
            triangle[j] = HLift(*vertices, 1.0f);
        }

        // The planar shadow effect is responsible for drawing the planes.
        visual->culling = CullingMode::ALWAYS;
    }
}

bool PlanarShadowEffect::GetProjectionMatrix(size_t i, Matrix4x4<float>& projection)
{
    // Compute the equation for the planeVisual in world coordinates.
    auto const& wMatrix = mPlaneVisuals[i]->worldTransform.GetHMatrix();
    auto const& msTriangle = mModelSpaceTriangles[i];
    std::array<Vector4<float>, 3> wsTriangle{};
    for (size_t j = 0; j < 3; ++j)
    {
        wsTriangle[j] = DoTransform(wMatrix, msTriangle[j]);
    }
    CullingPlane<float> wPlane(wsTriangle[0], wsTriangle[1], wsTriangle[2]);
    wPlane.Normalize();

    // This is a conservative test to see whether a shadow should be cast.
    // This can cause incorrect results if the caster is large and intersects
    // the plane, but ordinarily we are not trying to cast shadows in such
    // situations.
    if (mShadowCaster->worldBound.WhichSide(wPlane) < 0)
    {
        // The shadow caster is on the far side of plane, so it cannot cast
        // a shadow.
        projection = Matrix4x4<float>::Identity();
        return false;
    }

    // Compute the projection matrix for the light source.
    auto normal = wPlane.GetNormal();
    if (mLightProjector->isPointLight)
    {
        float NdE = Dot(normal, mLightProjector->position);
        if (NdE <= 0.0f)
        {
            // The projection must be onto the positive side of the plane.
            projection = Matrix4x4<float>::Identity();
            return false;
        }

        projection = MakePerspectiveProjection(wsTriangle[0], normal,
            mLightProjector->position);
    }
    else
    {
        float NdD = Dot(normal, mLightProjector->direction);
        if (NdD >= 0.0f)
        {
            // The projection must be onto the positive side of the plane.
            projection = Matrix4x4<float>::Identity();
            return false;
        }

        projection = MakeObliqueProjection(wsTriangle[0], normal,
            mLightProjector->direction);
    }

    return true;
}
