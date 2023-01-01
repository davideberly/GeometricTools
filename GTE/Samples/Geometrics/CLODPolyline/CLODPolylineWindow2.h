// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/CLODPolyline.h>
using namespace gte;

class CLODPolylineWindow2 : public Window2
{
public:
    CLODPolylineWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    inline void Get(Vector<3, float> const& vertex, int32_t& x, int32_t& y)
    {
        float fsize = static_cast<float>(mXSize);
        x = static_cast<int32_t>(0.25f * fsize * (vertex[0] + 2.0f));
        y = mXSize - 1 - static_cast<int32_t>(0.25f * fsize * (vertex[1] + 2.0f));
    }

    std::unique_ptr<CLODPolyline<3, float>> mPolyline;
};
