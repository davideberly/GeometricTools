// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2026.08.07

#pragma once

// Image utilities for Image3<std::int32_t> objects. TODO: Extend this to a
// template class to allow the pixel type to be std::int32_t*_t and uint*_t
// for * in {8,16,32,64}.
//
// All but the Draw* functions are operations on binary images. Let the image
// have d0 columns, d1 rows, and d2 slices. The input image must have zeros on
// its boundaries x = 0, x = d0-1, y = 0, y = d1-1, z = 0, and z = d2-1. The
// 0-valued voxels are considered to be background. The 1-valued voxels are
// considered to be foreground. In some of the operations, to save memory and
// time the input image is modified by the algorithms. If you need to preserve
// the input image, make a copy of it before calling these functions.

#include <Mathematics/Image3.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace gte
{
    class ImageUtility3
    {
    public:
        // Compute the N-connected components of a binary image
        // (N is 6, 18, or 26). The input image is modified to avoid the cost
        // of making a copy. On output, the image values are the labels for
        // the components. The array components[k], k >= 1, contains the
        // indices for the k-th component.
        template <std::size_t N>
        static void GetComponents(
            Image3<std::int32_t>& image,
            std::vector<std::vector<std::size_t>>& components)
        {
            static_assert(N == 6 || N == 18 || N == 26, "Invalid neighborhood type.");

            std::array<std::int32_t, N> neighbors{};
            image.GetNeighborhood(neighbors);
            GetComponents(image, neighbors.size(), neighbors.data(), components);
        }

        // Connected component labeling using depth-first search. The
        // neighborhood is specified by the caller as 1-dimensional indices
        // into the image.
        static void GetComponents(
            Image3<std::int32_t>& image, 
            std::size_t numNeighbors,
            std::int32_t const* neighbors,
            std::vector<std::vector<std::size_t>>& components)
        {
            LogAssert(
                numNeighbors > 0 && neighbors != nullptr,
                "Invalid neighbors.");

            std::size_t const numVoxels = image.GetNumPixels();
            std::vector<std::int32_t> numElements(numVoxels);
            std::vector<std::size_t> vstack(numVoxels);
            std::size_t numComponents = 0;
            std::int32_t label = 2;
            for (std::size_t i = 0; i < numVoxels; ++i)
            {
                if (image[i] == 1)
                {
                    std::int32_t top = -1;
                    vstack[++top] = i;

                    std::int32_t& count = numElements[numComponents + 1];
                    count = 0;
                    while (top >= 0)
                    {
                        std::size_t v = vstack[top];
                        image[v] = -1;
                        std::int32_t j;
                        for (j = 0; j < numNeighbors; ++j)
                        {
                            std::size_t adj = v + neighbors[j];
                            if (image[adj] == 1)
                            {
                                vstack[++top] = adj;
                                break;
                            }
                        }
                        if (j == numNeighbors)
                        {
                            image[v] = label;
                            ++count;
                            --top;
                        }
                    }

                    ++numComponents;
                    ++label;
                }
            }

            if (numComponents > 0)
            {
                components.resize(numComponents + 1);
                for (std::size_t i = 1; i <= numComponents; ++i)
                {
                    components[i].resize(numElements[i]);
                    numElements[i] = 0;
                }

                for (std::size_t i = 0; i < numVoxels; ++i)
                {
                    std::int32_t value = image[i];
                    if (value != 0)
                    {
                        // Labels started at 2 to support the depth-first
                        // search, so they need to be decremented for the
                        // correct labels.
                        image[i] = --value;
                        components[value][numElements[value]] = i;
                        ++numElements[value];
                    }
                }
            }
        }

        // Compute a dilation with a structuring element consisting of the
        // N-connected neighbors of each voxel (N is 6, 18, or 26). The input
        // image is binary with 0 for background and 1 for foreground. The
        // output image must be an object different from the input image.
        template <std::size_t N>
        static void Dilate(
            Image3<std::int32_t> const& inImage,
            Image3<std::int32_t>& outImage)
        {
            static_assert(N == 6 || N == 18 || N == 26, "Invalid neighborhood type.");

            std::array<std::array<std::int32_t, 3>, N> neighbors{};
            inImage.GetNeighborhood(neighbors);
            Dilate(inImage, neighbors.size(), neighbors.data(), outImage);
        }

        // Compute a dilation with a structing element consisting of neighbors
        // specified by offsets relative to the voxel. The input image is
        // binary with 0 for background and 1 for foreground. The output
        // image must be an object different from the input image.
        static void Dilate(
            Image3<std::int32_t> const& inImage,
            std::size_t numNeighbors,
            std::array<std::int32_t, 3> const* neighbors,
            Image3<std::int32_t>& outImage)
        {
            LogAssert(&outImage != &inImage, "Input and output must be different.");

            outImage = inImage;

            // If the voxel at (i0,i1,i2) is 1, then the voxels at
            // (k0,k1,k2) = (i0+nbr0,i1+nbr1,i2+nbr2) are set to 1 where
            // (nbr0,nbr1,nbr2) is in the neighbors array. Boundary
            // testing is used to avoid accessing out-of-range pixels.
            std::int32_t const dim0 = inImage.GetDimension(0);
            std::int32_t const dim1 = inImage.GetDimension(1);
            std::int32_t const dim2 = inImage.GetDimension(2);
            for (std::int32_t i2 = 0; i2 < dim2; ++i2)
            {
                for (std::int32_t i1 = 0; i1 < dim1; ++i1)
                {
                    for (std::int32_t i0 = 1; i0 < dim0; ++i0)
                    {
                        if (inImage(i0, i1, i2) == 1)
                        {
                            for (std::size_t n = 0; n < numNeighbors; ++n)
                            {
                                std::int32_t k0 = i0 + neighbors[n][0];
                                std::int32_t k1 = i1 + neighbors[n][1];
                                std::int32_t k2 = i2 + neighbors[n][2];
                                if (0 <= k0 && k0 < dim0 &&
                                    0 <= k1 && k1 < dim1 &&
                                    0 <= k2 && k2 < dim2)
                                {
                                    outImage(k0, k1, k2) = 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Compute an erosion with a structuring element consisting of the
        // N-connected neighbors of each voxel (N is 6, 18, or 26). The input
        // image is binary with 0 for background and 1 for foreground. The
        // output/ image must be an object different from the input image. If
        // zeroExterior is true, the image exterior is assumed to be 0, so
        // 1-valued boundary voxels are set to 0; otherwise, boundary voxels
        // are set to 0 only when they have neighboring image voxels that
        // are 0.
        template <std::size_t N>
        static void Erode(
            Image3<std::int32_t> const& inImage,
            bool zeroExterior,
            Image3<std::int32_t>& outImage)
        {
            static_assert(N == 6 || N == 18 || N == 26, "Invalid neighborhood type.");

            std::array<std::array<std::int32_t, 3>, N> neighbors{};
            inImage.GetNeighborhood(neighbors);
            Erode(inImage, zeroExterior, neighbors.size(), neighbors.data(), outImage);
        }

        // Compute an erosion with a structuring element consisting of
        // neighbors specified by offsets relative to the voxel. The input
        // image is binary with 0 for background and 1 for foreground. The
        // output image must be an object different from the input image. If
        // zeroExterior is true, the image exterior is assumed to be 0, so
        // 1-valued boundary voxels are set to 0; otherwise, boundary voxels
        // are set to 0 only when they have neighboring image voxels that
        // are 0.
        static void Erode(
            Image3<std::int32_t> const& inImage,
            bool zeroExterior,
            std::size_t numNeighbors,
            std::array<std::int32_t, 3> const* neighbors,
            Image3<std::int32_t>& outImage)
        {
            LogAssert(
                numNeighbors > 0 && neighbors != nullptr,
                "Invalid neighbors.");

            LogAssert(
                &outImage != &inImage,
                "The input and output images must be different.");

            outImage = inImage;

            // If the pixel at (i0,i1,i2) is 1, it is changed to 0 when at
            // least one neighbor (k0,k1,k2) = (i0+nbr0,i1+nbr1,i2+nbr2) is 0,
            // where (nbr0,nbr1,nbr2) is in the neighbors array.
            std::int32_t const dim0 = inImage.GetDimension(0);
            std::int32_t const dim1 = inImage.GetDimension(1);
            std::int32_t const dim2 = inImage.GetDimension(2);
            for (std::int32_t i2 = 0; i2 < dim2; ++i2)
            {
                for (std::int32_t i1 = 0; i1 < dim1; ++i1)
                {
                    for (std::int32_t i0 = 0; i0 < dim0; ++i0)
                    {
                        if (inImage(i0, i1, i2) == 1)
                        {
                            for (std::size_t j = 0; j < numNeighbors; ++j)
                            {
                                std::int32_t k0 = i0 + neighbors[j][0];
                                std::int32_t k1 = i1 + neighbors[j][1];
                                std::int32_t k2 = i2 + neighbors[j][2];
                                if (0 <= k0 && k0 < dim0 &&
                                    0 <= k1 && k1 < dim1 &&
                                    0 <= k2 && k2 < dim2)
                                {
                                    if (inImage(k0, k1, k2) == 0)
                                    {
                                        outImage(i0, i1, i2) = 0;
                                        break;
                                    }
                                }
                                else if (zeroExterior)
                                {
                                    outImage(i0, i1, i2) = 0;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Compute an opening with a structuring element consisting of the
        // N-connected neighbors of each pixel (N is 6, 18, or 26). The input image
        // is binary with 0 for background and 1 for foreground. The output
        // image must be an object different from the input image. If
        // zeroExterior is true, the image exterior is assumed to consist of
        // 0-valued pixels; otherwise, the image exterior is assumed to
        // consist of 1-valued pixels.
        template <std::size_t N>
        static void Open(
            Image3<std::int32_t> const& inImage,
            bool zeroExterior,
            Image3<std::int32_t>& outImage)
        {
            static_assert(N == 6 || N == 18 || N == 26, "Invalid neighborhood type.");

            Image3<std::int32_t> temp(inImage.GetDimension(0), inImage.GetDimension(1), inImage.GetDimension(2));
            Erode<N>(inImage, zeroExterior, temp);
            Dilate<N>(temp, outImage);
        }

        // Compute an opening with a structuring element consisting of
        // neighbors specified by offsets relative to the voxel. The input
        // image is binary with 0 for background and 1 for foreground. The
        // output image must be an object different from the input image. If
        // zeroExterior is true, the image exterior is assumed to consist of
        // 0-valued voxels; otherwise, the image exterior is assumed to
        // consist of 1-valued voxels.
        static void Open(
            Image3<std::int32_t> const& inImage,
            bool zeroExterior,
            std::size_t numNeighbors,
            std::array<std::int32_t, 3> const* neighbors,
            Image3<std::int32_t>& outImage)
        {
            Image3<std::int32_t> temp(inImage.GetDimension(0), inImage.GetDimension(1), inImage.GetDimension(2));
            Erode(inImage, zeroExterior, numNeighbors, neighbors, temp);
            Dilate(temp, numNeighbors, neighbors, outImage);
        }

        // Compute a closing with a structuring element consisting of the
        // N-connected neighbors of each voxel. The input image is binary
        // with 0 for background and 1 for foreground. The output image must
        // be an object different from the input image. If zeroExterior is
        // true, the image exterior is assumed to consist of 0-valued voxels;
        // otherwise, the image exterior is assumed to consist of 1-valued
        // voxels.
        template <std::size_t N>
        static void Close(
            Image3<std::int32_t> const& inImage,
            bool zeroExterior,
            Image3<std::int32_t>& outImage)
        {
            static_assert(N == 4 || N == 8, "Invalid neighborhood type.");

            Image3<std::int32_t> temp(inImage.GetDimension(0), inImage.GetDimension(1));
            Dilate<N>(inImage, temp);
            Erode<N>(temp, zeroExterior, outImage);
        }

        // Compute a closing with a structuring element consisting of
        // neighbors specified by offsets relative to the voxel. The input
        // image is binary with 0 for background and 1 for foreground. The
        // output image must be an object different from the input image. If
        // zeroExterior is true, the image exterior is assumed to consist of
        // 0-valued voxels; otherwise, the image exterior is assumed to
        // consist of 1-valued voxels.
        static void Close(
            Image3<std::int32_t> const& inImage,
            bool zeroExterior,
            std::size_t numNeighbors,
            std::array<std::int32_t, 3> const* neighbors,
            Image3<std::int32_t>& outImage)
        {
            Image3<std::int32_t> temp(inImage.GetDimension(0), inImage.GetDimension(1), inImage.GetDimension(2));
            Dilate(inImage, numNeighbors, neighbors, temp);
            Erode(temp, zeroExterior, numNeighbors, neighbors, outImage);
        }

        // Compute coordinate-directional convex set. For a given coordinate
        // direction (x, y, or z), identify the first and last 1-valued voxels
        // on a segment of voxels in that direction. All voxels from first to
        // last are set to 1. This is done for all segments in each of the
        // coordinate directions.
        static void ComputeCDConvex(
            Image3<std::int32_t>& image)
        {
            std::int32_t const dim0 = image.GetDimension(0);
            std::int32_t const dim1 = image.GetDimension(1);
            std::int32_t const dim2 = image.GetDimension(2);

            Image3<std::int32_t> temp = image;
            std::int32_t i0{}, i1{}, i2{};
            for (i1 = 0; i1 < dim1; ++i1)
            {
                for (i0 = 0; i0 < dim0; ++i0)
                {
                    std::int32_t i2min;
                    for (i2min = 0; i2min < dim2; ++i2min)
                    {
                        if ((temp(i0, i1, i2min) & 1) == 0)
                        {
                            temp(i0, i1, i2min) |= 2;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if (i2min < dim2)
                    {
                        std::int32_t i2max;
                        for (i2max = dim2 - 1; i2max >= i2min; --i2max)
                        {
                            if ((temp(i0, i1, i2max) & 1) == 0)
                            {
                                temp(i0, i1, i2max) |= 2;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
            }

            for (i2 = 0; i2 < dim2; ++i2)
            {
                for (i0 = 0; i0 < dim0; ++i0)
                {
                    std::int32_t i1min;
                    for (i1min = 0; i1min < dim1; ++i1min)
                    {
                        if ((temp(i0, i1min, i2) & 1) == 0)
                        {
                            temp(i0, i1min, i2) |= 2;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if (i1min < dim1)
                    {
                        std::int32_t i1max;
                        for (i1max = dim1 - 1; i1max >= i1min; --i1max)
                        {
                            if ((temp(i0, i1max, i2) & 1) == 0)
                            {
                                temp(i0, i1max, i2) |= 2;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
            }

            for (i2 = 0; i2 < dim2; ++i2)
            {
                for (i1 = 0; i1 < dim1; ++i1)
                {
                    std::int32_t i0min;
                    for (i0min = 0; i0min < dim0; ++i0min)
                    {
                        if ((temp(i0min, i1, i2) & 1) == 0)
                        {
                            temp(i0min, i1, i2) |= 2;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if (i0min < dim0)
                    {
                        std::int32_t i0max;
                        for (i0max = dim0 - 1; i0max >= i0min; --i0max)
                        {
                            if ((temp(i0max, i1, i2) & 1) == 0)
                            {
                                temp(i0max, i1, i2) |= 2;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
            }

            for (std::size_t i = 0; i < image.GetNumPixels(); ++i)
            {
                image[i] = (temp[i] & 2 ? 0 : 1);
            }
        }

        // Use a depth-first search for filling a 6-connected region. This is
        // nonrecursive, simulated by using a heap-allocated "stack". The input
        // (x,y,z) is the seed point that starts the fill.
        template <typename PixelType>
        static void FloodFill6(
            Image3<PixelType>& image,
            std::int32_t x,
            std::int32_t y,
            std::int32_t z,
            PixelType foreColor,
            PixelType backColor)
        {
            // Test for a valid seed.
            std::int32_t const dim0 = image.GetDimension(0);
            std::int32_t const dim1 = image.GetDimension(1);
            std::int32_t const dim2 = image.GetDimension(2);
            if (x < 0 || x >= dim0 || y < 0 || y >= dim1 || z < 0 || z >= dim2)
            {
                // The seed point is outside the image domain, so there is
                // nothing to fill.
                return;
            }

            // Allocate the maximum amount of space needed for the stack. An
            // empty stack has top == -1.
            std::size_t const numVoxels = image.GetNumPixels();
            std::vector<std::int32_t> xStack(numVoxels), yStack(numVoxels), zStack(numVoxels);

            // Push seed point onto stack if it has the background color. All
            // points pushed onto stack have background color backColor.
            std::int32_t top = 0;
            xStack[top] = x;
            yStack[top] = y;
            zStack[top] = z;

            while (top >= 0)  // stack is not empty
            {
                // Read top of stack. Do not pop since we need to return to
                // this top value later to restart the fill in a different
                // direction.
                x = xStack[top];
                y = yStack[top];
                z = zStack[top];

                // Fill the pixel.
                image(x, y, z) = foreColor;

                std::int32_t xp1 = x + 1;
                if (xp1 < dim0 && image(xp1, y, z) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = xp1;
                    yStack[top] = y;
                    zStack[top] = z;
                    continue;
                }

                std::int32_t xm1 = x - 1;
                if (0 <= xm1 && image(xm1, y, z) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = xm1;
                    yStack[top] = y;
                    zStack[top] = z;
                    continue;
                }

                std::int32_t yp1 = y + 1;
                if (yp1 < dim1 && image(x, yp1, z) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = x;
                    yStack[top] = yp1;
                    zStack[top] = z;
                    continue;
                }

                std::int32_t ym1 = y - 1;
                if (0 <= ym1 && image(x, ym1, z) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = x;
                    yStack[top] = ym1;
                    zStack[top] = z;
                    continue;
                }

                std::int32_t zp1 = z + 1;
                if (zp1 < dim2 && image(x, y, zp1) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = x;
                    yStack[top] = y;
                    zStack[top] = zp1;
                    continue;
                }

                std::int32_t zm1 = z - 1;
                if (0 <= zm1 && image(x, y, zm1) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = x;
                    yStack[top] = y;
                    zStack[top] = zm1;
                    continue;
                }

                // Done in all directions, pop and return to search other
                // directions for the predecessor.
                --top;
            }
        }

        // Visit pixels using Bresenham's line drawing algorithm. The callback
        // represents the action you want applied to each voxel as it is visited.
        static void DrawLine(
            std::int32_t x0,
            std::int32_t y0,
            std::int32_t z0,
            std::int32_t x1,
            std::int32_t y1,
            std::int32_t z1,
            std::function<void(std::int32_t, std::int32_t, std::int32_t)> const& callback)
        {
            // Starting point of line.
            std::int32_t x = x0, y = y0, z = z0;

            // Direction of line.
            std::int32_t dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;

            // Increment or decrement depending on direction of line.
            std::int32_t sx = (dx > 0 ? 1 : (dx < 0 ? -1 : 0));
            std::int32_t sy = (dy > 0 ? 1 : (dy < 0 ? -1 : 0));
            std::int32_t sz = (dz > 0 ? 1 : (dz < 0 ? -1 : 0));

            // Decision parameters for voxel selection.
            if (dx < 0)
            {
                dx = -dx;
            }
            if (dy < 0)
            {
                dy = -dy;
            }
            if (dz < 0)
            {
                dz = -dz;
            }
            std::int32_t ax = 2 * dx, ay = 2 * dy, az = 2 * dz;
            std::int32_t decX{}, decY{}, decZ{};

            // Determine largest direction component, single-step related
            // variable.
            std::int32_t maxValue = dx, var = 0;
            if (dy > maxValue)
            {
                maxValue = dy;
                var = 1;
            }
            if (dz > maxValue)
            {
                var = 2;
            }

            // Traverse Bresenham line.
            switch (var)
            {
            case 0:  // Single-step in x-direction.
                decY = ay - dx;
                decZ = az - dx;
                for (/**/; /**/; x += sx, decY += ay, decZ += az)
                {
                    // Process voxel.
                    callback(x, y, z);

                    // Take Bresenham step.
                    if (x == x1)
                    {
                        break;
                    }
                    if (decY >= 0)
                    {
                        decY -= ax;
                        y += sy;
                    }
                    if (decZ >= 0)
                    {
                        decZ -= ax;
                        z += sz;
                    }
                }
                break;
            case 1:  // Single-step in y-direction.
                decX = ax - dy;
                decZ = az - dy;
                for (/**/; /**/; y += sy, decX += ax, decZ += az)
                {
                    // Process voxel.
                    callback(x, y, z);

                    // Take Bresenham step.
                    if (y == y1)
                    {
                        break;
                    }
                    if (decX >= 0)
                    {
                        decX -= ay;
                        x += sx;
                    }
                    if (decZ >= 0)
                    {
                        decZ -= ay;
                        z += sz;
                    }
                }
                break;
            case 2:  // Single-step in z-direction.
                decX = ax - dz;
                decY = ay - dz;
                for (/**/; /**/; z += sz, decX += ax, decY += ay)
                {
                    // Process voxel.
                    callback(x, y, z);

                    // Take Bresenham step.
                    if (z == z1)
                    {
                        break;
                    }
                    if (decX >= 0)
                    {
                        decX -= az;
                        x += sx;
                    }
                    if (decY >= 0)
                    {
                        decY -= az;
                        y += sy;
                    }
                }
                break;
            }
        }
    };
}


