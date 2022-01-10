// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/Vector2.h>
#include <vector>

namespace gte
{
    template <typename InputType, typename ComputeType>
    class IntrSegment2AllPairs
    {
    public:
        IntrSegment2AllPairs() = default;
        virtual ~IntrSegment2AllPairs() = default;

        using ITSegment = std::array<Vector2<InputType>, 2>;
        using CTSegment = std::array<Vector2<ComputeType>, 2>;

        struct Intersection
        {
            Intersection()
                :
                index{ 0, 0 },
                numIntersections(0),
                t0{ static_cast<ComputeType>(0), static_cast<ComputeType>(0) },
                t1{ static_cast<ComputeType>(0), static_cast<ComputeType>(0) }
            {
            }

            // These are the indices into 'segments' for which this structure
            // corresponds.
            std::array<size_t, 2> index;

            // The number is 1 when the segments intersect in a single point
            // or 2 when the segments are collinear and intersect in a 
            // segment.
            size_t numIntersections;

            // The segment is represented by a endpoints p[0] and p[1]. Let
            // seg[0] and seg[1] be the two segments in a find-intersection
            // query.
            //
            // If numIntersections is 1, the intersection point is
            //   point[0]
            //     = (1 - t0[0]) * seg[0].p[0] + t0[0] * seg[0].p[1]
            //     = (1 - t1[0]) * seg[1].p[0] + t1[0] * seg[1].p[1]
            // The point[1] is invalid.
            //
            // If numIntersections is 2, the endpoints of the segment of
            // intersection are
            //   point[i]
            //     = (1 - t0[i]) * seg[0].p[0] + t0[i] * seg[0].p[1]
            //     = (1 - t1[i]) * seg[1].p[0] + t1[i] * seg[1].p[1]
            // for i in {0,1}, t0[i] in [0,1], t1[i] in [0,1] and
            // t0[0] < t0[1] and t1[0] < t1[1].
            std::array<ComputeType, 2> t0;
            std::array<ComputeType, 2> t1;
            std::array<Vector2<ComputeType>, 2> point;
        };

        // Member read-only access, including the results of the
        // FindIntersection query.
        inline std::vector<AlignedBox2<InputType>> const& GetBoundingRectangles() const
        {
            return mRectangles;
        }

        inline std::vector<std::array<size_t, 2>> const& GetOverlapping() const
        {
            return mOverlapping;
        }

        inline std::vector<Intersection> const& GetIntersections() const
        {
            return mIntersections;
        }

        // A sort-and-sweep algorithm is used. Choose 'sortDimension' to be 0
        // to sort in the x-dimension or 1 to sort in the y-dimension. If you
        // have no prior knowledge of which to choose, just choose 0 or 1.
        bool FindIntersections(std::vector<ITSegment> const& segments, int sortDimension)
        {
            LogAssert(
                sortDimension == 0 || sortDimension == 1,
                "Invalid sort dimension");

            mRectangles.clear();
            mEndpoints.clear();
            mOverlapping.clear();
            mIntersections.clear();

            if (segments.size() < 2)
            {
                return false;
            }

            // Compute the bounding rectangles for the segments.
            ComputeRectangles(segments);

            // Get the rectangle endpoints and sort them in the specified
            // dimension.
            SortRectangleEndpoints(sortDimension);

            // Sweep throught the sorted endpoints to find overlapping
            // rectangles.
            SweepRectangleEndpoints(1 - sortDimension);

            // TODO: Return 'true' if there are intersections.
            ComputeIntersections(segments);
            return mIntersections.size() > 0;
        }

    protected:
        struct Endpoint
        {
            Endpoint()
                :
                value(static_cast<InputType>(0)),
                type(0),
                index(0)
            {
            }

            bool operator<(Endpoint const& endpoint) const
            {
                if (value < endpoint.value)
                {
                    return true;
                }
                if (value > endpoint.value)
                {
                    return false;
                }
                return type < endpoint.type;
            }

            InputType value;     // endpoint value
            size_t type;    // 0 if interval min, 1 if interval max
            size_t index;   // index of rectangle;
        };

        // A derived class might have auxiliary data that it uses to
        // determine when to attempt a segment-segment find-intersection query
        // for a pair of overlapping rectangles. The derived class can
        // override ComputeRectangle to call the base-class function and
        // then provide additional code to manage the auxiliary data.
        virtual void ComputeRectangles(std::vector<ITSegment> const& segments)
        {
            mRectangles.resize(segments.size());
            for (size_t i = 0; i < segments.size(); ++i)
            {
                auto const& segment = segments[i];
                auto& rectangle = mRectangles[i];
                for (int j = 0; j < 2; ++j)
                {
                    if (segment[0][j] <= segment[1][j])
                    {
                        rectangle.min[j] = segment[0][j];
                        rectangle.max[j] = segment[1][j];
                    }
                    else
                    {
                        rectangle.min[j] = segment[1][j];
                        rectangle.max[j] = segment[0][j];
                    }
                }
            }
        }

        // If the derived class overrides ComputeRectangles to use
        // auxiliary data, it can also override AllowOverlapTest to
        // use the auxiliary data to control whether or not the
        // candidate rectangles should be tested for overlap. The
        // two size_t parameters are indices into mRectangles where
        // the candidates live.
        virtual bool AllowOverlapTest(size_t, size_t)
        {
            return true;
        }

        void SortRectangleEndpoints(int sortDimension)
        {
            size_t const numRectangles = mRectangles.size();
            size_t const numEndpoints = 2 * numRectangles;
            mEndpoints.resize(numEndpoints);

            for (size_t r = 0, e = 0; r < numRectangles; ++r)
            {
                auto const& rectangle = mRectangles[r];

                auto& emin = mEndpoints[e];
                emin.value = rectangle.min[sortDimension];
                emin.type = 0;
                emin.index = r;
                ++e;

                auto& emax = mEndpoints[e];
                emax.value = rectangle.max[sortDimension];
                emax.type = 1;
                emax.index = r;
                ++e;
            }

            std::sort(mEndpoints.begin(), mEndpoints.end());
        }

        void SweepRectangleEndpoints(int otherDimension)
        {
            // The active set of rectangles for the sweep phase, stored by
            // index into mRectangles[].
            std::set<size_t> active;

            for (auto const& p : mEndpoints)
            {
                if (p.type == 0)
                {
                    // We are at the minimum value of an interval. Test
                    // whether this interval overlaps with any other active
                    // interval.
                    for (auto activeIndex : active)
                    {
                        // The rectangles overlap in the sort direction.
                        if (AllowOverlapTest(activeIndex, p.index))
                        {
                            // The overlap test in the non-sort direction is
                            // allowed.
                            auto const& r0 = mRectangles[activeIndex];
                            auto const& r1 = mRectangles[p.index];
                            if (r0.max[otherDimension] >= r1.min[otherDimension] &&
                                r0.min[otherDimension] <= r1.max[otherDimension])
                            {
                                // The rectangles overlap. Always store the
                                // indices into mRectangles in ascending
                                // order for consistency.
                                if (activeIndex < p.index)
                                {
                                    mOverlapping.push_back({ activeIndex, p.index });
                                }
                                else
                                {
                                    mOverlapping.push_back({ p.index, activeIndex });
                                }
                            }
                        }
                    }
                    active.insert(p.index);
                }
                else
                {
                    // We are at the maximum value of an interval. The
                    // interval is no longer active and is removed from
                    // further comparisons.
                    active.erase(p.index);
                }
            }
        }

        void ComputeIntersections(std::vector<ITSegment> const& segments)
        {
            for (auto const& index : mOverlapping)
            {
                ITSegment const& seg0 = segments[index[0]];
                ITSegment const& seg1 = segments[index[1]];
                Vector2<ComputeType> u0{ seg0[0][0], seg0[0][1] };
                Vector2<ComputeType> u1{ seg0[1][0], seg0[1][1] };
                Vector2<ComputeType> v0{ seg1[0][0], seg1[0][1] };
                Vector2<ComputeType> v1{ seg1[1][0], seg1[1][1] };
                ComputeIntersection(index, u0, u1, v0, v1);
            }
        }

        void ComputeIntersection(
            std::array<size_t, 2> const& index,
            Vector2<ComputeType> const& u0,
            Vector2<ComputeType> const& u1,
            Vector2<ComputeType> const& v0,
            Vector2<ComputeType> const& v1)
        {
            ComputeType const zero = static_cast<ComputeType>(0);
            ComputeType const one = static_cast<ComputeType>(1);
            Vector2<ComputeType> v0mu0 = v0 - u0;
            Vector2<ComputeType> u1mu0 = u1 - u0;
            Vector2<ComputeType> v1mv0 = v1 - v0;
            ComputeType det = DotPerp(u1mu0, v1mv0);

            if (det != zero)
            {
                // The segments are not parallel.
            }
            else
            {
                // The segments are parallel.
                ComputeType dotperp = DotPerp(v0mu0, u1mu0);
                if (dotperp == zero)
                {
                    // The segments are collinear. Project v0 and v1 onto the
                    // line u0 + t * (u1 - u0). Sort the projections so that
                    // t[0] <= t[1]. The u-interval of projection is [0,1].
                    ComputeType sqrLen = Dot(u1mu0, u1mu0);
                    ComputeType dot0 = Dot(v0mu0, u1mu0);
                    ComputeType dot1 = Dot(v1 - u0, u1mu0);
                    std::array<ComputeType, 2> t;
                    bool v0LessThanV1;
                    if (dot0 < dot1)
                    {
                        t[0] = dot0 / sqrLen;
                        t[1] = dot1 / sqrLen;
                        v0LessThanV1 = true;
                    }
                    else
                    {
                        t[0] = dot1 / sqrLen;
                        t[1] = dot0 / sqrLen;
                        v0LessThanV1 = false;
                    }

                    if (zero <= t[1] && t[0] <= one)
                    {
                        Intersection intersection;
                        intersection.index = index;

                        if (t[1] > zero)
                        {
                            if (t[0] < one)
                            {
                                intersection.numIntersections = 2;
                                intersection.t0 = t;
                                intersection.point = { u0 + t[0] * u1mu0, u0 + t[1] * u1mu0 };
                                if (v0LessThanV1)
                                {

                                }
                                else
                                {

                                }
                            }
                            else  // t[0] = 1
                            {
                                intersection.numIntersections = 1;
                                intersection.t0 = { one, one };
                                intersection.point = { u1, u1 };
                                if (v0LessThanV1)
                                {
                                    intersection.t1 = { zero, zero };
                                }
                                else
                                {
                                    intersection.t1 = { one, one };
                                }
                                mIntersections.push_back(intersection);
                            }
                        }
                        else  // t[1] = 0
                        {
                            intersection.numIntersections = 1;
                            intersection.t0 = { zero, zero };
                            intersection.point = { u0, u0 };
                            if (v0LessThanV1)
                            {
                                intersection.t1 = { one, one };
                            }
                            else
                            {
                                intersection.t1 = { zero, zero };
                            }
                            mIntersections.push_back(intersection);
                        }
                    }
                }
            }
        }

        std::vector<AlignedBox2<InputType>> mRectangles;
        std::vector<Endpoint> mEndpoints;
        std::vector<std::array<size_t, 2>> mOverlapping;
        std::vector<Intersection> mIntersections;
    };
}
