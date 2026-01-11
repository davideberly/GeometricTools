// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/SurfaceExtractorMC.h>
#include <Graphics/VertexColorEffect.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
using namespace gte;

class VisualizeMarchingCubesWindow3 : public Window3
{
public:
    VisualizeMarchingCubesWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(std::uint8_t key, std::int32_t x, std::int32_t y) override;

private:
    void CreateScene();
    void CreateMesh();
    void GetCurrentString();

    std::array<float, 4> mTextColor;
    Environment mEnvironment;

    using Extractor = SurfaceExtractorMC<float, std::uint32_t>;
    Image3<float> mImage;
    Extractor mExtractor;

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    // Extractor::Topology::maxTriangles = 5
    std::array<Vector4<float>, Extractor::Topology::maxTriangles> mColors;

    std::shared_ptr<Node> mScene;
    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<VertexColorEffect> mEffect;
    std::shared_ptr<Visual> mBox;
    std::shared_ptr<Visual> mMesh;

    std::uint32_t mCurrentEntry;
    std::string mCurrentString;
};


