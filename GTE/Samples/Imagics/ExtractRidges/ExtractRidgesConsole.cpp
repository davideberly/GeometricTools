// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

// The original code was modified by Lev A. Melnikovsky based on his analysis
// https://github.com/melnikovsky/GeometricTools/blob/algoridge/algoridge.pdf

#include "ExtractRidgesConsole.h"
#include <Applications/WICFileIO.h>
#include <Mathematics/SymmetricEigensolver2x2.h>

ExtractRidgesConsole::ExtractRidgesConsole(Parameters& parameters)
    :
    Console(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }
}

void ExtractRidgesConsole::Execute()
{
    // Load the 10-bit-per-pixel image.
    std::string path = mEnvironment.GetPath("Head_U16_X256_Y256.binary");
    int32_t const xBound = 256, yBound = 256;
    std::vector<int16_t> original(xBound * yBound);
    std::ifstream input(path, std::ios::binary);
    input.read((char*)original.data(), original.size() * sizeof(int16_t));
    input.close();

    // Convert to a double-precision image with values in [0,1].  It is known
    // that the original image has minimum value 0.
    int16_t maxIValue = original[0];
    for (size_t i = 1; i < original.size(); ++i)
    {
        maxIValue = std::max(maxIValue, original[i]);
    }

    Image2<double> image(xBound, yBound);
    double maxDValue = static_cast<double>(maxIValue);
    for (size_t i = 0; i < image.GetNumPixels(); ++i)
    {
        image[i] = static_cast<double>(original[i]) / maxDValue;
    }

    SaveImage("head.png", image);

    // Use first-order centered finite differences to estimate the image
    // derivatives.  The gradient is DF = (df/dx, df/dy) and the Hessian
    // is D^2F = {{d^2f/dx^2, d^2f/dxdy}, {d^2f/dydx, d^2f/dy^2}}.
    int32_t xBoundM1 = xBound - 1;
    int32_t yBoundM1 = yBound - 1;
    Image2<double> dx(xBound, yBound);
    Image2<double> dy(xBound, yBound);
    Image2<double> dxx(xBound, yBound);
    Image2<double> dxy(xBound, yBound);
    Image2<double> dyy(xBound, yBound);
    Image2<double> hvx(xBound, yBound);
    Image2<double> hvy(xBound, yBound);
    Image2<double> cross(xBound, yBound);
    Image2<double> lambda(xBound, yBound);

    for (int32_t y = 1; y < yBoundM1; ++y)
    {
        for (int32_t x = 1; x < xBoundM1; ++x)
        {
            dx(x, y) = 0.5 * (image(x + 1, y) - image(x - 1, y));
            dy(x, y) = 0.5 * (image(x, y + 1) - image(x, y - 1));

            dxx(x, y) = image(x + 1, y) - 2.0 * image(x, y) + image(x - 1, y);
            dxy(x, y) = 0.25 * (image(x + 1, y + 1) + image(x - 1, y - 1)
                - image(x + 1, y - 1) - image(x - 1, y + 1));
            dyy(x, y) = image(x, y + 1) - 2.0 * image(x, y) + image(x, y - 1);

            hvx(x, y) = dxx(x, y) * dx(x, y) + dxy(x, y) * dy(x, y);
            hvy(x, y) = dxy(x, y) * dx(x, y) + dyy(x, y) * dy(x, y);

            lambda(x, y) = hvx(x, y) * dx(x, y) + hvy(x, y) * dy(x, y);
            cross(x, y) = hvx(x, y) * dy(x, y) - hvy(x, y) * dx(x, y);

            if ((std::fabs(lambda(x, y)) > 2.0 * std::fabs(hvx(x, y) * dxx(x, y) + hvy(x, y) * dxy(x, y))) &&
                (std::fabs(lambda(x, y)) > 2.0 * std::fabs(hvx(x, y) * dxy(x, y) + hvy(x, y) * dyy(x, y))))
            {
                lambda(x, y) /= dx(x, y) * dx(x, y) + dy(x, y) * dy(x, y);
            }
            else
            {
                lambda(x, y) = 0.0;
                if ((std::fabs(cross(x, y)) < 2.0 * std::fabs(hvx(x, y) * dxy(x, y) - hvy(x, y) * dxx(x, y))) ||
                    (std::fabs(cross(x, y)) < 2.0 * std::fabs(hvx(x, y) * dyy(x, y) - hvy(x, y) * dxy(x, y))))
                {
                    cross(x, y) = 0.0;
                }
            }
        }
    }
    SaveImage("dx.png", dx);
    SaveImage("dy.png", dy);
    SaveImage("dxx.png", dxx);
    SaveImage("dxy.png", dxy);
    SaveImage("dyy.png", dyy);
    SaveImage("hvx.png", hvx);
    SaveImage("hvy.png", hvy);
    SaveImage("cross.png", cross);
    SaveImage("lambda.png", lambda);

    // Use a cheap classification of the pixels by testing for sign changes
    // between neighboring pixels.
    Image2<uint32_t> result(xBound, yBound);
    for (int32_t y = 1; y < yBoundM1; ++y)
    {
        for (int32_t x = 1; x < xBoundM1; ++x)
        {
            uint32_t gray = static_cast<uint32_t>(255.0 * image(x, y));
            bool isRidge = false;
            bool isValley = false;

            if ((cross(x, y) == 0.0) ||
                ((cross(x - 1, y) * cross(x + 1, y)) < 0.0) ||
                ((cross(x, y - 1) * cross(x, y + 1)) < 0.0))
            {
                double traceH = dxx(x, y) + dyy(x, y);

                if (lambda(x, y) < 0.0)
                {
                    if ((traceH - 2.0 * lambda(x, y)) <= 0.0)
                    {
                        isRidge = true;
                    }
                    else if ((traceH - lambda(x, y)) >= 0.0)
                    {
                        isValley = true;
                    }
                }
                else
                {
                    if ((traceH - lambda(x, y)) <= 0.0)
                    {
                        isRidge = true;
                    }
                    else if ((traceH - 2.0 * lambda(x, y)) >= 0.0)
                    {
                        isValley = true;
                    }
                }
            }

            if (isRidge)
            {
                if (isValley)
                {
                    result(x, y) = gray | (gray << 16) | 0xFF000000;  // magenta
                }
                else
                {
                    result(x, y) = gray | 0xFF000000;  // red
                }
            }
            else if (isValley)
            {
                result(x, y) = (gray << 16) | 0xFF000000;  // blue
            }
            else
            {
                result(x, y) = gray | (gray << 8) | (gray << 16) | 0xFF000000;  // gray
            }
        }
    }

    auto texture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM,
        image.GetDimension(0), image.GetDimension(1));
    std::memcpy(texture->GetData(), result.GetPixels().data(), texture->GetNumBytes());
    WICFileIO::SaveToPNG("ridges.png", texture);
}

bool ExtractRidgesConsole::SetEnvironment()
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

void ExtractRidgesConsole::SaveImage(std::string const& name, Image2<double> const& image)
{
    double minValue = image[0], maxValue = minValue;
    for (size_t i = 1; i < image.GetNumPixels(); ++i)
    {
        double value = image[i];
        if (value > maxValue)
        {
            maxValue = value;
        }
        else if (value < minValue)
        {
            minValue = value;
        }
    }
    double mult = 255.0f / (maxValue - minValue);

    auto texture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM,
        image.GetDimension(0), image.GetDimension(1));
    auto texels = texture->Get<uint32_t>();
    for (size_t i = 0; i < image.GetNumPixels(); ++i)
    {
        uint32_t gray = static_cast<uint32_t>(mult * (image[i] - minValue));
        texels[i] = gray | (gray << 8) | (gray << 16) | 0xFF000000;
    }
    WICFileIO::SaveToPNG(name, texture);
}


