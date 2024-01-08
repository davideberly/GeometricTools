// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace gte
{
    template <typename PixelType>
    class Image
    {
    public:
        // Construction and destruction.
        virtual ~Image()
        {
        }

        Image()
        {
        }

        Image(std::vector<int32_t> const& dimensions)
        {
            Reconstruct(dimensions);
        }

        // Support for copy semantics.
        Image(Image const& image)
        {
            *this = image;
        }

        Image& operator=(Image const& image)
        {
            mDimensions = image.mDimensions;
            mOffsets = image.mOffsets;
            mPixels = image.mPixels;
            return *this;
        }

        // Support for move semantics.
        Image(Image&& image) noexcept
        {
            *this = std::move(image);
        }

        Image& operator=(Image&& image) noexcept
        {
            mDimensions = std::move(image.mDimensions);
            mOffsets = std::move(image.mOffsets);
            mPixels = std::move(image.mPixels);
            return *this;
        }

        // Support for changing the image dimensions.  All pixel data is lost
        // by this operation.
        void Reconstruct(std::vector<int32_t> const& dimensions)
        {
            mDimensions.clear();
            mOffsets.clear();
            mPixels.clear();

            if (dimensions.size() > 0)
            {
                for (auto dim : dimensions)
                {
                    if (dim <= 0)
                    {
                        return;
                    }
                }

                mDimensions = dimensions;
                mOffsets.resize(dimensions.size());

                size_t numPixels = 1;
                for (size_t d = 0; d < dimensions.size(); ++d)
                {
                    numPixels *= static_cast<size_t>(mDimensions[d]);
                }

                mOffsets[0] = 1;
                for (size_t d = 1; d < dimensions.size(); ++d)
                {
                    mOffsets[d] = static_cast<size_t>(mDimensions[d - 1]) * mOffsets[d - 1];
                }

                mPixels.resize(numPixels);
            }
        }

        // Access to image data.
        inline std::vector<int32_t> const& GetDimensions() const
        {
            return mDimensions;
        }

        inline int32_t GetNumDimensions() const
        {
            return static_cast<int32_t>(mDimensions.size());
        }

        inline int32_t GetDimension(int32_t d) const
        {
            return mDimensions[d];
        }

        inline std::vector<size_t> const& GetOffsets() const
        {
            return mOffsets;
        }

        inline size_t GetOffset(int32_t d) const
        {
            return mOffsets[d];
        }

        inline std::vector<PixelType> const& GetPixels() const
        {
            return mPixels;
        }

        inline std::vector<PixelType>& GetPixels()
        {
            return mPixels;
        }

        inline size_t GetNumPixels() const
        {
            return mPixels.size();
        }

        // Conversions between n-dim and 1-dim structures.  The 'coord' arrays
        // must have GetNumDimensions() elements.
        size_t GetIndex(int32_t const* coord) const
        {
            // assert:  coord is array of mNumDimensions elements
            int32_t const numDimensions = static_cast<int32_t>(mDimensions.size());
            size_t index = coord[0];
            for (int32_t d = 1; d < numDimensions; ++d)
            {
                index += mOffsets[d] * coord[d];
            }
            return index;
        }

        void GetCoordinates(size_t index, int32_t* coord) const
        {
            // assert:  coord is array of numDimensions elements
            int32_t const numDimensions = static_cast<int32_t>(mDimensions.size());
            for (int32_t d = 0; d < numDimensions; ++d)
            {
                coord[d] = index % mDimensions[d];
                index /= mDimensions[d];
            }
        }

        // Access the data as a 1-dimensional array.  The operator[] functions
        // test for valid i when iterator checking is enabled and assert on
        // invalid i.  The Get() functions test for valid i and clamp when
        // invalid; these functions cannot fail.
        inline PixelType& operator[] (size_t i)
        {
            return mPixels[i];
        }

        inline PixelType const& operator[] (size_t i) const
        {
            return mPixels[i];
        }

        PixelType& Get(size_t i)
        {
            return (i < mPixels.size() ? mPixels[i] : mPixels.front());
        }

        PixelType const& Get(size_t i) const
        {
            return (i < mPixels.size() ? mPixels[i] : mPixels.front());
        }

    protected:
        std::vector<int32_t> mDimensions;
        std::vector<size_t> mOffsets;
        std::vector<PixelType> mPixels;
    };
}
