// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <array>

class MTTriangle
{
public:
    MTTriangle(int32_t label = -1)
        :
        mLabel(label),
        mVertex{ -1, -1, -1 },
        mEdge{ -1, -1, -1 },
        mAdjacent{ -1, -1, -1 }
    {
    }

    ~MTTriangle() = default;

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
        for (int32_t i = 0; i < 3; ++i)
        {
            if (mVertex[i] == vOld)
            {
                mVertex[i] = vNew;
                return true;
            }
        }
        return false;
    }

    inline int32_t GetEdge(int32_t i) const
    {
        return mEdge[i];
    }

    inline void SetEdge(int32_t i, int32_t label)
    {
        mEdge[i] = label;
    }

    bool ReplaceEdge(int32_t eOld, int32_t eNew)
    {
        for (int32_t i = 0; i < 3; ++i)
        {
            if (mEdge[i] == eOld)
            {
                mEdge[i] = eNew;
                return true;
            }
        }

        return false;
    }

    inline int32_t GetAdjacent(int32_t i) const
    {
        return mAdjacent[i];
    }

    inline void SetAdjacent(int32_t i, int32_t label)
    {
        mAdjacent[i] = label;
    }

    bool ReplaceAdjacent(int32_t aOld, int32_t aNew)
    {
        for (int32_t i = 0; i < 3; ++i)
        {
            if (mAdjacent[i] == aOld)
            {
                mAdjacent[i] = aNew;
                return true;
            }
        }

        return false;
    }

    bool operator==(MTTriangle const& other) const
    {
        if (mVertex[0] == other.mVertex[0])
        {
            return mVertex[1] == other.mVertex[1] && mVertex[2] == other.mVertex[2];
        }

        if (mVertex[0] == other.mVertex[1])
        {
            return mVertex[1] == other.mVertex[2] && mVertex[2] == other.mVertex[0];
        }

        if (mVertex[0] == other.mVertex[2])
        {
            return mVertex[1] == other.mVertex[0] && mVertex[2] == other.mVertex[1];
        }

        return false;
    }

protected:
    int32_t mLabel;
    std::array<int32_t, 3> mVertex;
    std::array<int32_t, 3> mEdge;
    std::array<int32_t, 3> mAdjacent;
};
