// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.24

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/CLODMesh.h>
using namespace gte;

CLODMesh::CLODMesh(std::vector<CLODCollapseRecord> const& records)
    :
    Visual(),
    mRecords(records),
    mTargetRecord(0)
{
}

bool CLODMesh::SetTargetRecord(int32_t targetRecord)
{
    if (0 <= targetRecord &&
        targetRecord < GetNumRecords() &&
        targetRecord != mTargetRecord)
    {
        auto* indices = mIBuffer->Get<int32_t>();

        // Collapse the mesh, if necessary.
        while (mTargetRecord < targetRecord)
        {
            ++mTargetRecord;

            // Replace indices in the connectivity array.
            auto const& record = mRecords[mTargetRecord];
            for (size_t i = 0; i < record.indices.size(); ++i)
            {
                int32_t c = record.indices[i];
                LogAssert(
                    indices[c] == record.vThrow,
                    "Inconsistent record in SetTargetRecord.");

                indices[c] = record.vKeep;
            }

            // Reduce the vertex count; the vertices are properly ordered.
            mVBuffer->SetNumActiveElements(record.numVertices);

            // Reduce the triangle count; the triangles are properly ordered.
            mIBuffer->SetNumActiveElements(3 * record.numTriangles);
        }

        // Expand the mesh, if necessary.
        while (mTargetRecord > targetRecord)
        {
            // Restore indices in the connectivity array.
            auto const& record = mRecords[mTargetRecord];
            for (size_t i = 0; i < record.indices.size(); ++i)
            {
                int32_t c = record.indices[i];
                LogAssert(
                    indices[c] == record.vKeep,
                    "Inconsistent record in SetTargetRecord.");

                indices[c] = record.vThrow;
            }

            --mTargetRecord;
            auto const& prevRecord = mRecords[mTargetRecord];

            // Increase the vertex count; the vertices are properly ordered.
            mVBuffer->SetNumActiveElements(prevRecord.numVertices);

            // Increase the triangle count; the triangles are properly ordered.
            mIBuffer->SetNumActiveElements(3 * prevRecord.numTriangles);
        }

        return true;
    }

    return false;
}
