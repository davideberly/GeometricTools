// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "AdaptiveSkeletonClimbing3Console.h"
#include <Mathematics/AdaptiveSkeletonClimbing3.h>
#include <Mathematics/ImageUtility3.h>

AdaptiveSkeletonClimbing3Console::AdaptiveSkeletonClimbing3Console(Parameters& parameters)
    :
    Console(parameters)
{
}

void AdaptiveSkeletonClimbing3Console::Execute()
{
    // Create a 65x65x65 image by summing two Gaussian distributions.
    int32_t const N = 6, bound = (1 << N) + 1;
    Image3<int32_t> image(bound, bound, bound);
    std::fill(image.GetPixels().begin(), image.GetPixels().end(), 0);

    float a0 = 256.0f, a1 = 128.0f;
    float x0 = 0.5f * bound, y0 = 0.0f, z0 = 0.0f;
    float x1 = 0.75f * bound, y1 = 0.0f, z1 = 0.0f;
    float xs0 = 2.0f * bound, ys0 = 4.0f * bound, zs0 = 8.0f * bound;
    float xs1 = 8.0f * bound, ys1 = 4.0f * bound, zs1 = 2.0f * bound;

    for (int32_t z = 0; z < bound; ++z)
    {
        float vz0 = (z - z0) / zs0, vz1 = (z - z1) / zs1;
        vz0 *= vz0;
        vz1 *= vz1;
        for (int32_t y = 0; y < bound; ++y)
        {
            float vy0 = (y - y0) / ys0, vy1 = (y - y1) / ys1;
            vy0 *= vy0;
            vy1 *= vy1;
            for (int32_t x = 0; x < bound; ++x)
            {
                float vx0 = (x - x0) / xs0, vx1 = (x - x1) / xs1;
                vx0 *= vx0;
                vx1 *= vx1;

                float g0 = a0 * std::exp(-(vx0 + vy0 + vz0));
                float g1 = a1 * std::exp(-(vx1 + vy1 + vz1));
                image(x, y, z) = static_cast<int32_t>(g0 + g1);
            }
        }
    }

    // Extract a level set from the image.
    AdaptiveSkeletonClimbing3<int32_t, float> climb(N, image.GetPixels().data());
    std::vector<std::array<float, 3>> vertices;
    std::vector<TriangleKey<true>> triangles;
    std::vector<std::array<float, 3>> normals;
    climb.Extract(349.5f, -1, vertices, triangles);
    climb.MakeUnique(vertices, triangles);
    climb.OrientTriangles(vertices, triangles, false);
    climb.ComputeNormals(vertices, triangles, normals);

    std::ofstream output("vtdata.txt");
    output << vertices.size() << std::endl;
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        auto const& vertex = vertices[i];
        output << vertex[0] << " " << vertex[1] << " " << vertex[2] << std::endl;
    }
    output << std::endl;

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        auto const& normal = normals[i];
        output << normal[0] << " " << normal[1] << " " << normal[2] << std::endl;
    }
    output << std::endl;

    output << triangles.size() << std::endl;
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        auto const& triangle = triangles[i];
        output << triangle.V[0] << " " << triangle.V[1] << " " << triangle.V[2] << std::endl;
    }
    output.close();

    output.open("boxes.txt");
    climb.PrintBoxes(output);
    output.close();
}
