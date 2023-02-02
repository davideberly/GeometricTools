// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Node.h>
#include <Graphics/Visual.h>
#include <Graphics/ProgramFactory.h>
#include <Graphics/SkinController.h>
#include <Graphics/Texture2Effect.h>
#include <Graphics/TransformController.h>
#include <Mathematics/Vector2.h>
#include <map>
using namespace gte;

class BipedManager
{
public:
    // Construction.
    BipedManager(std::string const& rootPath, std::string const& bname,
        std::shared_ptr<ProgramFactory> const& factory, BufferUpdater const& postUpdate);

    // Member access.
    inline std::shared_ptr<Node> const& GetRoot() const
    {
        return mRoot;
    }

    // Pairs that need their transforms and pvw-buffers subscribed for
    // automatic updates.
    typedef std::pair<std::shared_ptr<Visual>, std::shared_ptr<ConstantBuffer>> Subscribers;
    inline std::array<Subscribers, 2> const& GetSubscribers() const
    {
        return mSubscribers;
    }

    // Get the extreme times for all the controllers of the animation.
    inline void GetIdle(double& minTime, double& maxTime) const
    {
        GetAnimation(mIdleArray, minTime, maxTime);
    }

    inline void GetWalk(double& minTime, double& maxTime) const
    {
        GetAnimation(mWalkArray, minTime, maxTime);
    }

    inline void GetRun(double& minTime, double& maxTime) const
    {
        GetAnimation(mRunArray, minTime, maxTime);
    }

    // Set time sampler parameters.
    inline void SetIdle(double frequency, double phase)
    {
        SetAnimation(mIdleArray, frequency, phase);
    }

    inline void SetWalk(double frequency, double phase)
    {
        SetAnimation(mWalkArray, frequency, phase);
    }

    inline void SetRun(double frequency, double phase)
    {
        SetAnimation(mRunArray, frequency, phase);
    }

    // Select animations.
    inline void DoIdle()
    {
        DoAnimation(mIdleArray);
    }

    inline void DoWalk()
    {
        DoAnimation(mWalkArray);
    }

    inline void DoRun()
    {
        DoAnimation(mRunArray);
    }

    inline void DoIdleWalk()
    {
        DoAnimation(mIdleWalkArray);
    }

    inline void DoWalkRun()
    {
        DoAnimation(mWalkRunArray);
    }

    // Set blending weight.
    inline void SetIdleWalk(float weight)
    {
        SetBlendAnimation(mIdleWalkArray, weight);
    }

    inline void SetWalkRun(float weight)
    {
        SetBlendAnimation(mWalkRunArray, weight);
    }

    // Finite state machine for switching and blending among animations.

    // The input idleWalkCount is the maximum number of times Update samples
    // the blend of idle and walk before transitioning to idle or walk.  The
    // input walkCount is the maximum number of times Update samples the
    // walk when blendWalkToRun is 'true' in the Update function.  The input
    // walkRunCount is the maximum number of times Update samples the blend
    // of walk and run before transitioning to walk or run.
    void Initialize(int32_t idleWalkCount, int32_t walkCount, int32_t walkRunCount);

    // Select and sample the appropriate animation.
    //
    // If blendIdleToWalk is 'false', the input blendWalkToRun is ignored.
    // The machine transitions from its current animation state (determined
    // by count) to the idle animation (count is decremented to zero).
    //
    // If blendIdleToWalk is 'true' and blendWalkToRun is 'false', then the
    // machine transitions from its current animation state (determined by
    // count) to the walk-animation state (count is incremented).  Once the
    // machine enters the walk-animation state, it stays in that state.
    //
    // If blendIdleToWalk is 'true' and blendWalkToRun is 'true', then the
    // machine transitions from its current animation state (determined by
    // count) to the run-animation state (count is incremented).  Once the
    // machine enters the run-animation state, it stays in that state.
    void Update(bool blendIdleToWalk, bool blendWalkToRun);

    // Get the speed of the biped.  This depends on the current animation
    // state.  The speed here is dimensionless, measured as the ratio
    // mCount/mCountMax[ANIM_RUN], which is in [0,1].
    float GetSpeed() const;

private:
    // Loading support.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };
    VertexFormat mVFormat;

    typedef std::vector<std::string> StringArray;
    typedef std::map<std::string, std::shared_ptr<Spatial>> SpatialMap;
    typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<TransformController>> NodeCtrl;
    typedef std::vector<NodeCtrl> NodeCtrlArray;

    class PreSpatial
    {
    public:
        std::shared_ptr<Spatial> Associate;
        StringArray ChildNames;
    };
    typedef std::vector<std::shared_ptr<PreSpatial>> PreSpatialArray;

    class PreSkin
    {
    public:
        std::shared_ptr<SkinController> Associate;
        StringArray BoneNames;
    };
    typedef std::vector<std::shared_ptr<PreSkin>> PreSkinArray;

    std::shared_ptr<PreSpatial> LoadNode(std::string const& rootPath,
        std::string const& name);

    std::shared_ptr<PreSpatial> LoadMesh(std::string const& rootPath,
        std::string const& name, std::shared_ptr<Texture2Effect> const& effect);

    std::shared_ptr<PreSkin> LoadSkinController(std::string const& rootPath,
        std::string const& name, BufferUpdater const& postUpdate);

    std::shared_ptr<TransformController> LoadTransformController(std::string const& rootPath,
        std::string const& name, std::string const& animation);

    // Run-time support.
    void GetAnimation(NodeCtrlArray const& ncArray, double& minTime, double& maxTime) const;
    void SetAnimation(NodeCtrlArray& ncArray, double frequency, double phase);
    void SetBlendAnimation(NodeCtrlArray& ncArray, float weight);
    void DoAnimation(NodeCtrlArray& ncArray);

    // The biped and animation sequences.
    std::shared_ptr<Node> mRoot;
    std::array<Subscribers, 2> mSubscribers;
    NodeCtrlArray mIdleArray, mWalkArray, mRunArray;
    NodeCtrlArray mIdleWalkArray, mWalkRunArray;

    // Finite state machine.
    struct Animation
    {
        static uint32_t constexpr IDLE = 0;
        static uint32_t constexpr IDLE_WALK = 1;
        static uint32_t constexpr WALK = 2;
        static uint32_t constexpr WALK_RUN = 3;
        static uint32_t constexpr RUN = 4;
        static uint32_t constexpr NUM_STATES = 5;
    };

    void ContinueIdleWalk();
    void ContinueWalkRun();
    void ContinueRunWalk();
    void ContinueWalkIdle();
    void TransitionIdleToIdleWalk();
    void TransitionIdleWalkToWalk();
    void TransitionWalkToWalkRun();
    void TransitionWalkRunToRun();
    void TransitionRunToRunWalk();
    void TransitionRunWalkToWalk();
    void TransitionWalkToWalkIdle();
    void TransitionWalkIdleToIdle();

    uint32_t mState;
    int32_t mCount;
    std::array<int32_t, Animation::NUM_STATES> mCountMax;
    float mWeight, mDeltaWeight0, mDeltaWeight1;
};
