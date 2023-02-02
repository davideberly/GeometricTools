// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.24

#pragma once

#include <Graphics/Visual.h>
#include <Graphics/CLODCollapseRecord.h>

namespace gte
{
    class CLODMesh : public Visual
    {
    public:
        CLODMesh(std::vector<CLODCollapseRecord> const& records);
        virtual ~CLODMesh() = default;

        // Member access.
        inline int32_t GetNumRecords() const
        {
            return static_cast<int32_t>(mRecords.size());
        }

        inline std::vector<CLODCollapseRecord> const& GetRecords() const
        {
            return mRecords;
        }

        inline int32_t GetTargetRecord() const
        {
            return mTargetRecord;
        }

        // Modify the level of detail. The function returns 'true' when the
        // selected target record is different from the current target record
        // but false when they are the same. When the return value is 'true',
        // the caller is responsible for copying the CPU memory of the index
        // buffer to the equivalent GPU memory, typically using the call
        // 'engine->Update(clodMesh->GetIndexBuffer()'.
        bool SetTargetRecord(int32_t targetRecord);

    protected:
        std::vector<CLODCollapseRecord> mRecords;
        int32_t mTargetRecord;
    };
}
