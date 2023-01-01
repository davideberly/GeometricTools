// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.11

#pragma once

#include <Graphics/Node.h>

namespace gte
{
    class SwitchNode : public Node
    {
    public:
        SwitchNode();
        virtual ~SwitchNode() = default;

        static int32_t constexpr invalidChild = -1;

        void SetActiveChild(int32_t activeChild);

        inline int32_t GetActiveChild() const
        {
            return mActiveChild;
        }

        inline void DisableAllChildren()
        {
            mActiveChild = invalidChild;
        }

    protected:
        // Support for hierarchical culling.
        virtual void GetVisibleSet(Culler& culler,
            std::shared_ptr<Camera> const& camera, bool noCull) override;

        int32_t mActiveChild;
    };
}
