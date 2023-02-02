// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.5.2022.12.12

#include "IntersectRayPolygonWindow2.h"

IntersectRayPolygonWindow2::IntersectRayPolygonWindow2(Parameters& parameters)
    :
    Window2(parameters),
#if defined(GTE_DO_RAY_CAST) 
    mRMQuery{},
#else
    mSMQuery{},
#endif
    mDrawLineInputs(maxDirections),
    mSegmentMesh{}
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mDoFlip = false;
    CreatePolygon();

    int32_t x{}, y{};
    GetMousePosition(x, y);
#if defined(GTE_DO_RAY_CAST) 
    DoRayCast(x, y);
#else
    DoSegmentCast(x, y);
#endif
    OnDisplay();
}

bool IntersectRayPolygonWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case ' ':
#if defined(GTE_DO_RAY_CAST) 
        DoRayCast(x, y);
#else
        DoSegmentCast(x, y);
#endif
        OnDisplay();
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}

bool IntersectRayPolygonWindow2::OnMouseMotion(int32_t button, int32_t x, int32_t y, uint32_t modifiers)
{
#if defined(GTE_DO_RAY_CAST) 
    DoRayCast(x, y);
#else
    DoSegmentCast(x, y);
#endif
    OnDisplay();
    return Window2::OnMouseMotion(button, x, y, modifiers);
}

void IntersectRayPolygonWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    int32_t x0{}, y0{}, x1{}, y1{};

    // Draw the polygon.
    auto const& points = mSegmentMesh.GetVertices();
    x0 = static_cast<int32_t>(points[0][0]);
    y0 = static_cast<int32_t>(points[0][1]);
    for (size_t i = 1; i < points.size(); ++i)
    {
        x1 = static_cast<int32_t>(points[i][0]);
        y1 = static_cast<int32_t>(points[i][1]);
        DrawLine(x0, y0, x1, y1, 0xFFFF0000);
        x0 = x1;
        y0 = y1;
    }
    x1 = static_cast<int32_t>(points[0][0]);
    y1 = static_cast<int32_t>(points[0][1]);
    DrawLine(x0, y0, x1, y1, 0xFFFF0000);

    // Draw from the ray origin (or segment first endpoint) to the first
    // visible intersection.
    for (auto const& p : mDrawLineInputs)
    {
        DrawLine(p[0], p[1], p[2], p[3], 0xFF0000FF);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool IntersectRayPolygonWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Intersection/IntersectRayPolygon/Data/");

    if (mEnvironment.GetPath("Polygon.txt") == "")
    {
        LogError("Cannot find file Polygon.txt.");
        return false;
    }

    return true;
}

void IntersectRayPolygonWindow2::CreatePolygon()
{
    std::vector<Vector2<double>> points{};
    std::string path = mEnvironment.GetPath("Polygon.txt");
    std::ifstream input(path);
    while (!input.eof())
    {
        Vector2<double> point{};
        input >> point[0] >> point[1];
        points.push_back(point);
    }
    input.close();

    mSegmentMesh = SegmentMesh2<double>(points, false);
}

#if defined(GTE_DO_RAY_CAST)
void IntersectRayPolygonWindow2::DoRayCast(int32_t x0, int32_t y0)
{
    // Compute the intersections for 360 rays emanating from the current
    // mouse location.

    Ray2<double> ray{};
    ray.origin = { static_cast<double>(x0), static_cast<double>(y0) };
    mDrawLineInputs.clear();
    for (size_t j = 0; j < maxDirections; ++j)
    {
        double angle = GTE_C_DEG_TO_RAD * static_cast<double>(j);
        ray.direction = { std::cos(angle), std::sin(angle) };
        auto result = mRMQuery(ray, mSegmentMesh);
        if (result.intersections.size() > 0)
        {
            Vector2<double> closest = result.intersections[0].point;
            int32_t x1 = static_cast<int32_t>(closest[0]);
            int32_t y1 = static_cast<int32_t>(closest[1]);
            mDrawLineInputs.push_back({ x0, y0, x1, y1 });
        }
    }
}
#else
void IntersectRayPolygonWindow2::DoSegmentCast(int32_t x0, int32_t y0)
{
    // Compute the intersections for 360 segments emanating from the current
    // mouse location and having length 256.

    double constexpr length = 256.0;
    Segment2<double> segment{};
    segment.p[0] = { static_cast<double>(x0), static_cast<double>(y0) };
    mDrawLineInputs.clear();
    for (size_t j = 0; j < maxDirections; ++j)
    {
        double angle = GTE_C_DEG_TO_RAD * static_cast<double>(j);
        Vector2<double> direction{ std::cos(angle), std::sin(angle) };
        segment.p[1] = segment.p[0] + length * direction;
        auto result = mSMQuery(segment, mSegmentMesh);
        if (result.intersections.size() > 0)
        {
            Vector2<double> closest = result.intersections[0].point;
            int32_t x1 = static_cast<int32_t>(closest[0]);
            int32_t y1 = static_cast<int32_t>(closest[1]);
            mDrawLineInputs.push_back({ x0, y0, x1, y1 });
        }
    }
}
#endif
