// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.12.22

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/MinimalCycleBasis.h>
using namespace gte;

class MinimalCycleBasisWindow2 : public Window2
{
public:
    MinimalCycleBasisWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    using Rational = BSNumber<UIntegerAP32>;
    using MCB = MinimalCycleBasis<Rational, int32_t>;

    bool SetEnvironment();
    void DrawTree(std::shared_ptr<MCB::Tree> const& tree);

    std::vector<MCB::Position> mPositions;
    std::vector<MCB::Edge> mEdges;
    std::vector<std::array<float, 2>> mFPositions;
    std::vector<std::array<int32_t, 2>> mSPositions;

    std::vector<std::shared_ptr<MCB::Tree>> mForest;
    std::vector<MCB::Filament> mFilaments;
    bool mDrawRawData;
};
