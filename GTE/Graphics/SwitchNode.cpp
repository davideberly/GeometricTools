// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.07

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/SwitchNode.h>
using namespace gte;

SwitchNode::SwitchNode()
    :
    mActiveChild(invalidChild)
{
}

void SwitchNode::SetActiveChild(int32_t activeChild)
{
    LogAssert(
        activeChild == invalidChild ||
        (0 <= activeChild && activeChild < GetNumChildren()),
        "Invalid active child specified.");

    mActiveChild = activeChild;
}

void SwitchNode::GetVisibleSet(Culler& culler,
    std::shared_ptr<Camera> const& camera, bool noCull)
{
    if (mActiveChild > invalidChild)
    {
        // All Visual objects in the active subtree are added to the
        // visible set.
        auto const& child = mChild[mActiveChild];
        if (child)
        {
            child->OnGetVisibleSet(culler, camera, noCull);
        }
    }
}
