// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "VertexCollapseMeshWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/VertexColorEffect.h>
#include <iostream>
#include <random>

VertexCollapseMeshWindow3::VertexCollapseMeshWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 1.0f, 0.01f,
        { 0.0f, 0.0f, 6.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });

    VertexFormat format;
    format.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(format);
    auto cylinder = mf.CreateCylinderOpen(8, 8, 1.0f, 2.0f);
    uint32_t numPositions = cylinder->GetVertexBuffer()->GetNumElements();
    mPositions.resize(numPositions);
    std::memcpy(mPositions.data(), cylinder->GetVertexBuffer()->GetData(), numPositions * sizeof(Vector3<float>));
    uint32_t numTriangles = cylinder->GetIndexBuffer()->GetNumPrimitives();
    mTriangles.resize(numTriangles);
    std::memcpy(mTriangles.data(), cylinder->GetIndexBuffer()->GetData(),
        static_cast<size_t>(numTriangles) * 3 * sizeof(int32_t));

    mVCMesh = std::make_shared<VertexCollapseMesh<float>>(numPositions, &mPositions[0],
        3 * numTriangles, (int32_t const*)&mTriangles[0]);

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numPositions);
    auto* vertex = vbuffer->Get<Vertex>();
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    Vector3<float> average{ 0.0f, 0.0f, 0.0f };
    for (uint32_t i = 0; i < numPositions; ++i)
    {
        vertex[i].position = mPositions[i];
        vertex[i].color[0] = rnd(mte);
        vertex[i].color[1] = rnd(mte);
        vertex[i].color[2] = rnd(mte);
        vertex[i].color[3] = 1.0f;
        average += mPositions[i];
    }
    average /= (float)numPositions;

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(int32_t));
    int32_t* index = ibuffer->Get<int32_t>();
    for (auto const& element : mVCMesh->GetMesh().GetTriangles())
    {
        *index++ = element.first.V[0];
        *index++ = element.first.V[1];
        *index++ = element.first.V[2];
    }

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mSurface = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSurface->localTransform.SetTranslation(-average);
    mTrackBall.Attach(mSurface);
    mTrackBall.Update();
    mPVWMatrices.Subscribe(mSurface->worldTransform, effect->GetPVWMatrixConstant());
    mPVWMatrices.Update();
}

void VertexCollapseMeshWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mSurface);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool VertexCollapseMeshWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullWireState != mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;

    case 'c':
    case 'C':
    {
        VertexCollapseMesh<float>::Record record;
        if (mVCMesh->DoCollapse(record))
        {
            std::cout << "v = " << record.vertex << " rs = " << record.removed.size() << " is = " << record.inserted.size() << std::endl;
            auto const& mesh = mVCMesh->GetMesh();
            uint32_t const numTriangles = static_cast<uint32_t>(mesh.GetTriangles().size());
            auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(uint32_t));
            uint32_t* index = ibuffer->Get<uint32_t>();
            for (auto const& element : mesh.GetTriangles())
            {
                *index++ = element.first.V[0];
                *index++ = element.first.V[1];
                *index++ = element.first.V[2];
            }

            mSurface->SetIndexBuffer(ibuffer);
        }
        return true;
    }
    }

    return Window3::OnCharPress(key, x, y);
}
