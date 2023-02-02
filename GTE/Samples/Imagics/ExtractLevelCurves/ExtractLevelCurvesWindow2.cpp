// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ExtractLevelCurvesWindow2.h"

ExtractLevelCurvesWindow2::ExtractLevelCurvesWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mOriginal(IMAGE_SIZE * IMAGE_SIZE),
    mEnlarged(ENLARGED_SIZE * ENLARGED_SIZE),
    mUseSquares(true),
    mMouseDown(false)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Read the 256x256 input image.  Each pixel is stored as a 16-bit
    // unsigned integer but using only 10 bits; that is, the pixel values
    // are in [0,1024).
    std::string path = mEnvironment.GetPath("Head_U16_X256_Y256.binary");
    std::ifstream input(path, std::ios::binary);
    input.read((char*)mOriginal.data(), mOriginal.size() * sizeof(int16_t));
    input.close();

    // Enlarge the image.
    int32_t const numer = IMAGE_SIZE - 1, denom = MAX_PIXEL - 1;
    for (int32_t y = 0, index = 0; y < IMAGE_SIZE; ++y)
    {
        for (int32_t x = 0; x < IMAGE_SIZE; ++x, ++index)
        {
            // Scale the pixel value to [0,256).
            uint8_t value = static_cast<uint8_t>(mOriginal[index] * numer / denom);
            uint32_t gray = value | (value << 8) | (value << 16) | 0xFF000000;
            for (int32_t dy = 0; dy < MAGNIFY; ++dy)
            {
                int32_t my = MAGNIFY * y + dy;
                for (int32_t dx = 0; dx < MAGNIFY; ++dx)
                {
                    int32_t mx = MAGNIFY * x + dx;
                    int32_t lookup = mx + ENLARGED_SIZE * my;
                    mEnlarged[static_cast<size_t>(lookup)] = gray;
                }
            }
        }
    }

    mExtractorSquares = std::make_unique<CurveExtractorSquares<int16_t, double>>(
        IMAGE_SIZE, IMAGE_SIZE, mOriginal.data());

    mExtractorTriangles = std::make_unique<CurveExtractorTriangles<int16_t, double>>(
        IMAGE_SIZE, IMAGE_SIZE, mOriginal.data());
}

void ExtractLevelCurvesWindow2::OnDisplay()
{
    // Copy the image as background.  Level curves are drawn on top of
    // this in color.
    auto texels = mScreenTexture->Get<uint32_t>();
    std::memcpy(texels, mEnlarged.data(), mEnlarged.size() * sizeof(uint32_t));

    uint32_t const color = (mUseSquares ? 0xFF00FF00 : 0xFF0000FF);
    for (auto const& edge : mEdges)
    {
        int32_t x0 = MAGNIFY * static_cast<int32_t>(mVertices[edge.v[0]][0]);
        int32_t y0 = MAGNIFY * static_cast<int32_t>(mVertices[edge.v[0]][1]);
        int32_t x1 = MAGNIFY * static_cast<int32_t>(mVertices[edge.v[1]][0]);
        int32_t y1 = MAGNIFY * static_cast<int32_t>(mVertices[edge.v[1]][1]);
        DrawLine(x0, y0, x1, y1, color);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool ExtractLevelCurvesWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'e':
    case 'E':
        mUseSquares = !mUseSquares;
        ExtractLevelCurves(x, y);
        return true;
    }
    return Window2::OnCharPress(key, x, y);
}

bool ExtractLevelCurvesWindow2::OnMouseClick(int32_t button, int32_t state, int32_t x, int32_t y, uint32_t)
{
    if (button == MOUSE_LEFT)
    {
        if (state == MOUSE_DOWN)
        {
            mMouseDown = true;
            ExtractLevelCurves(x, y);
        }
        else
        {
            mMouseDown = false;
        }
        return true;
    }
    return false;
}

bool ExtractLevelCurvesWindow2::OnMouseMotion(int32_t button, int32_t x, int32_t y, uint32_t)
{
    if (button == MOUSE_LEFT && mMouseDown)
    {
        ExtractLevelCurves(x, y);
        return true;
    }
    return false;
}

bool ExtractLevelCurvesWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    if (mEnvironment.GetPath("Head_U16_X256_Y256.binary") == "")
    {
        LogError("Cannot find file Head_U16_X256_Y256.binary");
        return false;
    }
    return true;
}

void ExtractLevelCurvesWindow2::ExtractLevelCurves(int32_t x, int32_t y)
{
    // If you uncomment the MakeUnique calls, the performance takes a minor
    // hit for mExtractorSquares but a major hit for mExtractorTriangles.
    if (0 <= x && x < mXSize && 0 <= y && y < mYSize)
    {
        x = x / MAGNIFY;
        y = y / MAGNIFY;
        int32_t index = x + IMAGE_SIZE * y;
        int16_t level = mOriginal[static_cast<size_t>(index)];
        std::vector<CurveExtractor<int16_t, double>::Vertex> rationalVertices;
        if (mUseSquares)
        {
            mExtractorSquares->Extract(level, rationalVertices, mEdges);
            // mExtractorSquares->MakeUnique(rationalVertices, mEdges);
            mExtractorSquares->Convert(rationalVertices, mVertices);
        }
        else
        {
            mExtractorTriangles->Extract(level, rationalVertices, mEdges);
            // mExtractorTriangles->MakeUnique(rationalVertices, mEdges);
            mExtractorTriangles->Convert(rationalVertices, mVertices);
        }
        OnDisplay();
    }
}
