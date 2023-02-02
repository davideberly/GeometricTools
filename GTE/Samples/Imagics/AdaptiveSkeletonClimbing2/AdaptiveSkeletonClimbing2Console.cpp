// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "AdaptiveSkeletonClimbing2Console.h"
#include <Applications/WICFileIO.h>
#include <Mathematics/AdaptiveSkeletonClimbing2.h>
#include <Mathematics/ImageUtility2.h>

AdaptiveSkeletonClimbing2Console::AdaptiveSkeletonClimbing2Console(Parameters& parameters)
    :
    Console(parameters)
{
}

void AdaptiveSkeletonClimbing2Console::Execute()
{
    Test0();
    Test1();
}

void AdaptiveSkeletonClimbing2Console::Test0()
{
    // 9x9 image
    std::array<int32_t, 81> image =
    {
        +1, -1, -1, +1, -1, -1, +1, -1, +1,
        +1, +1, +1, +1, +1, +1, +1, +1, +1,
        -1, -1, +1, +1, +1, +1, -1, -1, -1,
        -1, -1, +1, +1, +1, +1, -1, -1, -1,
        -1, -1, +1, +1, +1, +1, -1, -1, -1,
        +1, -1, -1, -1, -1, -1, -1, +1, +1,
        -1, -1, +1, -1, -1, -1, -1, -1, +1,
        +1, +1, +1, -1, -1, -1, +1, -1, -1,
        -1, +1, -1, +1, +1, +1, -1, +1, +1
    };

    AdaptiveSkeletonClimbing2<int32_t, float> climb(3, image.data());
    std::vector<std::array<float, 2>> vertices;
    std::vector<std::array<int32_t, 2>> edges;
    climb.Extract(0.0f, -1, vertices, edges);
    climb.MakeUnique(vertices, edges);

    std::ofstream output("vedata0.txt");
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        auto const& vertex = vertices[i];
        output << i << " " << vertex[0] << " , " << vertex[1] << std::endl;
    }
    output << std::endl;

    for (size_t i = 0; i < edges.size(); ++i)
    {
        auto const& edge = edges[i];
        output << i << " " << edge[0] << " " << edge[1] << std::endl;
    }
    output.close();
}

void AdaptiveSkeletonClimbing2Console::Test1()
{
    Image2<uint32_t> image(257, 257);
    std::fill(image.GetPixels().begin(), image.GetPixels().end(), 0);
    uint32_t const initial = 100;
    for (int32_t y = 32; y < 224; ++y)
    {
        for (int32_t x = 64; x < 192; ++x)
        {
            image(x, y) = initial;
        }
    }

    Image2<uint32_t> blur(257, 257);
    std::fill(blur.GetPixels().begin(), blur.GetPixels().end(), 0);
    for (int32_t i = 1; i <= 8; ++i)
    {
        for (int32_t y = 0; y < 257; ++y)
        {
            for (int32_t x = 0; x < 257; ++x)
            {
                if (image(x, y) != 0)
                {
                    uint32_t sum = 0;
                    for (int32_t dy = -1; dy <= 1; ++dy)
                    {
                        for (int32_t dx = -1; dx <= 1; ++dx)
                        {
                            sum += image(x + dx, y + dy);
                        }
                    }
                    blur(x, y) = sum / 9;
                }
            }
        }
        image = blur;
    }

    auto texture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, 257, 257);
    auto texels = texture->Get<uint32_t>();
    for (int32_t i = 0; i < 257 * 257; ++i)
    {
        uint32_t gray = 255 * image[i] / initial;
        texels[i] = gray | (gray << 8) | (gray << 16) | 0xFF000000;
    }
    WICFileIO::SaveToPNG("blur.png", texture);

    AdaptiveSkeletonClimbing2<uint32_t, float> climb(8, image.GetPixels().data());
    std::vector<std::array<float, 2>> vertices;
    std::vector<std::array<int32_t, 2>> edges;
    climb.Extract(75.5f, 0, vertices, edges);
    climb.MakeUnique(vertices, edges);
    std::ofstream output("vedata1.txt");
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        auto const& vertex = vertices[i];
        output << i << " " << vertex[0] << " , " << vertex[1] << std::endl;
    }
    output << std::endl;

    for (size_t i = 0; i < edges.size(); ++i)
    {
        auto const& edge = edges[i];
        output << i << " " << edge[0] << " " << edge[1] << std::endl;
    }
    output.close();

    // Copy the gray-scale blurred image to the color image.
    Image2<uint32_t> color(257, 257);
    for (int32_t i = 0; i < 257 * 257; ++i)
    {
        uint32_t gray = 255 * image[i] / initial;
        color[i] = gray | (gray << 8) | (gray << 16) | 0xFF000000;
    }

    // Draw vertices.
    std::function<void(int32_t, int32_t)> SetPixel = [&color](int32_t x, int32_t y)
    {
        color(x, y) = 0xFF0000FF;
    };

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        auto const& vertex = vertices[i];
        int32_t x = static_cast<int32_t>(vertex[0]);
        int32_t y = static_cast<int32_t>(vertex[1]);
        ImageUtility2::DrawThickPixel(x, y, 1, SetPixel);
    }

    // Draw edges.
    for (size_t i = 0; i < edges.size(); i++)
    {
        auto const& edge = edges[i];
        int32_t x0 = (int32_t)vertices[edge[0]][0];
        int32_t y0 = (int32_t)vertices[edge[0]][1];
        int32_t x1 = (int32_t)vertices[edge[1]][0];
        int32_t y1 = (int32_t)vertices[edge[1]][1];
        ImageUtility2::DrawLine(x0, y0, x1, y1, SetPixel);
    }

    for (int32_t i = 0; i < 257 * 257; ++i)
    {
        texels[i] = color[i];
    }
    WICFileIO::SaveToPNG("color.png", texture);
}
