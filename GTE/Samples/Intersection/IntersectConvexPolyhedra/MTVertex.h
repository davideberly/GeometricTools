// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include "UnorderedSet.h"

class MTVertex
{
public:
    // Construction and destruction.
    MTVertex(int32_t label = -1, int32_t eGrow = 0, int32_t tGrow = 0)
        :
        mLabel(label),
        mESet(eGrow, eGrow),
        mTSet(tGrow, tGrow)
    {
    }

    ~MTVertex() = default;

    // Vertex labels are read-only since they are used for maps in the MTMesh
    // class for inverse look-up.
    inline int32_t GetLabel() const
    {
        return mLabel;
    }

    inline int32_t GetNumEdges() const
    {
        return mESet.GetNumElements();
    }

    inline int32_t GetEdge(int32_t e) const
    {
        return mESet[e];
    }

    inline bool InsertEdge(int32_t e)
    {
        return mESet.Insert(e);
    }

    inline bool RemoveEdge(int32_t e)
    {
        return mESet.Remove(e);
    }

    bool ReplaceEdge(int32_t eOld, int32_t eNew)
    {
        int32_t const numElements = mESet.GetNumElements();
        for (int32_t i = 0; i < numElements; ++i)
        {
            if (mESet[i] == eOld)
            {
                mESet[i] = eNew;
                return true;
            }
        }
        return false;
    }

    inline int32_t GetNumTriangles() const
    {
        return mTSet.GetNumElements();
    }

    inline int32_t GetTriangle(int32_t t) const
    {
        return mTSet[t];
    }

    inline bool InsertTriangle(int32_t t)
    {
        return mTSet.Insert(t);
    }

    inline bool RemoveTriangle(int32_t t)
    {
        return mTSet.Remove(t);
    }

    bool ReplaceTriangle(int32_t tOld, int32_t tNew)
    {
        int32_t const numTriangles = mTSet.GetNumElements();
        for (int32_t i = 0; i < numTriangles; ++i)
        {
            if (mTSet[i] == tOld)
            {
                mTSet[i] = tNew;
                return true;
            }
        }
        return false;
    }

    inline bool operator==(MTVertex const& other) const
    {
        return mLabel == other.mLabel;
    }

protected:
    int32_t mLabel;
    UnorderedSet<int32_t> mESet, mTSet;
};
