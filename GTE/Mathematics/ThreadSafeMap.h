// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2026.02.21

#pragma once

#include <map>
#include <mutex>
#include <vector>

namespace gte
{
    template <typename Key, typename Value>
    class ThreadSafeMap
    {
    public:
        // Construction and destruction.
        ThreadSafeMap() = default;
        virtual ~ThreadSafeMap() = default;

        // All the operations are thread-safe.
        bool HasElements() const
        {
            bool hasElements;
            std::lock_guard<std::mutex> lock(mMutex);
            hasElements = (mMap.size() > 0);
            return hasElements;
        }

        bool Exists(Key key) const
        {
            bool exists;
            std::lock_guard<std::mutex> lock(mMutex);
            exists = (mMap.find(key) != mMap.end());
            return exists;
        }

        void Insert(Key key, Value value)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mMap[key] = value;
        }

        bool Remove(Key key, Value& value)
        {
            bool exists;
            std::lock_guard<std::mutex> lock(mMutex);
            auto iter = mMap.find(key);
            if (iter != mMap.end())
            {
                value = iter->second;
                mMap.erase(iter);
                exists = true;
            }
            else
            {
                exists = false;
            }
            return exists;
        }

        void RemoveAll()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mMap.clear();
        }

        bool Get(Key key, Value& value) const
        {
            bool exists;
            std::lock_guard<std::mutex> lock(mMutex);
            auto iter = mMap.find(key);
            if (iter != mMap.end())
            {
                value = iter->second;
                exists = true;
            }
            else
            {
                exists = false;
            }
            return exists;
        }

        void GatherAll(std::vector<Value>& values) const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (mMap.size() > 0)
            {
                values.resize(mMap.size());
                auto viter = values.begin();
                for (auto const& m : mMap)
                {
                    *viter++ = m.second;
                }
            }
            else
            {
                values.clear();
            }
        }

    protected:
        std::map<Key, Value> mMap;
        mutable std::mutex mMutex;
    };
}


