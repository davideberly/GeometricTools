// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2026.02.21

#pragma once

#include <cstddef>
#include <mutex>
#include <queue>

namespace gte
{
    template <typename Element>
    class ThreadSafeQueue
    {
    public:
        // Construction and destruction.
        ThreadSafeQueue(size_t maxNumElements = 0)
            :
            mMaxNumElements(maxNumElements)
        {
        }

        virtual ~ThreadSafeQueue() = default;

        // All the operations are thread-safe.
        size_t GetMaxNumElements() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mMaxNumElements;
        }

        size_t GetNumElements() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mQueue.size();
        }

        bool Push(Element const& element)
        {
            bool pushed;
            std::lock_guard<std::mutex> lock(mMutex);
            if (mQueue.size() < mMaxNumElements)
            {
                mQueue.push(element);
                pushed = true;
            }
            else
            {
                pushed = false;
            }
            return pushed;
        }

        bool Pop(Element& element)
        {
            bool popped;
            std::lock_guard<std::mutex> lock(mMutex);
            if (mQueue.size() > 0)
            {
                element = mQueue.front();
                mQueue.pop();
                popped = true;
            }
            else
            {
                popped = false;
            }
            return popped;
        }

    protected:
        size_t mMaxNumElements;
        std::queue<Element> mQueue;
        mutable std::mutex mMutex;
    };
}


