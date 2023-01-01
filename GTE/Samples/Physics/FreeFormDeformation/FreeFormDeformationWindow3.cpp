// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "FreeFormDeformationWindow3.h"
#include <Applications/WICFileIO.h>
#include <Mathematics/Array3.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/Texture2Effect.h>
#include <random>

FreeFormDeformationWindow3::FreeFormDeformationWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mRed{ 0.75f, 0.0f, 0.0f, 1.0f },
    mGreen{ 0.0f, 0.75f, 0.0f, 1.0f },
    mBlue{ 0.0f, 0.0f, 0.75f, 1.0f },
    mGray{ 0.75f, 0.75f, 0.75f, 1.0f },
    mQuantity(4),  // 4x4x4 control point grid
    mDegree(3),  // cubic polynomial per dimension
    mMin{ 0.0f, 0.0f, 0.0f },
    mMax{ 0.0f, 0.0f, 0.0f },
    mDelta{ 0.0f, 0.0f, 0.0f },
    mOldWorldPosition{ 0.0f, 0.0f, 0.0f, 1.0f },
    mAmplitude(0.01f),
    mRadius(0.25f),
    mLastUpdateTime(mMotionTimer.GetSeconds()),
    mDoRandom(false),
    mDrawSegmentsBoxes(true),
    mMouseDown(false)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.02f,
        { 0.0f, 0.0f, 6.6f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void FreeFormDeformationWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    if (mDoRandom)
    {
        // Deform the mesh no faster than 30 frames per second.
        double time = mMotionTimer.GetSeconds();
        if (30.0 * (time - mLastUpdateTime) >= 1.0)
        {
            mLastUpdateTime = time;
            DoRandomControlPoints();
            mTrackBall.Update();
        }
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    if (mDrawSegmentsBoxes)
    {
        for (auto const& segment : mSegments)
        {
            mEngine->Draw(segment);
        }
        for (auto const& box : mBoxes)
        {
            mEngine->Draw(box);
        }
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool FreeFormDeformationWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
    case 'W':
        if (mWireState == mEngine->GetRasterizerState())
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;

    case 'a':  // toggle between automated random and user-adjusted controls
    case 'A':
        mDoRandom = !mDoRandom;
        return true;

    case 'c':  // toggle whether or not the segments or boxes are drawn
    case 'C':
        mDrawSegmentsBoxes = !mDrawSegmentsBoxes;
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool FreeFormDeformationWindow3::OnMouseClick(MouseButton button, MouseState state,
    int32_t x, int32_t y, uint32_t modifiers)
{
    if (button == MOUSE_RIGHT && !mDoRandom)
    {
        if (state == MOUSE_DOWN)
        {
            mMouseDown = true;
            OnMouseDown(x, mYSize - 1 - y);  // switch to right-handed screen
        }
        else
        {
            mMouseDown = false;
        }
        return true;
    }

    return Window3::OnMouseClick(button, state, x, y, modifiers);
}

bool FreeFormDeformationWindow3::OnMouseMotion(MouseButton button, int32_t x, int32_t y,
    uint32_t modifiers)
{
    if (mMouseDown && mSelected)
    {
        OnMouseMove(x, mYSize - 1 - y);  // switch to right-handed screen
        return true;
    }

    return Window3::OnMouseMotion(button, x, y, modifiers);
}

bool FreeFormDeformationWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Checkerboard.png") == "")
    {
        LogError("Cannot find file Checkerboard.png");
        return false;
    }

    return true;
}

void FreeFormDeformationWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();
    mTrackBall.Attach(mScene);

    // Start with a torus that the user can deform during execution.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mMesh = mf.CreateTorus(32, 32, 2.0f, 0.5f);
    mMesh->GetVertexBuffer()->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto texture = WICFileIO::Load(mEnvironment.GetPath("Checkerboard.png"), true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mMesh->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mMesh);

    CreateBSplineVolume();
    CreateSegments();
    CreateBoxes();
    mTrackBall.Update();
}

void FreeFormDeformationWindow3::CreateBSplineVolume()
{
    // Create the B-spline volume function.  The control points are assigned
    // later in this function.
    BasisFunctionInput<float> input[3] =
    {
        BasisFunctionInput<float>(mQuantity, mDegree),
        BasisFunctionInput<float>(mQuantity, mDegree),
        BasisFunctionInput<float>(mQuantity, mDegree)
    };

    mVolume = std::make_unique<BSplineVolume<3, float>>(input, nullptr);

    // Compute a bounding box of the form [xmin,xmax]x[ymin,ymax]x[zmin,zmax].
    auto const& vbuffer = mMesh->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumElements();
    auto vertices = vbuffer->Get<Vertex>();
    mMin = vertices[0].position;
    mMax = mMin;
    for (uint32_t i = 1; i < numVertices; ++i)
    {
        Vector3<float> position = vertices[i].position;
        for (int32_t j = 0; j < 3; ++j)
        {
            if (position[j] < mMin[j])
            {
                mMin[j] = position[j];
            }
            else if (position[j] > mMax[j])
            {
                mMax[j] = position[j];
            }
        }
    }

    // Generate the control points.
    Vector3<float> range = mMax - mMin, ctrl{};
    mDelta = range / static_cast<float>(mQuantity - 1);
    for (int32_t i2 = 0; i2 < mQuantity; ++i2)
    {
        ctrl[2] = mMin[2] + mDelta[2] * i2;
        for (int32_t i1 = 0; i1 < mQuantity; ++i1)
        {
            ctrl[1] = mMin[1] + mDelta[1] * i1;
            for (int32_t i0 = 0; i0 < mQuantity; ++i0)
            {
                ctrl[0] = mMin[0] + mDelta[0] * i0;
                mVolume->SetControl(i0, i1, i2, ctrl);
            }
        }
    }

    // Compute the (u,v,w) values of the mesh relative to the B-spline volume.
    Vector3<float> invRange{ 1.0f / range[0], 1.0f / range[1], 1.0f / range[2] };
    mParameters.resize(numVertices);
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vector3<float> position = vertices[i].position;
        Vector3<float>& param = mParameters[i];
        for (int32_t j = 0; j < 3; ++j)
        {
            param[j] = (position[j] - mMin[j]) * invRange[j];
        }
    }
}

void FreeFormDeformationWindow3::CreateSegments()
{
    // Generate the polylines that connect adjacent control points.
    mPolysegmentRoot = std::make_shared<Node>();
    mScene->AttachChild(mPolysegmentRoot);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);

    std::shared_ptr<VertexBuffer> vbuffer;
    Vector3<float>* vertices;
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);
    std::shared_ptr<ConstantColorEffect> effect;
    std::shared_ptr<Visual> segment;

    // Create segments with direction (1,0,0).
    for (int32_t i2 = 0; i2 < mQuantity; ++i2)
    {
        for (int32_t i1 = 0; i1 < mQuantity; ++i1)
        {
            for (int32_t i0 = 0; i0 < mQuantity - 1; ++i0)
            {
                vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
                vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
                vertices = vbuffer->Get<Vector3<float>>();
                vertices[0] = mVolume->GetControl(i0, i1, i2);
                vertices[1] = mVolume->GetControl(i0 + 1, i1, i2);
                effect = std::make_shared<ConstantColorEffect>(mProgramFactory, mRed);
                segment = std::make_shared<Visual>(vbuffer, ibuffer, effect);
                mPVWMatrices.Subscribe(segment->worldTransform, effect->GetPVWMatrixConstant());
                mSegments.push_back(segment);
                mPolysegmentRoot->AttachChild(segment);
            }
        }
    }

    // Create segments with direction (0,1,0).
    for (int32_t i2 = 0; i2 < mQuantity; ++i2)
    {
        for (int32_t i0 = 0; i0 < mQuantity; ++i0)
        {
            for (int32_t i1 = 0; i1 < mQuantity - 1; ++i1)
            {
                vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
                vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
                vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
                vertices = vbuffer->Get<Vector3<float>>();
                vertices[0] = mVolume->GetControl(i0, i1, i2);
                vertices[1] = mVolume->GetControl(i0, i1 + 1, i2);
                effect = std::make_shared<ConstantColorEffect>(mProgramFactory, mGreen);
                segment = std::make_shared<Visual>(vbuffer, ibuffer, effect);
                mPVWMatrices.Subscribe(segment->worldTransform, effect->GetPVWMatrixConstant());
                mSegments.push_back(segment);
                mPolysegmentRoot->AttachChild(segment);
            }
        }
    }

    // Create segments with direction (0,0,1).
    for (int32_t i1 = 0; i1 < mQuantity; ++i1)
    {
        for (int32_t i0 = 0; i0 < mQuantity; ++i0)
        {
            for (int32_t i2 = 0; i2 < mQuantity - 1; ++i2)
            {
                vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
                vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
                vertices = vbuffer->Get<Vector3<float>>();
                vertices[0] = mVolume->GetControl(i0, i1, i2);
                vertices[1] = mVolume->GetControl(i0, i1, i2 + 1);
                effect = std::make_shared<ConstantColorEffect>(mProgramFactory, mBlue);
                segment = std::make_shared<Visual>(vbuffer, ibuffer, effect);
                mPVWMatrices.Subscribe(segment->worldTransform, effect->GetPVWMatrixConstant());
                mSegments.push_back(segment);
                mPolysegmentRoot->AttachChild(segment);
            }
        }
    }
}

void FreeFormDeformationWindow3::CreateBoxes()
{
    // Generate small boxes to represent the control points.
    mControlRoot = std::make_shared<Node>();
    mScene->AttachChild(mControlRoot);

    // Create a single box to be shared by each control point box.
    float const halfWidth = 0.05f;
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 8);
    auto vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { -halfWidth, -halfWidth, -halfWidth };
    vertices[1] = { +halfWidth, -halfWidth, -halfWidth };
    vertices[2] = { +halfWidth, +halfWidth, -halfWidth };
    vertices[3] = { -halfWidth, +halfWidth, -halfWidth };
    vertices[4] = { -halfWidth, -halfWidth, +halfWidth };
    vertices[5] = { +halfWidth, -halfWidth, +halfWidth };
    vertices[6] = { +halfWidth, +halfWidth, +halfWidth };
    vertices[7] = { -halfWidth, +halfWidth, +halfWidth };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 12, sizeof(uint32_t));
    auto indices = ibuffer->Get<uint32_t>();
    indices[0] = 0;  indices[1] = 2;  indices[2] = 1;
    indices[3] = 0;  indices[4] = 3;  indices[5] = 2;
    indices[6] = 4;  indices[7] = 5;  indices[8] = 6;
    indices[9] = 4;  indices[10] = 6;  indices[11] = 7;
    indices[12] = 0;  indices[13] = 5;  indices[14] = 4;
    indices[15] = 0;  indices[16] = 1;  indices[17] = 5;
    indices[18] = 3;  indices[19] = 7;  indices[20] = 6;
    indices[21] = 3;  indices[22] = 6;  indices[23] = 2;
    indices[24] = 1;  indices[25] = 2;  indices[26] = 6;
    indices[27] = 1;  indices[28] = 6;  indices[29] = 5;
    indices[30] = 0;  indices[31] = 4;  indices[32] = 7;
    indices[33] = 0;  indices[34] = 7;  indices[35] = 3;

    for (int32_t i2 = 0; i2 < mQuantity; ++i2)
    {
        for (int32_t i1 = 0; i1 < mQuantity; ++i1)
        {
            for (int32_t i0 = 0; i0 < mQuantity; ++i0)
            {
                auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, mGray);
                auto box = std::make_shared<Visual>(vbuffer, ibuffer, effect);
                box->localTransform.SetTranslation(mVolume->GetControl(i0, i1, i2));
                box->Update();
                box->UpdateModelBound();

                // Encode the indices in the name for later use.  This will
                // allow fast lookup of volume control points.
                box->name = std::to_string(i0) + " " + std::to_string(i1) +
                    " " + std::to_string(i2);

                mPVWMatrices.Subscribe(box->worldTransform, effect->GetPVWMatrixConstant());
                mBoxes.push_back(box);
                mControlRoot->AttachChild(box);
            }
        }
    }
}

void FreeFormDeformationWindow3::UpdateMesh()
{
    auto const& vbuffer = mMesh->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumElements();
    auto vertices = vbuffer->Get<Vertex>();
    Vector3<float> values[10];
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vector3<float> param = mParameters[i];
        mVolume->Evaluate(param[0], param[1], param[2], 0, values);
        vertices[i].position = values[0];
    }
    mEngine->Update(vbuffer);
}

void FreeFormDeformationWindow3::UpdateSegments()
{
    size_t bound = static_cast<size_t>(mQuantity);
    Array3<Vector3<float>> control(bound, bound, bound,
        const_cast<Vector3<float>*>(mVolume->GetControls()));

    // The segments are stored in an array as children of mPolysegmentRoot.
    // The order of updates using the triple loops is the same as that used
    // for creating the segments, so we need i to index into the array of
    // children to ensure consistency of pairing the control points with
    // the segments.
    int32_t i = 0;

    // Update segments with direction (1,0,0).
    for (int32_t i2 = 0; i2 < mQuantity; ++i2)
    {
        for (int32_t i1 = 0; i1 < mQuantity; ++i1)
        {
            for (int32_t i0 = 0; i0 < mQuantity - 1; ++i0)
            {
                auto segment = std::static_pointer_cast<Visual>(mPolysegmentRoot->GetChild(i));
                auto const& vbuffer = segment->GetVertexBuffer();
                auto vertices = vbuffer->Get<Vector3<float>>();
                vertices[0] = mVolume->GetControl(i0, i1, i2);
                vertices[1] = mVolume->GetControl(i0 + 1, i1, i2);
                mEngine->Update(vbuffer);
                ++i;
            }
        }
    }

    // Update segments with direction (0,1,0).
    for (int32_t i2 = 0; i2 < mQuantity; ++i2)
    {
        for (int32_t i0 = 0; i0 < mQuantity; ++i0)
        {
            for (int32_t i1 = 0; i1 < mQuantity - 1; ++i1)
            {
                auto segment = std::static_pointer_cast<Visual>(mPolysegmentRoot->GetChild(i));
                auto const& vbuffer = segment->GetVertexBuffer();
                auto vertices = vbuffer->Get<Vector3<float>>();
                vertices[0] = mVolume->GetControl(i0, i1, i2);
                vertices[1] = mVolume->GetControl(i0, i1 + 1, i2);
                mEngine->Update(vbuffer);
                ++i;
            }
        }
    }

    // Update segments with direction (0,0,1).
    for (int32_t i1 = 0; i1 < mQuantity; ++i1)
    {
        for (int32_t i0 = 0; i0 < mQuantity; ++i0)
        {
            for (int32_t i2 = 0; i2 < mQuantity - 1; ++i2)
            {
                auto segment = std::static_pointer_cast<Visual>(mPolysegmentRoot->GetChild(i));
                auto const& vbuffer = segment->GetVertexBuffer();
                auto vertices = vbuffer->Get<Vector3<float>>();
                vertices[0] = mVolume->GetControl(i0, i1, i2);
                vertices[1] = mVolume->GetControl(i0, i1, i2 + 1);
                mEngine->Update(vbuffer);
                ++i;
            }
        }
    }
}

void FreeFormDeformationWindow3::UpdateBoxes()
{
    size_t bound = static_cast<size_t>(mQuantity);
    Array3<Vector3<float>> control(bound, bound, bound,
        const_cast<Vector3<float>*>(mVolume->GetControls()));

    // The segments are stored in an array as children of mControlRoot.
    // The order of updates using the triple loops is the same as that used
    // for creating the boxes, so we need i to index into the array of
    // children to ensure consistency of pairing the control points with
    // the boxes.
    int32_t i = 0;

    for (int32_t i2 = 0; i2 < mQuantity; ++i2)
    {
        for (int32_t i1 = 0; i1 < mQuantity; ++i1)
        {
            for (int32_t i0 = 0; i2 < mQuantity; ++i2)
            {
                auto box = std::static_pointer_cast<Visual>(mControlRoot->GetChild(i));
                Vector3<float> ctrl = mVolume->GetControl(i0, i1, i2);
                box->localTransform.SetTranslation(ctrl);
                ++i;
            }
        }
    }
}

void FreeFormDeformationWindow3::DoRandomControlPoints()
{
    // Randomly perturb the control points, but stay near the original
    // control points.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    Vector3<float> ctrl{};
    for (int32_t i2 = 0; i2 < mQuantity; ++i2)
    {
        ctrl[2] = mMin[2] + mDelta[2] * i2;
        for (int32_t i1 = 0; i1 < mQuantity; ++i1)
        {
            ctrl[1] = mMin[1] + mDelta[1] * i1;
            for (int32_t i0 = 0; i0 < mQuantity; ++i0)
            {
                ctrl[0] = mMin[0] + mDelta[0] * i0;

                Vector3<float> newCtrl = mVolume->GetControl(i0, i1, i2) +
                    mAmplitude * Vector3<float>{ rnd(mte), rnd(mte), rnd(mte) };

                Vector3<float> diff = newCtrl - ctrl;
                float length = Length(diff);
                if (length > mRadius)
                {
                    diff *= mRadius / length;
                }

                mVolume->SetControl(i0, i1, i2, ctrl + diff);
            }
        }
    }

    UpdateMesh();
    UpdateSegments();
    UpdateBoxes();
}

void FreeFormDeformationWindow3::OnMouseDown(int32_t x, int32_t y)
{
    std::shared_ptr<ConstantColorEffect> effect;
    std::shared_ptr<ConstantBuffer> cbuffer;

    // The current selected control point is deactivated.
    if (mSelected)
    {
        effect = std::static_pointer_cast<ConstantColorEffect>(mSelected->GetEffect());
        mSelected = nullptr;
        cbuffer = effect->GetColorConstant();
        *cbuffer->Get<Vector4<float>>() = mGray;
        mEngine->Update(cbuffer);
    }

    // Determine which control point has been selected (if any).
    int32_t viewX, viewY, viewW, viewH;
    mEngine->GetViewport(viewX, viewY, viewW, viewH);
    Vector4<float> origin, direction;
    if (mCamera->GetPickLine(viewX, viewY, viewW, viewH, x, y, origin, direction))
    {
        // Use a ray for picking.
        float tmin = 0.0f;
        float constexpr tmax = std::numeric_limits<float>::max();

        // Request the results in model-space coordinates.  All the objects
        // in the scene have the same model space, so we can set the sphere
        // centers in model-space coordinates.
        mPicker(mControlRoot, origin, direction, tmin, tmax);
        if (mPicker.records.size() > 0)
        {
            PickRecord const& record = mPicker.GetClosestNonnegative();
            mOldWorldPosition = record.linePoint;
            mSelected = record.visual;
            effect = std::static_pointer_cast<ConstantColorEffect>(mSelected->GetEffect());
            cbuffer = effect->GetColorConstant();
            *cbuffer->Get<Vector4<float>>() = mRed;
            mEngine->Update(cbuffer);
        }
    }
}

void FreeFormDeformationWindow3::OnMouseMove(int32_t x, int32_t y)
{
    // Construct a pick ray.  We want to move the control point from its
    // current location to this ray.
    int32_t viewX, viewY, viewW, viewH;
    mEngine->GetViewport(viewX, viewY, viewW, viewH);
    Vector4<float> origin, direction;
    if (!mCamera->GetPickLine(viewX, viewY, viewW, viewH, x, y, origin, direction))
    {
        return;
    }

    // Let E be the camera world origin, D be the camera world direction, and
    // U be the pick ray world direction.  Let C0 be the current location of
    // the picked point and let C1 be its desired new location.  We need to
    // choose t for which C1 = E + t*U.  Two possibilities are provided here,
    // both requiring computing: Diff = C0 - E.
    Vector4<float> diff = mOldWorldPosition - mCamera->GetPosition();

    float t;
#if 0
    // The new world position is chosen to be at a distance along the pick
    // ray that is equal to the distance from the camera location to the old
    // world position.  That is, we require
    //   Length(C0-E) = Length(C1-E) = Length(t*U) = t.
    t = Length(diff);
#else
    // The new world position is in the same plane perpendicular to the
    // camera direction as the old world position is.  This plane is
    // Dot(D,X-C0) = 0, in which case we need
    //   0 = Dot(D,C1-C0) = Dot(D,E+t*U-C0) = Dot(D,E-C0) + t*Dot(D,U)
    // Solving for t, we have
    //   t = Dot(D,C0-E)/Dot(D,U)
    t = Dot(mCamera->GetDVector(), diff) / Dot(mCamera->GetDVector(), direction);
#endif
    Vector4<float> newWorldPosition = origin + t * direction;

    // Move the control point to the new world location.  The technical
    // problem is that we need to modify the world coordinates for the
    // selected control point.  Thus, we need to determine how to change the
    // local translation in order to produce the correct world translation.
    diff = newWorldPosition - mOldWorldPosition;
    Vector4<float> localDiff = mSelected->GetParent()->worldTransform.Inverse() * diff;
    mSelected->localTransform.SetTranslation(
        mSelected->localTransform.GetTranslationW0() + localDiff);
    mSelected->Update();
    mSelected->UpdateModelBound();
    mOldWorldPosition = newWorldPosition;

    // Modify the control point itself.  It is known that the name string
    // has three single-digit numbers separated by blanks.
    int32_t i0 = static_cast<int32_t>(mSelected->name[0] - '0');
    int32_t i1 = static_cast<int32_t>(mSelected->name[2] - '0');
    int32_t i2 = static_cast<int32_t>(mSelected->name[4] - '0');
    mVolume->SetControl(i0, i1, i2, mSelected->localTransform.GetTranslation());

    // TODO.  We need only update mesh vertices that are affected by the
    // change in one control point.  This requires working with the B-spline
    // basis function and knowing which (u,v,w) to evaluate at (i.e. determine
    // the local control region).
    UpdateMesh();

    // TODO.  We need only update neighboring lines.
    UpdateSegments();

    mTrackBall.Update();
    mPVWMatrices.Update();
}
