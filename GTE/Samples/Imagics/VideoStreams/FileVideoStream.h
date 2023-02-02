// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include "VideoStream.h"
#include <fstream>

class FileVideoStream : public VideoStream
{
public:
    // Construction and destruction.  The input file is binary with the
    // following structure:
    //   uint32_t numImages;
    //   DFType type;
    //   uint32_t width;
    //   uint32_t height;
    //   struct { uint32_t frameNumber; char data[N]; } image[numImages];
    // where N = width*height*TextureFormat::GetNumBytesPerTexel(type).  The
    // engine that stores the file images in GPU memory must be provided to the
    // constructor.  NOTE: No error checking is performed in the constructor.
    // The 'engine' must be nonnull and the file must exist and be of the
    // correct format.
    virtual ~FileVideoStream();
    FileVideoStream(std::string const& filename,
        std::shared_ptr<gte::GraphicsEngine> const& engine);

    // Access to input information.
    inline std::string const& GetFilename() const
    {
        return mFilename;
    }

    inline uint32_t GetNumImages() const
    {
        return mNumImages;
    }

private:
    // The override that is called by VideoStream::CaptureFrame().
    virtual char* GetImage();

    // The file and information corresponding to the video stream.  The
    // current image is tracked so that when it reaches the number of images,
    // it is wrapped around to zero.
    std::string mFilename;
    std::ifstream mInput;
    uint32_t mNumImages, mCurrentImage;

    // A temporary buffer for reading images from disk.
    std::vector<char> mBuffer;
};
