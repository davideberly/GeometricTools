// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <array>

class MTEdge
{
public:
    MTEdge(int label = -1)
        :
        mLabel(label),
        mVertex{ -1, -1 },
        mTriangle{ -1, -1 }
    {
    }

    ~MTEdge() = default;

    inline int GetLabel() const
    {
        return mLabel;
    }

    inline void SetLabel(int label)
    {
        mLabel = label;
    }

    inline int GetVertex(int i) const
    {
        return mVertex[i];
    }

    inline void SetVertex(int i, int label)
    {
        mVertex[i] = label;
    }

    bool ReplaceVertex(int vOld, int vNew)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (mVertex[i] == vOld)
            {
                mVertex[i] = vNew;
                return true;
            }
        }
        return false;
    }

    inline int GetTriangle(int i) const
    {
        return mTriangle[i];
    }

    inline void SetTriangle(int i, int label)
    {
        mTriangle[i] = label;
    }

    bool ReplaceTriangle(int tOld, int tNew)
    {
        for (int i = 0; i < 2; ++i)
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
    int mLabel;
    std::array<int, 2> mVertex;
    std::array<int, 2> mTriangle;
};
