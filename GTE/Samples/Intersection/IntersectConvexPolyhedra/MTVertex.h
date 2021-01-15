// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include "UnorderedSet.h"

class MTVertex
{
public:
    // Construction and destruction.
    MTVertex(int label = -1, int eGrow = 0, int tGrow = 0)
        :
        mLabel(label),
        mESet(eGrow, eGrow),
        mTSet(tGrow, tGrow)
    {
    }

    ~MTVertex() = default;

    // Vertex labels are read-only since they are used for maps in the MTMesh
    // class for inverse look-up.
    inline int GetLabel() const
    {
        return mLabel;
    }

    inline int GetNumEdges() const
    {
        return mESet.GetNumElements();
    }

    inline int GetEdge(int e) const
    {
        return mESet[e];
    }

    inline bool InsertEdge(int e)
    {
        return mESet.Insert(e);
    }

    inline bool RemoveEdge(int e)
    {
        return mESet.Remove(e);
    }

    bool ReplaceEdge(int eOld, int eNew)
    {
        int const numElements = mESet.GetNumElements();
        for (int i = 0; i < numElements; ++i)
        {
            if (mESet[i] == eOld)
            {
                mESet[i] = eNew;
                return true;
            }
        }
        return false;
    }

    inline int GetNumTriangles() const
    {
        return mTSet.GetNumElements();
    }

    inline int GetTriangle(int t) const
    {
        return mTSet[t];
    }

    inline bool InsertTriangle(int t)
    {
        return mTSet.Insert(t);
    }

    inline bool RemoveTriangle(int t)
    {
        return mTSet.Remove(t);
    }

    bool ReplaceTriangle(int tOld, int tNew)
    {
        int const numTriangles = mTSet.GetNumElements();
        for (int i = 0; i < numTriangles; ++i)
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
    int mLabel;
    UnorderedSet<int> mESet, mTSet;
};
