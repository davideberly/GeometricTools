// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/Picker.h>
#include <Mathematics/BSplineGeodesic.h>
using namespace gte;

class GeodesicHeightFieldWindow3 : public Window3
{
public:
    GeodesicHeightFieldWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int32_t x, int32_t y, uint32_t modifiers) override;

private:
    bool SetEnvironment();
    void CreateScene();

    struct Vertex
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;
    };

    std::function<void(int32_t, int32_t)> mDrawCallback;

    std::shared_ptr<RasterizerState> mNoCullState, mNoCullWireState;
    std::shared_ptr<Visual> mMesh;
    std::shared_ptr<Texture2> mTexture;
    Vector4<float> mLightWorldDirection;
    Picker mPicker;

    std::unique_ptr<BSplineSurface<3, double>> mSurface;
    std::unique_ptr<BSplineGeodesic<double>> mGeodesic;

    int32_t mSelected;
    std::array<int32_t, 2> mXIntr, mYIntr;
    std::array<GVector<double>, 2> mPoint;
    std::vector<GVector<double>> mPath;
    int32_t mPathQuantity;
    double mDistance, mCurvature;

    std::array<float, 4> mTextColor;
};
