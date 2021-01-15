// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "FileVideoStream.h"
using namespace gte;

FileVideoStream::~FileVideoStream()
{
    mInput.close();
}

FileVideoStream::FileVideoStream(std::string const& filename,
    std::shared_ptr<gte::GraphicsEngine> const& engine)
    :
    VideoStream(engine),
    mFilename(filename),
    mNumImages(0),
    mCurrentImage(0)
{
    mInput.open(filename, std::ios::in | std::ios::binary);
    mInput.read((char*)&mNumImages, sizeof(mNumImages));
    mInput.read((char*)&mType, sizeof(mType));
    mInput.read((char*)&mWidth, sizeof(mWidth));
    mInput.read((char*)&mHeight, sizeof(mHeight));

    mBuffer.resize(mWidth*mHeight*DataFormat::GetNumBytesPerStruct(mType));
}

char* FileVideoStream::GetImage()
{
    mInput.read((char*)&mFrame.number, sizeof(mFrame.number));
    mInput.read(&mBuffer[0], mBuffer.size());
    if (++mCurrentImage == mNumImages)
    {
        mCurrentImage = 0;

        // Seek to the first image after the file header.
        mInput.seekg(sizeof(mNumImages) + sizeof(mType) + sizeof(mWidth) +
            sizeof(mHeight));
    }
    return &mBuffer[0];
}
