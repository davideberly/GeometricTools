// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/PointLightEffect.h>
#include <Graphics/ViewVolumeNode.h>
using namespace gte;

class CameraAndLightNodesWindow3 : public Window3
{
public:
    CameraAndLightNodesWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnKeyDown(int32_t key, int32_t x, int32_t y) override;
    virtual bool OnKeyUp(int32_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void InitializeCameraNode();
    void CreateScene();
    std::shared_ptr<Visual> CreateGround();
    std::shared_ptr<Node> CreateLightFixture(int32_t i);
    std::shared_ptr<Visual> CreateLightTarget();

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<DepthStencilState> mNoDepthStencilState;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mGround, mLightTarget[2];
    std::shared_ptr<PointLightEffect> mEffect[2];

    // Support for the camera node and the light nodes.
    class CameraNodeRig : public CameraRig
    {
    public:
        void SetCameraNode(std::shared_ptr<ViewVolumeNode> const& cameraNode);

    private:
        virtual void MoveForward();
        virtual void MoveBackward();
        virtual void TurnRight();
        virtual void TurnLeft();

        std::shared_ptr<ViewVolumeNode> mCameraNode;
    };

    std::shared_ptr<ViewVolumeNode> mCameraNode, mLightNode[2];
    Vector4<float> mCameraModelPosition;
    CameraNodeRig mCameraNodeRig;
};
