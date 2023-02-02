// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <array>

class MTEdge
{
public:
    MTEdge(int32_t label = -1)
        :
        mLabel(label),
        mVertex{ -1, -1 },
        mTriangle{ -1, -1 }
    {
    }

    ~MTEdge() = default;

    inline int32_t GetLabel() const
    {
        return mLabel;
    }

    inline void SetLabel(int32_t label)
    {
        mLabel = label;
    }

    inline int32_t GetVertex(int32_t i) const
    {
        return mVertex[i];
    }

    inline void SetVertex(int32_t i, int32_t label)
    {
        mVertex[i] = label;
    }

    bool ReplaceVertex(int32_t vOld, int32_t vNew)
    {
        for (int32_t i = 0; i < 2; ++i)
        {
            if (mVertex[i] == vOld)
            {
                mVertex[i] = vNew;
                return true;
            }
        }
        return false;
    }

    inline int32_t GetTriangle(int32_t i) const
    {
        return mTriangle[i];
    }

    inline void SetTriangle(int32_t i, int32_t label)
    {
        mTriangle[i] = label;
    }

    bool ReplaceTriangle(int32_t tOld, int32_t tNew)
    {
        for (int32_t i = 0; i < 2; ++i)
        {
            if (mTriangle[i] == tOld)
            {
                mTriangle[i] = tNew;
                return true;
            }
        }
        return false;
    }

    inline bool operator==(MTEdge const& other) const
    {
        return
            (mVertex[0] == other.mVertex[0] &&
             mVertex[1] == other.mVertex[1]) ||
            (mVertex[0] == other.mVertex[1] &&
             mVertex[1] == other.mVertex[0]);
    }

protected:
    int32_t mLabel;
    std::array<int32_t, 2> mVertex;
    std::array<int32_t, 2> mTriangle;
};
