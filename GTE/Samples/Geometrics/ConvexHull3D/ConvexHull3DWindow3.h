// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/VertexColorEffect.h>
using namespace gte;

class ConvexHull3DWindow3 : public Window3
{
public:
    ConvexHull3DWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    bool LoadData();

    // The input data files are in the Data subfolder.  The files are of the
    // format "dataXX.txt", where XX is in {01,02,...,46}.
    int32_t mFileQuantity;  // = 46
    int32_t mCurrentFile;  // = 1 initially

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mMesh;
    std::shared_ptr<VertexColorEffect> mEffect;
    std::string mMessage;
};
