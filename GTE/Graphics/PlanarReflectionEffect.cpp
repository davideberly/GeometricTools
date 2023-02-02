// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.07

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/PlanarReflectionEffect.h>
using namespace gte;

PlanarReflectionEffect::PlanarReflectionEffect(
    std::shared_ptr<Node> const& reflectionCaster,
    std::vector<std::shared_ptr<Visual>> const& planeVisuals,
    std::vector<float> const& reflectances)
    :
    mReflectionCaster(reflectionCaster),
    mPlaneVisuals(planeVisuals),
    mReflectances(reflectances),
    mCasterVisuals{},
    mPlaneOrigins(planeVisuals.size()),
    mPlaneNormals(planeVisuals.size()),
    mNoColorWrites{},
    mReflectanceBlend{},
    mCullReverse{},
    mDSPass0{},
    mDSPass1{},
    mDSPass2{},
    mDSPass3{}
{
    // Recursively traverse the reflection caster hierarchy and gather all the
    // Visual objects.
    GatherVisuals(mReflectionCaster);

    // Verify the planeVisuals satisfy the constraints for the POSITION
    // semantic. Package the first triangle of vertices into the model-space
    // storage.
    GetModelSpacePlanes();

    // Turn off color writes.
    mNoColorWrites = std::make_shared<BlendState>();
    mNoColorWrites->target[0].enable = true;
    mNoColorWrites->target[0].mask = 0;

    // Blend with a constant alpha. The blend color is set for each
    // reflecting plane.
    mReflectanceBlend = std::make_shared<BlendState>();
    mReflectanceBlend->target[0].enable = true;
    mReflectanceBlend->target[0].srcColor = BlendState::Mode::INV_FACTOR;
    mReflectanceBlend->target[0].dstColor = BlendState::Mode::FACTOR;
    mReflectanceBlend->target[0].srcAlpha = BlendState::Mode::INV_FACTOR;
    mReflectanceBlend->target[0].dstAlpha = BlendState::Mode::FACTOR;

    // For toggling the current cull mode to the opposite of what is active.
    mCullReverse = std::make_shared<RasterizerState>();

    // The depth-stencil passes. The reference values are set for each
    // reflecting plane.
    mDSPass0 = std::make_shared<DepthStencilState>();
    mDSPass0->depthEnable = true;
    mDSPass0->writeMask = DepthStencilState::WriteMask::ZERO;
    mDSPass0->comparison = DepthStencilState::Comparison::LESS_EQUAL;
    mDSPass0->stencilEnable = true;
    mDSPass0->frontFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass0->frontFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass0->frontFace.pass = DepthStencilState::Operation::OP_REPLACE;
    mDSPass0->frontFace.comparison = DepthStencilState::Comparison::ALWAYS;
    mDSPass0->backFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass0->backFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass0->backFace.pass = DepthStencilState::Operation::OP_REPLACE;
    mDSPass0->backFace.comparison = DepthStencilState::Comparison::ALWAYS;

    mDSPass1 = std::make_shared<DepthStencilState>();
    mDSPass1->depthEnable = true;
    mDSPass1->writeMask = DepthStencilState::WriteMask::ALL;
    mDSPass1->comparison = DepthStencilState::Comparison::ALWAYS;
    mDSPass1->stencilEnable = true;
    mDSPass1->frontFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->frontFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->frontFace.pass = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->frontFace.comparison = DepthStencilState::Comparison::EQUAL;
    mDSPass1->backFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->backFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->backFace.pass = DepthStencilState::Operation::OP_KEEP;
    mDSPass1->backFace.comparison = DepthStencilState::Comparison::EQUAL;

    mDSPass2 = std::make_shared<DepthStencilState>();
    mDSPass2->depthEnable = true;
    mDSPass2->writeMask = DepthStencilState::WriteMask::ALL;
    mDSPass2->comparison = DepthStencilState::Comparison::LESS_EQUAL;
    mDSPass2->stencilEnable = true;
    mDSPass2->frontFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass2->frontFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass2->frontFace.pass = DepthStencilState::Operation::OP_KEEP;
    mDSPass2->frontFace.comparison = DepthStencilState::Comparison::EQUAL;
    mDSPass2->backFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass2->backFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass2->backFace.pass = DepthStencilState::Operation::OP_KEEP;
    mDSPass2->backFace.comparison = DepthStencilState::Comparison::EQUAL;

    mDSPass3 = std::make_shared<DepthStencilState>();
    mDSPass3->depthEnable = true;
    mDSPass3->writeMask = DepthStencilState::WriteMask::ALL;
    mDSPass3->comparison = DepthStencilState::Comparison::LESS_EQUAL;
    mDSPass3->stencilEnable = true;
    mDSPass3->frontFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass3->frontFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass3->frontFace.pass = DepthStencilState::Operation::OP_INVERT;
    mDSPass3->frontFace.comparison = DepthStencilState::Comparison::EQUAL;
    mDSPass3->backFace.fail = DepthStencilState::Operation::OP_KEEP;
    mDSPass3->backFace.depthFail = DepthStencilState::Operation::OP_KEEP;
    mDSPass3->backFace.pass = DepthStencilState::Operation::OP_INVERT;
    mDSPass3->backFace.comparison = DepthStencilState::Comparison::EQUAL;
}

void PlanarReflectionEffect::Draw(std::shared_ptr<GraphicsEngine> const& engine,
    PVWUpdater& pvwMatrices)
{
    // Save the global state, to be restored later.
    std::shared_ptr<BlendState> saveBState = engine->GetBlendState();
    std::shared_ptr<DepthStencilState> saveDSState = engine->GetDepthStencilState();
    std::shared_ptr<RasterizerState> saveRState = engine->GetRasterizerState();

    // The depth range will be modified during drawing, so save the current
    // depth range for restoration later.
    float minDepth{}, maxDepth{};
    engine->GetDepthRange(minDepth, maxDepth);

    // Get the camera to store post-world transformations.
    std::shared_ptr<Camera> camera = pvwMatrices.GetCamera();

    // Get the current cull mode and reverse it. Allow for models that are
    // not necessarily set up with front or back face culling.
    if (saveRState->cull == RasterizerState::Cull::BACK)
    {
        mCullReverse->cull = RasterizerState::Cull::FRONT;
    }
    else if (saveRState->cull == RasterizerState::Cull::FRONT)
    {
        mCullReverse->cull = RasterizerState::Cull::BACK;
    }
    else
    {
        mCullReverse->cull = RasterizerState::Cull::NONE;
    }
    engine->Bind(mCullReverse);

    uint32_t const numPlanes = static_cast<uint32_t>(mPlaneVisuals.size());
    for (uint32_t i = 0, reference = 1; i < numPlanes; ++i, ++reference)
    {
        auto const& plane = mPlaneVisuals[i];

        // Render the plane to the stencil buffer only; that is, there are no
        // color writes or depth writes. The depth buffer is read so that
        // plane pixels occluded by other already drawn geometry are not
        // drawn. The stencil buffer value for pixels from plane i is (i+1).
        // The stencil buffer is updated at a pixel only when the depth test
        // passes at that pixel (the plane pixel is visible):
        //   face.fail is always false, so value KEEP is irrelevant
        //   face.depthFail = true, KEEP current stencil value
        //   face.pass = false, REPLACE current stencil value with (i+1)
        // for each face in { frontFace, backFace .
        mDSPass0->reference = reference;
        engine->SetDepthStencilState(mDSPass0);
        engine->SetBlendState(mNoColorWrites);
        engine->Draw(plane);

        // Render the plane again. The stencil buffer comparison is EQUAL, so
        // the color and depth are updated only at pixels generated by the
        // plane; the stencil values for such pixels is necessarily (i+1). The
        // depth buffer comparison is ALWAYS and the depth range settings
        // cause the depth to be updated to maximum depth at all pixels where
        // where the stencil values are (i+1). This allows us to draw the
        // reflected object on the plane. Color writes are enabled, because
        // the portion of the plane not covered by the reflected object must
        // be drawn because it is visible.
        mDSPass1->reference = reference;
        engine->SetDepthStencilState(mDSPass1);
        engine->SetDefaultBlendState();
        engine->SetDepthRange(maxDepth, maxDepth);
        engine->Draw(plane);

        // Render the reflected object only at pixels corresponding to those
        // drawn for the current plane; that is, where the stencil buffer
        // value is (i+1). The reflection matrix is constructed from the
        // plane in world coordinates and must be applied in the
        // transformation pipeline before the world-to-view matrix is applied;
        // thus, we insert the reflection matrix into the pipeline via
        // SetPreViewMatrix. Because the pvw-matrices are dependent on this,
        // each time the full transformation is computed we must update the
        // pvw matrices in the constant buffers for the shaders. NOTE: The
        // reflected objects will generate pixels whose depth is larger than
        // that for the reflecting plane. This is not a problem, because we
        // will later draw the plane again and blend its pixels with the
        // reflected object pixels, after which the depth buffer values are
        // updated to the plane pixel depths.
        Matrix4x4<float> wMatrix = plane->worldTransform;
        Vector4<float> origin = DoTransform(wMatrix, mPlaneOrigins[i]);
        Vector4<float> normal = DoTransform(wMatrix, mPlaneNormals[i]);
        Normalize(normal);
        camera->SetPreViewMatrix(MakeReflection(origin, normal));
        pvwMatrices.Update();
        engine->SetDepthRange(minDepth, maxDepth);
        mDSPass2->reference = reference;
        engine->SetDepthStencilState(mDSPass2);
        engine->SetRasterizerState(mCullReverse);
        engine->Draw(mCasterVisuals);
        engine->SetRasterizerState(saveRState);
        camera->SetPreViewMatrix(Matrix4x4<float>::Identity());
        pvwMatrices.Update();

        // Render the plane a third time and blend its colors with the colors
        // of the reflect objects. The blending occurs only at the pixels
        // corresponding to the current plane; that is, where the stencil
        // values are (i+1). The stencil values are cleared (set to zero) at
        // pixels where the depth test passes. The blending uses the
        // reflectance value for the plane,
        //   (1 - reflectance) * plane.rgba + reflectance * backbuffer.rgba
        mDSPass3->reference = reference;
        mReflectanceBlend->blendColor = { mReflectances[i],
            mReflectances[i], mReflectances[i], mReflectances[i] };
        engine->SetDepthStencilState(mDSPass3);
        engine->SetBlendState(mReflectanceBlend);
        engine->Draw(plane);
    }

    // Restore the global state that existed before this function call.
    engine->SetBlendState(saveBState);
    engine->SetDepthStencilState(saveDSState);
    engine->SetRasterizerState(saveRState);

    // Render the objects using a normal drawing pass.
    engine->Draw(mCasterVisuals);
}

void PlanarReflectionEffect::GatherVisuals(std::shared_ptr<Spatial> const& spatial)
{
    auto visual = std::dynamic_pointer_cast<Visual>(spatial);
    if (visual)
    {
        mCasterVisuals.push_back(visual);
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
                GatherVisuals(child);
            }
        }
    }
}

void PlanarReflectionEffect::GetModelSpacePlanes()
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

        // Get the first triangle's vertex indices.
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
        std::array<Vector4<float>, 3> p{};
        for (size_t j = 0; j < 3; ++j)
        {
            auto vertices = reinterpret_cast<Vector3<float> const*>(
                rawData + v[j] * stride);
            p[j] = HLift(*vertices, 1.0f);
        }

        mPlaneOrigins[i] = p[0];
        mPlaneNormals[i] = UnitCross(p[2] - p[0], p[1] - p[0]);

        // The planar reflection effect is responsible for drawing the planes.
        visual->culling = CullingMode::ALWAYS;
    }
}
