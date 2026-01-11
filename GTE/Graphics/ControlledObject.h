// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/Controller.h>
#include <list>
#include <memory>

namespace gte
{
    class ControlledObject
    {
    protected:
        // Abstract base class.
        ControlledObject() = default;
    public:
        virtual ~ControlledObject() = default;

        // Access to the controllers that control this object.
        typedef std::list<std::shared_ptr<Controller>> List;

        inline List const& GetControllers() const
        {
            return mControllers;
        }

        void AttachController(std::shared_ptr<Controller> const& controller);
        void DetachController(std::shared_ptr<Controller> const& controller);
        void DetachAllControllers();
        bool UpdateControllers(double applicationTime);

    private:
        List mControllers;
    };
}


