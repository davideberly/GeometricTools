// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/CurveExtractorSquares.h>
#include <Mathematics/CurveExtractorTriangles.h>
using namespace gte;

class ExtractLevelCurvesWindow2 : public Window2
{
public:
    // Determine how large the window should be to draw an enlarged version of
    // the original image.  The pixel type is int16_t but only 10 bits are used
    // per pixel.  Feel free to modify MAGNIFY to 1, 2, 3 or 4.
    enum
    {
        IMAGE_SIZE = 256,
        MAGNIFY = 3,
        ENLARGED_SIZE = MAGNIFY * IMAGE_SIZE,
        MAX_PIXEL = 1024
    };

    ExtractLevelCurvesWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseClick(int button, int state, int x, int y, unsigned int modifiers) override;
    virtual bool OnMouseMotion(int button, int x, int y, unsigned int modifiers) override;

private:
    bool SetEnvironment();
    void ExtractLevelCurves(int x, int y);

    // The original image is 256x256 of int16_t with pixel values in
    // [0,1023); that is, only 10 bits are used per pixel.  The enlarged
    // image is 768x768 of R8G8B8A8 color, which is used for initializing
    // the background of the window.  The level curves are drawn on top
    // of the background.
    std::vector<int16_t> mOriginal;
    std::vector<uint32_t> mEnlarged;

    std::unique_ptr<CurveExtractorSquares<int16_t, double>> mExtractorSquares;
    std::unique_ptr<CurveExtractorTriangles<int16_t, double>> mExtractorTriangles;
    std::vector<std::array<double, 2>> mVertices;
    std::vector<CurveExtractor<int16_t, double>::Edge> mEdges;

    // When true, use mExtractorSquares.  When false, use mExtractorTriangles.
    bool mUseSquares;

    // For left-mouse-drag operations.
    bool mMouseDown;
};
